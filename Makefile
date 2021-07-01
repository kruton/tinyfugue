########################################################################
#
#  TinyFugue - programmable mud client
#  Copyright (C) 1993-2007 Ken Keys (kenkeys@users.sourceforge.net)
#
#  TinyFugue (aka "tf") is protected under the terms of the GNU
#  General Public License.  See the file "COPYING" for details.
#
########################################################################

# Note: the space on the end of the next line is intentional, so it will
# still work in unix for idiots who got ^M on the end of every line.
default:  all 

all:
	@echo :
	@echo : To build TinyFugue, use one of the following commands
	@echo : appropriate to your system.  For more information, see
	@echo : the README file in the directory for your system.
	@echo :
	@echo : UNIX, CYGWIN, MACOS X:
	@echo : ./configure [options]
	@echo : make install
	@echo :
	@echo : OS/2:
	@echo : os2make [options]
	@echo :

install files tf clean uninstall: all

# The next line is a hack to get around a bug in BSD/386 make.
make:

