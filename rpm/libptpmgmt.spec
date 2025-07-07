# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# RPM specification file for libptpmgmt rpm packages
# Spec file format:
# https://rpm-software-management.github.io/rpm/manual/spec.html
# scriptlet expansion
# https://rpm-software-management.github.io/rpm/manual/scriptlet_expansion.html
# RPM Macros (default path definitions)
# https://docs.fedoraproject.org/en-US/packaging-guidelines/RPMMacros/
# https://docs.fedoraproject.org/en-US/packaging-guidelines/Scriptlets/
# Inside the Spec File
# https://ftp.osuosl.org/pub/rpm/max-rpm/ch-rpm-inside.html
###############################################################################
Name:           libptpmgmt
Version:        1.4
Release:        1%{?dist}
URL:            https://%{name}.nwtime.org
BuildRequires:  gcc-c++ pkgconfig which swig
BuildRequires:  autoconf m4 automake libtool chrpath
# You may use 'patchelf' as alternative of `chrpath`
BuildRequires:  doxygen graphviz texlive-epstopdf
BuildRequires:  openssl-devel libgcrypt-devel gnutls-devel nettle-devel
BuildRequires:  perl perl-devel
BuildRequires:  python3 python3-devel
BuildRequires:  lua lua-devel
BuildRequires:  ruby ruby-devel
BuildRequires:  php php-devel
BuildRequires:  tcl tcl-devel
BuildRequires:  librtpi1 librtpi-devel
BuildRequires:  golang
BuildRequires:  systemd
#Source0:        https://github.com/erezgeva/%%{name}/archive/refs/tags/%%{version}.tar.gz
Source0:        %{name}-%{version}.txz

%define bname   ptpmgmt
%define cname   libclkmgr
%define cbname  clkmgr

License:        LGPL-3.0-or-later
Summary:        PTP management library, to communicate with ptp4l
%description
PTP management library, to communicate with ptp4l

%package        openssl
Summary:        PTP management library HMAC plugin using the openssl library
License:        LGPL-3.0-or-later
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       openssl-libs
%description    openssl
PTP management library HMAC plugin using the openssl library

%package        gcrypt
Summary:        PTP management library HMAC plugin using the gcrypt library
License:        LGPL-3.0-or-later
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       libgcrypt
%description    gcrypt
PTP management library HMAC plugin using the gcrypt library

%package        gnutls
Summary:        PTP management library HMAC plugin using the gnutls library
License:        LGPL-3.0-or-later
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       gnutls
%description    gnutls
PTP management library HMAC plugin using the gnutls library

%package        nettle
Summary:        PTP management library HMAC plugin using the nettle library
License:        LGPL-3.0-or-later
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       nettle
%description    nettle
PTP management library HMAC plugin using the nettle library

%package        devel
Summary:        Development files for the PTP management library
License:        LGPL-3.0-or-later
Provides:       %{name}-static = %{version}-%{release}
Provides:       %{cname}-devel = %{version}-%{release}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       %{cname}%{?_isa} = %{version}-%{release}
%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use the PTP management library,
And for clock manager client library.

%package        doc
Summary:        Documentation files for the PTP management library
License:        GFDL-1.3-no-invariants-or-later
Provides:       %{cname}-doc = %{version}-%{release}
BuildArch:      noarch
%description    doc
PTP management library documentation, to communicate with ptp4l
And clock manager documentation.

%package        perl
Summary:        PTP management library Perl wrapper
License:        LGPL-3.0-or-later
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       perl
%description    perl
PTP management library Perl wrapper

%package -n     python3-%{bname}
Summary:        PTP management library python version 3 wrapper
License:        LGPL-3.0-or-later
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       python3
%description -n python3-%{bname}
PTP management library python version 3 wrapper

%package -n     lua-%{bname}
Summary:        PTP management library Lua wrapper
License:        LGPL-3.0-or-later
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       lua
%description -n lua-%{bname}
PTP management library Lua wrapper

%package -n     ruby-%{bname}
Summary:        PTP management library ruby wrapper
License:        LGPL-3.0-or-later
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       ruby
%description -n ruby-%{bname}
PTP management library ruby wrapper

%package -n     php-%{bname}
Summary:        PTP management library php wrapper
License:        LGPL-3.0-or-later
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       php
%description -n php-%{bname}
PTP management library php wrapper

%package -n     tcl-%{bname}
Summary:        PTP management library tcl wrapper
License:        LGPL-3.0-or-later
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       tcl
%description -n tcl-%{bname}
PTP management library tcl wrapper

