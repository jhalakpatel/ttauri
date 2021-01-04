// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/pixel_map.inl"
#include "ttauri/Path.hpp"
#include "ttauri/BezierCurve.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(pixel_map_tests, rotate90) {
    auto mask = pixel_map<uint8_t>(2, 2);
    mask[1][0] = 3; mask[1][1] = 4;
    mask[0][0] = 1; mask[0][1] = 2;

    auto r = pixel_map<uint8_t>(2, 2);
    rotate90(r, mask);
    ASSERT_EQ(r[1][0], 4); ASSERT_EQ(r[1][1], 2);
    ASSERT_EQ(r[0][0], 3); ASSERT_EQ(r[0][1], 1);
}

TEST(pixel_map_tests, rotate270) {
    auto mask = pixel_map<uint8_t>(2, 2);
    mask[1][0] = 3; mask[1][1] = 4;
    mask[0][0] = 1; mask[0][1] = 2;

    auto r = pixel_map<uint8_t>(2, 2);
    rotate270(r, mask);
    ASSERT_EQ(r[1][0], 1); ASSERT_EQ(r[1][1], 3);
    ASSERT_EQ(r[0][0], 2); ASSERT_EQ(r[0][1], 4);
}

TEST(pixel_map_tests, renderMaskFromPath) {
    auto mask = pixel_map<uint8_t>(9, 3);
    fill(mask);

    auto path = Path();
    path.moveTo(f32x4::point({1, 1}));
    path.lineTo(f32x4::point({2, 1}));
    path.lineTo(f32x4::point({2, 2}));
    path.lineTo(f32x4::point({1, 2}));
    path.closeContour();

    auto beziers = path.getBeziers();
    for (auto &&bezier: beziers) {
        bezier *= mat::S(3.0, 1.0, 1.0);
    }

    fill(mask, beziers);
    ASSERT_EQ(mask[0][0], 0);
    ASSERT_EQ(mask[0][1], 0);
    ASSERT_EQ(mask[0][2], 0);
    ASSERT_EQ(mask[0][3], 0);
    ASSERT_EQ(mask[0][4], 0);
    ASSERT_EQ(mask[0][5], 0);
    ASSERT_EQ(mask[0][6], 0);
    ASSERT_EQ(mask[0][7], 0);
    ASSERT_EQ(mask[0][8], 0);
    ASSERT_EQ(mask[1][0], 0);
    ASSERT_EQ(mask[1][1], 0);
    ASSERT_EQ(mask[1][2], 0);
    ASSERT_EQ(mask[1][3], 255);
    ASSERT_EQ(mask[1][4], 255);
    ASSERT_EQ(mask[1][5], 255);
    ASSERT_EQ(mask[1][6], 0);
    ASSERT_EQ(mask[1][7], 0);
    ASSERT_EQ(mask[1][8], 0);
    ASSERT_EQ(mask[2][0], 0);
    ASSERT_EQ(mask[2][1], 0);
    ASSERT_EQ(mask[2][2], 0);
    ASSERT_EQ(mask[2][3], 0);
    ASSERT_EQ(mask[2][4], 0);
    ASSERT_EQ(mask[2][5], 0);
    ASSERT_EQ(mask[2][6], 0);
    ASSERT_EQ(mask[2][7], 0);
    ASSERT_EQ(mask[2][8], 0);
}

