%global commit 02b048204c5ffd1704b48541724b8ecebc9aee4c

Name:           tinyfugue
Version:        5.0.8
Release:        1%{?dist}
Summary:        Programmable MUD client
License:        GPL-2.0-or-later
URL:            https://github.com/kruton/tinyfugue
Source0:        %{url}/archive/%{commit}.tar.gz

BuildRequires:  cmake >= 3.20
BuildRequires:  gcc
BuildRequires:  libicu-devel
BuildRequires:  ncurses-devel
BuildRequires:  ninja-build
BuildRequires:  openssl-devel
BuildRequires:  pcre-devel
BuildRequires:  zlib-devel

%description
TinyFugue is a flexible, screen-oriented client for MUDs and other
line-based network services. It provides a macro language, triggers,
history, multiple worlds, Unicode, TLS, and MCCP support.

%prep
%autosetup -n %{name}-%{commit}

%build
%cmake -G Ninja \
    -DTF_WIDECHAR=ON \
    -DTF_TLS=ON \
    -DTF_TERMCAP=ON \
    -DTF_ZLIB=ON
%cmake_build

%check
%ctest

%install
%cmake_install

%files
%license COPYING
%doc README CHANGES CREDITS
%{_bindir}/tf
%{_mandir}/man1/tf.1*
%{_datadir}/tf-lib/

%changelog
* Sat Jun 13 2026 TinyFugue Maintainers <kruton@users.noreply.github.com> - 5.0.8-1
- Add native RPM packaging.
