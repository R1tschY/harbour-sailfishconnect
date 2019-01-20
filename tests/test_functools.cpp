/*
 * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test.h"

#include <cmath>
#include <vector>

#include <sailfishconnect/helper/functools.h>

using namespace SailfishConnect;

TEST(FuncToolsTests, byKey) {
    std::vector<double> x = { 1.1, 3.2, 2.9, 4.7, 5.6 };
    std::sort(x.begin(), x.end(), byKey([](double v) { return v - floor(v); }));

    std::vector<double> expect = { 1.1, 3.2, 5.6, 4.7, 2.9 };
    EXPECT_EQ(x, expect);
}

TEST(FuncToolsTests, byKeyDesc) {
    std::vector<double> x = { 1.1, 3.2, 2.9, 4.7, 5.6 };
    std::sort(x.begin(), x.end(),
              byKeyDesc([](double v) { return v - floor(v); }));

    std::vector<double> expect = { 2.9, 4.7, 5.6, 3.2, 1.1 };
    EXPECT_EQ(x, expect);
}
