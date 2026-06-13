########################################################################
#
#  TinyFugue - programmable mud client
#  Copyright (C) 1993-2007 Ken Keys (kenkeys@users.sourceforge.net)
#
#  TinyFugue (aka "tf") is protected under the terms of the GNU
#  General Public License.  See the file "LICENSE" for details.
#
########################################################################

CC=gcc
CFLAGS=-O2 -I../src

# CMake is the authoritative build. This Makefile remains as a convenience
# for regenerating a help file directly in this directory.

all: tf-help
	echo Building html help files available in the help/ directory

tf-help: html2tf commands/*.html topics/*.html
	./html2tf commands/*.html > tf-help || { rm tf-help; exit 1; }
	./html2tf topics/*.html >> tf-help || { rm tf-help; exit 1; }

#debug:
#	@# Test for missing quotes
#	egrep '<a href="[^"]*>' */*.html && false || true

clean:
	rm -f html2tf tf-help

# The next line is a hack to get around a bug in BSD/386 make.
make:
