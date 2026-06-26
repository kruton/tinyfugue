/*************************************************************************
 *  TinyFugue - programmable mud client
 *
 *  Unicode conversion and display-width helpers.
 ************************************************************************/

#include "tfconfig.h"
#include "port.h"
#include "tf.h"
#include "unicode.h"

#if WIDECHAR
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#if UNICODE_BACKEND_ICU
#include <unicode/udata.h>
#include <unicode/ubrk.h>
#include <unicode/uchar.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utf8.h>
#elif UNICODE_BACKEND_UTF8PROC
#include <utf8proc.h>
#endif
#endif

void init_unicode(void)
{
#if WIDECHAR && PLATFORM_WASM && UNICODE_BACKEND_ICU
    FILE *f = fopen("/icudt.dat", "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        void *data = malloc(size);
        if (data) {
            size_t read_bytes = fread(data, 1, size, f);
            (void)read_bytes;
            UErrorCode status = U_ZERO_ERROR;
            udata_setCommonData(data, &status);
            if (U_FAILURE(status)) {
                fprintf(stderr, "ICU udata_setCommonData failed: %d\n", status);
            }
        }
        fclose(f);
    } else {
        fprintf(stderr, "Failed to open /icudt.dat\n");
    }
#endif
}

const char *tf_unicode_version(void)
{
#if WIDECHAR && UNICODE_BACKEND_ICU
    return "ICU " U_ICU_VERSION;
#elif WIDECHAR && UNICODE_BACKEND_UTF8PROC
    static char buffer[128] = "";
    if (buffer[0] == '\0') {
        snprintf(buffer, sizeof(buffer), "utf8proc %s", utf8proc_version());
    }
    return buffer;
#else
    return "disabled";
#endif
}

#if WIDECHAR
struct TfConverter {
#if UNICODE_BACKEND_ICU
    UConverter *converter;
#else
    int utf8;
    char pending[4];
    int pending_len;
#endif
};

static int cluster_width(const char *str, int start, int end);

static int is_simple_ascii(const char *str, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];

        if (c >= 0x80 || c == '\r')
            return 0;
    }
    return 1;
}

#if UNICODE_BACKEND_UTF8PROC
static int utf8proc_boundaries(const char *str, int len, int **out)
{
    int *bounds;
    int count = 0;
    int offset = 0;
    int previous = -1;
    int state = 0;

    bounds = MALLOC((len + 2) * sizeof(*bounds));
    if (!bounds)
        return -1;
    bounds[count++] = 0;

    while (offset < len) {
        utf8proc_int32_t codepoint;
        utf8proc_ssize_t bytes = utf8proc_iterate(
            (const utf8proc_uint8_t *)str + offset, len - offset,
            &codepoint);

        if (bytes <= 0 || codepoint < 0) {
            FREE(bounds);
            return -1;
        }

        if (previous >= 0 &&
            utf8proc_grapheme_break_stateful(previous, codepoint, &state)) {
            bounds[count++] = offset;
            state = 0;
        }

        previous = codepoint;
        offset += (int)bytes;
    }

    if (bounds[count - 1] != len)
        bounds[count++] = len;
    *out = bounds;
    return count;
}
#endif
#endif