%package -n     golang-%{bname}
Summary:        PTP management library golang development wrapper
License:        LGPL-3.0-or-later
BuildArch:      noarch
Provides:       golang-%{cbname} = %{version}-%{release}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       golang
%description -n golang-%{bname}
PTP management library golang development wrapper
And for clock manager client library.

%package -n     pmc-%{bname}
Summary:        pmc tool
License:        GPL-3.0-or-later
Requires:       %{name}%{?_isa} = %{version}-%{release}
%description -n pmc-%{bname}
new rewrite of linuxptp pmc tool using the PTP managemen library.
This tool is faster than the original linuxptp tool.

%package -n     phc-ctl-%{bname}
Summary:        phc_ctl tool
License:        GPL-3.0-or-later
BuildArch:      noarch
Requires:       python3-%{bname}%{?_isa} = %{version}-%{release}
%description -n phc-ctl-%{bname}
new rewrite of linuxptp phc_ctl tool using the PTP managemen library.

%package -n     %{cbname}-proxy
Summary:        Clock Manager proxy service
License:        BSD-3-Clause
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       librtpi1
%description -n %{cbname}-proxy
Clock Manager proxy service.
framework for controlling and monitoring network time synchronization
on the local platform.

%package -n     %{cname}
Summary:        Clock Manager client library
License:        BSD-3-Clause
Requires:       %{cbname}-proxy%{?_isa} = %{version}-%{release}
Requires:       librtpi1
%description -n %{cname}
Clock Manager library
framework for controlling and monitoring network time synchronization
on the local platform.

%package -n     %{cname}-perl
Summary:        Clock Manager library Perl wrapper
License:        BSD-3-Clause
Requires:       %{cname}%{?_isa} = %{version}-%{release}
Requires:       perl
%description -n %{cname}-perl
Clock Manager library Perl wrapper

%package -n     python3-%{cbname}
Summary:        Clock Manager library python version 3 wrapper
License:        BSD-3-Clause
Requires:       %{cname}%{?_isa} = %{version}-%{release}
Requires:       python3
%description -n python3-%{cbname}
Clock Manager library python version 3 wrapper

%package -n     lua-%{cbname}
Summary:        Clock Manager library Lua wrapper
License:        BSD-3-Clause
Requires:       %{cname}%{?_isa} = %{version}-%{release}
Requires:       lua
%description -n lua-%{cbname}
Clock Manager library Lua wrapper

%package -n     ruby-%{cbname}
Summary:        Clock Manager library ruby wrapper
License:        BSD-3-Clause
Requires:       %{cname}%{?_isa} = %{version}-%{release}
Requires:       ruby
%description -n ruby-%{cbname}
Clock Manager library ruby wrapper

%package -n     php-%{cbname}
Summary:        Clock Manager library php wrapper
License:        BSD-3-Clause
Requires:       %{cname}%{?_isa} = %{version}-%{release}
Requires:       php
%description -n php-%{cbname}
Clock Manager library php wrapper

%package -n     tcl-%{cbname}
Summary:        Clock Manager library tcl wrapper
License:        BSD-3-Clause
Requires:       %{cname}%{?_isa} = %{version}-%{release}
Requires:       tcl
%description -n tcl-%{cbname}
Clock Manager library tcl wrapper

%prep
%setup -q

%build
autoreconf -i
%configure --with-pmc-flags='-fPIE' --enable-systemd
%make_build PMC_USE_LIB=so --no-print-directory all doxygen

%install
%make_install DEV_PKG=%{name}-devel --no-print-directory

%preun -n %{cbname}-proxy
%systemd_preun %{cbname}-proxy.service
%systemd_preun %{cbname}-proxy.socket

# Seems empty
# %postun -n %{cbname}-proxy
# %systemd_postun %{cbname}-proxy.service
# %systemd_postun %{cbname}-proxy.socket

%files
%{_libdir}/%{name}.so.*

%files openssl
%{_libdir}/%{name}_openssl.so.*

%files gcrypt
%{_libdir}/%{name}_gcrypt.so.*

%files gnutls
%{_libdir}/%{name}_gnutls.so.*

%files nettle
%{_libdir}/%{name}_nettle.so.*

