//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace gittoolkit {

/// Credential type for a repository.
enum class GtkCredentialType {
    Gcm,
    Ssh,
    Token
};

/// Convert credential type to/from string.
const char* CredentialTypeToString(GtkCredentialType type);
GtkCredentialType CredentialTypeFromString(const std::string& str);

/// Repository configuration within a source.
struct GtkRepoConfig {
    std::string name;
    GtkCredentialType credentialType = GtkCredentialType::Gcm;
    std::optional<std::string> userName;
    std::optional<std::string> userEmail;
    std::optional<std::string> folder;
};

/// Source (account) configuration.
struct GtkSourceConfig {
    std::string key;
    std::string url;
    std::string username;
    std::string folder;
    std::string userName;
    std::string userEmail;

    // GCM fields (optional)
    std::optional<std::string> gcmProvider;
    std::optional<std::string> gcmUseHttpPath;

    // SSH fields (optional)
    std::optional<std::string> sshHost;
    std::optional<std::string> sshHostname;
    std::optional<std::string> sshType;

    std::vector<GtkRepoConfig> repos;
};

/// SSH global configuration.
struct GtkSshConfig {
    bool enabled = false;
    std::string sshFolder;
};

/// GCM global configuration.
struct GtkGcmConfig {
    bool enabled = true;
    std::string helper;
    std::string credentialStore;
};

/// Global configuration.
struct GtkGlobalConfig {
    std::string folder;
    GtkSshConfig ssh;
    GtkGcmConfig gcm;
};

/// Top-level configuration.
struct GtkConfig {
    GtkGlobalConfig global;
    std::vector<GtkSourceConfig> sources;
};

/// Result of a config operation.
enum class GtkConfigResult {
    Ok,
    FileNotFound,
    ParseError,
    ValidationError,
    WriteError
};

/// Load configuration from a JSON file.
/// Supports both v1 (accounts) and v2 (sources) formats.
GtkConfigResult LoadConfig(const std::string& path, GtkConfig& config);

/// Save configuration to a JSON file in v2 format.
GtkConfigResult SaveConfig(const std::string& path, const GtkConfig& config);

/// Validate a JSON file against basic structural rules.
/// Returns Ok if valid, ValidationError with logged details if not.
GtkConfigResult ValidateConfigFile(const std::string& path);

} // namespace gittoolkit
