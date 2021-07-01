/*************************************************************************
 *  TinyFugue - programmable mud client
 *  Copyright (C) 1993-2007 Ken Keys (kenkeys@users.sourceforge.net)
 *
 *  TinyFugue (aka "tf") is protected under the terms of the GNU
 *  General Public License.  See the file "COPYING" for details.
 ************************************************************************/


/**************************************************************
 * Fugue help index builder
 *
 * Rewritten by Ken Keys to allow topic aliasing and subtopics;
 * be self-contained; and build index at install time.
 **************************************************************/

#include <stdio.h>
#include "help.h"

int main(int argc, char **argv)
{
    char line[HELPLEN];
    long offset = 0;

    while (fgets(line, sizeof(line), stdin) != NULL) {
        if ((line[0] == '&' || line[0] == '#') && line[1])
            printf("%ld%s", offset, line);
        offset = ftell(stdin);
    }
    return 0;
}