%files devel
%{_includedir}/*
%{_libdir}/%{name}*.a
%{_libdir}/%{name}*.so
%{_libdir}/%{cname}*.a
%{_libdir}/%{cname}*.so
%{_libdir}/pkgconfig
%{_datadir}/doc/%{name}-devel/*

%files doc
%{_datadir}/doc/%{name}-doc/*

%files perl
%{_libdir}/perl5/PtpMgmtLib.pm
%{_libdir}/perl5/auto/PtpMgmtLib/PtpMgmtLib.so

%files -n python3-%{bname}
%{_libdir}/python3*/*/_%{bname}.cpython-*.so
%{_libdir}/python3*/*/%{bname}.py
%{_libdir}/python3*/*/*/%{bname}.*.pyc

%files -n lua-%{bname}
%{_libdir}/lua/*/%{bname}.so

%files -n ruby-%{bname}
%{_libdir}/ruby/*/%{bname}.so

%files -n php-%{bname}
%{_libdir}/php/*/%{bname}.so

%files -n tcl-%{bname}
%{_libdir}/tcl*/%{bname}/%{bname}.so
%{_libdir}/tcl*/%{bname}/pkgIndex.tcl

%files -n golang-%{bname}
/usr/lib/golang/src/%{bname}/*
/usr/lib/golang/src/%{cbname}/*

%files -n pmc-%{bname}
%{_sbindir}/pmc-%{bname}
%{_mandir}/man8/pmc-%{bname}.8*

%files -n phc-ctl-%{bname}
%{_sbindir}/phc_ctl-%{bname}
%{_mandir}/man8/phc_ctl-%{bname}.8*

%files -n %{cbname}-proxy
%{_sbindir}/%{cbname}_proxy
%{_mandir}/man8/%{cbname}_proxy.8*
%{_sysconfdir}/%{cbname}/proxy_cfg.json
%{_prefix}/lib/systemd/system/%{cbname}-proxy.s*

%files -n %{cname}
%{_libdir}/%{cname}.so.*

%files -n %{cname}-perl
%{_libdir}/perl5/ClkMgrLib.pm
%{_libdir}/perl5/auto/ClkMgrLib/ClkMgrLib.so

%files -n python3-%{cbname}
%{_libdir}/python3*/*/_%{cbname}.cpython-*.so
%{_libdir}/python3*/*/%{cbname}.py
%{_libdir}/python3*/*/*/%{cbname}.*.pyc

%files -n lua-%{cbname}
%{_libdir}/lua/*/%{cbname}.so

%files -n ruby-%{cbname}
%{_libdir}/ruby/*/%{cbname}.so

%files -n php-%{cbname}
%{_libdir}/php/*/%{cbname}.so

%files -n tcl-%{cbname}
%{_libdir}/tcl*/%{cbname}/%{cbname}.so
%{_libdir}/tcl*/%{cbname}/pkgIndex.tcl

###############################################################################
# The changelog is updated with the 'tools/update_changelog.pl' script.
# Anything add after here will be toss, add only above.
# The changelog is under GFDL-1.3-no-invariants-or-later license.
###############################################################################
%changelog
* Fri Sep 13 2024 ErezGeva2@gmail.com 1.4-1
- Linuxptp version 4.4 do not add any new managment interface.
- Improve C++ pre-compile compare values.
- Fix spelling.
- Fix Gentoo build issue
  - Add Python3 byte compilation.
  - Move documents to a Gentoo friendly folder.
- Add new JSON parser.
  - Remove JSON wrapper libraries.
  - Keep backward compatible of JSON library load functions.
- Improve PTP backward compatible.
  improve handling of ptp_perout_request structure.
- Re-license test scripts.
  So users can use them to any way they wish.
  The code in sample left with GPL is intended as reference only.
- Make-file
  - Replace hash sign with hash variable.
  - Add SPDX IDs to generated package configuration file.
  - Use bank sign in regular replace expression.
  - Use hash variable as hash sign is used fore comments.
  - Improve verbose flag.
  - Update to C11 in C unit tests.
    We need the time structures.
  - Use libtool to link static and dynamic pair libraries.
  - Use chrpath or patchelf to remove RUN-PATH from
    shared libraries before installation.
- Remove warning suppression of removed code in SWIG files.
- ABI compare action
- Add final to virtual override functions of HMAC libraries.
  The libraries are internal,
  we can change without effect main library ABI.
- Fix ID type of SPP class.
- GPG sign of release tags.
- Improve documents.
- Improve ABI script
  - Skip ABI test if we do not have the release tag.
    Some developers may run the test on their own fork,
    which do not have the release tags.
    No point to indicate error in this case.
    The test will run on the official repository.
  - Store last version dump.
  - Add exit with error in case ABI fails after we save the report.
- Add Discount-Markdown to containers.
- Generate HTML from markdown using the discount markdown application.
- Move markdown files to doc folder.
- Add package configuration to project.
  It does not contain much information, but convenient for probing.
