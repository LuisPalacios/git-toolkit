//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include "gittoolkit/engine.h"
#include "gittoolkit/log.h"
#include "gittoolkit/platform.h"
#include "gittoolkit/sync_status.h"

#include <format>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

static void EnableAnsiColors() {
#ifdef _WIN32
    // Habilitar secuencias ANSI en la consola de Windows
    HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(hErr, &mode)) {
        SetConsoleMode(hErr, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleMode(hOut, &mode)) {
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

static void PrintUsage() {
    std::cout << "git-toolkit — Gestión de repositorios Git multi-cuenta\n"
              << "\n"
              << "Uso: git-toolkit <comando> [opciones]\n"
              << "\n"
              << "Comandos:\n"
              << "  validate          Validar el fichero de configuración\n"
              << "  sync              Configurar y clonar repositorios\n"
              << "  status            Mostrar estado de sincronización\n"
              << "  pull              Hacer pull donde sea seguro\n"
              << "\n"
              << "Opciones globales:\n"
              << "  --verbose, -v     Salida detallada (nivel debug)\n"
              << "  --config <ruta>   Ruta al fichero de configuración\n"
              << "  --source <name>   Filtrar por fuente (status/pull)\n"
              << "  --repo <name>     Filtrar por repositorio (status/pull)\n"
              << "  --dry-run         Mostrar acciones sin ejecutarlas (sync)\n"
              << "  --help, -h        Mostrar esta ayuda\n"
              << "  --version         Mostrar versión\n";
}

static void PrintVersion() {
    std::cout << "git-toolkit 0.1.0\n";
}

// Colores para la salida
static const char* COLOR_RESET  = "\033[0m";
static const char* COLOR_GREEN  = "\033[32m";
static const char* COLOR_YELLOW = "\033[33m";
static const char* COLOR_RED    = "\033[31m";
static const char* COLOR_BLUE   = "\033[34m";
static const char* COLOR_CYAN   = "\033[36m";

static const char* StatusColor(gittoolkit::sync::GtkRepoStatus status) {
    switch (status) {
        case gittoolkit::sync::GtkRepoStatus::Clean:       return COLOR_GREEN;
        case gittoolkit::sync::GtkRepoStatus::NeedsPull:   return COLOR_BLUE;
        case gittoolkit::sync::GtkRepoStatus::NeedsReview: return COLOR_YELLOW;
        case gittoolkit::sync::GtkRepoStatus::BehindMain:  return COLOR_YELLOW;
        case gittoolkit::sync::GtkRepoStatus::Error:       return COLOR_RED;
    }
    return COLOR_RESET;
}

static int CmdValidate(gittoolkit::GtkEngine& engine) {
    gittoolkit::GtkConfigResult result = engine.Validate();
    if (result == gittoolkit::GtkConfigResult::Ok) {
        std::cout << COLOR_GREEN << "✓" << COLOR_RESET << " Configuración válida\n";
        const gittoolkit::GtkConfig& config = engine.GetConfig();
        std::cout << "  Fuentes: " << config.sources.size() << "\n";
        int repoCount = 0;
        for (const gittoolkit::GtkSourceConfig& source : config.sources) {
            repoCount += static_cast<int>(source.repos.size());
        }
        std::cout << "  Repos:   " << repoCount << "\n";
        return 0;
    }
    std::cout << COLOR_RED << "✗" << COLOR_RESET << " Configuración inválida\n";
    return 1;
}

static int CmdSync(gittoolkit::GtkEngine& engine, bool dryRun) {
    gittoolkit::GtkSyncOptions options;
    options.dryRun = dryRun;

    gittoolkit::GtkEngineResult result = engine.Sync(options);
    if (result == gittoolkit::GtkEngineResult::Ok) {
        std::cout << COLOR_GREEN << "✓" << COLOR_RESET << " Sincronización completada\n";
        return 0;
    }
    if (result == gittoolkit::GtkEngineResult::PartialError) {
        std::cout << COLOR_YELLOW << "⚠" << COLOR_RESET << " Sincronización parcial (algunos errores)\n";
        return 1;
    }
    std::cout << COLOR_RED << "✗" << COLOR_RESET << " Error en la sincronización\n";
    return 1;
}

static int CmdStatus(gittoolkit::GtkEngine& engine, const gittoolkit::GtkStatusOptions& options) {
    std::vector<gittoolkit::sync::GtkRepoStatusInfo> statuses = engine.GetStatus(options);

    if (statuses.empty()) {
        std::cout << "No se encontraron repositorios\n";
        return 0;
    }

    for (const gittoolkit::sync::GtkRepoStatusInfo& info : statuses) {
        const char* color = StatusColor(info.status);
        const char* statusStr = gittoolkit::sync::StatusToString(info.status);

        std::cout << std::format("{} {}[{}]{}\n", info.path, color, statusStr, COLOR_RESET);

        if (options.verbose && info.status != gittoolkit::sync::GtkRepoStatus::Clean) {
            if (!info.branch.empty())
                std::cout << std::format("  Rama: {}{}{}\n", COLOR_CYAN, info.branch, COLOR_RESET);
            if (info.ahead > 0)
                std::cout << std::format("  Adelantados: {}{}{}\n", COLOR_RED, info.ahead, COLOR_RESET);
            if (info.behind > 0)
                std::cout << std::format("  Atrasados:   {}{}{}\n", COLOR_GREEN, info.behind, COLOR_RESET);
            if (info.diverged)
                std::cout << std::format("  Divergencia: {}sí{}\n", COLOR_RED, COLOR_RESET);
            if (info.stashed > 0)
                std::cout << std::format("  Stash:       {}{}{}\n", COLOR_RED, info.stashed, COLOR_RESET);
            if (info.staged > 0)
                std::cout << std::format("  Staged:      {}{}{}\n", COLOR_RED, info.staged, COLOR_RESET);
            if (info.modified > 0)
                std::cout << std::format("  Modificados: {}{}{}\n", COLOR_RED, info.modified, COLOR_RESET);
            if (info.untracked > 0)
                std::cout << std::format("  Sin rastrear:{}{}{}\n", COLOR_RED, info.untracked, COLOR_RESET);
            if (!info.errorMessage.empty())
                std::cout << std::format("  Error: {}{}{}\n", COLOR_RED, info.errorMessage, COLOR_RESET);
        }
    }

    // Resumen
    int clean = 0, needsPull = 0, review = 0, errors = 0;
    for (const gittoolkit::sync::GtkRepoStatusInfo& info : statuses) {
        switch (info.status) {
            case gittoolkit::sync::GtkRepoStatus::Clean:
            case gittoolkit::sync::GtkRepoStatus::BehindMain:
                clean++; break;
            case gittoolkit::sync::GtkRepoStatus::NeedsPull:  needsPull++; break;
            case gittoolkit::sync::GtkRepoStatus::NeedsReview: review++; break;
            case gittoolkit::sync::GtkRepoStatus::Error:       errors++; break;
        }
    }
    std::cout << std::format("\nTotal: {} repos — {}✓{} limpios, {}↓{} pull, {}⚠{} revisar, {}✗{} errores\n",
        statuses.size(),
        COLOR_GREEN, clean, COLOR_BLUE, needsPull, COLOR_YELLOW, review, COLOR_RED, errors);
    std::cout << COLOR_RESET;

    return 0;
}

static int CmdPull(gittoolkit::GtkEngine& engine, const gittoolkit::GtkStatusOptions& options) {
    gittoolkit::GtkEngineResult result = engine.PullSafe(options);
    return (result == gittoolkit::GtkEngineResult::Ok) ? 0 : 1;
}

int main(int argc, char* argv[]) {
    EnableAnsiColors();

    // Parsear argumentos
    std::string command;
    std::string configPath;
    std::string filterSource;
    std::string filterRepo;
    bool verbose = false;
    bool dryRun = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            return 0;
        }
        if (arg == "--version") {
            PrintVersion();
            return 0;
        }
        if (arg == "--verbose" || arg == "-v") {
            verbose = true;
            continue;
        }
        if (arg == "--dry-run") {
            dryRun = true;
            continue;
        }
        if (arg == "--config" && i + 1 < argc) {
            configPath = argv[++i];
            continue;
        }
        if (arg == "--source" && i + 1 < argc) {
            filterSource = argv[++i];
            continue;
        }
        if (arg == "--repo" && i + 1 < argc) {
            filterRepo = argv[++i];
            continue;
        }

        // Primer argumento sin flag es el comando
        if (command.empty() && !arg.starts_with("-")) {
            command = arg;
            continue;
        }

        std::cerr << std::format("Argumento desconocido: {}\n", arg);
        return 1;
    }

    if (command.empty()) {
        PrintUsage();
        return 0;
    }

    // Configurar logging
    if (verbose) {
        gittoolkit::GtkLog::Instance().SetLevel(gittoolkit::GtkLogLevel::Debug);
    }

    // Cargar configuración
    gittoolkit::GtkEngine engine;
    gittoolkit::GtkEngineResult loadResult = engine.LoadConfig(configPath);
    if (loadResult != gittoolkit::GtkEngineResult::Ok) {
        std::cerr << COLOR_RED << "Error cargando configuración" << COLOR_RESET << "\n";
        std::cerr << "Ruta esperada: " << gittoolkit::platform::GetLegacyConfigPath() << "\n";
        return 1;
    }

    // Ejecutar comando
    if (command == "validate") {
        return CmdValidate(engine);
    }
    if (command == "sync") {
        return CmdSync(engine, dryRun);
    }
    if (command == "status") {
        gittoolkit::GtkStatusOptions options;
        options.verbose = verbose;
        if (!filterSource.empty()) options.filterSource = filterSource;
        if (!filterRepo.empty()) options.filterRepo = filterRepo;
        return CmdStatus(engine, options);
    }
    if (command == "pull") {
        gittoolkit::GtkStatusOptions options;
        if (!filterSource.empty()) options.filterSource = filterSource;
        if (!filterRepo.empty()) options.filterRepo = filterRepo;
        return CmdPull(engine, options);
    }

    std::cerr << std::format("Comando desconocido: {}\n", command);
    PrintUsage();
    return 1;
}
