# SPDX-License-Identifier: LGPL-2.1-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2024 Erez Geva
#
# RPM specification file for libchrony
###############################################################################
Name:           libchrony
Version:        0.1.0
Release:        1%{?dist}
URL:            https:://gitlab.com/%{bname}/%{name}
BuildRequires:  autoconf m4 automake libtool
Source0:        https://gitlab.com/%%{bname}/%%{name}/-/archive/main/%%{name}-main.tar.bz2
License:        LGPL-2.1-or-later
Summary:        A C library for monitoring chronyd
%description
communicates with chronyd directly over Unix domain or UDP socket.

%define bname   chrony

Provides:       %{name}-static = %{version}-%{release}

%package        devel
Summary:        Development files for libchrony Library
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    devel
communicates with chronyd directly over Unix domain or UDP socket.

%prep
%setup -q

%build
%make_build libdir=%{_libdir} includedir=%{_includedir}

%install
%make_install libdir=%{_libdir} includedir=%{_includedir}

%files
%{_libdir}/%{name}.so.*

%files devel
%{_includedir}/*
%{_libdir}/%{name}.a
%{_libdir}/%{name}.so

%changelog

* Mon Dec 23 2024 ErezGeva2@gmail.com 0.1.0-1
- First version.