- Autoconf configuration improvements
  - Add more macros.
  - Add macros for conditions.
    Add PTPM_VAR_EMPTY_IF test for empty variable
    Add PTPM_VAR_ELSE_IF test variable value is other
    Add PTPM_FILE_IF test file exist
    Add AS_VAR_SET_IF to test a variable is set.
    Add AS_VAR_IF to test a variable value.
    Add macro looping a range.
  - Use AS_VAR_COPY for copying value from another variable.
    Use AS_VAR_APPEND when append text to a variable .
  - Add macros to all shell commands.
  - Use package configuration (pkgconf)
  - Replace AC_XXX_PROGS with AC_CHECK_TOOLS
  - Check that the user Lua setting is a real Lua executable.
  - Rename variables in AC_XXX_PROGS as they are substitute by definition.
  - Replace AS_VAR_SET_IF, as condition return true
    for variable set with empty value.
  - Improve list of folders to search.
  - Use 'printf' in m4 and when need formatting.
    Replace with AS_ECHO_N in the other cases.
  - Language scripts use 'print', to distinguish from shell echo.
  - Use AS_ECHO_N, as we do not need the new-line.
  - Use package configure with the exist flag to determine if a package exist.
  - Improve wrapper languages probing.
  - Add report for cross compilation.
  - Add more sub-versions of Python 3.
  - Integrate libtool with configuration.
  - Fix install of Doxygen documents.
    Verify the files exist before modifying.
- Remove standard auxiliary files and install them from system.
- Add Coverity Scan action. Fix defects found by scan.
  - Initialise class properties.
  - Ensure buffer is allocated in Binary::iResize.
  - Skip calling Binary::iResize when using length zero.
  - Improve Binary::clear().
  - Add protections on Binary::copy;
  - Add test in ConfigSection::set_val() before setting m_vals[].
  - Improve Token::dup().
  - Remove dead code in Message::mng2str_c().
  - Add MessageDispatcher::_noTlvClear() and
    remove dead code in MessageBuilder::buildTlv().
  - Improve code selection in MsgProc::proc(Float64_t).
  - Improve interface name copy in IfInfo::initUsingName().
  - In PMC dump: ensure key handler have a valid string to parse.
  - In read_cfg(): make sure we close the file.
  - In message parser: release the SMPTE TLV in case it is not used.
- Improve document of parse_options().
- Improve the Go architecture script.
- Add sys/types.h to pub/c/ptp.h.
- Create the example make files dynamically.
- Fix go package.
- Remove D_INC macro, We use SWIG to create the dependencies file.
- Fix library name in description.
- In continuos integration add test without the development package.
- Use proper name for shared libraries dynamic load.
  Make sure we use published libraries.
  And not developers libraries.
- Add the librtpi package to container.
- Reorder PHC controller tool help follow Linuxptp.
- Update GitHub actions versions, using deprecated Node.js version 16.
- Add link to project home page and issues.
- Remove MessageBuilder old note, the destructor was removed.
- Add Compatibility with Linuxptp document.
- Fix version 1.3 change log.
- Use C++ 11 default member initialising.
  Reduce use of direct initialising with constructors.
- Add C public header external "C" for C++ usage.
  Although C++ application can use the better C++ API.
  Some may defer the C API and call them from C++.
- Fix C API ptpmgmt_clock::fetchCaps() definition,
  use Linux clock capabilities structure.
- Move Perl SWIG Module name.
  from SWIG interface file to SWIG command line.
- Improve shell script distribution() function in util.sh.
- Fix Docker file description.
- Update README.md

* Fri Jun 07 2024 ErezGeva2@gmail.com 1.3-1
- Support Linuxptp version 4.3
- Add support for AUTHENTICATION.
  - Add HMAC wrapper libraries.
  - Probe the HMAC libraries.
  - Add HMAC libraries to containers.
    We need them for the new authentication TLV.
- Improve ABI compatible with version 1.1 and version 1.2.
  - Add ABI compare workflow.
- Improve header detection.
- MessageBuilder: move calling of clearData() to a new function, clear().
  The problem, we can not guarantee that the message object was
  not deleted before we call MessageBuilder destructor.
