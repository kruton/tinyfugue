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
#include <unicode/ubrk.h>
#include <unicode/utext.h>
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

byte_fallback:
#endif
    position += count;
    if (position < 0)
        return 0;
    if (position > len)
        return len;
    return position;
}

#if WIDECHAR

#include <stdlib.h>
#include <unicode/uchar.h>
#include <unicode/ustring.h>
#include <unicode/utf8.h>

#define CONVERT_BUFFER_SIZE 4096

static int codepoint_width(UChar32 c)
{
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
}

static int cluster_width(const char *str, int start, int end)
{
    int index = start;
    int width = 0;

    while (index < end) {
        UChar32 c;
        int current;

        U8_NEXT(str, index, end, c);
        if (c < 0)
            return 1;
        current = codepoint_width(c);
        if (current > width)
            width = current;
    }
    return width;
}

int tf_utf8_wraplen(const char *str, int len, int max_columns, int tab_width)
{
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
}

int tf_to_utf8(String *output, String *input, UConverter *converter,
    int flush, UErrorCode *error)
{
    const char *source = input->data;
    const char *source_limit = input->data + input->len;
    int consumed;

    *error = U_ZERO_ERROR;
    do {
        UChar utf16[CONVERT_BUFFER_SIZE];
        UChar *target = utf16;
        const UChar *utf16_limit;
        char utf8[CONVERT_BUFFER_SIZE * 4];
        int32_t utf8_len = 0;
        UErrorCode utf8_error = U_ZERO_ERROR;

        ucnv_toUnicode(converter, &target, utf16 + CONVERT_BUFFER_SIZE,
            &source, source_limit, NULL, flush, error);
        utf16_limit = target;
        u_strToUTF8(utf8, sizeof(utf8), &utf8_len, utf16,
            (int32_t)(utf16_limit - utf16), &utf8_error);
        if (U_FAILURE(utf8_error)) {
            *error = utf8_error;
            return -1;
        }
        Stringfncat(output, utf8, utf8_len);
    } while (*error == U_BUFFER_OVERFLOW_ERROR && (*error = U_ZERO_ERROR, 1));

    consumed = (int)(source - input->data);
    if (consumed > 0)
        Stringshift(input, consumed);
    return U_FAILURE(*error) ? -1 : consumed;
}

int tf_from_utf8(String *output, const char *input, int input_len,
    UConverter *converter, UErrorCode *error)
{
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
        *error = U_MEMORY_ALLOCATION_ERROR;
        return -1;
    }

    *error = U_ZERO_ERROR;
    utf8_converter = ucnv_open("UTF-8", error);
    if (U_SUCCESS(*error)) {
        ucnv_setToUCallBack(utf8_converter, UCNV_TO_U_CALLBACK_ESCAPE,
            UCNV_ESCAPE_XML_HEX, NULL, NULL, error);
    }
    if (U_FAILURE(*error)) {
        if (utf8_converter)
            ucnv_close(utf8_converter);
        free(utf16);
        free(encoded);
        return -1;
    }

    utf16_len = ucnv_toUChars(utf8_converter, utf16, utf16_capacity,
        input, input_len, error);
    ucnv_close(utf8_converter);
    if (U_FAILURE(*error)) {
        free(utf16);
        free(encoded);
        return -1;
    }

    *error = U_ZERO_ERROR;
    ucnv_resetFromUnicode(converter);
    encoded_len = ucnv_fromUChars(converter, encoded, encoded_capacity,
        utf16, utf16_len, error);
    if (U_SUCCESS(*error))
        Stringfncat(output, encoded, encoded_len);

    free(utf16);
    free(encoded);
    return U_FAILURE(*error) ? -1 : encoded_len;
}

#endif /* WIDECHAR */
