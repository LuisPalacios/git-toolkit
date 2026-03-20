//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include <catch2/catch_test_macros.hpp>
#include "gittoolkit/config.h"
#include "gittoolkit/platform.h"
#include "gittoolkit/log.h"

#include <fstream>
#include <filesystem>

using namespace gittoolkit;

static const char* TEST_V1_JSON = R"({
    "global": {
        "folder": "~/00.git",
        "credential_ssh": {
            "enabled": "false",
            "ssh_folder": "~/.ssh"
        },
        "credential_gcm": {
            "enabled": "true",
            "helper": "manager",
            "credentialStore": "wincredman"
        }
    },
    "accounts": {
        "github-luis": {
            "url": "https://github.com/luis",
            "username": "luis",
            "folder": "github-luis",
            "name": "Luis Palacios",
            "email": "luis@example.com",
            "gcm_provider": "github",
            "repos": {
                "repo-a": {
                    "credential_type": "gcm"
                },
                "repo-b": {
                    "credential_type": "ssh",
                    "folder": "~/custom/path"
                }
            }
        }
    }
})";

static const char* TEST_V2_JSON = R"({
    "global": {
        "folder": "~/projects",
        "credential_ssh": {
            "enabled": true,
            "ssh_folder": "~/.ssh"
        },
        "credential_gcm": {
            "enabled": true,
            "helper": "manager",
            "credentialStore": "keychain"
        }
    },
    "sources": {
        "gitea-infra": {
            "url": "https://git.example.org/infra",
            "username": "admin",
            "folder": "infra",
            "name": "Admin User",
            "email": "admin@example.org",
            "repos": {
                "deploy-scripts": {
                    "credential_type": "token"
                }
            }
        }
    }
})";

static std::string WriteTempJson(const char* content) {
    std::string path = (std::filesystem::temp_directory_path() / "gittoolkit_test.json").string();
    std::ofstream f(path);
    f << content;
    f.close();
    return path;
}

TEST_CASE("CredentialType conversions", "[config]") {
    REQUIRE(CredentialTypeFromString("gcm") == GtkCredentialType::Gcm);
    REQUIRE(CredentialTypeFromString("ssh") == GtkCredentialType::Ssh);
    REQUIRE(CredentialTypeFromString("token") == GtkCredentialType::Token);
    REQUIRE(CredentialTypeFromString("unknown") == GtkCredentialType::Gcm);

    REQUIRE(std::string(CredentialTypeToString(GtkCredentialType::Gcm)) == "gcm");
    REQUIRE(std::string(CredentialTypeToString(GtkCredentialType::Ssh)) == "ssh");
    REQUIRE(std::string(CredentialTypeToString(GtkCredentialType::Token)) == "token");
}

TEST_CASE("LoadConfig parses v1 format (accounts)", "[config]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    std::string path = WriteTempJson(TEST_V1_JSON);

    GtkConfig config;
    GtkConfigResult result = LoadConfig(path, config);
    REQUIRE(result == GtkConfigResult::Ok);

    // Global
    REQUIRE(config.global.folder.find("00.git") != std::string::npos);
    REQUIRE(config.global.ssh.enabled == false);
    REQUIRE(config.global.gcm.enabled == true);
    REQUIRE(config.global.gcm.helper == "manager");
    REQUIRE(config.global.gcm.credentialStore == "wincredman");

    // Sources (loaded from "accounts")
    REQUIRE(config.sources.size() == 1);
    REQUIRE(config.sources[0].key == "github-luis");
    REQUIRE(config.sources[0].url == "https://github.com/luis");
    REQUIRE(config.sources[0].userName == "Luis Palacios");
    REQUIRE(config.sources[0].gcmProvider.has_value());
    REQUIRE(config.sources[0].gcmProvider.value() == "github");

    // Repos
    REQUIRE(config.sources[0].repos.size() == 2);
    REQUIRE(config.sources[0].repos[0].name == "repo-a");
    REQUIRE(config.sources[0].repos[0].credentialType == GtkCredentialType::Gcm);
    REQUIRE(config.sources[0].repos[1].name == "repo-b");
    REQUIRE(config.sources[0].repos[1].credentialType == GtkCredentialType::Ssh);
    REQUIRE(config.sources[0].repos[1].folder.has_value());

    std::filesystem::remove(path);
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}

