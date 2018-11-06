#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <unistd.h>

using namespace testing;

TEST(y, y)
{
    char cwd[1024];
    printf("%s", getcwd(cwd, sizeof(cwd)));
    EXPECT_EQ(1, 1);
    ASSERT_THAT(0, Eq(0));
}