- Fix go MessageBuilderBuildTlv(): free the TLV in case SetAction fails.
- Add explanation on C++/C standards.
- Add note on GO wrapper.
- Improve Message Builder build TLV method.
- Add support for new Linux kernel IOCTL.
- Change kernel PTP mask for external pins.
- Swig can create dependencies by itself.
- New version of Arch Linux package manager need the debugedit tool.
- Add GNU debugger to containers.
- Message handle message size properly.
- Add separator to Binary to ID function.
- Add BASE64 encoding and decoding functions in Binary class.
- Use Ruby SWIG director.
- Fix internal Token class.
- Improve unit test assertions.
- Fix allocation of long option strings in Options class.
- Fix and improve Doxygen documentation.
- Fix parsing and building errors text.
- Add GitLab:
  - docker.
  - CI.
  - Add cross compilation to GitLab CI.
- Improve naming of scripts.
- Add GnuPG Libgcrypt to containers.
- Small improvements in configuration parser.
- Move internal Token class to comp.
  So we can reuse it in configuration parsing and use it in the PMC tool.
- Fix types in C configuration unit test.
- Improve cppcheck.
- Improve JSON from linking code.
- Improve class option properties.
- The PMC tool do not use interface name in configuration file.
- Improve C wrapper code.
- Improve JSON libraries probing.
- Place shared library links in proper packages.
- Add skip Doxygen flag in Gentoo ebuild.
- Move using 'std' to internal header.
- Add missing depending in make file.
- Update the how to document code document.
- Update and improve text in read-me.
- Improve socket C API naming.
- Use line variable instead of a specific number in error unit test.
- Add probing headers and features to public headers.
- Improve including headers and probing.
- As we test packaging and install them on system.
  We can remove the packaging from the main CI.
- Clang
  - Add support for clang.
  - Use the CC and CXX variables directly to select Clang.
  - Clang miss '__FLOAT_WORD_ORDER__'.
  - Use Clang with Gentoo CI.
  - Update ptp4l_sim for compilation with clang.
  - Fix warnings found by clang.
  - Add test with clang.
- Add configuration header to probe headers.
- Fix deprecated functions macros.
- Remove deprecated functions from GO.
  As GO wrapper is used during building/development.
- Split pkgs test to 2 stages.
- Add new Linuxptp Common Mean Link Delay TLV and notify.
- Add compiler warn if an undefined identifier is evaluated.
- Fix type range limits and deprecated copy constructor warnings.
- Add missing system functions to libsys sources.
- Fix PTP_PEROUT_REQUEST IOCTL.
- Fix of Lua probing in testing scripts.
- github_pages: remove unused files from site.
- Update build dependencies in Debian packaging.
- Improve read_defs() function.
- Update build dependencies in RPM packaging.
- Improve Arch Linux PKGBUILD.
- Fix adding SPDX to generated HTMLs.
- Add Lua POSIX to Arch Linux.
- Fix Manual full test.
- Gentoo add USE flags to skip wrappers.
- Fix github_address.sh dispatcher.
- Add SPDX IDs to source installed in system.
- Use MIT to GO language source as there are linked in the executable.
- Fix GO message builder dependency.
- The other are used as libraries so LGPL is fine.
- Replace 'auto' with actual type in unit test code.
- Add SPDX IDs to markdown using HTML comment.
  and to generated Doxygen HTML and JavaScripts.
- Add traversSigTlvs to C and traversSigTlvsCl for script languages.
- Fix signalling LoopTwoManagementTlvs unit test to loop over the 2 TLVs.
- Spell signalling in text, leave American spelling in code.
- Add Gentoo to packages CI.
- Use "using" standard name space in C++ source code.
- Collect small tool scripts into a single one.
- Move testing with Linuxptp and a dummy clock to the test with
  the packaging to ensure the packing contain all the files needed.
- Fix Gentoo build QA warnings.
- Improve Gentoo support.
- Add filters for C unit tests.
- Skip pre-compile '#error' when using cppcheck.
- Use swig director for message dispatcher and builder.
- Improve Unix socket abstract address support.
- Add C wrapper
- Update Build.md and Improve grammar.
- Small improvements.
  - Improve git ignore.
  - Add external C to libsys.
- Improve release scripts.
- Update credits.
- Update pmc.8
- Fix grammar in MANAGEMENT IDS section and remove trailing line
  which looks like a cut error.
- Add the new phase command to 'phc_ctl' tool.

* Wed Dec 06 2023 ErezGeva2@gmail.com 1.2-1
- Support Linuxptp version 4.2
- Improve Linuxptp subscribe events TLV.
- Use a separate flag for Python unit test with Valgrid
  as it breaks on Ubuntu.