TEST_CASE("LoadConfig parses v2 format (sources)", "[config]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    std::string path = WriteTempJson(TEST_V2_JSON);

    GtkConfig config;
    GtkConfigResult result = LoadConfig(path, config);
    REQUIRE(result == GtkConfigResult::Ok);

    REQUIRE(config.global.ssh.enabled == true);
    REQUIRE(config.global.gcm.credentialStore == "keychain");

    REQUIRE(config.sources.size() == 1);
    REQUIRE(config.sources[0].key == "gitea-infra");
    REQUIRE(config.sources[0].repos.size() == 1);
    REQUIRE(config.sources[0].repos[0].credentialType == GtkCredentialType::Token);

    std::filesystem::remove(path);
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}

TEST_CASE("LoadConfig returns FileNotFound for missing file", "[config]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    GtkConfig config;
    GtkConfigResult result = LoadConfig("/nonexistent/path.json", config);
    REQUIRE(result == GtkConfigResult::FileNotFound);
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}

TEST_CASE("LoadConfig returns ParseError for invalid JSON", "[config]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    std::string path = WriteTempJson("{ this is not valid json }");

    GtkConfig config;
    GtkConfigResult result = LoadConfig(path, config);
    REQUIRE(result == GtkConfigResult::ParseError);

    std::filesystem::remove(path);
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}

TEST_CASE("SaveConfig writes valid JSON in v2 format", "[config]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    std::string path = WriteTempJson(TEST_V1_JSON);

    // Cargar v1
    GtkConfig config;
    LoadConfig(path, config);

    // Guardar como v2
    std::string outPath = (std::filesystem::temp_directory_path() / "gittoolkit_test_out.json").string();
    GtkConfigResult result = SaveConfig(outPath, config);
    REQUIRE(result == GtkConfigResult::Ok);

    // Recargar y verificar que es v2 (sources, no accounts)
    {
        std::ifstream f(outPath);
        nlohmann::json root = nlohmann::json::parse(f);
        REQUIRE(root.contains("sources"));
        REQUIRE_FALSE(root.contains("accounts"));
        REQUIRE(root["sources"].contains("github-luis"));

        // Verificar que booleans son reales, no strings
        REQUIRE(root["global"]["credential_ssh"]["enabled"].is_boolean());
        REQUIRE(root["global"]["credential_gcm"]["enabled"].is_boolean());
    } // ifstream se cierra aquí antes de remove

    std::filesystem::remove(outPath);
    std::filesystem::remove(path);
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}

TEST_CASE("ValidateConfigFile validates v1 correctly", "[config]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    std::string path = WriteTempJson(TEST_V1_JSON);
    REQUIRE(ValidateConfigFile(path) == GtkConfigResult::Ok);
    std::filesystem::remove(path);
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}

TEST_CASE("ValidateConfigFile rejects missing global", "[config]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    std::string path = WriteTempJson(R"({"accounts": {}})");
    REQUIRE(ValidateConfigFile(path) == GtkConfigResult::ValidationError);
    std::filesystem::remove(path);
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}

TEST_CASE("ValidateConfigFile rejects source without url", "[config]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    std::string path = WriteTempJson(R"({
        "global": {"folder": "~/git"},
        "sources": {"bad": {"username": "u", "folder": "f"}}
    })");
    REQUIRE(ValidateConfigFile(path) == GtkConfigResult::ValidationError);
    std::filesystem::remove(path);
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}

TEST_CASE("ValidateConfigFile works on real legacy config", "[config][integration]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    std::string legacyPath = gittoolkit::platform::GetLegacyConfigPath();
    if (std::filesystem::exists(legacyPath)) {
        REQUIRE(ValidateConfigFile(legacyPath) == GtkConfigResult::Ok);
    }
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}
