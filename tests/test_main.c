#include <stdio.h>
#include <string.h>

#include "tfconfig.h"
#include "port.h"
#include "tf.h"
#include "attr.h"
#include "output.h"
#include "unicode.h"
#include "keyboard.h"
#include "parse.h"
#include "pattern.h"
#include "search.h"
#include "tfio.h"
#include "variable.h"

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

static void test_display_width_helpers(void)
{
    int end_idx;

    EXPECT_INT(5, tf_grapheme_width("\t", 1, 0, 3, 8, &end_idx));
    EXPECT_INT(1, end_idx);
    EXPECT_INT(6, tf_string_width("\tX", 2, 3, 8));
    EXPECT_INT(1, tf_column_to_byte_offset("\tX", 2, 4, 8));
    EXPECT_INT(0, tf_bytes_for_width("\tX", 2, 0, 0, 7, 8));
    EXPECT_INT(1, tf_bytes_for_width("\tX", 2, 0, 0, 8, 8));
    EXPECT_INT(2, tf_bytes_for_width("\tX", 2, 0, 3, 6, 8));
}

static void test_default_tflibdir(void)
{
    EXPECT_TRUE(strcmp(DEFAULT_TFLIBD, EXPECTED_TFLIBDIR) == 0);
}

#if WIDECHAR
static void test_character_offsets(void)
{
    EXPECT_INT(3, tf_character_offset("ascii", 5, 1, 2));
    EXPECT_INT(0, tf_character_offset("ascii", 5, 1, -2));

    /* CRLF is one grapheme cluster and must still use ICU segmentation. */
    EXPECT_INT(2, tf_character_offset("\r\nX", 3, 0, 1));
    EXPECT_INT(0, tf_character_offset("\r\nX", 3, 2, -1));
}

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
    EXPECT_INT(3, tf_string_width("A\xe7\x95\x8c", 4, 0, 8));
    EXPECT_INT(4, tf_column_to_byte_offset("A\xe7\x95\x8cX", 5, 2, 8));
    EXPECT_INT(1, tf_bytes_for_width("A\xe7\x95\x8cX", 5, 0, 0, 2, 8));

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

    /* Test 2: Overwrite mode, overwriting a 2-byte character with a 1-byte character */
    special_var[VAR_insert].val.u.ival = 0;
    Stringtrunc(keybuf, 0);
    Stringcat(keybuf, "a\xc3\xa9X"); // a, é (2 bytes), X
    keyboard_pos = 1; // start of 'é'

    /* Simulate typing 'u' (1 byte) in overwrite mode */
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

    /* cx is one-based, so a tab in terminal column 1 occupies 8 columns. */
    cx = 1;
    Stringtrunc(str, 0);
    Stringcat(str, "\t");
    hwrite(CS(str), 0, str->len, 0);
    EXPECT_INT(9, cx);

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

static void test_do_kbword_func(void)
{
    int old_pos = keyboard_pos;
    Stringtrunc(keybuf, 0);
    Stringcat(keybuf, "  abc  \xc3\xa9X  ");

    EXPECT_INT(5, do_kbword(0, 1));
    EXPECT_INT(5, do_kbword(2, 1));
    EXPECT_INT(10, do_kbword(5, 1));
    EXPECT_INT(10, do_kbword(7, 1));

    EXPECT_INT(7, do_kbword(12, -1));
    EXPECT_INT(7, do_kbword(10, -1));
    EXPECT_INT(2, do_kbword(7, -1));
    EXPECT_INT(2, do_kbword(5, -1));

    Stringtrunc(keybuf, 0);
    keyboard_pos = old_pos;
}

