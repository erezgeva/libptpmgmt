# SPDX-License-Identifier: GPL-3.0-or-later
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
# RPM specification file for libpmc rpm packages
###############################################################################
Name:           libpmc
Version:        0.5
Release:        1%{?dist}
Summary:        %{bname} library, to communicate with ptp4l
License:        LGPLv3+, GPLv3+, GFDLv1.3+
URL:            https://%{name}.sf.net
BuildRequires:  swig
BuildRequires:  perl perl-devel perl-ExtUtils-Embed
BuildRequires:  which
BuildRequires:  python2 python2-devel
BuildRequires:  python3 python3-devel
BuildRequires:  lua lua-devel
BuildRequires:  ruby ruby-devel
BuildRequires:  php php-devel
BuildRequires:  tcl tcl-devel
BuildRequires:  doxygen graphviz
Source0:        %{name}-%{version}.txz

%define bname   pmc

%description
%{bname} library, to communicate with ptp4l

%package        devel
Summary:        Development files for %{name}
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        doc
Summary:        Documentation files for %{name}
License:        GFDLv1.3+
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    doc
%{bname} library documentation, to communicate with ptp4l

%package        perl
Summary:        %{bname} library Perl wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       perl
%description    perl
%{bname} library Perl wrapper

%package -n     python2-%{bname}
summary:        %{bname} library python version 2 wrapper
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       python2
%description -n python2-%{bname}
%{bname} library python version 2 wrapper

%package -n     python3-%{bname}
Summary:        %{bname} library python version 3 wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       python3
%description -n python3-%{bname}
%{bname} library python version 3 wrapper

%package -n     lua-%{bname}
Summary:        %{bname} library Lua wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       lua
%description -n lua-%{bname}
%{bname} library Lua wrapper

%package -n     ruby-%{bname}
Summary:        %{bname} library ruby wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       ruby
%description -n ruby-%{bname}
%{bname} library ruby wrapper

%package -n     php-%{bname}
Summary:        %{bname} library php wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       php
%description -n php-%{bname}
%{bname} library php wrapper

%package -n     tcl-%{bname}
Summary:        %{bname} library tcl wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       tcl
%description -n tcl-%{bname}
%{bname} library tcl wrapper

%package -n     %{bname}
Summary:        %{bname} tool
License:        GPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description -n %{bname}
 new rewrite of linuxptp %{bname} tool using the %{name} library.
 This tool is faster than the original linuxptp tool.

%prep
%setup -q

%build
%make_build LD_SONAME=1 CPPFLAGS_OPT=-Ofast --no-print-directory

%install
%make_install LIBDIR=%{_libdir} DEV_PKG=%{name}-devel\
  PY_LIBDIR=%{_libdir}/python --no-print-directory

%files
%{_libdir}/%{name}.so

%files devel
%{_includedir}/*
%{_libdir}/%{name}.a
%{_datadir}/%{name}-devel/*.mk

%files doc
%{_datadir}/doc/%{name}-doc/*

%files perl
%{_prefix}/local/lib*/perl*/*/PmcLib.pm
%{_prefix}/local/lib*/perl*/*/auto/PmcLib/PmcLib.so

%files -n python2-%{bname}
%{_libdir}/python2*/*/_%{bname}*.so
%{_libdir}/python2*/*/%{bname}.py*

%files -n python3-%{bname}
%{_libdir}/python3*/*/_%{bname}.cpython-*.so
%{_libdir}/python3*/*/%{bname}.py
%{_libdir}/python3*/*/*/%{bname}.*.pyc

%files -n lua-%{bname}
%{_libdir}/lua/*/%{bname}.so
%{_libdir}/liblua*-%{bname}.so

%files -n ruby-%{bname}
%{_libdir}/ruby/*/%{bname}.so

%files -n php-%{bname}
%{_libdir}/php/*/%{bname}.so
%{_datadir}/php/%{bname}.php

%files -n tcl-%{bname}
%{_libdir}/tcl*/%{bname}/%{bname}.so
%{_libdir}/tcl*/%{bname}/pkgIndex.tcl

%files -n %{bname}
%{_sbindir}/%{bname}.lib
%{_mandir}/man8/%{bname}.lib.8*

%changelog
* Sun Oct 31 2021 ErezGeva2@gmail.com 0.5-1
- Add equal and less than operators to Binary, ClockIdentity_t,
  PortIdentity_t and PortAddress_t.
- Update the read me.
- Add sample code after Vladimir Oltean checksync application.
- Add constant modifier to methods that do not modify the object.
- Comply to format and improve Doxygen comments.
- Remove python 2 from mandatory list of testing.
- Improve includes in source code.
- Fix make file help errors.
- Add init class for the PMC application.
- Add class to process command line parameters for the pmc tool.
  So users can use it for other applications.
