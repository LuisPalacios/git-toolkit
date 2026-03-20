//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include "gittoolkit/engine.h"
#include "gittoolkit/git_ops.h"
#include "gittoolkit/platform.h"
#include "gittoolkit/log.h"

#include <format>
#include <filesystem>

namespace gittoolkit {

GtkEngineResult GtkEngine::LoadConfig(const std::string& configPath) {
    // Determinar ruta del fichero de configuración
    if (!configPath.empty()) {
        _configPath = configPath;
    } else {
        // Intentar v2 primero, luego v1
        _configPath = platform::GetConfigPath();
        if (!std::filesystem::exists(_configPath)) {
            std::string legacyPath = platform::GetLegacyConfigPath();
            if (std::filesystem::exists(legacyPath)) {
                LogInfo(std::format("Usando configuración legacy: {}", legacyPath));
                _configPath = legacyPath;
            }
        }
    }

    GtkConfigResult result = gittoolkit::LoadConfig(_configPath, _config);
    if (result != GtkConfigResult::Ok) {
        LogError(std::format("Error cargando configuración: {}", _configPath));
        return GtkEngineResult::ConfigError;
    }

    _configLoaded = true;
    return GtkEngineResult::Ok;
}

GtkConfigResult GtkEngine::Validate() {
    return ValidateConfigFile(_configPath);
}

const GtkConfig& GtkEngine::GetConfig() const {
    return _config;
}

// Normalizar separadores de ruta a / en Windows
static std::string NormalizePath(const std::string& path) {
    std::string result = path;
    std::replace(result.begin(), result.end(), '\\', '/');
    return result;
}

std::string GtkEngine::BuildRepoPath(const GtkSourceConfig& source, const GtkRepoConfig& repo) const {
    std::string result;
    // Si el repo tiene folder definido
    if (repo.folder.has_value()) {
        std::string folder = platform::ExpandTilde(repo.folder.value());
        if (!folder.empty() && (folder[0] == '/' || (folder.size() > 1 && folder[1] == ':'))) {
            result = folder;
        } else {
            result = _config.global.folder + "/" + source.folder + "/" + folder;
        }
    } else {
        result = _config.global.folder + "/" + source.folder + "/" + repo.name;
    }
    return NormalizePath(result);
}

std::string GtkEngine::BuildCloneUrl(const GtkSourceConfig& source, const GtkRepoConfig& repo) const {
    if (repo.credentialType == GtkCredentialType::Ssh) {
        if (!source.sshHost.has_value()) return "";
        // Extraer usuario de la URL
        std::string url = source.url;
        std::string userPath = url.substr(url.rfind('/') + 1);
        return source.sshHost.value() + ":" + userPath + "/" + repo.name + ".git";
    }
    // GCM o Token: HTTPS
    return source.url + "/" + repo.name + ".git";
}

std::string GtkEngine::BuildRemoteUrl(const GtkSourceConfig& source, const GtkRepoConfig& repo) const {
    if (repo.credentialType == GtkCredentialType::Ssh) {
        return BuildCloneUrl(source, repo);
    }
    return source.url + "/" + repo.name + ".git";
}

GtkEngineResult GtkEngine::Sync(const GtkSyncOptions& options) {
    if (!_configLoaded) {
        LogError("Configuración no cargada");
        return GtkEngineResult::ConfigError;
    }

    bool hasErrors = false;

    // Crear directorio global
    if (!options.dryRun) {
        std::filesystem::create_directories(_config.global.folder);
    }
    LogInfo(std::format("Directorio git: {}", _config.global.folder));

    for (const GtkSourceConfig& source : _config.sources) {
        std::string sourceDir = _config.global.folder + "/" + source.folder;

        // Crear directorio de la fuente
        if (!options.dryRun) {
            std::filesystem::create_directories(sourceDir);
        }
        LogInfo(std::format("  {}", source.folder));

        for (const GtkRepoConfig& repo : source.repos) {
            std::string repoPath = BuildRepoPath(source, repo);

            if (!std::filesystem::exists(repoPath)) {
                // Clonar
                std::string cloneUrl = BuildCloneUrl(source, repo);
                if (options.dryRun) {
                    LogInfo(std::format("    [dry-run] Clonaría {} → {}", cloneUrl, repoPath));
                } else {
                    LogInfo(std::format("    ⬇ {}", repoPath));
                    git::GtkGitResult cloneResult = git::Clone(cloneUrl, repoPath);
                    if (cloneResult != git::GtkGitResult::Ok) {
                        LogError(std::format("    Error clonando: {}", repo.name));
                        hasErrors = true;
                        continue;
                    }
                }
            } else {
                LogInfo(std::format("   - {}", repoPath));
            }

            // Configurar el repositorio local
            if (!options.dryRun && std::filesystem::exists(repoPath)) {
                std::string remoteUrl = BuildRemoteUrl(source, repo);
                git::SetRemoteUrl(repoPath, remoteUrl);

                // user.name y user.email (repo override o de la fuente)
                std::string name = repo.userName.value_or(source.userName);
                std::string email = repo.userEmail.value_or(source.userEmail);
                if (!name.empty()) git::SetUserName(repoPath, name);
                if (!email.empty()) git::SetUserEmail(repoPath, email);
            }
        }
    }

    return hasErrors ? GtkEngineResult::PartialError : GtkEngineResult::Ok;
}

std::vector<sync::GtkRepoStatusInfo> GtkEngine::GetStatus(const GtkStatusOptions& options) {
    std::vector<sync::GtkRepoStatusInfo> results;

    if (!_configLoaded) {
        LogError("Configuración no cargada");
        return results;
    }

    for (const GtkSourceConfig& source : _config.sources) {
        // Filtrar por fuente
        if (options.filterSource.has_value() &&
            source.key != options.filterSource.value()) {
            continue;
        }

        for (const GtkRepoConfig& repo : source.repos) {
            // Filtrar por repo
            if (options.filterRepo.has_value() &&
                repo.name != options.filterRepo.value()) {
                continue;
            }

            std::string repoPath = BuildRepoPath(source, repo);
            if (!std::filesystem::exists(repoPath)) {
                sync::GtkRepoStatusInfo info;
                info.path = repoPath;
                info.status = sync::GtkRepoStatus::Error;
                info.errorMessage = "Directorio no existe";
                results.push_back(info);
                continue;
            }

            results.push_back(sync::CheckRepoStatus(repoPath));
        }
    }

    return results;
}

GtkEngineResult GtkEngine::PullSafe(const GtkStatusOptions& options) {
    std::vector<sync::GtkRepoStatusInfo> statuses = GetStatus(options);
    bool hasErrors = false;
    int pullCount = 0;

    for (const sync::GtkRepoStatusInfo& info : statuses) {
        if (info.status == sync::GtkRepoStatus::NeedsPull) {
            git::GtkGitResult pullResult = git::Pull(info.path);
            if (pullResult == git::GtkGitResult::Ok) {
                LogInfo(std::format("Pull completado: {}", info.path));
                pullCount++;
            } else {
                LogError(std::format("Error en pull: {}", info.path));
                hasErrors = true;
            }
        }
    }

    LogInfo(std::format("{} repositorios actualizados", pullCount));
    return hasErrors ? GtkEngineResult::PartialError : GtkEngineResult::Ok;
}

} // namespace gittoolkit
