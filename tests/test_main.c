#include <stdio.h>
#include <string.h>

#include "tfconfig.h"
#include "port.h"
#include "tf.h"
#include "attr.h"
#include "output.h"
#include "unicode.h"
#include "keyboard.h"

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

static void test_string_shift_attributes(void)
{
    String *value = owned_string("abcd", 4, 0);

    check_charattrs(value, value->len, 0, __FILE__, __LINE__);
    value->charattrs[0] = 10;
    value->charattrs[1] = 11;
    value->charattrs[2] = 12;
    value->charattrs[3] = 13;
    value->charattrs[4] = 14;

    Stringshift(value, 2);
    expect_bytes("cd", 2, value);
    EXPECT_INT(12, value->charattrs[0]);
    EXPECT_INT(13, value->charattrs[1]);
    EXPECT_INT(14, value->charattrs[2]);
    Stringfree(value);
}

#if WIDECHAR
static void test_decode_ansi_utf8(void)
{
    String *actual;
    const char styled[] = "\033[1m\xc3\xa9\033[0mX";

    special_var[VAR_expand_tabs].val.u.ival = 0;
    actual = decode_ansi(styled, 0, EMUL_ANSI_ATTR, NULL);
    expect_bytes("\xc3\xa9X", 3, actual);
    EXPECT_TRUE(actual->charattrs != NULL);
    if (actual->charattrs) {
        EXPECT_TRUE((actual->charattrs[0] & F_BOLD) != 0);
        EXPECT_TRUE((actual->charattrs[1] & F_BOLD) != 0);
        EXPECT_TRUE((actual->charattrs[2] & F_BOLD) == 0);
    }
    release_string(actual);

    actual = decode_ansi("\xc3\xa9\xe7\x95\x8c\xf0\x9f\x98\x80",
        0, EMUL_ANSI_ATTR, NULL);
    expect_bytes("\xc3\xa9\xe7\x95\x8c\xf0\x9f\x98\x80", 9, actual);
    release_string(actual);

    actual = decode_ansi("\xc3\xa9\bX", 0, EMUL_ANSI_ATTR, NULL);
    expect_bytes("X", 1, actual);
    release_string(actual);

    actual = decode_ansi("A\xff" "B", 0, EMUL_ANSI_ATTR, NULL);
    expect_bytes("AB", 2, actual);
    release_string(actual);
}

static void test_unicode_wrapping(void)
{
    const char zwj_emoji[] =
        "\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbbX";

    EXPECT_INT(4, tf_utf8_wraplen("abcd", 4, 4, 8));
    EXPECT_INT(5, tf_utf8_wraplen("ab\xe7\x95\x8c" "c", 6, 4, 8));
    EXPECT_INT(3, tf_utf8_wraplen("e\xcc\x81x", 4, 1, 8));
    EXPECT_INT(4, tf_utf8_wraplen("\xf0\x9f\x98\x80x", 5, 2, 8));
    EXPECT_INT(3, tf_utf8_wraplen("ab cd", 5, 4, 8));
    EXPECT_INT(1, tf_utf8_wraplen("a\tb", 3, 4, 8));
    EXPECT_INT(3, tf_utf8_wraplen("\xe7\x95\x8cX", 4, 1, 8));
    EXPECT_INT(3, tf_utf8_wraplen("e\xcc\x81x", 4, 0, 8));
    EXPECT_INT(11, tf_utf8_wraplen(zwj_emoji, 12, 2, 8));
    EXPECT_INT(11, tf_utf8_wraplen(zwj_emoji, 12, 1, 8));

    {
        int end_idx;
        /* U+2600 (ambiguous/text emoji) default width should be 1 */
        EXPECT_INT(1, tf_grapheme_width("\xe2\x98\x80", 3, 0, 0, 8, &end_idx));
        EXPECT_INT(3, end_idx);

        /* U+2600 + VS16 should be 2 */
        EXPECT_INT(2, tf_grapheme_width("\xe2\x98\x80\xef\xb8\x8f", 6, 0, 0, 8, &end_idx));
        EXPECT_INT(6, end_idx);

        /* Keycap sequence should be 2 */
        EXPECT_INT(2, tf_grapheme_width("1\xef\xb8\x8f\xe2\x83\xa3", 7, 0, 0, 8, &end_idx));
        EXPECT_INT(7, end_idx);
    }
}

