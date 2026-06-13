#!/bin/sh

set -eu

root=${1:-/}
prefix=${2-/usr}
base="${root%/}${prefix}"
libdir="$base/share/tf-lib"

require_file()
{
    if [ ! -f "$1" ]; then
        echo "missing installed file: $1" >&2
        exit 1
    fi
}

require_link()
{
    if [ ! -L "$1" ]; then
        echo "missing installed symlink: $1" >&2
        exit 1
    fi
}

executable="$base/bin/tf"
if [ ! -f "$executable" ] && [ -f "${executable}.exe" ]; then
    executable="${executable}.exe"
fi
require_file "$executable"
require_file "$libdir/tf-help"
require_file "$libdir/tf-help.idx"
require_file "$libdir/stdlib.tf"
require_file "$libdir/CHANGES"

manpage=
for suffix in "" .gz .bz2 .xz .zst
do
    candidate="$base/share/man/man1/tf.1$suffix"
    if [ -f "$candidate" ] || [ -L "$candidate" ]; then
        manpage=$candidate
        break
    fi
done
if [ -z "$manpage" ]; then
    echo "missing installed man page" >&2
    exit 1
fi

for link in bind-bash.tf bind-emacs.tf completion.tf factorial.tf \
    file-xfer.tf local.tf.sample pref-shell.tf space_page.tf speedwalk.tf \
    stack_queue.tf worldqueue.tf
do
    require_link "$libdir/$link"
done

set +e
output=$("$executable" -? 2>&1)
status=$?
set -e
if [ "$status" -ne 1 ] || ! printf '%s\n' "$output" | grep -q "Usage:"; then
    echo "installed executable failed its usage check" >&2
    exit 1
fi
