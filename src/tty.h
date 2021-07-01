/*************************************************************************
 *  TinyFugue - programmable mud client
 *  Copyright (C) 1993-2007 Ken Keys (kenkeys@users.sourceforge.net)
 *
 *  TinyFugue (aka "tf") is protected under the terms of the GNU
 *  General Public License.  See the file "COPYING" for details.
 ************************************************************************/

#ifndef TTY_H
#define TTY_H

extern void init_tty(void);
extern void cbreak_noecho_mode(void);
extern void reset_tty(void);
extern int  get_window_size(void);

extern int no_tty;

#endif /* TTY_H */
