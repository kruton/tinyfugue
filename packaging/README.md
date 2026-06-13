# TinyFugue packaging

This directory contains package definitions for the primary platforms built
by CI:

| Platform | Definition |
| --- | --- |
| Debian/Ubuntu | `../debian/` |
| Fedora/RHEL | `rpm/tinyfugue.spec` |
| macOS/Homebrew | `homebrew/tinyfugue.rb` |
| FreeBSD | `freebsd/` |
| Cygwin | `cygwin/tinyfugue.cygport` |

All packages build TinyFugue with ICU, TLS, termcap/curses, and zlib support.
The package version is `5.0.8`; the initial downstream package release is
`1`.

## Source archives

Published recipes are pinned to commit
`02b048204c5ffd1704b48541724b8ecebc9aee4c`. The GitHub archive has SHA-256:

```text
02d11e70251a823f29f7faf2155d77744a91319125efd21a9d90325b7984725e
```

Use `make-source-archive.sh` to package the current checkout for local builds:

```sh
packaging/make-source-archive.sh dist/tinyfugue-5.0.8.tar.gz
packaging/make-source-archive.sh --exclude-debian \
    dist/tinyfugue_5.0.8.orig.tar.gz
packaging/make-source-archive.sh \
    --prefix tinyfugue-02b048204c5ffd1704b48541724b8ecebc9aee4c \
    dist/tinyfugue-02b048204c5ffd1704b48541724b8ecebc9aee4c.tar.gz
```

The archive is generated from tracked files using `git archive`, has a stable
top-level directory, and uses deterministic gzip metadata. Uncommitted and
untracked files are intentionally excluded.

## Native builds

Debian and Ubuntu:

```sh
dpkg-buildpackage --build=binary --no-sign
```

Fedora:

```sh
packaging/make-source-archive.sh \
    --prefix tinyfugue-02b048204c5ffd1704b48541724b8ecebc9aee4c \
    "$HOME/rpmbuild/SOURCES/02b048204c5ffd1704b48541724b8ecebc9aee4c.tar.gz"
rpmbuild -ba packaging/rpm/tinyfugue.spec
```

Homebrew:

```sh
brew install --build-from-source packaging/homebrew/tinyfugue.rb
brew test tinyfugue
```

FreeBSD:

```sh
cd packaging/freebsd
make test package
```

Cygwin:

```sh
cd packaging/cygwin
cygport tinyfugue.cygport download prep compile check install package
```

The published Homebrew, FreeBSD, and Cygwin definitions download the pinned
GitHub archive. CI replaces only the source URL and checksum in temporary
copies so the same rules can build the current checkout.

## Updating a release

1. Update the project version in `CMakeLists.txt` and every package definition.
2. Pin the recipes to the release tag, or to the release commit until tags are
   available.
3. Download the GitHub archive and update its SHA-256 and size in the Homebrew
   formula and FreeBSD `distinfo`.
4. Increment or reset the Debian, RPM, FreeBSD, and Cygwin package release
   values as appropriate.
5. Run every job in `.github/workflows/packages.yml`.
