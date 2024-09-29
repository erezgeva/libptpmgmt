# SPDX-License-Identifier: LGPL-2.1-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2024 Erez Geva
#
# RPM specification file for librtpi
###############################################################################
Name:           librtpi
Version:        1.0.0
Release:        1%{?dist}
URL:            https://gitlab.com/linux-rt/%{name}
BuildRequires:  autoconf m4 automake libtool
Source0:        https://gitlab.com/linux-rt/%%{name}/-/archive/%%{version}/%%{name}-%%{version}.tar.bz2
License:        LGPL-2.1-or-later
Summary:        The Real-Time Priority Inheritance Library
%description
realtime capable pthread locking primitives (lib)
The Real-Time Priority Inheritance Library (%{name}) is
intended to bridge the gap between the glibc pthread
implementation and a functionally correct priority inheritance
for pthread locking primitives, such as pthread_mutex and
pthread_condvar. Specifically, priority based wakeup is required
for correct operation, in  contrast to the more time-based
ordered wakeup groups in the glibc pthread condvar implementation.
This package contains the runtime library.

%define bname   rtpi

%package -n     %{name}1
Summary:        The Real-Time Priority Inheritance Library
Provides:       %{name}1-static = %{version}-%{release}
%description -n %{name}1
realtime capable pthread locking primitives (lib)
The Real-Time Priority Inheritance Library (%{name}) is
intended to bridge the gap between the glibc pthread
implementation and a functionally correct priority inheritance
for pthread locking primitives, such as pthread_mutex and
pthread_condvar. Specifically, priority based wakeup is required
for correct operation, in  contrast to the more time-based
ordered wakeup groups in the glibc pthread condvar implementation.
This package contains the runtime library.

%package        devel
Summary:        Development files for the Real-Time Priority Inheritance Library
Requires:       %{name}1%{?_isa} = %{version}-%{release}
%description    devel
realtime capable pthread locking primitives (dev)
The Real-Time Priority Inheritance Library (%{name}) is
intended to bridge the gap between the glibc pthread
implementation and a functionally correct priority inheritance
for pthread locking primitives, such as pthread_mutex and
pthread_condvar. Specifically, priority based wakeup is required
for correct operation, in  contrast to the more time-based
ordered wakeup groups in the glibc pthread condvar implementation.
This package contains the development files.

%prep
%setup -q

%build
autoreconf --install
%configure
%make_build

%install
%make_install

%files -n %{name}1
%{_libdir}/%{name}.so.*

%files devel
%{_includedir}/*
%{_libdir}/%{name}*.a
%{_libdir}/%{name}*.so

%changelog

* Sat Sep 28 2024 ErezGeva2@gmail.com 1.0.0-1
- First version.