static void test_grapheme_expr_functions(void)
{
    String *attributed;
    Value *val;
    Var *var;

    val = expr_value("grapheme_count('a\xc3\xa9X')");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_INT(3, valint(val));
        freeval(val);
    }

    val = expr_value("grapheme_offset('a\xc3\xa9X', 0, 2)");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_INT(3, valint(val));
        freeval(val);
    }

    val = expr_value("grapheme_offset('a\xc3\xa9X', 3, -1)");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_INT(1, valint(val));
        freeval(val);
    }

    val = expr_value("grapheme_substr('a\xc3\xa9X', 1, 1)");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_TRUE((val->type & TYPE_STR) != 0);
        if (val->type & TYPE_STR) {
            expect_bytes("\xc3\xa9", 2, (const String *)valstr(val));
        }
        freeval(val);
    }

    val = expr_value("grapheme_substr('a\xc3\xa9X', 1)");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_TRUE((val->type & TYPE_STR) != 0);
        if (val->type & TYPE_STR) {
            expect_bytes("\xc3\xa9X", 3, (const String *)valstr(val));
        }
        freeval(val);
    }

    val = expr_value("tolower('A\xc3\x89X')");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_TRUE((val->type & TYPE_STR) != 0);
        if (val->type & TYPE_STR) {
            expect_bytes("a\xc3\xa9x", 4, (const String *)valstr(val));
        }
        freeval(val);
    }

    val = expr_value("toupper('a\xc3\xa9x')");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_TRUE((val->type & TYPE_STR) != 0);
        if (val->type & TYPE_STR) {
            expect_bytes("A\xc3\x89X", 4, (const String *)valstr(val));
        }
        freeval(val);
    }

    val = expr_value("tolower('A\xc3\x89X', 3)");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_TRUE((val->type & TYPE_STR) != 0);
        if (val->type & TYPE_STR) {
            expect_bytes("a\xc3\xa9X", 4, (const String *)valstr(val));
        }
        freeval(val);
    }

    attributed = owned_string("A\xc3\x89X", 4, 0);
    check_charattrs(attributed, attributed->len + 1, 0, __FILE__, __LINE__);
    attributed->charattrs[0] = 1;
    attributed->charattrs[1] = 2;
    attributed->charattrs[2] = 2;
    attributed->charattrs[3] = 4;
    var = setstrvar(newglobalvar("__case_attr"), CS(attributed), 0);
    val = expr_value("tolower(__case_attr)");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_TRUE(valstr(val)->charattrs != NULL);
        if (valstr(val)->charattrs) {
            EXPECT_INT(1, valstr(val)->charattrs[0]);
            EXPECT_INT(2, valstr(val)->charattrs[1]);
            EXPECT_INT(2, valstr(val)->charattrs[2]);
            EXPECT_INT(4, valstr(val)->charattrs[3]);
        }
        freeval(val);
    }
    unsetvar(var);
    Stringfree(attributed);

    attributed = owned_string("a\xc3\xa9x", 4, 0);
    check_charattrs(attributed, attributed->len + 1, 0, __FILE__, __LINE__);
    attributed->charattrs[0] = 5;
    attributed->charattrs[1] = 6;
    attributed->charattrs[2] = 6;
    attributed->charattrs[3] = 8;
    var = setstrvar(newglobalvar("__case_attr"), CS(attributed), 0);
    val = expr_value("toupper(__case_attr)");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_TRUE(valstr(val)->charattrs != NULL);
        if (valstr(val)->charattrs) {
            EXPECT_INT(5, valstr(val)->charattrs[0]);
            EXPECT_INT(6, valstr(val)->charattrs[1]);
            EXPECT_INT(6, valstr(val)->charattrs[2]);
            EXPECT_INT(8, valstr(val)->charattrs[3]);
        }
        freeval(val);
    }
    unsetvar(var);
    Stringfree(attributed);

    val = expr_value("regmatch('^.X$', '\xc3\xa9X')");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_INT(1, valint(val));
        freeval(val);
    }

    val = expr_value("regmatch('^\\\\w+$', '\xe4\xb8\xaa')");
    EXPECT_TRUE(val != NULL);
    if (val) {
        EXPECT_INT(1, valint(val));
        freeval(val);
    }
}

static void test_tf_utf8_incomplete_bytes(void)
{
    EXPECT_INT(1, tf_utf8_incomplete_bytes("\xc3", 1));
    EXPECT_INT(0, tf_utf8_incomplete_bytes("\xc3\xa9", 2));
    EXPECT_INT(0, tf_utf8_incomplete_bytes("abc", 3));
    EXPECT_INT(2, tf_utf8_incomplete_bytes("a\xe4\xb8", 3));
    EXPECT_INT(0, tf_utf8_incomplete_bytes("a\xe4\xb8\xaa", 4));
    EXPECT_INT(3, tf_utf8_incomplete_bytes("xyz\xf0\x9f\x98", 6));
    EXPECT_INT(0, tf_utf8_incomplete_bytes("xyz\xf0\x9f\x98\x80", 7));
}

#include <unistd.h>
#include <fcntl.h>

extern const char *clear_screen;
extern const char *clear_to_eol;
extern const char *cursor_address;
extern int can_have_visual;
extern void bufflush(void);

static int stdout_pipe[2];
static int saved_stdout;

static void start_capturing_stdout(void)
{
    fflush(stdout);
    bufflush();
    saved_stdout = dup(STDOUT_FILENO);
    if (pipe(stdout_pipe) < 0) {
        perror("pipe");
        return;
    }
    fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    close(stdout_pipe[1]);
}

