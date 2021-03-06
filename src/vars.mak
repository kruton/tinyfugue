########################################################################
#  TinyFugue - programmable mud client
#  Copyright (C) 1998, 1999, 2002, 2003, 2004, 2005, 2006-2007 Ken Keys
#
#  TinyFugue (aka "tf") is protected under the terms of the GNU
#  General Public License.  See the file "COPYING" for details.
#
#  DO NOT EDIT THIS FILE.
#  For instructions on changing configuration, see the README file in the
#  directory for your operating system.
########################################################################

# Makefile variables common to all systems.
# This file should be included or concatenated into a system Makefile.
# Predefined variables:
#   O - object file suffix (e.g., "o" or "obj")

TFVER=50b8

SOURCE = attr.c command.c dstring.c expand.c expr.c help.c history.c \
  keyboard.c macro.c main.c malloc.c output.c pattern.c process.c search.c \
  signals.c socket.c tfio.c tty.c util.c variable.c world.c

OBJS = $(subst .c,.$O,$(SOURCE)) $(OTHER_OBJS)

