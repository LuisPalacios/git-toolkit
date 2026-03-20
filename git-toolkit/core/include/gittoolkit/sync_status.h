//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <vector>

namespace gittoolkit::sync {

/// Estado de sincronización de un repositorio.
enum class GtkRepoStatus {
    Clean,            /// Sincronizado, sin cambios
    NeedsPull,        /// Atrasado respecto al remoto, se puede hacer pull
    NeedsReview,      /// Cambios locales que impiden pull automático
    BehindMain,       /// Limpio pero la rama está atrasada respecto a main/master
    Error             /// Error al comprobar (sin upstream, etc.)
};

/// Información detallada del estado de un repositorio.
struct GtkRepoStatusInfo {
    std::string path;
    std::string branch;
    GtkRepoStatus status = GtkRepoStatus::Error;

    int ahead = 0;
    int behind = 0;
    bool diverged = false;
    int stashed = 0;
    int staged = 0;
    int untracked = 0;
    int modified = 0;
    int moved = 0;

    std::string errorMessage;
};

/// Nombre del estado para mostrar.
const char* StatusToString(GtkRepoStatus status);

/// Comprobar el estado de un repositorio individual.
/// Hace fetch automáticamente.
GtkRepoStatusInfo CheckRepoStatus(const std::string& repoPath);

/// Buscar todos los repos git bajo un directorio y obtener su estado.
std::vector<GtkRepoStatusInfo> CheckAllRepos(const std::string& rootDir);

} // namespace gittoolkit::sync