int tf_character_offset(const char *str, int len, int position, int count)
{
    if (!str || len <= 0)
        return 0;
    if (position < 0)
        position = 0;
    if (position > len)
        position = len;

#if WIDECHAR
    if (is_simple_ascii(str, len))
        goto byte_fallback;

#if UNICODE_BACKEND_ICU
    {
        UBreakIterator *characters;
        UText *text;
        UErrorCode error = U_ZERO_ERROR;
        int result = position;

        text = utext_openUTF8(NULL, str, len, &error);
        if (U_FAILURE(error))
            goto byte_fallback;

        characters = ubrk_open(UBRK_CHARACTER, NULL, NULL, 0, &error);
        if (U_FAILURE(error)) {
            utext_close(text);
            goto byte_fallback;
        }
        ubrk_setUText(characters, text, &error);
        if (U_FAILURE(error)) {
            ubrk_close(characters);
            utext_close(text);
            goto byte_fallback;
        }

        if (count < 0) {
            if (!ubrk_isBoundary(characters, position)) {
                result = ubrk_preceding(characters, result);
                count++;
            }
            while (count < 0 && result > 0) {
                result = ubrk_preceding(characters, result);
                count++;
            }
        } else if (count > 0) {
            if (!ubrk_isBoundary(characters, position)) {
                result = ubrk_following(characters, result);
                count--;
            }
            while (count > 0 && result < len) {
                result = ubrk_following(characters, result);
                count--;
            }
        }

        ubrk_close(characters);
        utext_close(text);
        if (result == UBRK_DONE)
            return count < 0 ? 0 : len;
        return result;
    }
#elif UNICODE_BACKEND_UTF8PROC
    {
        int *bounds;
        int nbounds = utf8proc_boundaries(str, len, &bounds);
        int i, idx = 0, exact = 0, result;

        if (nbounds < 0)
            goto byte_fallback;
        for (i = 0; i < nbounds; i++) {
            if (bounds[i] == position) {
                idx = i;
                exact = 1;
                break;
            }
            if (bounds[i] > position) {
                idx = i;
                break;
            }
        }
        if (i == nbounds)
            idx = nbounds - 1;

        if (count < 0) {
            if (!exact) {
                idx--;
                count++;
            }
            idx += count;
        } else if (count > 0) {
            if (!exact)
                count--;
            idx += count;
        }

        if (idx < 0)
            idx = 0;
        if (idx >= nbounds)
            idx = nbounds - 1;
        result = bounds[idx];
        FREE(bounds);
        return result;
    }
#endif

byte_fallback:
#endif
    position += count;
    if (position < 0)
        return 0;
    if (position > len)
        return len;
    return position;
}

int tf_grapheme_width(const char *str, int len, int start, int column,
    int tab_width, int *end)
{
    int next;

    if (!str || start >= len) {
        *end = len;
        return 0;
    }
    next = tf_character_offset(str, len, start, 1);
    if (next <= start)
        next = start + 1;
    *end = next;

    if (str[start] == '\t') {
        int effective_tab_width = tab_width > 0 ? tab_width : 1;
        return effective_tab_width - column % effective_tab_width;
    }
#if WIDECHAR
    return cluster_width(str, start, next);
#else
    return 1;
#endif
}

int tf_string_width(const char *str, int len, int start_column, int tab_width)
{
    int offset = 0;
    int column = start_column;

    if (!str || len <= 0)
	return 0;
    while (offset < len) {
	int end;

	column += tf_grapheme_width(str, len, offset, column, tab_width,
				    &end);
	offset = end;
    }
    return column - start_column;
}

int tf_column_to_byte_offset(const char *str, int len, int target_column,
			     int tab_width)
{
    int offset = 0;
    int column = 0;

    if (!str || len <= 0 || target_column <= 0)
	return 0;
    while (offset < len && column < target_column) {
	int end;

	column += tf_grapheme_width(str, len, offset, column, tab_width,
				    &end);
	offset = end;
    }
    return offset;
}

int tf_bytes_for_width(const char *str, int len, int start_byte,
		       int start_column, int max_width, int tab_width)
{
    int offset = start_byte;
    int column = start_column;
    int limit = start_column + max_width;

    if (!str || len <= 0 || start_byte < 0 || start_byte >= len ||
	max_width <= 0) {
	return 0;
    }
    while (offset < len && column < limit) {
	int end;
	int width = tf_grapheme_width(str, len, offset, column, tab_width,
				      &end);

	if (column + width > limit)
	    break;
	column += width;
	offset = end;
    }
    return offset - start_byte;
}

void tf_display_position(const char *str, int len, int position,
    int start_column, int wrap_width, int tab_width, int *row, int *column)
{
    int offset = 0;
    int current_row = 0;
    int current_column = start_column;

    if (position > len)
        position = len;
    while (offset < position) {
        int end;
        int width = tf_grapheme_width(str, len, offset, current_column,
            tab_width, &end);

        if (current_column > 0 &&
            current_column + width > wrap_width)
        {
            current_row++;
            current_column = 0;
            width = tf_grapheme_width(str, len, offset, current_column,
                tab_width, &end);
        }
        current_column += width;
        offset = end;
        if (current_column >= wrap_width) {
            current_row++;
            current_column = 0;
        }
    }
    *row = current_row;
    *column = current_column;
}

