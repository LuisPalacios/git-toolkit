//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#pragma once

#include <string>

namespace gittoolkit::platform {

/// Detected platform type.
enum class GtkPlatform {
    Linux,
    MacOS,
    Windows,  // Git Bash or native Windows
    WSL2
};

/// Detect the current platform.
GtkPlatform DetectPlatform();

/// Get platform name as string.
const char* PlatformToString(GtkPlatform platform);

/// Expand ~ prefix to the user's home directory.
/// On WSL2, expands to USERPROFILE (Windows home) not Linux home.
std::string ExpandTilde(const std::string& path);

/// Get the user's home directory.
std::string GetHomeDirectory();

/// Get the git command to use (git.exe on WSL2, git elsewhere).
const char* GetGitCommand();

/// Get the default config file path for git-toolkit.json.
std::string GetConfigPath();

/// Get the legacy config file path for git-config-repos.json.
std::string GetLegacyConfigPath();

} // namespace gittoolkit::platform
