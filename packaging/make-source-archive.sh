#!/bin/sh

set -eu

usage()
{
    echo "usage: $0 [--prefix directory] [--exclude-debian] output.tar.gz" >&2
    exit 2
}

prefix=tinyfugue-5.0.8
exclude_debian=false

while [ "$#" -gt 1 ]; do
    case "$1" in
        --prefix)
            [ "$#" -ge 3 ] || usage
            prefix=$2
            shift 2
            ;;
        --exclude-debian)
            exclude_debian=true
            shift
            ;;
        *)
            usage
            ;;
    esac
done

[ "$#" -eq 1 ] || usage

output=$1
case "$prefix" in
    ""|*/*) echo "invalid archive prefix: $prefix" >&2; exit 2 ;;
esac

script_dir=$(dirname "$0")
repo=$(CDPATH='' cd "$script_dir/.." && pwd)
case "$output" in
    /*) ;;
    *) output=$PWD/$output ;;
esac

mkdir -p "$(dirname "$output")"
tmp="${output}.tmp.$$"
tmp_tar="${output}.tar.tmp.$$"
trap 'rm -f "$tmp" "$tmp_tar"' EXIT HUP INT TERM

if $exclude_debian; then
    git -c "safe.directory=$repo" -C "$repo" archive \
        --format=tar --prefix="${prefix}/" \
        --output="$tmp_tar" HEAD -- . ':(exclude)debian'
else
    git -c "safe.directory=$repo" -C "$repo" archive \
        --format=tar --prefix="${prefix}/" \
        --output="$tmp_tar" HEAD
fi

# Determine the fork version to write
if git -c "safe.directory=$repo" -C "$repo" describe --tags --exact-match --match "v*" >/dev/null 2>&1; then
    GIT_TAG=$(git -c "safe.directory=$repo" -C "$repo" describe --tags --exact-match --match "v*")
    FORK_VERSION="kruton-${GIT_TAG#v}"
else
    GIT_COMMIT_HASH=$(git -c "safe.directory=$repo" -C "$repo" rev-parse --short HEAD)
    FORK_VERSION="kruton-dev-g${GIT_COMMIT_HASH}"
fi

# Append fork_version.txt to the tarball under the prefix directory
mkdir -p "${prefix}"
echo "${FORK_VERSION}" > "${prefix}/fork_version.txt"
tar -rf "$tmp_tar" "${prefix}/fork_version.txt"
rm -rf "${prefix}"

gzip -n <"$tmp_tar" >"$tmp"
mv "$tmp" "$output"
rm -f "$tmp_tar"
trap - EXIT HUP INT TERM

if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$output"
else
    shasum -a 256 "$output"
fi