static int get_captured_stdout(char *buf, int max_len)
{
    fflush(stdout);
    bufflush();
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    int n = read(stdout_pipe[0], buf, max_len - 1);
    close(stdout_pipe[0]);
    if (n < 0) n = 0;
    buf[n] = '\0';
    return n;
}

extern void (*tp)(const char *str);
static void test_tp(const char *str)
{
    if (str) {
        printf("%s", str);
    }
}

static void test_pseudo_terminal_rendering(void)
{
    conString *old_prompt = prompt;
    int old_pos = keyboard_pos;
    int old_ix = ix;
    int old_iendx = iendx;
    int old_visual = special_var[VAR_visual].val.u.ival;
    int old_wrapsize = special_var[VAR_wrapsize].val.u.ival;

    char buf[1024];
    int n;

    tp = test_tp;

    special_var[VAR_visual].val.u.ival = 0;
    special_var[VAR_wrapsize].val.u.ival = 80;
    prompt = (conString *)owned_string("a\xc3\xa9", 3, 0);
    keyboard_pos = 0;

    start_capturing_stdout();
    physical_refresh();
    n = get_captured_stdout(buf, sizeof(buf));

    EXPECT_TRUE(n > 0);
    EXPECT_TRUE(strstr(buf, "a\xc3\xa9") != NULL);

    clear_screen = "\033[H\033[J";
    clear_to_eol = "\033[K";
    cursor_address = "\033[%d;%dH";
    can_have_visual = 1;

    special_var[VAR_visual].val.u.ival = 1;
    keyboard_pos = 0;

    extern void logical_refresh(void);
    start_capturing_stdout();
    logical_refresh();
    n = get_captured_stdout(buf, sizeof(buf));

    EXPECT_TRUE(n > 0);
    EXPECT_TRUE(strstr(buf, "a\xc3\xa9") != NULL);

    release_string((String *)prompt);
    prompt = old_prompt;
    keyboard_pos = old_pos;
    ix = old_ix;
    iendx = old_iendx;
    special_var[VAR_visual].val.u.ival = old_visual;
    special_var[VAR_wrapsize].val.u.ival = old_wrapsize;

    clear_screen = NULL;
    clear_to_eol = NULL;
    cursor_address = NULL;
    can_have_visual = 0;
    tp = NULL;
}

static void test_screen_redraw_utf8(void)
{
    const char box_line[] =
	"\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x90";
    const char *old_cursor_address = cursor_address;
    void (*old_tp)(const char *) = tp;
    int old_visual = special_var[VAR_visual].val.u.ival;
    int old_wrapsize = special_var[VAR_wrapsize].val.u.ival;
    Screen *screen = new_screen(10);
    String *line = owned_string(box_line, sizeof(box_line) - 1, 0);
    char buf[1024];
    int n;

    tp = test_tp;
    cursor_address = "\033[%d;%dH";
    special_var[VAR_visual].val.u.ival = 0;
    special_var[VAR_wrapsize].val.u.ival = 4;

    enscreen(screen, CS(line));
    Stringfree(line);
    screen->top = screen->bot = screen->maxbot = screen->pline.head;
    screen->viewsize = 1;

    start_capturing_stdout();
    redraw_window(screen, 1);
    n = get_captured_stdout(buf, sizeof(buf));

    EXPECT_TRUE(n > 0);
    EXPECT_TRUE(strstr(buf, box_line) != NULL);

    free_screen(screen);
    special_var[VAR_visual].val.u.ival = old_visual;
    special_var[VAR_wrapsize].val.u.ival = old_wrapsize;
    cursor_address = old_cursor_address;
    tp = old_tp;
}