- Fix cross compilation errors.
- Add tcl wrapper.
- Remove Debian depends on python2, as it will be removed in the future.
- Ensure port number is 16 bits,
  as Linux process ID can be larger than 16 bits.
- Ignore deprecated ruby functions created by swig.
- Use python 3 configuration application to set the library extension.
- Add rule for Debian cross target build.
- Fix proper capitalization in RPM specification.

* Mon Jun 14 2021 ErezGeva2@gmail.com 0.4-1
- Improve check after calling strtol.
- Use C++ short loop form.
- Add reference and constant for automatic when possible.
- Add constant when possible.
- Add message clear send data, to prevent use after caller delete the data.
- Binary module:
  - Encapsulate strtok_r in a class.
  - Add function to call strtol.
- Add header for the pmc tool.
- Testing scripts for script wrappers:
  - Set domainNumber from config file
  - Set domain number and transport Specific form configuration file.
  - Add new Master only TLV to testing.
  - Add function for next sequence with range check.
  - return -1 on error and 0 on success.
  - Fix indentation to 2 spaces.
  - Add global statement in python.
- Move Debian rules to Debian make
- Adding Arch Linux packages build.
- Adding RPM build with Fedora container.
- Make:
  - Support single python version.
  - improve verCheck to support 3 version numbers.
  - PHP 7 need Swig 3.0.12

* Tue Apr 20 2021 ErezGeva2@gmail.com 0.3-1
- Add licence to Javadoc comments for Doxygen process in addition to SPDX tag.
- Set document licence to GNU Free Documentation License version 1.3
- JSON module: for
  - Message to JSON and JSON to message
  - JSON to message require C JSON library or the fast C JSON library.
  - Parse signaling messages.
  - Handle TLVs with array.
  - Handle linuxptp Events and statistics TLVs.
  - Add testing for JSON module.
  - Add macros for JSON library function and types,
    In case we need to change then in future.
  - Add function to parse from JSON object,
    User can embedded the message in a JSON message.
  - Add convector of JSON types.
- Add error macros
- Add PHP wrapper.
- Replace use of std::move with unique_ptr reset() function.
- Parse MANAGEMENT_ERROR_STATUS in signaling message.
- PMC tool:
  - Set unique_ptr after socket creation and before internalizing.
    In case socket internalize fials, unique_ptr will release it.
  - Use unique_ptr reset() function.
  - Add macros in pmc tool for both errors and normal dumps.
  - Fix TimeInterval sign, update peerMeanPathDelay,
- Fix headers in development package.
- Make file
  - Move SONAME definition from Debian rules.
  - Debian rules only set a flag to link with soname.
- Improve socket for wrappers.
  - Use Buffer object in send and receive functions.
  - Move all virtual functions to protected,
    and add functions in the base class to call them.
  - Mark SockBase and SockBaseIf in SWIG file.
  - Add rcvFrom with from address split to additional function for scripting.
- Add parse and build in message that uses reference to Buffer object.
- Fix TimeInterval sign.
  - getIntervalInt() return sign integer.
- Add getBin() to Binary to fetch octet from Binary.
- Update the read-me and the Time-Interval documentation in message module.

* Mon Apr 05 2021 ErezGeva2@gmail.com 0.2-1
- Add Ruby to read-me.
- Add long options to the pmc tool.
- Add pmc tool help after linuxptp.
- Add support for padding get action management TLVs.
- Fix Debian cross compilation.
- Support Debian Stretch rename python2 to python.
- Designated initializers are not supported in old compilers.
- Old math.h header uses DOMAIN macro, as we do not use math macro,
-  just remove it.
- Add help for make file.
- Add macros in make file to prevent Swig targets.
- testing script support linuxptp location with spaces.
- Fix python3 by adding a new class for allocating the buffer.
- Remove convert to buffer. All scripts use the buffer class.
- pmc tool: add mode for PTP network layer in run mode.
- Add check in Ruby for capitalizing first letter.
- testing script: fix check for installed libraries on system.
- Add support for ruby
- Start classes with capital.
- Prevent format of rebuild all.
- Improve testing scripts.
- Add masking for flags in proc module.
- Add flag for build only and for TLVs with variable length.
- Add vector handling classes for scripts.
- Prepare for using a different implementation specific management TLVs.
- Add key flag to pmc build.
- Use optimization for fast execution When packaging.
- Add signaling messages support.
- Add IEEE 754 64-bit floating point.
- Add install goal in make file.
- Debian rules uses make file install goal.
- Fix overflow in configuration class.
- Fix compilation warnings.
- Spelling.
- Use Debian site man pages as they are more updated.

* Sun Mar 21 2021 ErezGeva2@gmail.com 0.1-1
- Alpha version.
