// Copyright 2019 Pokitec
// All rights reserved.

#include <TTauri/small_map.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(SmallMap, Default) {
    small_map<int,int,3> items;
    ASSERT_EQ(items.size(), 0);

    ASSERT_EQ(items.insert(10, 100), true);
    ASSERT_EQ(items.size(), 1);

    ASSERT_EQ(items.insert(20, 200), true);
    ASSERT_EQ(items.size(), 2);

    ASSERT_EQ(items.insert(10, 1000), true);
    ASSERT_EQ(items.size(), 2);

    ASSERT_EQ(items.insert(30, 300), true);
    ASSERT_EQ(items.size(), 3);

    ASSERT_EQ(items.insert(40, 400), false);
    ASSERT_EQ(items.size(), 3);

    ASSERT_EQ(items.get(10), std::optional<int>{1000});
    ASSERT_EQ(items.get(20), std::optional<int>{200});
    ASSERT_EQ(items.get(30), std::optional<int>{300});
    ASSERT_EQ(items.get(40), std::optional<int>{});

    ASSERT_EQ(items.get(10, 42), 1000);
    ASSERT_EQ(items.get(20, 42), 200);
    ASSERT_EQ(items.get(30, 42), 300);
    ASSERT_EQ(items.get(40, 42), 42);
}