int tf_display_row_offset(const char *str, int len, int target_row,
    int start_column, int wrap_width, int tab_width)
{
    int offset = 0;
    int row = 0;
    int column = start_column;

    if (target_row <= 0)
        return 0;
    while (offset < len) {
        int end;
        int width = tf_grapheme_width(str, len, offset, column,
            tab_width, &end);

        if (column > 0 && column + width > wrap_width) {
            row++;
            column = 0;
            if (row >= target_row)
                return offset;
            width = tf_grapheme_width(str, len, offset, column,
                tab_width, &end);
        }
        column += width;
        offset = end;
        if (column >= wrap_width) {
            row++;
            column = 0;
            if (row >= target_row)
                return offset;
        }
    }
    return len;
}

#if WIDECHAR

#define CONVERT_BUFFER_SIZE 4096

int tf_utf8_decode(const char *str, int len, int *index, int *codepoint)
{
#if UNICODE_BACKEND_ICU
    UChar32 c;
    int oldindex = *index;

    U8_NEXT(str, *index, len, c);
    if (c < 0) {
        *index = oldindex + 1;
        *codepoint = -1;
        return 0;
    }
    *codepoint = c;
    return 1;
#else
    utf8proc_int32_t c;
    utf8proc_ssize_t bytes;

    bytes = utf8proc_iterate((const utf8proc_uint8_t *)str + *index,
        len - *index, &c);
    if (bytes <= 0 || c < 0) {
        (*index)++;
        *codepoint = -1;
        return 0;
    }
    *index += (int)bytes;
    *codepoint = c;
    return 1;
#endif
}

int tf_utf8_encode(int codepoint, char *dst)
{
#if UNICODE_BACKEND_ICU
    int len = 0;

    U8_APPEND_UNSAFE(dst, len, codepoint);
    return len;
#else
    return (int)utf8proc_encode_char(codepoint, (utf8proc_uint8_t *)dst);
#endif
}

int tf_utf8_prev_offset(const char *str, int start, int offset)
{
#if UNICODE_BACKEND_ICU
    U8_BACK_1((const uint8_t *)str, start, offset);
    return offset;
#else
    int previous = start;
    int index = start;

    while (index < offset) {
        int codepoint;
        previous = index;
        if (!tf_utf8_decode(str, offset, &index, &codepoint))
            break;
    }
    return previous;
#endif
}

int tf_unicode_isalnum(int codepoint)
{
    if (codepoint < 0)
        return 0;
#if UNICODE_BACKEND_ICU
    return u_isalnum((UChar32)codepoint);
#else
    utf8proc_category_t category = utf8proc_category(codepoint);

    return category == UTF8PROC_CATEGORY_LU ||
        category == UTF8PROC_CATEGORY_LL ||
        category == UTF8PROC_CATEGORY_LT ||
        category == UTF8PROC_CATEGORY_LM ||
        category == UTF8PROC_CATEGORY_LO ||
        category == UTF8PROC_CATEGORY_ND ||
        category == UTF8PROC_CATEGORY_NL ||
        category == UTF8PROC_CATEGORY_NO;
#endif
}

int tf_unicode_tolower(int codepoint)
{
#if UNICODE_BACKEND_ICU
    return u_tolower((UChar32)codepoint);
#else
    return utf8proc_tolower(codepoint);
#endif
}

int tf_unicode_toupper(int codepoint)
{
#if UNICODE_BACKEND_ICU
    return u_toupper((UChar32)codepoint);
#else
    return utf8proc_toupper(codepoint);
#endif
}

static int codepoint_width(int c)
{
#if UNICODE_BACKEND_ICU
    int category = u_charType(c);
    int east_asian_width;

    if (category == U_NON_SPACING_MARK ||
        category == U_ENCLOSING_MARK ||
        category == U_FORMAT_CHAR)
    {
        return 0;
    }

    east_asian_width = u_getIntPropertyValue(c, UCHAR_EAST_ASIAN_WIDTH);
    if (east_asian_width == U_EA_WIDE ||
        east_asian_width == U_EA_FULLWIDTH)
    {
        return 2;
    }
    return 1;
#else
    utf8proc_category_t category = utf8proc_category(c);
    int width;

    if (category == UTF8PROC_CATEGORY_MN ||
        category == UTF8PROC_CATEGORY_ME ||
        category == UTF8PROC_CATEGORY_CF)
    {
        return 0;
    }
    width = utf8proc_charwidth(c);
    return width > 0 ? width : 0;
#endif
}

