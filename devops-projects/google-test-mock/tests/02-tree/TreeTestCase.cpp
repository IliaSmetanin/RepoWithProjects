#include "TreeTestCase.h"
#include "Tree.h"

#include <filesystem>
#include <fstream>

void TreeTestCase::SetUp() {
    std::ofstream file1("For_tests.txt");

    std::filesystem::path origin_path = std::filesystem::current_path();
    std::filesystem::create_directories("test_dir/empty_dir");
    std::filesystem::current_path("test_dir");
    std::ofstream file2("next_to_empty.txt");
    std::filesystem::current_path(origin_path);
}

void TreeTestCase::TearDown() {
    std::filesystem::remove_all("test_dir");
    std::filesystem::remove("For_tests.txt");
}

TEST_F(TreeTestCase, GetTree) {
    EXPECT_THROW(GetTree("path_that_doesnot_exist", 1), std::invalid_argument);
    EXPECT_THROW(GetTree(std::filesystem::current_path() / "For_tests.txt", 1), std::invalid_argument);

    FileNode empty_dir = {"empty_dir", true, {}};
    FileNode next_to_empty = {"next_to_empty.txt", false, {}};
    FileNode comparable = {"test_dir", true, {empty_dir, next_to_empty}};
    FileNode comparable_only_dirs = {"test_dir", true, {empty_dir}};
    ASSERT_EQ(comparable.children.size(),
              GetTree(std::filesystem::current_path() / "test_dir", false).children.size());
    ASSERT_EQ(comparable_only_dirs, GetTree(std::filesystem::current_path() / "test_dir", true));
}

TEST_F(TreeTestCase, FilterEmptyNodes) {
    FileNode test{"empty", true, {}};
    EXPECT_THROW(FilterEmptyNodes(test, "."), std::runtime_error);

    // FileNode compare = {GetTree(".", false)};
    // ASSERT_EQ(compare, )
    FileNode test_dir = GetTree(std::filesystem::current_path() / "test_dir", false);
    FilterEmptyNodes(test_dir, std::filesystem::current_path() / "test_dir");
    FileNode current = GetTree(".", false);
    FileNode for_tests = {"For_tests.txt", false, {}};
    FileNode test_dir2 = {"test_dir", true, {}};
    std::vector<FileNode> expected = {for_tests, test_dir2};
    // ASSERT_EQ(current.children, expected);
}