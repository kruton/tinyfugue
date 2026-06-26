/*************************************************************************
 *  TinyFugue - programmable mud client
 *
 *  Unicode conversion and display-width helpers.
 ************************************************************************/

#ifndef UNICODE_H
#define UNICODE_H

extern void init_unicode(void);
extern int tf_character_offset(const char *str, int len, int position,
    int count);
extern int tf_grapheme_width(const char *str, int len, int start, int column,
    int tab_width, int *end);
extern int tf_string_width(const char *str, int len, int start_column,
			   int tab_width);
extern int tf_column_to_byte_offset(const char *str, int len,
				    int target_column, int tab_width);
extern int tf_bytes_for_width(const char *str, int len, int start_byte,
			      int start_column, int max_width, int tab_width);
extern void tf_display_position(const char *str, int len, int position,
    int start_column, int wrap_width, int tab_width, int *row, int *column);
extern int tf_display_row_offset(const char *str, int len, int target_row,
    int start_column, int wrap_width, int tab_width);
extern int tf_utf8_incomplete_bytes(const char *str, int len);
extern const char *tf_unicode_version(void);

#if WIDECHAR
typedef struct TfConverter TfConverter;

extern TfConverter *tf_converter_open(const char *charset);
extern void tf_converter_close(TfConverter *converter);
extern int tf_converter_is_valid_charset(const char *charset);
extern int tf_to_utf8(String *output, String *input, TfConverter *converter,
    int flush);
extern int tf_from_utf8(String *output, const char *input, int input_len,
    TfConverter *converter);
extern int tf_utf8_wraplen(const char *str, int len, int max_columns,
    int tab_width);
extern int tf_utf8_decode(const char *str, int len, int *index,
    int *codepoint);
extern int tf_utf8_encode(int codepoint, char *dst);
extern int tf_utf8_prev_offset(const char *str, int start, int offset);
extern int tf_unicode_isalnum(int codepoint);
extern int tf_unicode_tolower(int codepoint);
extern int tf_unicode_toupper(int codepoint);
#endif

#endif /* UNICODE_H */
