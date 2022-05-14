# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2021 Erez Geva
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
# RPM specification file for libptpmgmt rpm packages
###############################################################################
Name:           libptpmgmt
Version:        0.8
Release:        1%{?dist}
URL:            https://github.com/erezgeva/%{name}
BuildRequires:  swig
BuildRequires:  perl perl-devel perl-ExtUtils-Embed
BuildRequires:  which
BuildRequires:  python2 python2-devel
BuildRequires:  python3 python3-devel
BuildRequires:  lua lua-devel
BuildRequires:  ruby ruby-devel
BuildRequires:  php php-devel
BuildRequires:  tcl tcl-devel
BuildRequires:  libfastjson libfastjson-devel json-c-devel
BuildRequires:  doxygen graphviz
Source0:        %{name}-%{version}.txz

%define bname   ptpmgmt

License:        LGPLv3+
Summary:        PTP management library, to communicate with ptp4l
%description
PTP management library, to communicate with ptp4l

%package        jsonc
Summary:        PTP management library JSON plugin using the json-c library
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       json-c
%description    jsonc
PTP management library JSON plugin using the json-c library

%package        fastjson
Summary:        PTP management library JSON plugin using the fastjson library
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       libfastjson
%description    fastjson
PTP management library JSON plugin using the fastjson library

%package        devel
Summary:        Development files for the PTP management library
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use the PTP management library.

%package        doc
Summary:        Documentation files for the PTP management library
License:        GFDLv1.3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description    doc
PTP management library documentation, to communicate with ptp4l

%package        perl
Summary:        PTP management library Perl wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       perl
%description    perl
PTP management library Perl wrapper

%package -n     python2-%{bname}
summary:        PTP management library python version 2 wrapper
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       python2
%description -n python2-%{bname}
PTP management library python version 2 wrapper

%package -n     python3-%{bname}
Summary:        PTP management library python version 3 wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       python3
%description -n python3-%{bname}
PTP management library python version 3 wrapper

%package -n     lua-%{bname}
Summary:        PTP management library Lua wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       lua
%description -n lua-%{bname}
PTP management library Lua wrapper

%package -n     ruby-%{bname}
Summary:        PTP management library ruby wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       ruby
%description -n ruby-%{bname}
PTP management library ruby wrapper

%package -n     php-%{bname}
Summary:        PTP management library php wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       php
%description -n php-%{bname}
PTP management library php wrapper

%package -n     tcl-%{bname}
Summary:        PTP management library tcl wrapper
License:        LGPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       tcl
%description -n tcl-%{bname}
PTP management library tcl wrapper

%package -n     pmc-%{bname}
Summary:        pmc tool
License:        GPLv3+
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description -n pmc-%{bname}
 new rewrite of linuxptp pmc tool using the PTP managemen library.
 This tool is faster than the original linuxptp tool.

%prep
%setup -q

%build
%make_build LD_SONAME=1 CPPFLAGS_OPT=-Ofast --no-print-directory all doxygen

%install
%make_install LIBDIR=%{_libdir} DEV_PKG=%{name}-devel\
  PY_LIBDIR=%{_libdir}/python --no-print-directory

%clean
make distclean

%files
%{_libdir}/%{name}.so

%files jsonc
%{_libdir}/%{name}_jsonc.so

%files fastjson
%{_libdir}/%{name}_fastjson.so

