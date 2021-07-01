########################################################################
#
#  TinyFugue - programmable mud client
#  Copyright (C) 1993-2007 Ken Keys (kenkeys@users.sourceforge.net)
#
#  TinyFugue (aka "tf") is protected under the terms of the GNU
#  General Public License.  See the file "COPYING" for details.
#
########################################################################

CC=gcc
CFLAGS=-O2 -I../src

# Help files were being maintained as html and then converting, which
# is a PITA to maintain. For now, we're preventing the html2tf from
# blowing away our updated tf-help in tf-lib. In the future, there
# should be some easily maintained source to generate html and tf-help
# from. Likely whichever Markdown system allows for the most pain free
# conversion from html.

all: tf-help
	echo Building html help files available in the help/ directory
#	cp tf-help ../tf-lib

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