/*
 * Policy for East Asian Ambiguous Width:
 * Ambiguous characters (like Greek, Cyrillic, line drawings, and some symbols)
 * are treated as narrow (width 1) by default to match Western terminal behavior.
 * Variation Selector 16 (VS16, U+FE0F) or enclosing mark sequence overrides this
 * to width 2 for emoji presentation.
 */
static int cluster_width(const char *str, int start, int end)
{
    int index = start;
    int width = 0;
    int has_emoji_selector = 0;
    int has_text_selector = 0;
    int has_keycap = 0;
    int has_zwj = 0;
    int has_emoji = 0;

    while (index < end) {
        int c;
        int current;

        if (!tf_utf8_decode(str, end, &index, &c))
            return 1;

        if (c == 0xFE0F) {
            has_emoji_selector = 1;
        } else if (c == 0xFE0E) {
            has_text_selector = 1;
        } else if (c == 0x20E3) {
            has_keycap = 1;
        } else if (c == 0x200D) {
            has_zwj = 1;
        }

#if UNICODE_BACKEND_ICU
        if (u_hasBinaryProperty(c, UCHAR_EMOJI)) {
            has_emoji = 1;
        }
#else
        if (utf8proc_get_property(c)->boundclass ==
            UTF8PROC_BOUNDCLASS_EXTENDED_PICTOGRAPHIC ||
            utf8proc_charwidth(c) == 2) {
            has_emoji = 1;
        }
#endif

        current = codepoint_width(c);
        if (current > width)
            width = current;
    }

    if (has_keycap || has_zwj || (has_emoji && has_emoji_selector)) {
        return 2;
    }
    if (has_text_selector && !has_keycap && !has_zwj) {
        return 1;
    }

    return width;
}


int tf_utf8_wraplen(const char *str, int len, int max_columns, int tab_width)
{
#if UNICODE_BACKEND_ICU
    UBreakIterator *characters = NULL;
    UBreakIterator *lines = NULL;
    UText *text = NULL;
    UErrorCode error = U_ZERO_ERROR;
    int start, end, fit_end = 0, columns = 0, line_end = 0;
    int forced_grapheme = 0;

    if (len <= 0)
        return 0;

    text = utext_openUTF8(NULL, str, len, &error);
    if (U_FAILURE(error))
        return len;

    characters = ubrk_open(UBRK_CHARACTER, NULL, NULL, 0, &error);
    if (U_FAILURE(error))
        goto fail;
    ubrk_setUText(characters, text, &error);
    if (U_FAILURE(error))
        goto fail;

    start = ubrk_first(characters);
    while ((end = ubrk_next(characters)) != UBRK_DONE) {
        int width;

        if (str[start] == '\t') {
            int effective_tab_width = tab_width > 0 ? tab_width : 1;
            width = effective_tab_width - columns % effective_tab_width;
        } else {
            width = cluster_width(str, start, end);
        }
        if (columns + width > max_columns) {
            if (fit_end == 0) {
                fit_end = end;
                forced_grapheme = 1;
            }
            break;
        }
        columns += width;
        fit_end = end;
        start = end;
    }

    if (fit_end == len) {
        ubrk_close(characters);
        utext_close(text);
        return len;
    }

    if (forced_grapheme) {
        ubrk_close(characters);
        utext_close(text);
        return fit_end;
    }

    error = U_ZERO_ERROR;
    lines = ubrk_open(UBRK_LINE, NULL, NULL, 0, &error);
    if (U_FAILURE(error))
        goto done;
    ubrk_setUText(lines, text, &error);
    if (U_FAILURE(error))
        goto done;

    for (end = ubrk_first(lines);
         end != UBRK_DONE && end <= fit_end;
         end = ubrk_next(lines))
    {
        if (end > 0)
            line_end = end;
    }

done:
    if (lines)
        ubrk_close(lines);
    ubrk_close(characters);
    utext_close(text);
    return line_end ? line_end : fit_end;

fail:
    if (characters)
        ubrk_close(characters);
    utext_close(text);
    return len;
#else
    int start = 0, end, fit_end = 0, columns = 0, line_end = 0;
    int forced_grapheme = 0;

    if (len <= 0)
        return 0;

    while (start < len) {
        int width;

        end = tf_character_offset(str, len, start, 1);
        if (end <= start)
            end = start + 1;
        if (str[start] == '\t') {
            int effective_tab_width = tab_width > 0 ? tab_width : 1;
            width = effective_tab_width - columns % effective_tab_width;
        } else {
            width = cluster_width(str, start, end);
        }
        if (columns + width > max_columns) {
            if (fit_end == 0) {
                fit_end = end;
                forced_grapheme = 1;
            }
            break;
        }
        columns += width;
        fit_end = end;
        if (str[start] == ' ' || str[start] == '\t')
            line_end = end;
        start = end;
    }

    if (fit_end == len || forced_grapheme)
        return fit_end;
    return line_end ? line_end : fit_end;
#endif
}

TfConverter *tf_converter_open(const char *charset)
{
    TfConverter *converter;

    if (!charset || !*charset)
        charset = "UTF-8";

    converter = MALLOC(sizeof(*converter));
    if (!converter)
        return NULL;
#if UNICODE_BACKEND_ICU
    {
        UErrorCode error = U_ZERO_ERROR;
        converter->converter = ucnv_open(charset, &error);
        if (U_FAILURE(error)) {
            FREE(converter);
            return NULL;
        }
    }
#else
    if (cstrcmp(charset, "UTF-8") != 0 && cstrcmp(charset, "UTF8") != 0) {
        FREE(converter);
        return NULL;
    }
    converter->utf8 = 1;
    converter->pending_len = 0;
#endif
    return converter;
}

void tf_converter_close(TfConverter *converter)
{
    if (!converter)
        return;
#if UNICODE_BACKEND_ICU
    ucnv_close(converter->converter);
#endif
    FREE(converter);
}

int tf_converter_is_valid_charset(const char *charset)
{
    TfConverter *converter = tf_converter_open(charset);

    if (!converter)
        return 0;
    tf_converter_close(converter);
    return 1;
}

int tf_to_utf8(String *output, String *input, TfConverter *converter,
    int flush)
{
#if UNICODE_BACKEND_ICU
    const char *source = input->data;
    const char *source_limit = input->data + input->len;
    UErrorCode error;
    int consumed;

    error = U_ZERO_ERROR;
    do {
        UChar utf16[CONVERT_BUFFER_SIZE];
        UChar *target = utf16;
        const UChar *utf16_limit;
        char utf8[CONVERT_BUFFER_SIZE * 4];
        int32_t utf8_len = 0;
        UErrorCode utf8_error = U_ZERO_ERROR;

        ucnv_toUnicode(converter->converter, &target,
            utf16 + CONVERT_BUFFER_SIZE, &source, source_limit, NULL, flush,
            &error);
        utf16_limit = target;
        u_strToUTF8(utf8, sizeof(utf8), &utf8_len, utf16,
            (int32_t)(utf16_limit - utf16), &utf8_error);
        if (U_FAILURE(utf8_error)) {
            return -1;
        }
        Stringfncat(output, utf8, utf8_len);
    } while (error == U_BUFFER_OVERFLOW_ERROR && (error = U_ZERO_ERROR, 1));

    consumed = (int)(source - input->data);
    if (consumed > 0)
        Stringshift(input, consumed);
    return U_FAILURE(error) ? -1 : consumed;
#else
    int offset = 0;
    int consumed = 0;
    int copy_start = 0;

    if (converter->pending_len > 0) {
        char pending[8];
        int pending_len = converter->pending_len;
        int needed = 4 - pending_len;
        int use = input->len < needed ? input->len : needed;
        int idx = 0;
        int codepoint;

        memcpy(pending, converter->pending, pending_len);
        if (use > 0)
            memcpy(pending + pending_len, input->data, use);
        pending_len += use;

        if (tf_utf8_decode(pending, pending_len, &idx, &codepoint)) {
            char buf[8];
            int out_len = tf_utf8_encode(codepoint, buf);
            Stringncat(output, buf, out_len);
            converter->pending_len = 0;
            consumed += use;
            offset = use;
            copy_start = use;
        } else if (flush) {
            Stringcat(output, "\xef\xbf\xbd");
            Stringshift(input, use);
            converter->pending_len = 0;
            consumed += use;
            return consumed;
        } else {
            if (use > 0) {
                memcpy(converter->pending + converter->pending_len,
                    input->data, use);
                converter->pending_len += use;
                Stringshift(input, use);
                consumed += use;
            }
            return consumed;
        }
    }

    while (offset < input->len) {
        int old_offset = offset;
        int codepoint;

        if (tf_utf8_decode(input->data, input->len, &offset, &codepoint)) {
            consumed = offset;
            continue;
        }
        if (!flush && old_offset + tf_utf8_incomplete_bytes(input->data,
            input->len) == input->len) {
            converter->pending_len = input->len - old_offset;
            memcpy(converter->pending, input->data + old_offset,
                converter->pending_len);
            consumed += input->len - old_offset;
            if (old_offset > copy_start)
                Stringfncat(output, input->data + copy_start,
                    old_offset - copy_start);
            Stringshift(input, input->len);
            return consumed;
        }
        if (old_offset > copy_start)
            Stringfncat(output, input->data + copy_start,
                old_offset - copy_start);
        Stringcat(output, "\xef\xbf\xbd");
        if (old_offset + tf_utf8_incomplete_bytes(input->data,
            input->len) == input->len) {
            offset = input->len;
        } else {
            offset = old_offset + 1;
        }
        copy_start = offset;
        consumed = offset;
    }

    if (consumed > 0) {
        if (consumed > copy_start)
            Stringfncat(output, input->data + copy_start,
                consumed - copy_start);
        Stringshift(input, consumed);
    }
    return consumed;
#endif
}

int tf_from_utf8(String *output, const char *input, int input_len,
    TfConverter *converter)
{
#if UNICODE_BACKEND_ICU
    UConverter *utf8_converter;
    UChar *utf16;
    char *encoded;
    int32_t utf16_capacity;
    int32_t utf16_len;
    int32_t encoded_capacity;
    int32_t encoded_len;

    utf16_capacity = input_len * 2 + 8;
    encoded_capacity = input_len * 12 + 8;
    utf16 = malloc(sizeof(UChar) * utf16_capacity);
    encoded = malloc(encoded_capacity);
    if (!utf16 || !encoded) {
        free(utf16);
        free(encoded);
        return -1;
    }

    {
        UErrorCode error = U_ZERO_ERROR;
        utf8_converter = ucnv_open("UTF-8", &error);
        if (U_SUCCESS(error)) {
            ucnv_setToUCallBack(utf8_converter, UCNV_TO_U_CALLBACK_ESCAPE,
                UCNV_ESCAPE_XML_HEX, NULL, NULL, &error);
        }
        if (U_FAILURE(error)) {
            if (utf8_converter)
                ucnv_close(utf8_converter);
            free(utf16);
            free(encoded);
            return -1;
        }

        utf16_len = ucnv_toUChars(utf8_converter, utf16, utf16_capacity,
            input, input_len, &error);
        ucnv_close(utf8_converter);
        if (U_FAILURE(error)) {
            free(utf16);
            free(encoded);
            return -1;
        }

        error = U_ZERO_ERROR;
        ucnv_resetFromUnicode(converter->converter);
        encoded_len = ucnv_fromUChars(converter->converter, encoded,
            encoded_capacity, utf16, utf16_len, &error);
        if (U_SUCCESS(error))
            Stringfncat(output, encoded, encoded_len);

        free(utf16);
        free(encoded);
        return U_FAILURE(error) ? -1 : encoded_len;
    }
#else
    int offset = 0;
    (void)converter;

    while (offset < input_len) {
        int codepoint;
        if (!tf_utf8_decode(input, input_len, &offset, &codepoint))
            return -1;
    }
    Stringfncat(output, input, input_len);
    return input_len;
#endif
}

#endif /* WIDECHAR */

int tf_utf8_incomplete_bytes(const char *str, int len)
{
    int i;
    if (len <= 0 || !str) return 0;

    for (i = 1; i <= 4 && i <= len; i++) {
        unsigned char c = (unsigned char)str[len - i];

        if (c >= 0xC0) {
            int expected_len = 0;
            if (c >= 0xC0 && c <= 0xDF) expected_len = 2;
            else if (c >= 0xE0 && c <= 0xEF) expected_len = 3;
            else if (c >= 0xF0 && c <= 0xF7) expected_len = 4;

            if (i < expected_len) {
                return i;
            } else {
                return 0;
            }
        }

        if (c >= 0x80 && c <= 0xBF) {
            continue;
        }

        break;
    }
    return 0;
}
