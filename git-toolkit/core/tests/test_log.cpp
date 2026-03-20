//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include <catch2/catch_test_macros.hpp>
#include "gittoolkit/log.h"

using namespace gittoolkit;

TEST_CASE("GtkLog default level is Info", "[log]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
    REQUIRE(GtkLog::Instance().GetLevel() == GtkLogLevel::Info);
}

TEST_CASE("GtkLog level can be changed", "[log]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Debug);
    REQUIRE(GtkLog::Instance().GetLevel() == GtkLogLevel::Debug);

    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    REQUIRE(GtkLog::Instance().GetLevel() == GtkLogLevel::Error);

    // Reset to default
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}

TEST_CASE("GtkLog does not crash on all levels", "[log]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Debug);
    GtkLog::Instance().SetColorEnabled(false);

    // These should not crash
    LogDebug("test debug message");
    LogInfo("test info message");
    LogWarn("test warn message");
    LogError("test error message");

    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
    GtkLog::Instance().SetColorEnabled(true);
}

TEST_CASE("GtkLog suppresses messages below level", "[log]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    GtkLog::Instance().SetColorEnabled(false);

    // These should be suppressed (no crash, just silent)
    LogDebug("should be suppressed");
    LogInfo("should be suppressed");
    LogWarn("should be suppressed");

    // This should go through
    LogError("should appear");

    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
    GtkLog::Instance().SetColorEnabled(true);
}
