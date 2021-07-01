/*************************************************************************
 *  TinyFugue - programmable mud client
 *  Copyright (C) 1993-2007 Ken Keys (kenkeys@users.sourceforge.net)
 *
 *  TinyFugue (aka "tf") is protected under the terms of the GNU
 *  General Public License.  See the file "COPYING" for details.
 ************************************************************************/

#ifndef EXPR_H
#define EXPR_H

extern int    expr(Program *prog);

#if USE_DMALLOC
extern void   free_expr(void);
#endif

#endif /* EXPR_H */
