//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include "gittoolkit/sync_status.h"
#include "gittoolkit/git_ops.h"
#include "gittoolkit/log.h"

#include <format>
#include <filesystem>
#include <algorithm>

namespace gittoolkit::sync {

const char* StatusToString(GtkRepoStatus status) {
    switch (status) {
        case GtkRepoStatus::Clean:       return "LIMPIO";
        case GtkRepoStatus::NeedsPull:   return "NECESITA PULL";
        case GtkRepoStatus::NeedsReview: return "REVISAR";
        case GtkRepoStatus::BehindMain:  return "ATRASADO VS MAIN";
        case GtkRepoStatus::Error:       return "ERROR";
    }
    return "DESCONOCIDO";
}

// Parsear un entero de la salida de un comando, eliminando espacios
static int ParseCount(const std::string& output) {
    std::string trimmed = output;
    while (!trimmed.empty() && (trimmed.back() == '\n' || trimmed.back() == '\r' || trimmed.back() == ' ')) {
        trimmed.pop_back();
    }
    while (!trimmed.empty() && (trimmed.front() == ' ')) {
        trimmed.erase(trimmed.begin());
    }
    if (trimmed.empty()) return 0;
    try {
        return std::stoi(trimmed);
    } catch (...) {
        return 0;
    }
}

GtkRepoStatusInfo CheckRepoStatus(const std::string& repoPath) {
    GtkRepoStatusInfo info;
    info.path = repoPath;

    // Verificar que es un repo git
    if (!git::IsGitRepo(repoPath)) {
        info.status = GtkRepoStatus::Error;
        info.errorMessage = "No es un repositorio git";
        return info;
    }

    // Obtener rama actual
    std::optional<std::string> branch = git::GetCurrentBranch(repoPath);
    if (!branch.has_value()) {
        info.status = GtkRepoStatus::Error;
        info.errorMessage = "No se pudo obtener la rama actual";
        return info;
    }
    info.branch = branch.value();

    // Verificar upstream
    git::GtkCommandOutput upstreamOut = git::RunGit(repoPath,
        {"rev-parse", "--symbolic-full-name", "--abbrev-ref", "@{u}"});
    if (upstreamOut.exitCode != 0) {
        info.status = GtkRepoStatus::Error;
        info.errorMessage = "Sin upstream configurado";
        return info;
    }

    // Fetch
    git::Fetch(repoPath);

    // Ahead/behind
    git::GtkCommandOutput aheadOut = git::RunGit(repoPath,
        {"rev-list", "--count", "@{u}..HEAD"});
    info.ahead = ParseCount(aheadOut.stdOut);

    git::GtkCommandOutput behindOut = git::RunGit(repoPath,
        {"rev-list", "--count", "HEAD..@{u}"});
    info.behind = ParseCount(behindOut.stdOut);

    info.diverged = (info.ahead > 0 && info.behind > 0);

    // Stash
    git::GtkCommandOutput stashOut = git::RunGit(repoPath, {"stash", "list"});
    info.stashed = static_cast<int>(std::count(stashOut.stdOut.begin(), stashOut.stdOut.end(), '\n'));

    // Staged
    git::GtkCommandOutput stagedOut = git::RunGit(repoPath,
        {"diff", "--cached", "--name-only"});
    info.staged = static_cast<int>(std::count(stagedOut.stdOut.begin(), stagedOut.stdOut.end(), '\n'));

    // Untracked
    git::GtkCommandOutput untrackedOut = git::RunGit(repoPath,
        {"ls-files", "--others", "--exclude-standard"});
    info.untracked = static_cast<int>(std::count(untrackedOut.stdOut.begin(), untrackedOut.stdOut.end(), '\n'));

    // Modified
    git::GtkCommandOutput modifiedOut = git::RunGit(repoPath, {"ls-files", "-m"});
    info.modified = static_cast<int>(std::count(modifiedOut.stdOut.begin(), modifiedOut.stdOut.end(), '\n'));

    // Moved/renamed
    git::GtkCommandOutput movedOut = git::RunGit(repoPath, {"diff", "--name-status"});
    info.moved = static_cast<int>(std::count(movedOut.stdOut.begin(), movedOut.stdOut.end(), 'R'));

    // Determinar estado
    bool isClean = (info.ahead == 0 && !info.diverged && info.stashed == 0 &&
                    info.staged == 0 && info.untracked == 0 && info.modified == 0 &&
                    info.moved == 0);

    if (isClean) {
        if (info.behind > 0) {
            info.status = GtkRepoStatus::NeedsPull;
        } else {
            info.status = GtkRepoStatus::Clean;
        }
    } else {
        info.status = GtkRepoStatus::NeedsReview;
    }

    LogDebug(std::format("{}: {} (ahead={}, behind={}, staged={}, modified={}, untracked={})",
        repoPath, StatusToString(info.status),
        info.ahead, info.behind, info.staged, info.modified, info.untracked));

    return info;
}

std::vector<GtkRepoStatusInfo> CheckAllRepos(const std::string& rootDir) {
    std::vector<GtkRepoStatusInfo> results;

    LogInfo(std::format("Buscando repositorios en: {}", rootDir));

    if (!std::filesystem::exists(rootDir)) {
        LogError(std::format("Directorio no existe: {}", rootDir));
        return results;
    }

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::recursive_directory_iterator(rootDir,
             std::filesystem::directory_options::skip_permission_denied)) {
        if (!entry.is_directory()) continue;
        if (entry.path().filename() != ".git") continue;

        std::string repoPath = entry.path().parent_path().string();

        // Normalizar separadores a /
        std::replace(repoPath.begin(), repoPath.end(), '\\', '/');

        // Evitar repos anidados (si el repo ya está dentro de uno evaluado)
        bool nested = false;
        for (const GtkRepoStatusInfo& existing : results) {
            if (repoPath.starts_with(existing.path + "/")) {
                nested = true;
                break;
            }
        }
        if (nested) continue;

        results.push_back(CheckRepoStatus(repoPath));
    }

    return results;
}

} // namespace gittoolkit::sync
