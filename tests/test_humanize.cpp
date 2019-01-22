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

#include <QString>

#include <sailfishconnect/helper/humanize.h>

using namespace SailfishConnect;

TEST(Humanize, humanizeBytes_bytes) {
    EXPECT_EQ(humanizeBytes(0), "0 B");
    EXPECT_EQ(humanizeBytes(1), "1 B");
    EXPECT_EQ(humanizeBytes(-1), "-1 B");
}

TEST(Humanize, humanizeBytes_xbytes) {
    EXPECT_EQ(humanizeBytes(1024), "1.00 kB");
    EXPECT_EQ(humanizeBytes(2 * 1024), "2.00 kB");
    EXPECT_EQ(humanizeBytes(1.5 * 1024), "1.50 kB");
    EXPECT_EQ(humanizeBytes(-1024), "-1.00 kB");
}

TEST(Humanize, humanizeBytes_allbytes) {
    EXPECT_EQ(humanizeBytes(1024), "1.00 kB");
    EXPECT_EQ(humanizeBytes(1024 * 1024), "1.00 MB");
    EXPECT_EQ(humanizeBytes(1024 * 1024 * 1024), "1.00 GB");
    EXPECT_EQ(humanizeBytes(1024LL * 1024LL * 1024LL * 1024LL), "1.00 TB");
}

TEST(Humanize, humanizeBytes_toobig) {
    EXPECT_EQ(humanizeBytes(
                  1024LL * 1024LL * 1024LL * 1024LL * 1024LL), "1024.00 TB");
}