- Add Doxygen bug workaround.
- Fix Arch container, add GPG keys.
- Add and probe epstopdf, as alternative to dot tool.
- Split m4 rule for creating headers as some make file differ.
- Add documentation for build.
- Support reuse version 2.1
- Add NOTIFY_PARENT_DATA_SET event after Linuxptp.
- Follow Linuxptp pmc, avoid conflicting port IDs over UDS.
- Use separate management TLV ID to build and parse in management class.
- Add credits for contributors of project.
- Fix example make files for development package.
- Support abstract Unix Socket Address.
  Skip deleting abstract Unix socket as they do not use real files.
- Add copyright sign in copyright files.
- Add criterion unit testing framework to docker files.
- Init class: add getNetSelect() method to specify which socket is created.
- PtpClock class: add function to return socket file description.
- Small library improvements.
- Remove semicolon from comments in unit test source code.
- Order home sites: Network Time Foundation, GitHub, SourceForge.
- Change main home page to Network Time Foundation.
- Use constant string reference in Error class,
  when stored as class property.
- Move public headers to pub folder.
- Add new GitHub action Linuxptp with dummy simulated clock:
  - With full build.
  - with installed Debian packages in system.
    To verify the integrity of the installed Debian packages.
- Move wrapper scripts into a wrapper folder.
- Add astyle to fedora and Arch Linux docker containers.
- Fix configuration report script.
- Move docker label to start of docker file.
- Debian docker: Move Perl developer library to packages list,
  as it is multi-architecture on all used Debian versions.
- Use proper macros for time conversions in socket poll function.
- Resolve warnings found with clang.
- Remove Debian bullseye and support for old versions of swig and GO.
- Parse SMPTE Organization Extension TLV.
- Fix GO cross compilation.
- Add GitHub action for cross compilation.
- Update Doxygen configuration file to Doxygen version 1.9.4.
- Fix grammar in Doxygen configuration file and documents.
- Improve unit test configuration probing.
- Improve GitHub actions script.
- Improve and fix grammar in README.md.
- Add new Debian trixie version.
- Replace automatic type with defined type for short types.
- Binary::fromHex uses reference to constant C++ string input.
- Replace maps and stack classes with a single class in fromJSON.
- Probe if C++ compiler support, comparing pointer of inherited methods.
- Fix TCL version.

* Sat Jun 10 2023 ErezGeva2@gmail.com 1.1-1
- Support Linuxptp version 4.0
- GitHub workflow
  - Pages: Install Doxygen and generate documents on Ubuntu.
  - Continuous Integration
  - Add docker label.
  - Upload Docker containers.
  - Unit tests with Valgrind and Address Sanitizer.
  - Run testing with a Dummy clock for Linuxptp daemon.
    With Valgrind and Address Sanitizer.
- Add setPhase() that call clock_adjtime() with ADJ_OFFSET.
- Improve Linux kernel support for PTP clases
- Fix wrong alignment assignment in MsgProc::proc.
- Add external "C" to system function override in
  unit test and dummy clock simulation.
- Fix phc_tool testing.
- Multiple indirect includes headers cleaning.
- Add unit test for the pmc tool build and output of different TLVs.
- Move common part of make docker script to tools.
- Improve format script.
- Move tools from root folder to tools folder.
  Previous tool was moved to ptp-tools
- Update Swig files from Swig project.
  The files are provided for backward compatible of old Swig.
- Remove the '-j' flag from make files.
  The number of jobs should be set by user.
- CI script
   - Use the CI script for all distributions.
   - Use proper unit tests for each distribution.
   - Build packages per distribution.
- Restore PHP in RPM, as new Fedora swig support PHP 8.
- Fix RPM 'config' target.
- Use system include for swig files.
- Add `utest_cpp` target to run unit test for C++ only.
- Remove stretch, as it's Long Term Support is over.
  https://wiki.debian.org/LTS/Stretch
  Remove `buster` as it is old Debian version.
- Add Go language wrapper.
- Add enumeration for delay mechanism.
- Add checks in buildTlv(), so we do not call callback for false cases.
- Handle cppcheck warning.
- Add cookie to docker creation, for updating docker containers.
- Ignore PTP message control Field as it was used by PTP v1.
- Add remark on Arc Linux license for documents.
  Most call them GFDL, yet Arc Linux use only FDL.
- Add support for Linuxptp power profile TLV.
- Remove restriction on domain number.
- Add maintainer tag to the Gentoo docker file.
- Follow Linuxptp.
  Alphabetise configuration options and bump date in the pmc man page.
