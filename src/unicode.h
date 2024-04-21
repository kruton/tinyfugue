/*************************************************************************
 *  TinyFugue - programmable mud client
 *
 *  Unicode conversion and display-width helpers.
 ************************************************************************/

#ifndef UNICODE_H
#define UNICODE_H

extern int tf_character_offset(const char *str, int len, int position,
    int count);
extern int tf_grapheme_width(const char *str, int len, int start, int column,
    int tab_width, int *end);
extern void tf_display_position(const char *str, int len, int position,
    int start_column, int wrap_width, int tab_width, int *row, int *column);
extern int tf_display_row_offset(const char *str, int len, int target_row,
    int start_column, int wrap_width, int tab_width);
extern int tf_utf8_incomplete_bytes(const char *str, int len);

#if WIDECHAR
#include <unicode/ucnv.h>

extern int tf_to_utf8(String *output, String *input, UConverter *converter,
    int flush, UErrorCode *error);
extern int tf_from_utf8(String *output, const char *input, int input_len,
    UConverter *converter, UErrorCode *error);
extern int tf_utf8_wraplen(const char *str, int len, int max_columns,
    int tab_width);
#endif

#endif /* UNICODE_H */
