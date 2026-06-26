/*************************************************************************
 *  TinyFugue - programmable mud client
 *  Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2002, 2003, 2004, 2005, 2006-2007 Ken Keys
 *
 *  TinyFugue (aka "tf") is protected under the terms of the GNU
 *  General Public License.  See the file "COPYING" for details.
 ************************************************************************/

#ifndef WASM_PLATFORM_H
#define WASM_PLATFORM_H

extern void init_wasm_platform(void);
extern void run_wasm_startup_script(void);
extern int tf_wasm_resize(int new_columns, int new_lines);

#endif /* WASM_PLATFORM_H */
