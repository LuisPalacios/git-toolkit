//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include "gittoolkit/config.h"
#include "gittoolkit/log.h"
#include "gittoolkit/platform.h"

#include <fstream>
#include <format>
#include <algorithm>

namespace gittoolkit {

const char* CredentialTypeToString(GtkCredentialType type) {
    switch (type) {
        case GtkCredentialType::Gcm:   return "gcm";
        case GtkCredentialType::Ssh:   return "ssh";
        case GtkCredentialType::Token: return "token";
    }
    return "gcm";
}

GtkCredentialType CredentialTypeFromString(const std::string& str) {
    if (str == "ssh") return GtkCredentialType::Ssh;
    if (str == "token") return GtkCredentialType::Token;
    return GtkCredentialType::Gcm;
}

// Parse a bool that might be a JSON bool or a string "true"/"false" (v1 compat)
static bool ParseBoolField(const nlohmann::json& j, const std::string& key, bool defaultValue) {
    if (!j.contains(key)) return defaultValue;
    const nlohmann::json& val = j[key];
    if (val.is_boolean()) return val.get<bool>();
    if (val.is_string()) return val.get<std::string>() == "true";
    return defaultValue;
}

// Parse optional string field, returns nullopt if missing or "null"
static std::optional<std::string> ParseOptionalString(const nlohmann::json& j, const std::string& key) {
    if (!j.contains(key)) return std::nullopt;
    const nlohmann::json& val = j[key];
    if (val.is_null()) return std::nullopt;
    std::string str = val.get<std::string>();
    if (str == "null") return std::nullopt;
    return str;
}

static GtkRepoConfig ParseRepo(const std::string& repoName, const nlohmann::json& j) {
    GtkRepoConfig repo;
    repo.name = repoName;

    if (j.contains("credential_type")) {
        repo.credentialType = CredentialTypeFromString(j["credential_type"].get<std::string>());
    }

    repo.userName = ParseOptionalString(j, "name");
    repo.userEmail = ParseOptionalString(j, "email");
    repo.folder = ParseOptionalString(j, "folder");

    return repo;
}

static GtkSourceConfig ParseSource(const std::string& key, const nlohmann::json& j) {
    GtkSourceConfig source;
    source.key = key;
    source.url = j.value("url", "");
    source.username = j.value("username", "");
    source.folder = j.value("folder", "");
    source.userName = j.value("name", "");
    source.userEmail = j.value("email", "");

    source.gcmProvider = ParseOptionalString(j, "gcm_provider");
    source.gcmUseHttpPath = ParseOptionalString(j, "gcm_useHttpPath");
    source.sshHost = ParseOptionalString(j, "ssh_host");
    source.sshHostname = ParseOptionalString(j, "ssh_hostname");
    source.sshType = ParseOptionalString(j, "ssh_type");

    if (j.contains("repos") && j["repos"].is_object()) {
        for (const auto& [repoName, repoJson] : j["repos"].items()) {
            source.repos.push_back(ParseRepo(repoName, repoJson));
        }
        // Ordenar repos por nombre
        std::sort(source.repos.begin(), source.repos.end(),
            [](const GtkRepoConfig& a, const GtkRepoConfig& b) {
                return a.name < b.name;
            });
    }

    return source;
}

GtkConfigResult LoadConfig(const std::string& path, GtkConfig& config) {
    LogDebug(std::format("Cargando configuración desde: {}", path));

    std::ifstream file(path);
    if (!file.is_open()) {
        LogError(std::format("No se pudo abrir el fichero: {}", path));
        return GtkConfigResult::FileNotFound;
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(file);
    } catch (const nlohmann::json::parse_error& e) {
        LogError(std::format("Error de parseo JSON: {}", e.what()));
        return GtkConfigResult::ParseError;
    }

    // Parsear sección global
    if (root.contains("global") && root["global"].is_object()) {
        const nlohmann::json& g = root["global"];
        config.global.folder = g.value("folder", "");

        if (g.contains("credential_ssh") && g["credential_ssh"].is_object()) {
            const nlohmann::json& ssh = g["credential_ssh"];
            config.global.ssh.enabled = ParseBoolField(ssh, "enabled", false);
            config.global.ssh.sshFolder = ssh.value("ssh_folder", "");
        }

        if (g.contains("credential_gcm") && g["credential_gcm"].is_object()) {
            const nlohmann::json& gcm = g["credential_gcm"];
            config.global.gcm.enabled = ParseBoolField(gcm, "enabled", true);
            config.global.gcm.helper = gcm.value("helper", "");
            config.global.gcm.credentialStore = gcm.value("credentialStore", "");
        }
    }

    // Expandir ~ en rutas globales y normalizar separadores
    config.global.folder = platform::ExpandTilde(config.global.folder);
    config.global.ssh.sshFolder = platform::ExpandTilde(config.global.ssh.sshFolder);
    std::replace(config.global.folder.begin(), config.global.folder.end(), '\\', '/');
    std::replace(config.global.ssh.sshFolder.begin(), config.global.ssh.sshFolder.end(), '\\', '/');

    // Parsear fuentes (v2: "sources", v1: "accounts")
    std::string sourcesKey = "sources";
    if (!root.contains("sources") && root.contains("accounts")) {
        sourcesKey = "accounts";
        LogDebug("Formato v1 detectado (accounts), se migrará a sources");
    }

    if (root.contains(sourcesKey) && root[sourcesKey].is_object()) {
        for (const auto& [key, sourceJson] : root[sourcesKey].items()) {
            config.sources.push_back(ParseSource(key, sourceJson));
        }
        // Ordenar fuentes por clave
        std::sort(config.sources.begin(), config.sources.end(),
            [](const GtkSourceConfig& a, const GtkSourceConfig& b) {
                return a.key < b.key;
            });
    }

    LogInfo(std::format("Configuración cargada: {} fuentes", config.sources.size()));
    return GtkConfigResult::Ok;
}

static nlohmann::json RepoToJson(const GtkRepoConfig& repo) {
    nlohmann::json j;
    j["credential_type"] = CredentialTypeToString(repo.credentialType);
    if (repo.userName.has_value()) j["name"] = repo.userName.value();
    if (repo.userEmail.has_value()) j["email"] = repo.userEmail.value();
    if (repo.folder.has_value()) j["folder"] = repo.folder.value();
    return j;
}

static nlohmann::json SourceToJson(const GtkSourceConfig& source) {
    nlohmann::json j;
    j["url"] = source.url;
    j["username"] = source.username;
    j["folder"] = source.folder;
    j["name"] = source.userName;
    j["email"] = source.userEmail;

    if (source.gcmProvider.has_value()) j["gcm_provider"] = source.gcmProvider.value();
    if (source.gcmUseHttpPath.has_value()) j["gcm_useHttpPath"] = source.gcmUseHttpPath.value();
    if (source.sshHost.has_value()) j["ssh_host"] = source.sshHost.value();
    if (source.sshHostname.has_value()) j["ssh_hostname"] = source.sshHostname.value();
    if (source.sshType.has_value()) j["ssh_type"] = source.sshType.value();

    nlohmann::json repos = nlohmann::json::object();
    for (const GtkRepoConfig& repo : source.repos) {
        repos[repo.name] = RepoToJson(repo);
    }
    j["repos"] = repos;

    return j;
}

GtkConfigResult SaveConfig(const std::string& path, const GtkConfig& config) {
    LogDebug(std::format("Guardando configuración en: {}", path));

    nlohmann::json root;

    // Global
    nlohmann::json global;
    global["folder"] = config.global.folder;

    nlohmann::json ssh;
    ssh["enabled"] = config.global.ssh.enabled;
    ssh["ssh_folder"] = config.global.ssh.sshFolder;
    global["credential_ssh"] = ssh;

    nlohmann::json gcm;
    gcm["enabled"] = config.global.gcm.enabled;
    gcm["helper"] = config.global.gcm.helper;
    gcm["credentialStore"] = config.global.gcm.credentialStore;
    global["credential_gcm"] = gcm;

    root["global"] = global;

    // Sources (v2 format)
    nlohmann::json sources = nlohmann::json::object();
    for (const GtkSourceConfig& source : config.sources) {
        sources[source.key] = SourceToJson(source);
    }
    root["sources"] = sources;

    std::ofstream file(path);
    if (!file.is_open()) {
        LogError(std::format("No se pudo abrir para escritura: {}", path));
        return GtkConfigResult::WriteError;
    }

    file << root.dump(4) << "\n";
    LogInfo("Configuración guardada correctamente");
    return GtkConfigResult::Ok;
}

GtkConfigResult ValidateConfigFile(const std::string& path) {
    LogDebug(std::format("Validando: {}", path));

    std::ifstream file(path);
    if (!file.is_open()) {
        LogError(std::format("Fichero no encontrado: {}", path));
        return GtkConfigResult::FileNotFound;
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(file);
    } catch (const nlohmann::json::parse_error& e) {
        LogError(std::format("JSON inválido: {}", e.what()));
        return GtkConfigResult::ParseError;
    }

    // Validar estructura básica
    if (!root.contains("global") || !root["global"].is_object()) {
        LogError("Falta la sección 'global'");
        return GtkConfigResult::ValidationError;
    }

    if (!root["global"].contains("folder")) {
        LogError("Falta 'global.folder'");
        return GtkConfigResult::ValidationError;
    }

    // Aceptar tanto "sources" como "accounts"
    bool hasSources = root.contains("sources") && root["sources"].is_object();
    bool hasAccounts = root.contains("accounts") && root["accounts"].is_object();
    if (!hasSources && !hasAccounts) {
        LogError("Falta la sección 'sources' (o 'accounts')");
        return GtkConfigResult::ValidationError;
    }

    const std::string sourcesKey = hasSources ? "sources" : "accounts";
    for (const auto& [key, sourceJson] : root[sourcesKey].items()) {
        if (!sourceJson.contains("url")) {
            LogError(std::format("Fuente '{}': falta 'url'", key));
            return GtkConfigResult::ValidationError;
        }
        if (!sourceJson.contains("username")) {
            LogError(std::format("Fuente '{}': falta 'username'", key));
            return GtkConfigResult::ValidationError;
        }
        if (!sourceJson.contains("folder")) {
            LogError(std::format("Fuente '{}': falta 'folder'", key));
            return GtkConfigResult::ValidationError;
        }

        if (sourceJson.contains("repos") && sourceJson["repos"].is_object()) {
            for (const auto& [repoName, repoJson] : sourceJson["repos"].items()) {
                if (!repoJson.contains("credential_type")) {
                    LogError(std::format("Repo '{}/{}': falta 'credential_type'", key, repoName));
                    return GtkConfigResult::ValidationError;
                }
            }
        }
    }

    LogInfo("Validación correcta");
    return GtkConfigResult::Ok;
}

} // namespace gittoolkit
