//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include "gittoolkit/git_ops.h"
#include "gittoolkit/platform.h"
#include "gittoolkit/log.h"

#include <format>
#include <array>
#include <cstdio>
#include <filesystem>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace gittoolkit::git {

GtkCommandOutput RunGit(const std::string& workDir, const std::vector<std::string>& args) {
    GtkCommandOutput result;
    const char* gitCmd = platform::GetGitCommand();

    std::string command = std::format("cd \"{}\" && \"{}\"", workDir, gitCmd);
    for (const std::string& arg : args) {
        command += " " + arg;
    }
    command += " 2>&1";

    LogDebug(std::format("Ejecutando: {}", command));

    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        LogError("No se pudo ejecutar el comando git");
        result.exitCode = -1;
        return result;
    }

    std::array<char, 4096> buffer;
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result.stdOut += buffer.data();
    }

    result.exitCode = pclose(pipe);
#ifdef _WIN32
    // pclose en Windows devuelve el exit code directamente
#else
    result.exitCode = WEXITSTATUS(result.exitCode);
#endif

    if (result.exitCode != 0) {
        LogDebug(std::format("git terminó con código {}: {}", result.exitCode, result.stdOut));
    }

    return result;
}

GtkCommandOutput RunGitGlobal(const std::vector<std::string>& args) {
    GtkCommandOutput result;
    const char* gitCmd = platform::GetGitCommand();

    std::string command = std::format("\"{}\"", gitCmd);
    for (const std::string& arg : args) {
        command += " " + arg;
    }
    command += " 2>&1";

    LogDebug(std::format("Ejecutando (global): {}", command));

    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        result.exitCode = -1;
        return result;
    }

    std::array<char, 4096> buffer;
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result.stdOut += buffer.data();
    }

    result.exitCode = pclose(pipe);
#ifdef _WIN32
#else
    result.exitCode = WEXITSTATUS(result.exitCode);
#endif

    return result;
}

GtkGitResult Clone(const std::string& url, const std::string& destPath) {
    LogInfo(std::format("Clonando {} → {}", url, destPath));
    GtkCommandOutput out = RunGitGlobal({"clone", "\"" + url + "\"", "\"" + destPath + "\""});
    return out.exitCode == 0 ? GtkGitResult::Ok : GtkGitResult::Error;
}

GtkGitResult SetRemoteUrl(const std::string& repoPath, const std::string& url) {
    GtkCommandOutput out = RunGit(repoPath, {"remote", "set-url", "origin", url});
    if (out.exitCode != 0) return GtkGitResult::Error;
    out = RunGit(repoPath, {"remote", "set-url", "--push", "origin", url});
    return out.exitCode == 0 ? GtkGitResult::Ok : GtkGitResult::Error;
}

GtkGitResult SetUserName(const std::string& repoPath, const std::string& name) {
    GtkCommandOutput out = RunGit(repoPath, {"config", "user.name", "\"" + name + "\""});
    return out.exitCode == 0 ? GtkGitResult::Ok : GtkGitResult::Error;
}

GtkGitResult SetUserEmail(const std::string& repoPath, const std::string& email) {
    GtkCommandOutput out = RunGit(repoPath, {"config", "user.email", email});
    return out.exitCode == 0 ? GtkGitResult::Ok : GtkGitResult::Error;
}

GtkGitResult SetCredentialUsername(const std::string& repoPath,
                                   const std::string& credUrl,
                                   const std::string& username) {
    std::string key = std::format("credential.{}.username", credUrl);
    GtkCommandOutput out = RunGit(repoPath, {"config", key, username});
    return out.exitCode == 0 ? GtkGitResult::Ok : GtkGitResult::Error;
}

GtkGitResult SetGlobalConfig(const std::string& key, const std::string& value) {
    GtkCommandOutput out = RunGitGlobal({"config", "--global", key, "\"" + value + "\""});
    return out.exitCode == 0 ? GtkGitResult::Ok : GtkGitResult::Error;
}

std::optional<std::string> GetCurrentBranch(const std::string& repoPath) {
    GtkCommandOutput out = RunGit(repoPath, {"rev-parse", "--abbrev-ref", "HEAD"});
    if (out.exitCode != 0) return std::nullopt;
    // Eliminar salto de línea
    std::string branch = out.stdOut;
    while (!branch.empty() && (branch.back() == '\n' || branch.back() == '\r')) {
        branch.pop_back();
    }
    return branch;
}

GtkGitResult Fetch(const std::string& repoPath) {
    GtkCommandOutput out = RunGit(repoPath, {"fetch", "origin", "--quiet"});
    return out.exitCode == 0 ? GtkGitResult::Ok : GtkGitResult::Error;
}

GtkGitResult Pull(const std::string& repoPath) {
    LogInfo(std::format("Pull en: {}", repoPath));
    GtkCommandOutput out = RunGit(repoPath, {"pull", "--quiet"});
    return out.exitCode == 0 ? GtkGitResult::Ok : GtkGitResult::Error;
}

bool IsGitRepo(const std::string& path) {
    return std::filesystem::exists(path + "/.git");
}

} // namespace gittoolkit::git
