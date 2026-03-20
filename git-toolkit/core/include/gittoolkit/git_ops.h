//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <vector>
#include <optional>

namespace gittoolkit::git {

/// Resultado de una operación git.
enum class GtkGitResult {
    Ok,
    Error,
    NotARepo
};

/// Resultado de ejecutar un comando git.
struct GtkCommandOutput {
    int exitCode = -1;
    std::string stdOut;
    std::string stdErr;
};

/// Ejecutar un comando git con los argumentos dados en el directorio especificado.
GtkCommandOutput RunGit(const std::string& workDir, const std::vector<std::string>& args);

/// Ejecutar un comando git global (sin directorio específico).
GtkCommandOutput RunGitGlobal(const std::vector<std::string>& args);

/// Clonar un repositorio.
GtkGitResult Clone(const std::string& url, const std::string& destPath);

/// Configurar remote origin URL.
GtkGitResult SetRemoteUrl(const std::string& repoPath, const std::string& url);

/// Configurar user.name local del repo.
GtkGitResult SetUserName(const std::string& repoPath, const std::string& name);

/// Configurar user.email local del repo.
GtkGitResult SetUserEmail(const std::string& repoPath, const std::string& email);

/// Configurar credential.<url>.username local del repo.
GtkGitResult SetCredentialUsername(const std::string& repoPath,
                                   const std::string& credUrl,
                                   const std::string& username);

/// Configurar una opción global de git.
GtkGitResult SetGlobalConfig(const std::string& key, const std::string& value);

/// Obtener la rama actual de un repo.
std::optional<std::string> GetCurrentBranch(const std::string& repoPath);

/// Hacer fetch del remote origin.
GtkGitResult Fetch(const std::string& repoPath);

/// Hacer pull del remote origin.
GtkGitResult Pull(const std::string& repoPath);

/// Comprobar si un directorio es un repositorio git.
bool IsGitRepo(const std::string& path);

} // namespace gittoolkit::git
