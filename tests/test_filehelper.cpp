/*
 * Copyright 2018 Richard Liebscher <richard.liebscher@gmail.com>.
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
#include <QTemporaryDir>
#include <QFile>
#include <QDebug>

#include <sailfishconnect/helper/filehelper.h>

using namespace SailfishConnect;

TEST(FileHelperTests, escapeForFilePath) {
    EXPECT_EQ(escapeForFilePath("image.png"), "image.png");

    auto bad_path = QStringLiteral("file/path.\x00?");
    EXPECT_EQ(escapeForFilePath(bad_path), "filepath.?");
}

static void touchFile(const QString& path) {
    ASSERT_TRUE(QFile(path).open(QIODevice::WriteOnly));
}

TEST(FileHelperTests, nonexistingFile) {
    QTemporaryDir tmpdir;
    ASSERT_TRUE(tmpdir.isValid());

    touchFile(tmpdir.path() + "/download.txt");
    touchFile(tmpdir.path() + "/download.txt.gz");
    touchFile(tmpdir.path() + "/image.txt");
    touchFile(tmpdir.path() + "/image (1).txt");
    touchFile(tmpdir.path() + "/image (2).txt");
    touchFile(tmpdir.path() + "/.bashrc");
    touchFile(tmpdir.path() + "/dotend.");
    touchFile(tmpdir.path() + "/_");
    touchFile(tmpdir.path() + "/_ (1)");

    EXPECT_EQ(
                nonexistingFile(tmpdir.path() + "/download.txt").filePath(),
                tmpdir.path() + "/download (1).txt");

    EXPECT_EQ(
                nonexistingFile(tmpdir.path() + "/download.txt.gz").filePath(),
                tmpdir.path() + "/download (1).txt.gz");

    EXPECT_EQ(
                nonexistingFile(tmpdir.path() + "/image.txt").filePath(),
                tmpdir.path() + "/image (3).txt");

    EXPECT_EQ(
                nonexistingFile(tmpdir.path() + "/.bashrc").filePath(),
                tmpdir.path() + "/.bashrc (1)");

    EXPECT_EQ(
                nonexistingFile(tmpdir.path() + "/dotend.").filePath(),
                tmpdir.path() + "/dotend (1).");

    EXPECT_EQ(
                nonexistingFile(tmpdir.path() + "/_").filePath(),
                tmpdir.path() + "/_ (2)");

    EXPECT_EQ(
                nonexistingFile(tmpdir.path() + "/_ (1)").filePath(),
                tmpdir.path() + "/_ (1) (1)");

    EXPECT_EQ(
                nonexistingFile(tmpdir.path() + "/nonexisting.txt").filePath(),
                tmpdir.path() + "/nonexisting.txt");
}
