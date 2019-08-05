// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/required.hpp"

#include <complex>
#include <cmath>
#include <limits>
#include <boost/assert.hpp>
#include <glm/glm.hpp>
#include <gsl/gsl>

namespace TTauri {

constexpr long double pi = 3.141592653589793238462643383279502884L;

template<typename T, typename M>
constexpr T modulo(T x, M m)
{
    if (x >= 0) {
        return x % m;
    } else {
        return m - (-x % m);
    }
}

}