- Add support to Gentoo.
- Add release scripts to Arc Linux and for RPM.
- Improve make docker scripts.
- Add copyright sign to SPDX copyright.
- pmc tool: add macros for flags.
- Unit test: Use expect near for range assertion.
- Unit test: Use float compare for float numbers.
- Fix the source filed in reuse copyright file.
- Improve comments in SWIG definitions files.
- improve comments in callDef header.
- Add float types for seconds, floating and frequency.
- Swig
   - Apply int on clockid_t type.
   - Timestamp_t operator uses the float_seconds type.
- Add shell scripts to check format.
- Add working directory to docker run.
- Fix TCL package version to match library version.
- Fix JsonProcFromJson::procMng() it should not return the string.
- Remove small git ignore files.

* Mon Nov 21 2022 ErezGeva2@gmail.com 1.0-1
- Project uses 5 script languages, 2 JSON libraries,
  support 3 Linux distribution and support cross compilation.
- Add autoconf configuration as most distribution provide autoconf
  probing of current system.
- Remove all options from make file that are probed or
   set by the configuration script.
- Add unit tests
  - ptpmgmt library
  - scripts languages Message dispatcher and builder classes
  - Json to messages
  - load Json to messages libraries
- Build
  - Split make file
  - Catch Doxygen warning and exit with error.
  - Add unit test main to the unit test make file.
  - Improve format script and exit with error on error.
  - Probe astyle change and exit with error.
    So we can use the make format goal in
    a continuous integration checking container.
  - Check for dot application to use with Doxygen.
  - Fix clean.
  - Update source files list for archive.
  - Fix installed man pages file mode.
  - Prevent copy of man pages on seconds install.
  - Add all markdown documentation to documentation package.
- Add update_doxygen.pl script file header.
- Fix Doxygen configuration.
- Update Doxygen to version 1.9.1
- Improve Doxygen handling.
- testing
  - Fix Lua using local library.
  - Improve testing.
  - Run all testing from root folder.
  - Use system libraries only if specify with a flag.
  - Add rule to run default configuration of distribution system.
- Add Message Builder Base C++ class to Lua.
  The C++ class destructor free the send TLV in the message object.
- Debian Bookworm have a installation bug with GCC C++ cross compiler.
- Add CI script to run all steps needed in CI:
   - check licenses with reuse tool
   - build Debian packages
   - pass format and Doxygen
   - run unit tests
   - verify no left over after clean and distribution clean.
   - The system test and build packages on other distribution is done
     out side CI.
- Merge headers used during compilation only,
  into a single compilation only header.
  The compilation header is used during compilation only,
  and do not hold any public API.
- Add a new error class to store the last error happened in the library.
  The library do not print the error to standard error no longer.
  It is up to the application how to handle the error.
- Improve error in: socket, PTP and clock classes.
- Rebase binary class to support operator [] with reference.
- Add support for Debian bookworm.
- Add docker for Debian to use on GitHub CI.
- Create the version header by the make file.
  So we have only one configuration file created by configure.
  All the reset is created by the make file.
- Fix tcl library package version in index file.
- Fix Perl library folder location.
- Move source code and objects to sub-folders.
- Replace GCC prepossess with m4 to generate headers files.

* Tue Jul 26 2022 ErezGeva2@gmail.com 0.9-1
- Add header to define the C++ namespace.
- Swig generated code
  - Clean compilation warnings of swig generated code.
  - Define swig vector classes for new PTP structure.
  - Add vector class for LinuxptpUnicastMaster_t.
  - Remove Python2, it is discontinue for 2 years.
  - Use proper typemap to initialise variables in argcargv.
- Swig project files
  - Add readme for swig files.
  - Follow swig 4.1.0
  - Add tcl std_vector.i from version 4.1.0 with bug fix.
- Message Dispatcher and builder classes.
  - Add Message.isValidId() to verify an management TLV ID.
  - Add C++ classes and native classes in each script language.
  - Call Message.clearData() in destructor of MessageBuilder.
  - Lua use C++ MessageBuilder object to ensure
    calling Message.clearData(), as Lua native do not use destructors.
  - Beside Lua using C++ MessageBuilder, C++ classes are not wrapped by Swig!
  - Use abstract classes in PHP
  - MessageDispatcher::callHadler() call virtual noTlvCallBack()
    if the TLV call back is not inherited.
  - Use C++ dispatcher and builder classes in the pmc tool.
- Remove ids.h header from public headers, use generated headers instead.
- Move process functions of the message class to a private class.
- Testing
  - Use Python and Lua environment to load the wrapper library.
  - Add support to test with AddressSanitizer.
  - Add simple test for std::vector<> class in scripts.
  - Add testing with phc_ctl and Valgrind with phc_ctl.
  - And Valgrind test of testJson.pl.
