//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#pragma once

#include "gittoolkit/config.h"
#include "gittoolkit/sync_status.h"

#include <string>
#include <vector>
#include <optional>

namespace gittoolkit {

/// Resultado de una operación del engine.
enum class GtkEngineResult {
    Ok,
    ConfigError,
    PartialError,
    Error
};

/// Opciones para la operación de sincronización (configurar repos).
struct GtkSyncOptions {
    bool dryRun = false;
};

/// Opciones para la operación de estado/pull.
struct GtkStatusOptions {
    std::optional<std::string> filterSource;
    std::optional<std::string> filterRepo;
    bool verbose = false;
    bool doPull = false;
};

/// Motor principal que orquesta todas las operaciones.
class GtkEngine {
public:
    /// Cargar configuración. Intenta v2 primero, luego v1.
    GtkEngineResult LoadConfig(const std::string& configPath = "");

    /// Validar la configuración cargada.
    GtkConfigResult Validate();

    /// Sincronizar: configurar credenciales, clonar repos, configurar repos existentes.
    GtkEngineResult Sync(const GtkSyncOptions& options = {});

    /// Obtener el estado de todos los repos configurados.
    std::vector<sync::GtkRepoStatusInfo> GetStatus(const GtkStatusOptions& options = {});

    /// Hacer pull de los repos donde sea seguro.
    GtkEngineResult PullSafe(const GtkStatusOptions& options = {});

    /// Acceso a la configuración cargada.
    const GtkConfig& GetConfig() const;

private:
    GtkConfig _config;
    std::string _configPath;
    bool _configLoaded = false;

    /// Construir la ruta completa de un repo.
    std::string BuildRepoPath(const GtkSourceConfig& source, const GtkRepoConfig& repo) const;

    /// Construir la URL de clonación.
    std::string BuildCloneUrl(const GtkSourceConfig& source, const GtkRepoConfig& repo) const;

    /// Construir la URL del remote origin.
    std::string BuildRemoteUrl(const GtkSourceConfig& source, const GtkRepoConfig& repo) const;
};

} // namespace gittoolkit