static void test_status_bar_utf8(void)
{
    extern int columns;
    extern String status_line[][1];
    extern void format_status_line(void);
    extern struct Value *handle_status_add_command(String * args, int offset);

    int old_columns = columns;
    columns = 20;

    /* Test 1: Single-byte and 2-byte UTF-8 character (visual width 3, bytes 4) */
    {
	Var *my_var = newglobalvar("my_status_var");
	String *val_str = owned_string("a\xc3\xa9X", -1, 0);  // "aéX"
	setstrvar(my_var, CS(val_str), 0);
	Stringfree(val_str);

	String *args = owned_string("-c my_status_var:0 \"hello\":5", -1, 0);
	struct Value *ret = handle_status_add_command(args, 0);
	Stringfree(args);
	freeval(ret);

	format_status_line();

	int vis_width = tf_string_width(status_line[0]->data,
					status_line[0]->len, 0, tabsize);
	EXPECT_INT(20, vis_width);
	EXPECT_INT(21, status_line[0]->len);
	expect_bytes("a\xc3\xa9X____________hello", 21, status_line[0]);

	/* Clean up status fields */
	String *reset_args = owned_string("-c", -1, 0);
	struct Value *reset_ret = handle_status_add_command(reset_args, 0);
	Stringfree(reset_args);
	freeval(reset_ret);

	unsetvar(my_var);
    }

    /* Test 2: Double-width Chinese character (visual width 4, bytes 5) */
    {
	Var *my_var = newglobalvar("my_status_var");
	String *val_str = owned_string("a\xe7\x95\x8cX", -1, 0);  // "a界X", '界' is 3 bytes, width 2
	setstrvar(my_var, CS(val_str), 0);
	Stringfree(val_str);

	String *args = owned_string("-c my_status_var:0 \"hello\":5", -1, 0);
	struct Value *ret = handle_status_add_command(args, 0);
	Stringfree(args);
	freeval(ret);

	format_status_line();

	int vis_width = tf_string_width(status_line[0]->data,
					status_line[0]->len, 0, tabsize);
	EXPECT_INT(20, vis_width);
	EXPECT_INT(21, status_line[0]->len);
	expect_bytes("a\xe7\x95\x8cX___________hello", 21, status_line[0]);

	/* Clean up status fields */
	String *reset_args = owned_string("-c", -1, 0);
	struct Value *reset_ret = handle_status_add_command(reset_args, 0);
	Stringfree(reset_args);
	freeval(reset_ret);

	unsetvar(my_var);
    }

    /* Test 3: A grapheme wider than the remaining field must be omitted. */
    {
	Var *my_var = newglobalvar("my_status_var");
	String *val_str = owned_string("A\xe7\x95\x8c", -1, 0);	 // "A界"
	setstrvar(my_var, CS(val_str), 0);
	Stringfree(val_str);

	String *args = owned_string("-c my_status_var:2 \"hello\":5", -1, 0);
	struct Value *ret = handle_status_add_command(args, 0);
	Stringfree(args);
	freeval(ret);

	format_status_line();

	EXPECT_INT(20, tf_string_width(status_line[0]->data,
				       status_line[0]->len, 0, tabsize));
	expect_bytes("A_hello_____________", 20, status_line[0]);

	String *reset_args = owned_string("-c", -1, 0);
	struct Value *reset_ret = handle_status_add_command(reset_args, 0);
	Stringfree(reset_args);
	freeval(reset_ret);

	unsetvar(my_var);
    }

    /* Test 4: Tabs expand from the field's actual screen column. */
    {
	Var *my_var = newglobalvar("my_status_var");
	String *val_str = owned_string("\tX", -1, 0);
	setstrvar(my_var, CS(val_str), 0);
	Stringfree(val_str);

	String *args = owned_string("-c \"abc\":3 my_status_var:6 \"z\":1",
				    -1, 0);
	struct Value *ret = handle_status_add_command(args, 0);
	Stringfree(args);
	freeval(ret);

	format_status_line();

	EXPECT_INT(20, tf_string_width(status_line[0]->data,
				       status_line[0]->len, 0, tabsize));
	expect_bytes("abc\tXz__________", 16, status_line[0]);

	String *reset_args = owned_string("-c", -1, 0);
	struct Value *reset_ret = handle_status_add_command(reset_args, 0);
	Stringfree(reset_args);
	freeval(reset_ret);

	unsetvar(my_var);
    }

    columns = old_columns;
}
#endif

extern void init_util1(void);
extern void init_expand(void);
extern void init_variables(void);
extern void init_macros(void);
extern void init_util2(void);

int main(void)
{
    init_util1();
    init_expand();
    init_variables();
    init_macros();
    init_util2();

    test_encode_ansi();
    test_string_shift_attributes();
    test_display_width_helpers();
    test_default_tflibdir();
#if WIDECHAR
    test_character_offsets();
    test_decode_ansi_utf8();
    test_unicode_wrapping();
    test_display_positions();
    test_incoming_conversion();
    test_outgoing_conversion();
    test_kb_visual_move_func();
    test_overwrite_and_insert();
    test_hwrite_column_tracking();
    test_prompt_clipping();
    test_do_kbword_func();
    test_grapheme_expr_functions();
    test_tf_utf8_incomplete_bytes();
    test_pseudo_terminal_rendering();
    test_screen_redraw_utf8();
    test_status_bar_utf8();
#endif

    if (failures) {
        fprintf(stderr, "%d test assertion(s) failed\n", failures);
        return 1;
    }
    puts("All tests passed");
    return 0;
}
