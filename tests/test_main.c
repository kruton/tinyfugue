#include <stdio.h>
#include <string.h>

#include "tfconfig.h"
#include "port.h"
#include "tf.h"
#include "attr.h"
#include "output.h"

static int failures;

static String *owned_string(const char *data, int len, attr_t attrs)
{
    String *result = Stringnew(data, len, attrs);
    result->links++;
    return result;
}

static void release_string(String *value)
{
    value->links = 1;
    Stringfree(value);
}

#define EXPECT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "%s:%d: expected %s\n", \
                __FILE__, __LINE__, #condition); \
            failures++; \
        } \
    } while (0)

#define EXPECT_INT(expected, actual) \
    do { \
        long expected_value = (expected); \
        long actual_value = (actual); \
        if (expected_value != actual_value) { \
            fprintf(stderr, "%s:%d: expected %ld, got %ld\n", \
                __FILE__, __LINE__, expected_value, actual_value); \
            failures++; \
        } \
    } while (0)

static void expect_bytes(const char *expected, int expected_len,
    const String *actual)
{
    if (actual->len != expected_len ||
        memcmp(expected, actual->data, expected_len) != 0)
    {
        fprintf(stderr, "%s:%d: byte string mismatch; expected length %d, "
            "got %d\n", __FILE__, __LINE__, expected_len, actual->len);
        failures++;
    }
}

static void test_encode_ansi(void)
{
    String *input = owned_string("test", 4, F_BOLD);
    String *actual = encode_ansi(CS(input), 0);

    expect_bytes("\033[1mtest\033[m", 11, actual);
    release_string(actual);
    Stringfree(input);
}

int main(void)
{
    test_encode_ansi();

    if (failures) {
        fprintf(stderr, "%d test assertion(s) failed\n", failures);
        return 1;
    }
    puts("All tests passed");
    return 0;
}