%files devel
%{_includedir}/*
%{_libdir}/%{name}.a
%{_datadir}/%{name}-devel/*.mk

%files doc
%{_datadir}/doc/%{name}-doc/*

%files perl
%{_prefix}/local/lib*/perl*/*/PtpMgmtLib.pm
%{_prefix}/local/lib*/perl*/*/auto/PtpMgmtLib/PtpMgmtLib.so

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

%files -n pmc-%{bname}
%{_sbindir}/pmc-%{bname}
%{_mandir}/man8/pmc-%{bname}.8*

###############################################################################
# The changelog is updated with the 'update_changelog.pl' script
# Anything add after here will be toss, add only above.
%changelog
* Sat May 14 2022 ErezGeva2@gmail.com 0.8-1
- Fix cross compilation in make-file.
  install target should install and not build.
- Remove end.h err.h headers from Doxygen and development package.
- Properly read all dependencies files in make-file.
- Socket class
  - Default socket file for non-root users.
    Prefer system run folder per user over home directory.
  - Improve socket closing.
- From JSON to message support
  - Create separate libraries to process JSON into message.
    Each of them depends on a JSON library.
    Load one of them on run time when user try to process a JSON.
    This way we keep the main library loading
     with standard libraries only.
  - Add function to load a specific jsonFrom library.
  - Static library uses only one JSON library.
      User will need to link with this JSON library
      once the user application needs the functionality.
- Add support for Options class in scripts
  - Add SWIG support to call argc-argv using C main style from scripts.
  - Add Lua, Perl, Tcl and PHP SWIG argcargv.i
    that are missing from the SWIG projecy.
    These argcargv.i are plan for the SWIG project.
  - Use the Options class in testing scripts.
  - Change the string types in Pmc_option structure to C++.
  - Add the Option and Init class to SWIG build.
- Create a new header with types used by the message class.
- Reduce the number of C macros and use project style for the reset
  - As macros could conflict with other projects or headers.
    We use C++, so we can define them in enumerators as possible,
    use project style, to reduce conflicts, and move macros to module,
    and reduce the number of public macros.
  - Use project macro name style in SWIG definition file.
  - Remove macros from ids header.
  - Rename 'caseXXX' macros. But leave 'A' unchanged for now.
  - Move first and last IDs to the new 'types' header, in the enumerator.
    So, they are not macros anymore.
  - Move shortcut macro of allowed action for
    a management ID to the message module. No reason to have them public.
  - Replace vector macros with template functions.
  - Move macros from message header to process module.
    No reason to have them public.
  - We left:
    14 headers with protection macros follow the __PTPMGMT_XXX_H format.
    3 macros with library version on compilation, LIBPTPMGMT_VER...
    3 macros for 48 bits limit INT48_XXX, follow the standard.
    5 _ptpmCaseXX macros and 'A' macro used internally with the ids.h header.
    _ptpmParseFunc macro used internally in the message class.
- Improve documentation in headers.
- Add functions for using the allowSigTlvs map in MsgParams.
  So, scripts can not access the map.

* Mon May 02 2022 ErezGeva2@gmail.com 0.7-1
- Change socket receive functions to non-block be default.
- Make file
  - Use macros for system includes in dependencies files.
  - Add dependencies files to SWIG created source code.
- Use C++ namespace and Tcl namespace.
- Use bash internal test, save calling external test.
- Arch Linux
  - Use SHA256
  - Fix description of development and PMC packages.

* Mon Apr 18 2022 ErezGeva2@gmail.com 0.6-1
- Improve make file
  - Add missing files to source tar file for RPM and Arch Linux
  - Add Cppcheck code analysis on format rule.
- Compliant with the FSF REUSE Specification 3.0
- Python SWIG
  - Support multithread, make sure poll() and tpoll()
-     do not block other threads
  - Use flag for single thread mode
- Add fileno() interface as it follow POSIX function and
-   also supported by Python select module, and probably more.
- Convert C macros for NP_SUBSCRIBE_EVENTS with method
-   to support scripting.
- Sample code
  - Add sample/sync_watch.py by Martin Pecka form
-     Czech Technical University in Prague
  - Add sample code for probing PTP daemon for synchronization.
- Improve RPM specification.
- Add no return attribute to signal handlers.
- LinuxPTP new management TLVs:
  - PORT_HWCLOCK_NP TLV.
  - UNICAST_MASTER_TABLE_NP TLV.
- Improve copyright.
- Add header with version for compilation.
-   in addition to version in run time.
- Improve testing script:
  - Verify Linux PTP configuration file exit.
  - Check that script can use sudo unless we use the root user.
  - Probe Unix socket file location from configuration file.
  - Try to change Linux PTP Unix socket file permissions.
  - Patch Linux PTP daemon if Unix socket file does not exist.
  - Show Linux PTP daemon command line only if we find the folder.
- Message process of float 64
  - Remove wrong calling to move
  - Improve function syntax
  - Add macro for rare hardware which does not use IEEE 754.
- Add new linuxptp linuxptpPowerProfileVersion_e enumerator
- Fix wrong process of linuxptpTimeStamp_e enumerator in json module.
- Use short string form of clockAccuracy_e enumerator.
- cast characters in Binary::eui48ToEui64().
- Add constant modifier to configuration file reference
-   in the socket classes.

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
  So, users can use it for other applications.
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
