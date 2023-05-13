#include <gtest/gtest.h>

#include "tfconfig.h"
#include "port.h"
#include "tf.h"

#include "attr.h"

TEST(AttrTest, EncodeAnsi_EmptyString_Success)
{
    conString *input = CS(Stringnew(NULL, 0, 0));
    String *actual = encode_ansi(input, 0);

    EXPECT_EQ(0, actual->len);
    EXPECT_STREQ("", actual->data);
}

TEST(AttrTest, EncodeAnsi_BoldString_Success)
{
    conString *input = CS(Stringnew("test", 4, F_BOLD));
    String *actual = encode_ansi(input, 0);

    EXPECT_EQ(11, actual->len);
    EXPECT_STREQ("\033[1mtest\033[m", actual->data);
}
