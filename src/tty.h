/*************************************************************************
 *  TinyFugue - programmable mud client
 *  Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2002, 2003, 2004, 2005, 2006-2007 Ken Keys
 *
 *  TinyFugue (aka "tf") is protected under the terms of the GNU
 *  General Public License.  See the file "COPYING" for details.
 ************************************************************************/

#ifndef TTY_H
#define TTY_H

typedef int (*tf_tty_isatty_func_t)(int fd);
typedef void (*tf_tty_mode_func_t)(void);
typedef int (*tf_window_size_func_t)(int *columns, int *lines);

extern void init_tty(void);
extern void cbreak_noecho_mode(void);
extern void reset_tty(void);
extern int  get_window_size(void);
extern int  set_window_size(int new_columns, int new_lines);
extern int  tf_tty_get_window_size(int *new_columns, int *new_lines);
extern void tf_set_window_size_func(tf_window_size_func_t func);
extern int  tf_tty_isatty(int fd);
extern void tf_set_tty_isatty_func(tf_tty_isatty_func_t func);
extern void tf_set_tty_mode_funcs(tf_tty_mode_func_t cbreak_func,
    tf_tty_mode_func_t reset_func);

extern int no_tty;

#endif /* TTY_H */
