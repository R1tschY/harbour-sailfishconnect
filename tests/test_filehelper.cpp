#include <test.h>

#include <QString>
#include <QTemporaryDir>
#include <QFile>
#include <QDebug>

#include <sailfishconnect/helper/filehelper.h>

using namespace SailfishConnect;

TEST(FileHelper, escapeForFilePath) {
    EXPECT_EQ(escapeForFilePath("image.png"), "image.png");

    auto bad_path = QStringLiteral("file/path.\x00?");
    EXPECT_EQ(escapeForFilePath(bad_path), "filepath.?");
}

static void touchFile(const QString& path) {
    ASSERT_TRUE(QFile(path).open(QIODevice::WriteOnly));
}

TEST(FileHelper, nonexistingFile) {
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
