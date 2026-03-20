//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include "gittoolkit/platform.h"
#include "gittoolkit/log.h"

#include <cstdlib>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>
#endif

namespace gittoolkit::platform {

GtkPlatform DetectPlatform() {
#ifdef _WIN32
    // Check for MSYSTEM env var (Git Bash / MSYS2)
    const char* msystem = std::getenv("MSYSTEM");
    if (msystem != nullptr) {
        return GtkPlatform::Windows;
    }
    return GtkPlatform::Windows;
#elif defined(__APPLE__)
    return GtkPlatform::MacOS;
#else
    // Check for WSL2
    std::ifstream procVersion("/proc/version");
    if (procVersion.is_open()) {
        std::string line;
        std::getline(procVersion, line);
        if (line.find("Microsoft") != std::string::npos ||
            line.find("WSL") != std::string::npos) {
            return GtkPlatform::WSL2;
        }
    }
    return GtkPlatform::Linux;
#endif
}

const char* PlatformToString(GtkPlatform platform) {
    switch (platform) {
        case GtkPlatform::Linux:   return "linux";
        case GtkPlatform::MacOS:   return "macos";
        case GtkPlatform::Windows: return "windows";
        case GtkPlatform::WSL2:    return "wsl2";
    }
    return "unknown";
}

std::string GetHomeDirectory() {
#ifdef _WIN32
    // On Windows (Git Bash or native), use USERPROFILE
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile != nullptr) {
        return std::string(userProfile);
    }
    const char* homeDrive = std::getenv("HOMEDRIVE");
    const char* homePath = std::getenv("HOMEPATH");
    if (homeDrive != nullptr && homePath != nullptr) {
        return std::string(homeDrive) + std::string(homePath);
    }
    // Fallback: use HOME (set by Git Bash)
    const char* home = std::getenv("HOME");
    if (home != nullptr) {
        return std::string(home);
    }
    return "";
#else
    GtkPlatform plat = DetectPlatform();
    if (plat == GtkPlatform::WSL2) {
        // On WSL2, prefer USERPROFILE via wslpath
        // For now, use HOME as fallback
        const char* home = std::getenv("HOME");
        return home ? std::string(home) : "";
    }
    const char* home = std::getenv("HOME");
    return home ? std::string(home) : "";
#endif
}

std::string ExpandTilde(const std::string& path) {
    if (path.empty() || path[0] != '~') {
        return path;
    }
    std::string home = GetHomeDirectory();
    if (home.empty()) {
        LogWarn("Cannot expand ~: HOME not set");
        return path;
    }
    // Replace leading ~ with home
    return home + path.substr(1);
}

const char* GetGitCommand() {
#ifdef _WIN32
    return "git";
#else
    GtkPlatform plat = DetectPlatform();
    if (plat == GtkPlatform::WSL2) {
        return "git.exe";
    }
    return "git";
#endif
}

std::string GetConfigPath() {
    std::string home = GetHomeDirectory();
    if (home.empty()) {
        return "";
    }
#ifdef _WIN32
    // Normalize to forward slashes for consistency
    for (char& c : home) {
        if (c == '\\') {
            c = '/';
        }
    }
#endif
    return home + "/.config/git-toolkit/git-toolkit.json";
}

std::string GetLegacyConfigPath() {
    std::string home = GetHomeDirectory();
    if (home.empty()) {
        return "";
    }
#ifdef _WIN32
    for (char& c : home) {
        if (c == '\\') {
            c = '/';
        }
    }
#endif
    return home + "/.config/git-config-repos/git-config-repos.json";
}

} // namespace gittoolkit::platform