static void test_display_positions(void)
{
    const char zwj_emoji[] =
        "\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbbX";
    int row, column;

    tf_display_position("ab\xe7\x95\x8c" "c", 6, 5, 0, 4, 8,
        &row, &column);
    EXPECT_INT(1, row);
    EXPECT_INT(0, column);

    tf_display_position("ab\xe7\x95\x8c" "c", 6, 6, 0, 4, 8,
        &row, &column);
    EXPECT_INT(1, row);
    EXPECT_INT(1, column);

    tf_display_position("Ae\xcc\x81" "B", 5, 4, 0, 2, 8,
        &row, &column);
    EXPECT_INT(1, row);
    EXPECT_INT(0, column);

    tf_display_position(zwj_emoji, 12, 11, 0, 2, 8, &row, &column);
    EXPECT_INT(1, row);
    EXPECT_INT(0, column);

    tf_display_position("\xe7\x95\x8cX", 4, 4, 1, 4, 8,
        &row, &column);
    EXPECT_INT(1, row);
    EXPECT_INT(0, column);

    EXPECT_INT(5, tf_display_row_offset("ab\xe7\x95\x8c" "c", 6,
        1, 0, 4, 8));
    EXPECT_INT(4, tf_display_row_offset("e\xcc\x81xy", 5,
        1, 0, 2, 8));
    EXPECT_INT(11, tf_display_row_offset(zwj_emoji, 12,
        1, 0, 2, 8));
}

static void test_incoming_conversion(void)
{
    UConverter *converter;
    UErrorCode error = U_ZERO_ERROR;
    String *input;
    String *output;
    const char latin1[] = "caf\xe9";

    converter = ucnv_open("ISO-8859-1", &error);
    EXPECT_TRUE(U_SUCCESS(error));
    input = owned_string(latin1, 4, 0);
    output = owned_string(NULL, 0, 0);
    EXPECT_INT(4, tf_to_utf8(output, input, converter, 1, &error));
    EXPECT_TRUE(U_SUCCESS(error));
    expect_bytes("caf\xc3\xa9", 5, output);
    EXPECT_INT(0, input->len);
    Stringfree(output);
    Stringfree(input);
    ucnv_close(converter);

    error = U_ZERO_ERROR;
    converter = ucnv_open("UTF-8", &error);
    input = owned_string("\xe2\x82", 2, 0);
    output = owned_string(NULL, 0, 0);
    EXPECT_INT(2, tf_to_utf8(output, input, converter, 1, &error));
    EXPECT_TRUE(U_SUCCESS(error));
    expect_bytes("\xef\xbf\xbd", 3, output);
    Stringfree(output);
    Stringfree(input);
    ucnv_close(converter);

    error = U_ZERO_ERROR;
    converter = ucnv_open("UTF-8", &error);
    input = owned_string("\xe2\x82", 2, 0);
    output = owned_string(NULL, 0, 0);
    EXPECT_INT(2, tf_to_utf8(output, input, converter, 0, &error));
    EXPECT_INT(0, output->len);
    Stringcat(input, "\xac");
    EXPECT_INT(1, tf_to_utf8(output, input, converter, 1, &error));
    EXPECT_TRUE(U_SUCCESS(error));
    expect_bytes("\xe2\x82\xac", 3, output);
    Stringfree(output);
    Stringfree(input);
    ucnv_close(converter);
}

static void test_outgoing_conversion(void)
{
    UConverter *converter;
    UErrorCode error = U_ZERO_ERROR;
    String *output = owned_string(NULL, 0, 0);

    converter = ucnv_open("ISO-8859-1", &error);
    EXPECT_TRUE(U_SUCCESS(error));
    EXPECT_INT(4, tf_from_utf8(output, "caf\xc3\xa9", 5, converter, &error));
    EXPECT_TRUE(U_SUCCESS(error));
    expect_bytes("caf\xe9", 4, output);
    ucnv_close(converter);
    Stringfree(output);
}

extern Stringp keybuf;
extern int keyboard_pos;
extern conString *prompt;
extern int desired_column;
extern int kb_visual_move(int delta);

static void test_kb_visual_move_func(void)
{
    int old_pos = keyboard_pos;
    conString *old_prompt = prompt;
    int old_desired = desired_column;
    char old_content[1024];
    int old_len = keybuf->len;

    if (old_len < (int)sizeof(old_content)) {
        if (keybuf->data && old_len > 0) {
            memcpy(old_content, keybuf->data, old_len);
        }
        old_content[old_len] = '\0';
    } else {
        old_content[0] = '\0';
    }

    special_var[VAR_wrapsize].val.u.ival = 5;
    special_var[VAR_tabsize].val.u.ival = 8;

    Stringtrunc(keybuf, 0);
    Stringcat(keybuf, "abcdefgh");
    keyboard_pos = 7; // 'h'
    prompt = NULL;
    desired_column = -1;

    /* Move up 1 row */
    kb_visual_move(-1);
    EXPECT_INT(2, keyboard_pos); // 'c' (column 2)

    /* Move down 1 row */
    kb_visual_move(1);
    EXPECT_INT(7, keyboard_pos); // 'h' (column 2)

    /* Clean up */
    Stringtrunc(keybuf, 0);
    if (old_len > 0 && old_len < (int)sizeof(old_content)) {
        Stringncat(keybuf, old_content, old_len);
    }
    keyboard_pos = old_pos;
    prompt = old_prompt;
    desired_column = old_desired;
}

