// Copyright 2019 Pokitec
// All rights reserved.

#include <gtest/gtest.h>
#include <Windows.h>
#include "Config.hpp"

using namespace std;
using namespace TTauri::Config;

TEST(Config_Config, ConfigTest) {
    auto config = Config("Config/TestFiles/config_test.txt");
    //ASSERT_TRUE(config.success());
    ASSERT_EQ(config.errorMessage(), "");

    // Accessing
    ASSERT_EQ(config.value<int64_t>("a"), 1);
    ASSERT_EQ(config.value<int64_t>("foo.bar.b"), 2);
    ASSERT_EQ(config.value<int64_t>("foo.bar.c.2"), 3);
    ASSERT_EQ(config.value<int64_t>("foo.bar.d.0.value"), 3);

    // Promoting
    ASSERT_EQ(config.value<double>("a"), 1.0);
    ASSERT_EQ(config.value<TTauri::URL>("foo.bar.d.2.value"), TTauri:: URL("nein"));

    // Modifying
    config["foo.bar.d.0.value"] = "hello"s;
    ASSERT_EQ(config.value<std::string>("foo.bar.d.0.value"), "hello"s);
}

TEST(Config_Config, SyntaxError) {
    {
        auto config = Config("file:Config/TestFiles/syntax_error.txt");
        ASSERT_TRUE(!config.success());
        ASSERT_EQ(config.errorMessage(), "file:Config/TestFiles/syntax_error.txt:4:1: syntax error, unexpected T_IDENTIFIER.");
    }

    {
        auto config = Config("file:Config/TestFiles/include_syntax_error.txt");
        ASSERT_TRUE(!config.success());
        ASSERT_EQ(config.errorMessage(),
            "file:Config/TestFiles/syntax_error.txt:4:1: syntax error, unexpected T_IDENTIFIER.\n"
            "file:Config/TestFiles/include_syntax_error.txt:2:1: Could not include file 'file:Config/TestFiles/syntax_error.txt'.");
    }
}
