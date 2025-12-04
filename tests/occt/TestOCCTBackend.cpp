#include <gtest/gtest.h>

#include <filesystem>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/occt/OCCTBackend.hpp>

using namespace of::domain;
using namespace of::occt;

// Minimal test to check linking and basic functionality of OCCT backend
// Requires OCCT to be installed and available.

class TestOCCTBackend : public ::testing::Test {
   protected:
    OCCTBackend backend;

    // Helper to get a temporary file path
    std::string getTempFilePath(const std::string& extension) {
        // Use std::filesystem::temp_directory_path()
        std::filesystem::path tempDir = std::filesystem::temp_directory_path();
        std::filesystem::path tempFile = tempDir / ("temp_occt_test_" + std::to_string(std::rand()) + extension);
        return tempFile.string();
    }
};

TEST_F(TestOCCTBackend, CreateExtrudedBodyBasic) {
    // Create a simple square profile
    Polygon profile;
    profile.vertices = {{0.0, 0.0, 0.0}, {10.0, 0.0, 0.0}, {10.0, 10.0, 0.0}, {0.0, 10.0, 0.0}};

    double height = 5.0;
    BackendShapeHandle handle = backend.CreateExtrudedBody(profile, height);

    EXPECT_GT(handle, 0u);
}

TEST_F(TestOCCTBackend, CreateExtrudedBodyInvalidProfile) {
    Polygon emptyProfile;
    BackendShapeHandle handle = backend.CreateExtrudedBody(emptyProfile, 10.0);
    EXPECT_EQ(handle, 0u);  // Should fail
}

TEST_F(TestOCCTBackend, ExportSTEP) {
    Polygon profile;
    profile.vertices = {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0}, {0.0, 1.0, 0.0}};
    BackendShapeHandle handle = backend.CreateExtrudedBody(profile, 1.0);
    ASSERT_GT(handle, 0u);

    std::string filePath = getTempFilePath(".step");
    bool success = backend.ExportShapeToSTEP(handle, filePath);
    EXPECT_TRUE(success);

    // Verify file exists and is not empty
    std::filesystem::path p(filePath);
    EXPECT_TRUE(std::filesystem::exists(p));
    EXPECT_GT(std::filesystem::file_size(p), 0u);

    std::filesystem::remove(p);  // Clean up
    EXPECT_FALSE(std::filesystem::exists(p));
}

TEST_F(TestOCCTBackend, ExportSTL) {
    Polygon profile;
    profile.vertices = {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0}, {0.0, 1.0, 0.0}};
    BackendShapeHandle handle = backend.CreateExtrudedBody(profile, 1.0);
    ASSERT_GT(handle, 0u);

    std::string filePath = getTempFilePath(".stl");
    bool success = backend.ExportShapeToSTL(handle, filePath, 0.1);
    EXPECT_TRUE(success);

    // Verify file exists and is not empty
    std::filesystem::path p(filePath);
    EXPECT_TRUE(std::filesystem::exists(p));
    EXPECT_GT(std::filesystem::file_size(p), 0u);

    std::filesystem::remove(p);  // Clean up
    EXPECT_FALSE(std::filesystem::exists(p));
}