static void test_overwrite_and_insert(void)
{
    int old_pos = keyboard_pos;
    conString *old_prompt = prompt;
    int old_insert = special_var[VAR_insert].val.u.ival;
    char old_content[1024];
    int old_len = keybuf->len;

    if (old_len < (int)sizeof(old_content)) {
        if (keybuf->data && old_len > 0) {
            memcpy(old_content, keybuf->data, old_len);
        }
        old_content[old_len] = '\0';
    } else {
        old_content[0] = '\0';
    }

    /* Test 1: Insert mode with UTF-8 */
    special_var[VAR_insert].val.u.ival = 1;
    Stringtrunc(keybuf, 0);
    Stringcat(keybuf, "aX");
    keyboard_pos = 1; // between 'a' and 'X'

    /* Simulate typing 'é' (\xc3\xa9) */
    handle_input_string("\xc3\xa9", 2);
    expect_bytes("a\xc3\xa9X", 4, keybuf);
    EXPECT_INT(3, keyboard_pos);

    fprintf(stderr, "TEST: Starting Test 2\n"); fflush(stderr);
    /* Test 2: Overwrite mode, overwriting a 2-byte character with a 1-byte character */
    special_var[VAR_insert].val.u.ival = 0;
    fprintf(stderr, "TEST: Truncating keybuf\n"); fflush(stderr);
    Stringtrunc(keybuf, 0);
    fprintf(stderr, "TEST: Catting to keybuf\n"); fflush(stderr);
    Stringcat(keybuf, "a\xc3\xa9X"); // a, é (2 bytes), X
    fprintf(stderr, "TEST: Setting keyboard_pos\n"); fflush(stderr);
    keyboard_pos = 1; // start of 'é'

    /* Simulate typing 'u' (1 byte) in overwrite mode */
    fprintf(stderr, "TEST: Calling handle_input_string\n"); fflush(stderr);
    handle_input_string("u", 1);
    expect_bytes("auX", 3, keybuf);
    EXPECT_INT(2, keyboard_pos);

    /* Test 3: Overwrite mode, overwriting a 1-byte character with a 2-byte character */
    special_var[VAR_insert].val.u.ival = 0;
    Stringtrunc(keybuf, 0);
    Stringcat(keybuf, "abX");
    keyboard_pos = 1; // start of 'b'

    /* Simulate typing 'é' (2 bytes) in overwrite mode */
    handle_input_string("\xc3\xa9", 2);
    expect_bytes("a\xc3\xa9X", 4, keybuf);
    EXPECT_INT(3, keyboard_pos);

    /* Clean up */
    Stringtrunc(keybuf, 0);
    if (old_len > 0 && old_len < (int)sizeof(old_content)) {
        Stringncat(keybuf, old_content, old_len);
    }
    keyboard_pos = old_pos;
    prompt = old_prompt;
    special_var[VAR_insert].val.u.ival = old_insert;
}

extern int cx;
static void test_hwrite_column_tracking(void)
{
    /* Test that printing UTF-8 string advances cx by grapheme width, not byte length */
    String *str = owned_string("a\xc3\xa9X", 4, 0); // "aéX", length 4 bytes, width 3
    int old_cx = cx;

    cx = 0;
    special_var[VAR_tabsize].val.u.ival = 8;
    hwrite(CS(str), 0, str->len, 0);
    EXPECT_INT(3, cx); // should advance by 3 (grapheme width), not 4 (byte length)

    /* Test tab handling: tab should advance to next tab stop */
    cx = 1;
    Stringtrunc(str, 0);
    Stringcat(str, "\t");
    hwrite(CS(str), 0, str->len, 0);
    EXPECT_INT(8, cx); // should advance to 8 from 1 (7 columns)

    Stringfree(str);
    cx = old_cx;
}

extern int ix;
extern int iendx;
extern void physical_refresh(void);

static void test_prompt_clipping(void)
{
    conString *old_prompt = prompt;
    int old_pos = keyboard_pos;
    int old_ix = ix;
    int old_iendx = iendx;
    int old_expnonvis = special_var[VAR_expnonvis].val.u.ival;

    special_var[VAR_wrapsize].val.u.ival = 2;
    special_var[VAR_tabsize].val.u.ival = 8;
    special_var[VAR_expnonvis].val.u.ival = 1;

    prompt = (conString *)owned_string("a\xc3\xa9X", 4, 0);
    keyboard_pos = 0;

    physical_refresh();

    EXPECT_INT(2, iendx); // display width of 'X' is 1, so iendx is 2

    release_string((String *)prompt);
    prompt = old_prompt;
    keyboard_pos = old_pos;
    ix = old_ix;
    iendx = old_iendx;
    special_var[VAR_expnonvis].val.u.ival = old_expnonvis;
}
#endif

int main(void)
{
    test_encode_ansi();
    test_string_shift_attributes();
#if WIDECHAR
    test_decode_ansi_utf8();
    test_unicode_wrapping();
    test_display_positions();
    test_incoming_conversion();
    test_outgoing_conversion();
    test_kb_visual_move_func();
    test_overwrite_and_insert();
    test_hwrite_column_tracking();
    test_prompt_clipping();
#endif

    if (failures) {
        fprintf(stderr, "%d test assertion(s) failed\n", failures);
        return 1;
    }
    puts("All tests passed");
    return 0;
}
