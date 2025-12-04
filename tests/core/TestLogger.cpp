#include <gtest/gtest.h>

#include <fstream>
#include <ontoflow/core/Colors.hpp>
#include <ontoflow/core/Logger.hpp>
#include <string>

using namespace of::core;

class TestLogger : public ::testing::Test {
   protected:
    void SetUp() override {
        // Reset logger to default state before each test
        Logger::getInstance().setLogFile("");
        Logger::getInstance().setLogLevel(LogLevel::Info | LogLevel::Debug | LogLevel::Warn | LogLevel::Error);
    }
};

TEST_F(TestLogger, SingletonInstance) {
    Logger& logger1 = Logger::getInstance();
    Logger& logger2 = Logger::getInstance();
    EXPECT_EQ(&logger1, &logger2);
}

TEST_F(TestLogger, LogLevelBitmask) {
    Logger::getInstance().setLogLevel(LogLevel::Info | LogLevel::Error);
    LogLevel level = Logger::getInstance().getLogLevel();

    EXPECT_TRUE((level & LogLevel::Info) != LogLevel::None);
    EXPECT_TRUE((level & LogLevel::Error) != LogLevel::None);
    EXPECT_FALSE((level & LogLevel::Debug) != LogLevel::None);
    EXPECT_FALSE((level & LogLevel::Warn) != LogLevel::None);
}

TEST_F(TestLogger, FileLogging) {
    std::string filename = "test_log.txt";
    Logger::getInstance().setLogFile(filename);

    LOG(Info) << "Test Message 1";
    LOG(Error) << "Test Message 2";

    // Verify file content
    std::ifstream infile(filename);
    ASSERT_TRUE(infile.is_open());

    std::string line;
    bool found1 = false;
    bool found2 = false;

    while (std::getline(infile, line)) {
        if (line.find("Test Message 1") != std::string::npos && line.find("[Info]") != std::string::npos)
            found1 = true;
        if (line.find("Test Message 2") != std::string::npos && line.find("[Error]") != std::string::npos)
            found2 = true;
    }

    infile.close();
    std::remove(filename.c_str());

    EXPECT_TRUE(found1);
    EXPECT_TRUE(found2);
}

TEST_F(TestLogger, ColorsExist) {
    EXPECT_FLOAT_EQ(nord::Nord0.x, 0.180f);
    EXPECT_FLOAT_EQ(ghibli::Sky.x, 0.702f);
}
