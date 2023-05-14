/*************************************************************************
 *  TinyFugue - programmable mud client
 *
 *  Unicode conversion and display-width helpers.
 ************************************************************************/

#ifndef UNICODE_H
#define UNICODE_H

extern int tf_character_offset(const char *str, int len, int position,
    int count);

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
