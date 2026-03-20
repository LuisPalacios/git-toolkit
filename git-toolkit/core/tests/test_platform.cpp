//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include <catch2/catch_test_macros.hpp>
#include "gittoolkit/platform.h"

using namespace gittoolkit::platform;

TEST_CASE("DetectPlatform returns a valid platform", "[platform]") {
    GtkPlatform plat = DetectPlatform();
    // On this Windows machine, should be Windows
    REQUIRE(plat == GtkPlatform::Windows);
}

TEST_CASE("PlatformToString returns correct strings", "[platform]") {
    REQUIRE(std::string(PlatformToString(GtkPlatform::Linux)) == "linux");
    REQUIRE(std::string(PlatformToString(GtkPlatform::MacOS)) == "macos");
    REQUIRE(std::string(PlatformToString(GtkPlatform::Windows)) == "windows");
    REQUIRE(std::string(PlatformToString(GtkPlatform::WSL2)) == "wsl2");
}

TEST_CASE("GetHomeDirectory returns a non-empty path", "[platform]") {
    std::string home = GetHomeDirectory();
    REQUIRE_FALSE(home.empty());
}

TEST_CASE("ExpandTilde expands ~ to home", "[platform]") {
    std::string home = GetHomeDirectory();
    REQUIRE_FALSE(home.empty());

    std::string expanded = ExpandTilde("~/some/path");
    REQUIRE(expanded == home + "/some/path");
}

TEST_CASE("ExpandTilde returns absolute paths unchanged", "[platform]") {
    REQUIRE(ExpandTilde("/absolute/path") == "/absolute/path");
    REQUIRE(ExpandTilde("C:/Windows/path") == "C:/Windows/path");
}

TEST_CASE("ExpandTilde returns empty string unchanged", "[platform]") {
    REQUIRE(ExpandTilde("") == "");
}

TEST_CASE("GetGitCommand returns git on Windows", "[platform]") {
    REQUIRE(std::string(GetGitCommand()) == "git");
}

TEST_CASE("GetConfigPath returns a valid path", "[platform]") {
    std::string configPath = GetConfigPath();
    REQUIRE_FALSE(configPath.empty());
    REQUIRE(configPath.find("git-toolkit.json") != std::string::npos);
}

TEST_CASE("GetLegacyConfigPath returns a valid path", "[platform]") {
    std::string legacyPath = GetLegacyConfigPath();
    REQUIRE_FALSE(legacyPath.empty());
    REQUIRE(legacyPath.find("git-config-repos.json") != std::string::npos);
}
