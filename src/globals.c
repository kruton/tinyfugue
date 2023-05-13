/*************************************************************************
 *  TinyFugue - programmable mud client
 *  Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2002, 2003, 2004, 2005, 2006-2007 Ken Keys
 *
 *  TinyFugue (aka "tf") is protected under the terms of the GNU
 *  General Public License.  See the file "COPYING" for details.
 ************************************************************************/

#include "tfconfig.h"
#include "port.h"
#include "tf.h"

const char sysname[] = UNAME;

/* For customized versions, please add a unique identifer (e.g., your initials)
 * to the version number, and put a brief description of the modifications
 * in the mods[] string.
 */
const char version[] =
#if DEVELOPMENT
    "DEVELOPMENT VERSION: "
#endif
    "TinyFugue version 5.0 beta 8";

const char mods[] = "";

const char copyright[] =
    "Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2002, 2003, 2004, 2005, 2006-2007 Ken Keys (kenkeys@users.sourceforge.net)";

const char contrib[] =
    "";

int restriction = 0;
int debug = 0;