- Improve Debian build.
- List source files with git when possible.
- JSON
  - Move From JSON part to a new source file.
    So user can call msg2json() using static link without linking
     the jsonFrom static library.
  - Move jsonFrom to separate static libraries.
    One library using JSON-c, the other one using the fast-JSON.
  - Users can select which library they wish to use during link.
    The shared library is loaded dynamically on run time.
    User can prefer shared library to use by calling selectLib().
  - Add function to get name of used fromJson library.
- PTP class
  - Add all PHC Linux kernel ioctls.
  - Add PTP and system clock reading and setting.
  - Use Timestamp_t for clock read and set.
  - Add PTP clock frequency and offset functions.
  - Add PHC pins functionality.
  - Add header with time conversion constants.
  - Clone the PHC control tool using the Python wrapper library,
    and add new package for it.
- Add pure attribute for proper functions.
- Make file
  - Use Make file one shell option.
  - Improve make file syntax.
  - Use one rule for all swig creation.
  - Remove empty lines when using V=1.
  - Improve Doxygen filter of warnings.
- Follow IEEE 1558 alternative terms for "master" and "slave".
  Source and client are not confirming to IEEE 1558 amendment.
- Perform Valgrind test on pmc.
  - Valgrind found an issue in the ConfigFile
    Change cfgGlobal type from a reference to a pointer.
- Documentation.
  - Update documentation.
  - Add pmc.8 from Linuxptp with updates.
  - Add phc_ctl.8 from Linuxptp with updates.
  - Update vecDef.cc and
    add documentation on std::vector<> mapping.
  - Add Frequently Asked Questions.
  - Doxygen configuration: remark all default setting.
  - New Doxygen on Arch Linux have:
      https://github.com/doxygen/doxygen/issues/9319
      The bug needs a fix in Arch Linux, it is out of scope of this project.

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
- Linuxptp new management TLVs:
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
- Add new Linuxptp linuxptpPowerProfileVersion_e enumerator
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
- Remove Python 2 from mandatory list of testing.
- Improve includes in source code.
- Fix make file help errors.
- Add init class for the PMC application.
- Add class to process command line parameters for the pmc tool.
  So, users can use it for other applications.
- Fix cross compilation errors.
- Add tcl wrapper.
- Remove Debian depends on Python2, as it will be removed in the future.
- Ensure port number is 16 bits,
  as Linux process ID can be larger than 16 bits.
- Ignore deprecated ruby functions created by swig.
- Use Python 3 configuration application to set the library extension.
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
  - Add global statement in Python.
- Move Debian rules to Debian make
- Adding Arch Linux packages build.
- Adding RPM build with Fedora container.
- Make:
  - Support single Python version.
  - improve verCheck to support 3 version numbers.
  - PHP 7 need Swig 3.0.12

* Tue Apr 20 2021 ErezGeva2@gmail.com 0.3-1
- Add licence to Javadoc comments for Doxygen process in addition to SPDX tag.
- Set document licence to GNU Free Documentation License version 1.3
- JSON module: for
  - Message to JSON and JSON to message
  - JSON to message require C JSON library or the fast C JSON library.
  - Parse signalling messages.
  - Handle TLVs with array.
  - Handle Linuxptp Events and statistics TLVs.
  - Add testing for JSON module.
  - Add macros for JSON library function and types,
    In case we need to change then in future.
  - Add function to parse from JSON object,
    User can embedded the message in a JSON message.
  - Add convector of JSON types.
- Add error macros
- Add PHP wrapper.
- Replace use of std::move with unique_ptr reset() function.
- Parse MANAGEMENT_ERROR_STATUS in signalling message.
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
- Add pmc tool help after Linuxptp.
- Add support for padding get action management TLVs.
- Fix Debian cross compilation.
- Support Debian Stretch rename Python2 to Python.
- Designated initializers are not supported in old compilers.
- Old math.h header uses DOMAIN macro, as we do not use math macro,
-  just remove it.
- Add help for make file.
- Add macros in make file to prevent Swig targets.
- testing script support Linuxptp location with spaces.
- Fix Python3 by adding a new class for allocating the buffer.
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
- Add signalling messages support.
- Add IEEE 754 64-bit floating point.
- Add install goal in make file.
- Debian rules uses make file install goal.
- Fix overflow in configuration class.
- Fix compilation warnings.
- Spelling.
- Use Debian site man pages as they are more updated.

* Sun Mar 21 2021 ErezGeva2@gmail.com 0.1-1
- Alpha version.
