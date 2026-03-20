//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include <catch2/catch_test_macros.hpp>
#include "gittoolkit/sync_status.h"
#include "gittoolkit/git_ops.h"
#include "gittoolkit/log.h"

using namespace gittoolkit;
using namespace gittoolkit::sync;

TEST_CASE("StatusToString returns correct strings", "[sync]") {
    REQUIRE(std::string(StatusToString(GtkRepoStatus::Clean)) == "LIMPIO");
    REQUIRE(std::string(StatusToString(GtkRepoStatus::NeedsPull)) == "NECESITA PULL");
    REQUIRE(std::string(StatusToString(GtkRepoStatus::NeedsReview)) == "REVISAR");
    REQUIRE(std::string(StatusToString(GtkRepoStatus::BehindMain)) == "ATRASADO VS MAIN");
    REQUIRE(std::string(StatusToString(GtkRepoStatus::Error)) == "ERROR");
}

TEST_CASE("CheckRepoStatus on non-repo returns Error", "[sync]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    GtkRepoStatusInfo info = CheckRepoStatus("/tmp/nonexistent_dir_12345");
    REQUIRE(info.status == GtkRepoStatus::Error);
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}

TEST_CASE("CheckRepoStatus on current repo returns valid status", "[sync][integration]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    // Este repo (git-toolkit) debería ser un repo git válido
    std::string thisRepo = "C:/Users/luis/00.git/github-luispa/git-toolkit";
    if (git::IsGitRepo(thisRepo)) {
        GtkRepoStatusInfo info = CheckRepoStatus(thisRepo);
        REQUIRE(info.status != GtkRepoStatus::Error);
        REQUIRE_FALSE(info.branch.empty());
    }
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}

TEST_CASE("IsGitRepo detects valid repo", "[git]") {
    REQUIRE(git::IsGitRepo("C:/Users/luis/00.git/github-luispa/git-toolkit"));
}

TEST_CASE("IsGitRepo rejects non-repo", "[git]") {
    REQUIRE_FALSE(git::IsGitRepo("/tmp/nonexistent_12345"));
}

TEST_CASE("GetCurrentBranch returns branch name", "[git][integration]") {
    GtkLog::Instance().SetLevel(GtkLogLevel::Error);
    std::string thisRepo = "C:/Users/luis/00.git/github-luispa/git-toolkit";
    std::optional<std::string> branch = git::GetCurrentBranch(thisRepo);
    REQUIRE(branch.has_value());
    REQUIRE(branch.value() == "main");
    GtkLog::Instance().SetLevel(GtkLogLevel::Info);
}
