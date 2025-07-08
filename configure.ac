# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# @file
# @brief configuration
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
#----------------------------------------------------------------

AC_COPYRIGHT([Copyright © 2022 Erez Geva])
AC_PREREQ([2.71])
m4_define([ptpm_VER],
          m4_esyscmd_s([. ./tools/version && printf '%s' "$ver_maj.$ver_min"]))
AC_INIT([libptpmgmt], m4_defn([ptpm_VER]), [],
        [libptpmgmt-]m4_defn([ptpm_VER]),
        [https://libptpmgmt.nwtime.org])
m4_define([ptpm_dqt], m4_esyscmd_s([printf '"']))

#----------------------------------------------------------------
# Start
AC_CONFIG_FILES([defs.mk])
dnl Unique file to verify we use the proper folder for source
AC_CONFIG_SRCDIR([src/mngIds.m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_LANG([C++])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_GREP
AC_PROG_EGREP
AC_PROG_FGREP
AC_PROG_SED
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_LN_S
AC_CHECK_TOOL([CHRPATH], [chrpath])
AC_CHECK_TOOL([PATCHELF], [patchelf])
AC_CHECK_TOOL([MARKDOWN], [markdown])
AC_CHECK_TOOL([TOUCH], [touch])
dnl libtool
LT_INIT
AC_CONFIG_MACRO_DIRS([m4])

#----------------------------------------------------------------
# Macroes and Globals
dnl Pipes to clean output
AC_DEFUN([PTPM_DUMP_OUT], [>/dev/null])
AC_DEFUN([PTPM_DUMP_ERR], [2>/dev/null])
AC_DEFUN([PTPM_DUMP_ALL], [>/dev/null 2>&1])
dnl Basic conditions
AC_DEFUN([PTPM_VAR_SET_TEST], [test -n "${$1}"])
AC_DEFUN([PTPM_VAR_EMPTY_TEST], [test -z "${$1}"])
AC_DEFUN([PTPM_VAR_TEST], [test "x${$1}" = x$2])
AC_DEFUN([PTPM_VAR_ELSE_TEST], [test "x${$1}" != x$2])
AC_DEFUN([PTPM_FILE_TEST], [test -r $1])
AC_DEFUN([PTPM_NOT_FILE_TEST], [test ! -r $1])
AC_DEFUN([PTPM_DIR_TEST], [test -d $1])
AC_DEFUN([PTPM_SYMBOLIC_LINK_TEST], [test -h $1])
AC_DEFUN([PTPM_GREATER_OR_EQUAL_TEST], [test ${$1} -ge $2])
AC_DEFUN([PTPM_GREP_TEST],
         [AS_ECHO_N(["${$1}"]) | $GREP $2 PTPM_DUMP_ALL])
AC_DEFUN([PTPM_NOT_GREP_TEST],
         [AS_ECHO_N(["${$1}"]) | $GREP -v $2 PTPM_DUMP_ALL])
AC_DEFUN([PTPM_NOT_IN_LIST_TEST],
         [AS_ECHO_N([" ${$2} "]) | $GREP -v " ${$1} " PTPM_DUMP_ALL])
dnl Source shell script
AC_DEFUN([PTPM_SOURCE], [. $1])
dnl Similar to AS_VAR_SET_IF except to set and empty
AC_DEFUN([PTPM_VAR_SET_IF],
         [AS_IF([PTPM_VAR_SET_TEST([$1])], [$2], [$3])])
dnl Not AS_VAR_SET_IF, Variable is empty
AC_DEFUN([PTPM_VAR_EMPTY_IF],
         [AS_IF([PTPM_VAR_EMPTY_TEST([$1])], [$2], [$3])])
dnl AS_VAR_IF condition is PTPM_VAR_TEST
dnl Not AS_VAR_IF, Variable value is different
AC_DEFUN([PTPM_VAR_ELSE_IF],
         [AS_IF([PTPM_VAR_ELSE_TEST([$1], [$2])], [$3], [$4])])
dnl Test for file exist
AC_DEFUN([PTPM_FILE_IF],
         [AS_IF([PTPM_FILE_TEST([$1])], [$2], [$3])])
dnl Test for file not exist
AC_DEFUN([PTPM_NOT_FILE_IF],
         [AS_IF([PTPM_NOT_FILE_TEST([$1])], [$2], [$3])])
dnl Test for folder exist
AC_DEFUN([PTPM_DIR_IF],
         [AS_IF([PTPM_DIR_TEST([$1])], [$2], [$3])])
dnl general
AC_DEFUN([PTPM_VAR_SED_COPY],
         [AS_VAR_SET([$1],
                     ["`AS_ECHO_N(["${$2}"]) | $SED $3 PTPM_DUMP_ERR`"])])
AC_DEFUN([PTPM_MSG_RESULT],
         [AS_IF([PTPM_VAR_EMPTY_TEST([$1])], [AC_MSG_RESULT([not found])],
         [AC_MSG_RESULT([${$1}])])])
AC_DEFUN([PTPM_BASENAME],
         [AS_VAR_SET([$1], ["`expr "X$2" : 'X.*/\(.*\)'`"])])
AC_DEFUN([PTPM_CROSS],
         [AS_IF([PTPM_VAR_TEST([cross_compiling], ['yes']) &&\
                 PTPM_GREP_TEST([$1], ["$build_cpu"])],
         [PTPM_VAR_SED_COPY([$1], [$1], ["s%$build_cpu%$host_cpu%g"])])])
AC_DEFUN([PTPM_FOR],
         [for $1 in $2
          do :
          $3
          done])
AC_DEFUN([PTPM_FOR_BREAK], [break $1])
AC_DEFUN([PTPM_LOOP_RANGE],
         [AS_VAR_SET([$1], [$2])
          while :
          do $5
          AS_VAR_IF([$1], [$4], [break])
          AS_VAR_ARITH([$1], [${$1} + $3])
          done])
AC_DEFUN([PTPM_COUNT_WORDS],
         [AS_VAR_SET([$1], [0])
          PTPM_FOR([$2], [$3],
                   [AS_VAR_ARITH([$1], [${$1} + 1])])])
AC_DEFUN([PTPM_LIB_PKG_CONFIG],
         [AS_IF([PTPM_VAR_SET_TEST([PKG_CONFIG]) &&\
                $PKG_CONFIG --exists $2],
                [AS_VAR_SET([$1_INC_FLAGS],
                            ["`$PKG_CONFIG --cflags $2 PTPM_DUMP_ERR`"])
                 AS_VAR_SET([$1_LIB_FLAGS],
                            ["`$PKG_CONFIG --libs $2 PTPM_DUMP_ERR`"])])])
AC_DEFUN([PTPM_VAR_PKG_CONFIG],
         [AS_IF([PTPM_VAR_SET_TEST([PKG_CONFIG]) &&\
                $PKG_CONFIG --exists $3],
                [AS_VAR_SET([$1],
                            ["`$PKG_CONFIG --variable=$2 $3 PTPM_DUMP_ERR`"])])])
dnl globals
AS_VAR_SET([ptpm_c_include], ['#include'])
AS_VAR_IF([prefix], ['NONE'],
          [AS_VAR_SET([ptpm_prefix], ['/usr'])],
          [AS_VAR_COPY([ptpm_prefix], [prefix])])
AS_IF([PTPM_VAR_TEST([exec_prefix], ['NONE']) ||\
       PTPM_VAR_TEST([prefix], ['NONE'])],
      [AS_VAR_SET([ptpm_n1], ['${exec_prefix}'])
       AS_VAR_SET([ptpm_n2], ['${prefix}'])
       PTPM_VAR_SED_COPY([ptpm_libdir], [libdir],
                         ["s%$ptpm_n1%$ptpm_prefix%;s%$ptpm_n2%$ptpm_prefix%"])
       PTPM_VAR_SED_COPY([ptpm_libexecdir], [libexecdir],
                         ["s%$ptpm_n1%$ptpm_prefix%;s%$ptpm_n2%$ptpm_prefix%"])
       PTPM_VAR_SED_COPY([ptpm_includedir], [includedir],
                         ["s%$ptpm_n1%$ptpm_prefix%;s%$ptpm_n2%$ptpm_prefix%"])],
      [AS_VAR_COPY([ptpm_libdir], [libdir])
       AS_VAR_COPY([ptpm_libexecdir], [libexecdir])
       AS_VAR_COPY([ptpm_includedir], [includedir])])
AS_VAR_SET([ptpm_libdir], ["`eval "AS_ECHO_N([$ptpm_libdir])"`"])
AS_VAR_SET([ptpm_libexecdir], ["`eval "AS_ECHO_N([$ptpm_libexecdir])"`"])
AS_VAR_SET([ptpm_includedir], ["`eval "AS_ECHO_N([$ptpm_includedir])"`"])
AS_VAR_COPY([ptpm_lib_dirs], [ptpm_libdir])
dnl common locations for libraries
PTPM_FOR([d], [$ptpm_libexecdir /usr/lib /usr/local/lib /opt/local/lib],
         [PTPM_VAR_ELSE_IF([ptpm_libdir], ["$d"],
                           [AS_VAR_APPEND([ptpm_lib_dirs], [" $d"])])])
AS_VAR_COPY([ptpm_include_dirs], [ptpm_includedir])
dnl compiler own headers folders
AS_VAR_SET([ptpm_cc_include],
           [`$CC -E -Wp,-v -xc /dev/null 2>&1 | $GREP '^\s*/'`])
dnl common locations for headers
PTPM_FOR([d],
         [/usr/include /usr/local/include /opt/local/include $ptpm_cc_include],
         [AS_IF([PTPM_NOT_IN_LIST_TEST([d], [ptpm_include_dirs])],
                [AS_VAR_APPEND([ptpm_include_dirs], [" $d"])])])
AS_VAR_COPY([ptpm_keep_cppflags], [CPPFLAGS])

#----------------------------------------------------------------
# Tools

# Try first the CommonMark with GitHub Flavored
AC_CHECK_TOOL([CMARK], [cmark-gfm])
# Else use the original CommonMark convert tool
PTPM_VAR_EMPTY_IF([CMARK],
                  [AC_CHECK_TOOL([CMARK], [cmark])])

AC_CHECK_TOOL([DATE], [date])
PTPM_VAR_SET_IF([DATE],
                [AS_VAR_SET([ptpm_date], ["`$DATE "+%s"`"])
                 AS_VAR_IF([ptpm_date], ['%s'], [AS_UNSET([DATE])])])

AS_UNSET([ptpm_astylever])
AS_UNSET([ptpm_doxygenver])
AS_UNSET([PKG_CONFIG_DIR])
AS_UNSET([ASTYLE_MINVER])
AS_UNSET([DOXYGEN_MINVER])
AS_UNSET([CPPCHECK_OPT_BASE])

AC_CHECK_TOOLS([M4], [m4 autom4te])
AC_CHECK_TOOLS([CTAGS], [ctags ctags-exuberant])
AC_CHECK_TOOLS([READLINK], [readlink])
AC_CHECK_TOOL([LDD], [ldd])
AC_CHECK_TOOL([PKG_CONFIG], [pkg-config])
PTPM_VAR_SET_IF([PKG_CONFIG],
    [PTPM_FOR([d],
          [`$PKG_CONFIG --variable pc_path pkg-config |\
            $SED 's%:% %g' PTPM_DUMP_ERR`],
          [PTPM_CROSS([d])
           AS_IF([PTPM_GREP_TEST([d], ["^$ptpm_libdir"])],
                 [AS_VAR_COPY([PKG_CONFIG_DIR], [d])
                  PTPM_FOR_BREAK])])])
AC_CHECK_TOOL([ASTYLE], [astyle])
AC_MSG_CHECKING([for astyle version])
PTPM_VAR_SET_IF([ASTYLE],
      [PTPM_FOR([d], [`$ASTYLE -V PTPM_DUMP_ERR`])
       AS_VAR_COPY([ptpm_astylever], [d])])
PTPM_MSG_RESULT([ptpm_astylever])
AC_MSG_CHECKING([for astyle minimum version])
AS_VERSION_COMPARE([3.1], [$ptpm_astylever],
                   [AS_VAR_COPY([ASTYLE_MINVER], [ptpm_astylever])], dnl above 3.1
                   [AS_VAR_COPY([ASTYLE_MINVER], [ptpm_astylever])]) dnl equal 3.1

dnl version 3.3 add some changes, we will use in the future
AS_VERSION_COMPARE([3.2], [$ptpm_astylever],
                   [AS_UNSET([ASTYLE_MINVER])]) dnl above 3.2

PTPM_MSG_RESULT([ASTYLE_MINVER])
AC_CHECK_TOOL([CPPCHECK], [cppcheck])
PTPM_VAR_SET_IF([CPPCHECK],
      dnl Ignore C error pre-compile
      [AS_VAR_SET([CPPCHECK_OPT_BASE],
                  ["--suppress=preprocessorErrorDirective"])
       dnl Save en empty conftest.cpp
       AC_LANG_CONFTEST([AC_LANG_SOURCE])
       dnl Use exhaustive checking, support in new cppcheck version
       AS_IF([$CPPCHECK --check-level=exhaustive conftest.cpp PTPM_DUMP_ALL],
             [AS_VAR_SET([CPPCHECK_OPT_BASE],
                         ["$CPPCHECK_OPT_BASE --check-level=exhaustive"])])])
AC_CHECK_TOOL([DOXYGEN], [doxygen])
AC_MSG_CHECKING([for doxygen version])
PTPM_VAR_SET_IF([DOXYGEN],
      [AS_VAR_SET([ptpm_doxygenver], ["`$DOXYGEN -v PTPM_DUMP_ERR`"])])
PTPM_MSG_RESULT([ptpm_doxygenver])
AC_MSG_CHECKING([for doxygen minimum version])
AS_VERSION_COMPARE([1.8], [$ptpm_doxygenver],
                   dnl above 1.8
                   [AS_VAR_COPY([DOXYGEN_MINVER], [ptpm_doxygenver])],
                   dnl equal 1.8
                   [AS_VAR_COPY([DOXYGEN_MINVER], [ptpm_doxygenver])])
PTPM_MSG_RESULT([DOXYGEN_MINVER])
AC_CHECK_TOOL([DOTTOOL], [dot])
AC_CHECK_TOOL([EPSTOPDF_TOOL], [epstopdf])
AC_CHECK_TOOL([VALGRIND], [valgrind])

AC_SUBST([PKG_CONFIG_DIR])
AC_SUBST([ASTYLE_MINVER])
AC_SUBST([DOXYGEN_MINVER])
AC_SUBST([CPPCHECK_OPT_BASE])

#----------------------------------------------------------------
# Compilation
AS_UNSET([USE_CROSS_COMPILE])
AS_UNSET([USE_CLANG_CPP_COMPILE])
AS_UNSET([USE_CLANG_C_COMPILE])
AS_UNSET([CC_VERSION])
AS_UNSET([CXX_VERSION])
AS_UNSET([CXX_COLOR_USE])
AS_UNSET([CXX_COLOR_NONE])
AS_UNSET([CXXFLAGS_PMC])
AS_UNSET([USE_ENDIAN])
AS_UNSET([HAVE_NETINET_IN_H])
AS_UNSET([HAVE_SYS_UN_H])
AS_UNSET([HAVE_GETOPT_H])
AS_UNSET([HAVE_SYS_TYPES_H])
AS_UNSET([HAVE_UNISTD_H])

AC_ARG_WITH([pmc-flags],
            [AS_HELP_STRING([--with-pmc-flags=flags],
                            [compilation flags for pmc tool])],
            [AS_VAR_COPY([CXXFLAGS_PMC], [withval])])

AS_IF([PTPM_GREP_TEST([CXX], ['clang'])],
      [AS_VAR_SET([USE_CLANG_CPP_COMPILE], ['use_clang'])])
AS_IF([PTPM_GREP_TEST([CC], ['clang'])],
      [AS_VAR_SET([USE_CLANG_C_COMPILE], ['use_clang'])])

AS_VAR_SET([CC_VERSION], ["`$CC -dumpversion PTPM_DUMP_ERR`"])
AS_VAR_SET([CXX_VERSION], ["`$CXX -dumpversion PTPM_DUMP_ERR`"])

AC_MSG_CHECKING([for cross compilation])
AS_VAR_IF([cross_compiling], ['yes'],
      [AS_VAR_SET([USE_CROSS_COMPILE], ['true'])])
PTPM_MSG_RESULT([USE_CROSS_COMPILE])

AC_MSG_CHECKING([compiler support colors])
dnl https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Message-Formatting-Options.html
AS_VAR_SET([ptpm_color], ['-fdiagnostics-color=always'])
AS_VAR_SET([ptpm_color_none], ['-fno-diagnostics-color'])
AS_VAR_APPEND([CPPFLAGS], [" $ptpm_color"])

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM],
   [AS_VAR_COPY([CXX_COLOR_USE], [ptpm_color])
    AS_VAR_COPY([CXX_COLOR_NONE], [ptpm_color_none])])
dnl https://clang.llvm.org/docs/UsersManual.html
PTPM_VAR_EMPTY_IF([CXX_COLOR_USE],
      [AS_VAR_SET([ptpm_color], ['-fcolor-diagnostics'])
       AS_VAR_SET([ptpm_color_none], ['-fno-color-diagnostics'])
       AS_VAR_SET([CPPFLAGS], ["$ptpm_keep_cppflags $ptpm_color"])
       AC_COMPILE_IFELSE(
          [AC_LANG_PROGRAM],
          [AS_VAR_COPY([CXX_COLOR_USE], [ptpm_color])
           AS_VAR_COPY([CXX_COLOR_NONE], [ptpm_color_none])])])

PTPM_MSG_RESULT([CXX_COLOR_USE])
dnl Restore C++ compiler flags
AS_VAR_COPY([CPPFLAGS], [ptpm_keep_cppflags])

dnl POSIX/GNU headers included by inner source code
AC_CHECK_HEADERS([byteswap.h dirent.h dlfcn.h endian.h fcntl.h pwd.h poll.h
   sys/ioctl.h sys/select.h sys/socket.h sys/time.h sys/timex.h
   net/if.h arpa/inet.h])
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([strtok_r strtok_s strnlen_s strerror_s strerrorlen_s])
AC_CONFIG_HEADERS([src/config.h])

dnl POSIX/GNU headers included by library public headers
AC_CHECK_HEADER([unistd.h], [AS_VAR_SET([HAVE_UNISTD_H], [1])])
AC_CHECK_HEADER([sys/types.h], [AS_VAR_SET([HAVE_SYS_TYPES_H], [1])])
AC_CHECK_HEADER([sys/un.h], [AS_VAR_SET([HAVE_SYS_UN_H], [1])])
AC_CHECK_HEADER([netinet/in.h], [AS_VAR_SET([HAVE_NETINET_IN_H], [1])])
AC_CHECK_HEADER([getopt.h], [AS_VAR_SET([HAVE_GETOPT_H], [1])])

AC_C_BIGENDIAN([AS_VAR_SET([USE_ENDIAN], ['big'])],
               [AS_VAR_SET([USE_ENDIAN], ['little'])],
               [AS_VAR_SET([USE_ENDIAN], ['unknown'])],
               [AS_VAR_SET([USE_ENDIAN], ['universal'])])

AC_SUBST([USE_CROSS_COMPILE])
AC_SUBST([USE_CLANG_CPP_COMPILE])
AC_SUBST([USE_CLANG_C_COMPILE])
AC_SUBST([CC_VERSION])
AC_SUBST([CXX_VERSION])
AC_SUBST([CXX_COLOR_USE])
AC_SUBST([CXX_COLOR_NONE])
AC_SUBST([CXXFLAGS_PMC])
AC_SUBST([USE_ENDIAN])
AC_SUBST([HAVE_NETINET_IN_H])
AC_SUBST([HAVE_SYS_UN_H])
AC_SUBST([HAVE_GETOPT_H])
AC_SUBST([HAVE_SYS_TYPES_H])
AC_SUBST([HAVE_UNISTD_H])

#----------------------------------------------------------------
# Selecting of init systems for Clock Manager proxy

AS_UNSET([USE_SYSTEMD])
AS_UNSET([USE_LSB_INIT])
AS_UNSET([USE_OPENRC])
AS_UNSET([USE_RUNIT])

# Install systemd clkmgr-proxy.service, clkmgr-proxy.socket
AC_ARG_ENABLE([systemd],
              [AS_HELP_STRING([--enable-systemd],
                              [Enable installing of systemd unit files])],
              [AS_VAR_COPY([USE_SYSTEMD], [enableval])])

# Install LSB init in /etc/init.d/clkmgr-proxy
AC_ARG_ENABLE([lsb-init],
              [AS_HELP_STRING([--enable-lsb-init],
                              [Enable installing of lsb init])],
              [AS_VAR_COPY([USE_LSB_INIT], [enableval])])

# Install OpenRC in /etc/init.d/clkmgr-proxy
AC_ARG_ENABLE([openrc],
              [AS_HELP_STRING([--enable-openrc],
                              [Enable installing of openrc init])],
              [AS_VAR_COPY([USE_OPENRC], [enableval])])

# OpenRC and LSB init can not coexist
AS_IF([PTPM_VAR_TEST([USE_LSB_INIT], ['yes']) &&\
       PTPM_VAR_TEST([USE_OPENRC], ['yes'])],
      [AC_MSG_ERROR([lsb-init and openrc can not be used together!])])

# Install Runit
dnl We use BusyBox default.
dnl Some system link /var/service to /service, which is Runit default.
AC_ARG_WITH([runit],
            [AS_HELP_STRING([--with-runit=path],
                            [Set Runit SVDIR, folder for services])],
            [AS_VAR_COPY([USE_RUNIT], [withval])])

AS_VAR_IF([USE_RUNIT], ['yes'],
          [AS_VAR_SET([USE_RUNIT], ['/var/service'])])

AC_SUBST([USE_SYSTEMD])
AC_SUBST([USE_LSB_INIT])
AC_SUBST([USE_OPENRC])
AC_SUBST([USE_RUNIT])

#----------------------------------------------------------------
# Testing tools
AS_UNSET([ptpm_skip_libsys])
AS_UNSET([HAVE_GTEST_HEADER])
AS_UNSET([GTEST_INC_FLAGS])
AS_UNSET([GTEST_LIB_FLAGS])
AS_UNSET([HAVE_GMOCK_HEADER])
AS_UNSET([HAVE_CRITERION_HEADER])
AS_UNSET([CRITERION_INC_FLAGS])
AS_UNSET([CRITERION_LIB_FLAGS])
AS_UNSET([SKIP_LIBSYS])

AC_ARG_ENABLE([libsys],
              [AS_HELP_STRING([--disable-libsys],
                              [Skip unit tests using libsys.so])],
              [AS_VAR_COPY([ptpm_skip_libsys], [enableval])])

AC_CHECK_HEADER([gtest/gtest.h],
                [AS_VAR_SET([HAVE_GTEST_HEADER], ['gtest/gtest.h'])])
dnl We do not use unit tests with cross compilation
AS_IF([PTPM_VAR_SET_TEST([HAVE_GTEST_HEADER]) &&\
       PTPM_VAR_EMPTY_TEST([USE_CROSS_COMPILE])],
      [PTPM_LIB_PKG_CONFIG([GTEST], [gtest])
       AC_CHECK_LIB([gtest], [main],
                    [PTPM_VAR_EMPTY_IF([GTEST_LIB_FLAGS],
                           [AS_VAR_SET([GTEST_LIB_FLAGS],
                                       ['-lgtest -lpthread'])])],
                    [AS_UNSET([GTEST_LIB_FLAGS])],
                    ["$GTEST_LIB_FLAGS"])])
AC_CHECK_HEADER([gmock/gmock.h],
                [AS_VAR_SET([HAVE_GMOCK_HEADER], ['gmock/gmock.h'])])

AC_CHECK_HEADER([criterion/criterion.h],
                [AS_VAR_SET([HAVE_CRITERION_HEADER], ['criterion/criterion.h'])])
AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM([
       $ptpm_c_include <stdint.h>
       $ptpm_c_include <$HAVE_CRITERION_HEADER>
       $ptpm_c_include <criterion/new/assert.h>
       $ptpm_c_include <criterion/new/memory.h>])],
   [AS_VAR_SET([HAVE_CRITERION_HEADER],
       ["stdint.h $HAVE_CRITERION_HEADER"])
    AS_VAR_APPEND([HAVE_CRITERION_HEADER],
       [" criterion/new/assert.h criterion/new/memory.h"])])
dnl We do not use unit test with cross compilation
AS_IF([PTPM_VAR_SET_TEST([HAVE_CRITERION_HEADER]) &&\
       PTPM_VAR_EMPTY_TEST([USE_CROSS_COMPILE])],
      [PTPM_LIB_PKG_CONFIG([CRITERION], [criterion])
       AC_CHECK_LIB([criterion], [main],
                    [PTPM_VAR_EMPTY_IF([CRITERION_LIB_FLAGS],
                           [AS_VAR_SET([CRITERION_LIB_FLAGS],
                                       ['-lcriterion'])])],
                    [AS_UNSET([CRITERION_LIB_FLAGS])],
                    ["$CRITERION_LIB_FLAGS"])])

dnl libsys is design after GNU libc, it breaks with clang
AS_VAR_IF([ptpm_skip_libsys], ['no'],
      [AS_VAR_SET([SKIP_LIBSYS], [1])])

AC_SUBST([HAVE_GTEST_HEADER])
AC_SUBST([GTEST_INC_FLAGS])
AC_SUBST([GTEST_LIB_FLAGS])
AC_SUBST([HAVE_GMOCK_HEADER])
AC_SUBST([HAVE_CRITERION_HEADER])
AC_SUBST([CRITERION_INC_FLAGS])
AC_SUBST([CRITERION_LIB_FLAGS])
AC_SUBST([SKIP_LIBSYS])

#----------------------------------------------------------------
# Address Sanitizer libraries
AS_UNSET([ASAN_LIBS])

AS_VAR_COPY([ptpm_keep_ldflags], [LDFLAGS])
AS_VAR_SET([ptpm_san], [m4_defn([ptpm_dqt])m4_map_args_w([address
            pointer-compare pointer-subtract undefined leak],
            [-fsanitize=], [], [ ])m4_defn([ptpm_dqt])])

AS_VAR_APPEND([CPPFLAGS], [" $ptpm_san"])
AS_VAR_APPEND([LDFLAGS], [" $ptpm_san"])

AC_MSG_CHECKING([for Address Sanitizer libraries])
AC_LINK_IFELSE(
   [AC_LANG_PROGRAM],
   [AS_VAR_SET([ASAN_LIBS],
               ["`$LDD conftest$EXEEXT | $GREP 'san.so' |\
                  $SED 's%=>.*%%;s%\n%%' | tr '\n' ' ' PTPM_DUMP_ERR`"])
    AC_MSG_RESULT([$ASAN_LIBS])],
   [AC_MSG_RESULT([no])])

dnl Restore C++ compiler and linker flags
AS_VAR_COPY([CPPFLAGS], [ptpm_keep_cppflags])
AS_VAR_COPY([LDFLAGS], [ptpm_keep_ldflags])

AC_SUBST([ASAN_LIBS])

#----------------------------------------------------------------
# HMAC libraries
AS_UNSET([HAVE_SSL_HEADER])
AS_UNSET([SSL_INC_FLAGS])
AS_UNSET([SSL_LIB_FLAGS])
AS_UNSET([HAVE_GCRYPT_HEADER])
AS_UNSET([GCRYPT_INC_FLAGS])
AS_UNSET([GCRYPT_LIB_FLAGS])
AS_UNSET([HAVE_GNUTLS_HEADER])
AS_UNSET([GNUTLS_INC_FLAGS])
AS_UNSET([GNUTLS_LIB_FLAGS])
AS_UNSET([HAVE_NETTLE_HEADER])
AS_UNSET([NETTLE_INC_FLAGS])
AS_UNSET([NETTLE_LIB_FLAGS])

AC_CHECK_HEADER([openssl/crypto.h], [AS_VAR_SET([HAVE_SSL_HEADER], [1])])
PTPM_VAR_SET_IF([HAVE_SSL_HEADER],
      [PTPM_LIB_PKG_CONFIG([SSL], [libcrypto])
       AC_CHECK_LIB([crypto], [main],
                    [PTPM_VAR_EMPTY_IF([SSL_LIB_FLAGS],
                           [AS_VAR_SET([SSL_LIB_FLAGS], ['-lcrypto'])])],
                    [AS_UNSET([HAVE_SSL_HEADER])])
       PTPM_CROSS([SSL_INC_FLAGS])
       PTPM_CROSS([SSL_LIB_FLAGS])])

AC_CHECK_HEADER([gcrypt.h], [AS_VAR_SET([HAVE_GCRYPT_HEADER], [1])])
PTPM_VAR_SET_IF([HAVE_GCRYPT_HEADER],
      [PTPM_LIB_PKG_CONFIG([GCRYPT], [libgcrypt])
       AC_CHECK_LIB([crypto], [main],
                    [PTPM_VAR_EMPTY_IF([GCRYPT_LIB_FLAGS],
                           [AS_VAR_SET([GCRYPT_LIB_FLAGS], ['-lgcrypt'])])],
                    [AS_UNSET([HAVE_GCRYPT_HEADER])])
       PTPM_CROSS([GCRYPT_INC_FLAGS])
       PTPM_CROSS([GCRYPT_LIB_FLAGS])])

AC_CHECK_HEADER([gnutls/crypto.h], [AS_VAR_SET([HAVE_GNUTLS_HEADER], [1])])
PTPM_VAR_SET_IF([HAVE_GNUTLS_HEADER],
      [PTPM_LIB_PKG_CONFIG([GNUTLS], [gnutls])
       AC_CHECK_LIB([gnutls], [gnutls_global_init],
                    [PTPM_VAR_EMPTY_IF([GNUTLS_LIB_FLAGS],
                           [AS_VAR_SET([GNUTLS_LIB_FLAGS], ['-lgnutls'])])],
                    [AS_UNSET([HAVE_GNUTLS_HEADER])])
       PTPM_CROSS([GNUTLS_INC_FLAGS])
       PTPM_CROSS([GNUTLS_LIB_FLAGS])])

AC_CHECK_HEADER([nettle/hmac.h], [AS_VAR_SET([HAVE_NETTLE_HEADER], [1])])
PTPM_VAR_SET_IF([HAVE_NETTLE_HEADER],
      [PTPM_LIB_PKG_CONFIG([NETTLE], [nettle])
       AC_CHECK_LIB([nettle], [main],
                    [PTPM_VAR_EMPTY_IF([NETTLE_LIB_FLAGS],
                           [AS_VAR_SET([NETTLE_LIB_FLAGS], ['-lnettle'])])],
                    [AS_UNSET([HAVE_NETTLE_HEADER])])
       PTPM_CROSS([NETTLE_INC_FLAGS])
       PTPM_CROSS([NETTLE_LIB_FLAGS])])

AC_SUBST([HAVE_SSL_HEADER])
AC_SUBST([SSL_INC_FLAGS])
AC_SUBST([SSL_LIB_FLAGS])
AC_SUBST([HAVE_GCRYPT_HEADER])
AC_SUBST([GCRYPT_INC_FLAGS])
AC_SUBST([GCRYPT_LIB_FLAGS])
AC_SUBST([HAVE_GNUTLS_HEADER])
AC_SUBST([GNUTLS_INC_FLAGS])
AC_SUBST([GNUTLS_LIB_FLAGS])
AC_SUBST([HAVE_NETTLE_HEADER])
AC_SUBST([NETTLE_INC_FLAGS])
AC_SUBST([NETTLE_LIB_FLAGS])

#----------------------------------------------------------------
# Version
AS_UNSET([PACKAGE_VERSION_MAJ])
AS_UNSET([PACKAGE_VERSION_MIN])
AS_UNSET([PACKAGE_VERSION_VAL])

AC_MSG_CHECKING([for package version string])
PTPM_MSG_RESULT([PACKAGE_VERSION])
PTPM_SOURCE([./tools/version])
AC_MSG_CHECKING([for package version major])
AS_VAR_COPY([PACKAGE_VERSION_MAJ], [ver_maj])
PTPM_MSG_RESULT([PACKAGE_VERSION_MAJ])
AC_MSG_CHECKING([for package version minor])
AS_VAR_COPY([PACKAGE_VERSION_MIN], [ver_min])
PTPM_MSG_RESULT([PACKAGE_VERSION_MIN])
AC_MSG_CHECKING([for package version value])
AS_VAR_SET([PACKAGE_VERSION_VAL], ["`printf '0x%.2x%.2x' $ver_maj $ver_min`"])
PTPM_MSG_RESULT([PACKAGE_VERSION_VAL])
AC_SUBST([PACKAGE_VERSION_MAJ])
AC_SUBST([PACKAGE_VERSION_MIN])
AC_SUBST([PACKAGE_VERSION_VAL])

#----------------------------------------------------------------
# Automake flags
AS_UNSET([ptpm_use_deps])
AS_UNSET([ptpm_disable_silent])
AS_UNSET([USE_DEPS])
AS_UNSET([DO_VERBOSE])

AC_ARG_ENABLE([dependency-tracking],
              [AS_HELP_STRING([--disable-dependency-tracking],
                              [Skip creating dependencies durig build])],
              [AS_VAR_COPY([ptpm_use_deps], [enableval])])
AC_ARG_ENABLE([silent-rules],
              [AS_HELP_STRING([--disable-silent-rules], [use verbose output])],
              [AS_VAR_COPY([ptpm_disable_silent], [enableval])])
AC_ARG_ENABLE([maintainer-mode],
              [AS_HELP_STRING([--enable-maintainer-mode], [ignored])])

PTPM_VAR_ELSE_IF([ptpm_use_deps], ['no'],
                 [AS_VAR_SET([USE_DEPS], [1])])
AS_VAR_IF([ptpm_disable_silent], ['no'],
          [AS_VAR_SET([DO_VERBOSE], ['true'])])

AC_SUBST([USE_DEPS])
AC_SUBST([DO_VERBOSE])

#----------------------------------------------------------------
# Clock Manager
AS_UNSET([SKIP_CLKMGR])
AS_UNSET([HAVE_LIBCHRONY_HEADER])
AS_UNSET([CHRONY_LIB_FLAGS])
AS_UNSET([CHRONY_INC_FLAGS])

AC_ARG_ENABLE([clkmgr],
            [AS_HELP_STRING([--disable-clkmgr], [Skip Clock Manager build])],
            [AS_VAR_COPY([ptpm_with_clkmgr], [enableval])])

AS_VAR_IF([ptpm_with_clkmgr], ['no'],
          [AS_VAR_SET([SKIP_CLKMGR], [1])])

AC_CHECK_HEADER([chrony.h], [AS_VAR_SET([HAVE_LIBCHRONY_HEADER], [1])])
PTPM_VAR_SET_IF([HAVE_LIBCHRONY_HEADER],
          [AS_VAR_SET([CHRONY_LIB_FLAGS], [-lchrony])])

AC_SUBST([SKIP_CLKMGR])
AC_SUBST([HAVE_LIBCHRONY_HEADER])
AC_SUBST([CHRONY_LIB_FLAGS])
AC_SUBST([CHRONY_INC_FLAGS])

#----------------------------------------------------------------
# SWIG
AS_UNSET([ptpm_swigbin])
AS_UNSET([ptpm_with_alllang])
AS_UNSET([ptpm_swigver])
AS_VAR_SET([ptpm_alllang_default], ['no'])
AS_UNSET([SWIG])
AS_UNSET([SWIG_MINVER])
AS_UNSET([SWIGLIB])
AS_UNSET([SWIG_ARGCARGV_GO])

AC_ARG_WITH([swig],
            [AS_HELP_STRING([--without-swig], [Disable swig])]
            [AS_HELP_STRING([--with-swig=path],
                            [Set location of swig executable])],
            [AS_VAR_COPY([ptpm_swigbin], [withval])],
            [AS_VAR_SET([ptpm_swigbin], ['yes'])])

AC_ARG_ENABLE([alllang],
            [AS_HELP_STRING([--disable-alllang], [Disable all languages])],
            [AS_VAR_COPY([ptpm_with_alllang], [enableval])])

AS_VAR_IF([ptpm_swigbin], ['no'],
  [AC_MSG_NOTICE([Disabling swig])],
  [AS_VAR_IF([ptpm_swigbin], ['yes'],
         [AC_CHECK_TOOL([SWIG], [swig])],
         [AS_VAR_COPY([SWIG], [ptpm_swigbin])])
   AC_MSG_CHECKING([for swig version])
   AS_VAR_SET([ptpm_swigver],
              ["`$SWIG -version | $GREP Version | $SED 's%.* %%' PTPM_DUMP_ERR`"])
   PTPM_MSG_RESULT([ptpm_swigver])
   AC_MSG_CHECKING([for swig minimum version])
   AS_VERSION_COMPARE([4.1], [$ptpm_swigver],
                      [AS_VAR_COPY([SWIG_MINVER], [ptpm_swigver])], dnl above 4.1
                      [AS_VAR_COPY([SWIG_MINVER], [ptpm_swigver])]) dnl equal 4.1

   PTPM_MSG_RESULT([SWIG_MINVER])
   AC_MSG_CHECKING([for swig library])
   AS_VAR_SET([SWIGLIB], ["`$SWIG -swiglib PTPM_DUMP_ERR`"])
   PTPM_MSG_RESULT([SWIGLIB])

   PTPM_VAR_SET_IF([SWIGLIB],
         [AC_MSG_CHECKING([for swig argcargv])
          PTPM_NOT_FILE_IF(["$SWIGLIB/go/argcargv.i"],
                [AS_VAR_SET([SWIG_ARGCARGV_GO], ['true'])])
          PTPM_MSG_RESULT([SWIG_ARGCARGV_GO])])
   AS_IF([PTPM_VAR_SET_TEST([SWIG_MINVER]) &&\
          PTPM_VAR_ELSE_TEST([ptpm_with_alllang], ['no'])],
         [AS_VAR_SET([ptpm_alllang_default], ['yes'])])])

AC_MSG_CHECKING([for build wrapper])
PTPM_MSG_RESULT([ptpm_alllang_default])

AC_SUBST([SWIG])
AC_SUBST([SWIG_MINVER])
AC_SUBST([SWIGLIB])
AC_SUBST([SWIG_ARGCARGV_GO])

#----------------------------------------------------------------
# Tcl
dnl Configured parameters
AS_UNSET([ptpm_tclpackage])
AS_UNSET([ptpm_with_tclconfig])
AS_UNSET([ptpm_tclincdir])
dnl Probed parameters
AS_UNSET([ptpm_tclconfig])
AS_UNSET([ptpm_tclver])
AS_UNSET([TCLSH])
AS_UNSET([TCL_MINVER])
AS_UNSET([TCL_INC_FLAGS])
AS_UNSET([TCL_SITE])

AC_ARG_WITH([tcl],
            [AS_HELP_STRING([--without-tcl], [Disable Tcl])]
            [AS_HELP_STRING([--with-tcl=path],
                            [Set location of Tcl package])],
            [AS_VAR_COPY([ptpm_tclpackage], [withval])],
            [AS_VAR_COPY([ptpm_tclpackage], [ptpm_alllang_default])])
AC_ARG_WITH([tclconfig],
            [AS_HELP_STRING([--with-tclconfig=path],
                            [Set location of tclConfig.sh])],
            [AS_VAR_COPY([ptpm_with_tclconfig],[withval])])
AC_ARG_WITH([tclincl],
            [AS_HELP_STRING([--with-tclincl=path],
                            [Set location of Tcl include directory])],
            [AS_VAR_COPY([ptpm_tclincdir], [withval])])
AC_ARG_WITH([tcllib],
            [AS_HELP_STRING([--with-tcllib=path], [ignored])])

AS_VAR_IF([ptpm_tclpackage], ['no'],
  [AC_MSG_NOTICE([Disabling Tcl])],
  [AC_CHECK_TOOL([TCLSH], [tclsh])
   AC_MSG_CHECKING([for Tcl configuration])

   dnl First check to use --with-tclconfig value
   PTPM_VAR_SET_IF([ptpm_with_tclconfig],
     [PTPM_FILE_IF(["$ptpm_with_tclconfig/tclConfig.sh"],
            [AS_VAR_COPY([ptpm_tclconfig], [ptpm_with_tclconfig])],
            [AC_MSG_ERROR([$with_tcl folder does not contain tclConfig.sh])])])
   PTPM_VAR_SET_IF([ptpm_tclincdir],
     [PTPM_DIR_IF(["$ptpm_tclincdir"],
            [AS_VAR_SET([TCL_INC_FLAGS], ["-I$ptpm_tclincdir"])],
            [AC_MSG_ERROR([$ptpm_tclincdir folder for headers does not exist])])])

   dnl Use pkg-config to fetch TCL_INC_FLAGS
   AS_IF([PTPM_VAR_EMPTY_TEST([ptpm_tclconfig]) &&\
          PTPM_VAR_EMPTY_TEST([TCL_INC_FLAGS])],
         [PTPM_VAR_SET_IF([TCLSH],
                        [AS_VAR_SET([ptpm_tclver],
                                    [`AS_ECHO_N(['puts $tcl_version']) |\
                                      $TCLSH PTPM_DUMP_ERR`])])
          PTPM_LIB_PKG_CONFIG([TCL], [tcl$ptpm_tclver])
          AS_IF([PTPM_VAR_EMPTY_TEST([TCL_INC_FLAGS]) &&\
                 PTPM_VAR_SET_TEST([ptpm_tclver])],
                [PTPM_LIB_PKG_CONFIG([TCL], [tcl])])])

   dnl Second try find config on TCL libdir
   AS_IF([PTPM_VAR_EMPTY_TEST([ptpm_tclconfig]) &&\
          PTPM_VAR_SET_TEST([TCLSH])],
     [AS_VAR_SET([ptpm_tclconfig],
                 ["`AS_ECHO_N(['puts [[::tcl::pkgconfig get libdir,install]]']) |\
                    $TCLSH PTPM_DUMP_ERR`"])
      PTPM_CROSS([ptpm_tclconfig])
      PTPM_NOT_FILE_IF(["$ptpm_tclconfig/tclConfig.sh"],
                   [AS_UNSET([ptpm_tclconfig])])])

   dnl check in a few common install locations, as last resort
   PTPM_VAR_EMPTY_IF([ptpm_tclconfig],
     [AS_UNSET([ptpm_dirs])
      PTPM_FOR([d], [$ptpm_lib_dirs],
               [AS_VAR_APPEND([ptpm_dirs], [" $d*/ $d*/tcl*/"])])
      PTPM_FOR([d], [$ptpm_dirs],
        [PTPM_FOR([i], [`ls -d -r $d PTPM_DUMP_ERR`],
           [PTPM_FILE_IF(["${i}tclConfig.sh"],
                  [AS_VAR_COPY([ptpm_tclconfig], [i])
                   PTPM_FOR_BREAK])])])])

   dnl Use parameters from tclConfig.sh
   PTPM_VAR_EMPTY_IF([ptpm_tclconfig],
     [AC_MSG_RESULT([no])],
     [AC_MSG_RESULT([found $ptpm_tclconfig/tclConfig.sh])
      PTPM_SOURCE(["$ptpm_tclconfig/tclConfig.sh"])

      dnl Use the tclConfig.sh value
      PTPM_VAR_EMPTY_IF([TCL_INC_FLAGS],
                        [AS_VAR_COPY([TCL_INC_FLAGS], [TCL_INCLUDE_SPEC])])
      dnl Use TCL package folder
      AS_IF([PTPM_VAR_EMPTY_TEST([TCL_INC_FLAGS]) &&\
             PTPM_VAR_ELSE_TEST([ptpm_tclpackage], ['yes'])],
            [PTPM_DIR_IF(["$ptpm_tclpackage/include"],
                         [AS_VAR_SET([TCL_INC_FLAGS],
                                     ["-I$ptpm_tclpackage/include"])])])
      dnl Fall-back and look for tcl headers in common locations
      PTPM_VAR_EMPTY_IF([TCL_INC_FLAGS],
            [PTPM_FOR([d], [$ptpm_include_dirs],
               [PTPM_FILE_IF(["$d/tcl.h"],
                  [AS_VAR_SET([TCL_INC_FLAGS], ["-I$d"])
                   PTPM_FOR_BREAK])
                dnl Header may be inside a tcl folder
                PTPM_FOR([i], [$d/tcl*],
                  [PTPM_FILE_IF(["$i/tcl.h"],
                     [AS_VAR_SET([TCL_INC_FLAGS], ["-I$i"])
                      PTPM_FOR_BREAK([2])])])])])

      dnl We do not need TCL library for linking our TCL wrapper library
      dnl As we do not call any TCL code, only TCL call our library!
      dnl AS_VAR_COPY([TCL_LIB_FLAGS], [TCL_LIB_SPEC])

      PTPM_VAR_EMPTY_IF([ptpm_tclver],
            [AS_VAR_COPY([ptpm_tclver], [TCL_VERSION])])
      PTPM_VAR_SET_IF([TCL_PACKAGE_PATH],
        [PTPM_VAR_SED_COPY([ptpm_tcl_package_path],
                           [TCL_PACKAGE_PATH], ['s%:% %g'])
         PTPM_CROSS([ptpm_tcl_package_path])
         dnl Try first folder under main libdir
         PTPM_FOR([d], [$ptpm_tcl_package_path],
           [AS_IF([PTPM_GREP_TEST([d], ["^$ptpm_libdir"])],
                  [AS_VAR_COPY([TCL_SITE], [d])
                   PTPM_FOR_BREAK])])
         dnl Try first folder with architecture
         PTPM_VAR_EMPTY_IF([TCL_SITE],
           [PTPM_FOR([d], [$ptpm_tcl_package_path],
              [AS_IF([PTPM_GREP_TEST([d], ["$host_cpu"])],
                     [AS_VAR_COPY([TCL_SITE], [d])
                      PTPM_FOR_BREAK])])])
         dnl Try folder which match tclConfig folder
         PTPM_VAR_EMPTY_IF([TCL_SITE],
               [PTPM_FOR([d], [$ptpm_tcl_package_path],
                  [AS_VAR_IF([d], ["^$ptpm_tclconfig"],
                         [AS_VAR_COPY([TCL_SITE], [d])
                         PTPM_FOR_BREAK])])])
         dnl Just take the first folder as fall-back
         PTPM_VAR_EMPTY_IF([TCL_SITE],
               [PTPM_FOR([d], [$ptpm_tcl_package_path],
                  [AS_VAR_COPY([TCL_SITE], [d])
                   PTPM_FOR_BREAK])])])])

   AC_MSG_CHECKING([for Tcl version])
   PTPM_VAR_SET_IF([ptpm_tclver],
         [AS_VERSION_COMPARE([8.0], [$ptpm_tclver],
                             dnl above 8.0
                             [AS_VAR_COPY([TCL_MINVER], [ptpm_tclver])],
                             dnl equal 8.0
                             [AS_VAR_COPY([TCL_MINVER], [ptpm_tclver])])])
   PTPM_MSG_RESULT([TCL_MINVER])
   AC_MSG_CHECKING([for Tcl header files])
   PTPM_MSG_RESULT([TCL_INC_FLAGS])
   AC_MSG_CHECKING([for Tcl package path])
   PTPM_CROSS([TCL_SITE])
   PTPM_MSG_RESULT([TCL_SITE])])

AC_SUBST([TCLSH])
AC_SUBST([TCL_MINVER])
AC_SUBST([TCL_INC_FLAGS])
AC_SUBST([TCL_SITE])

#----------------------------------------------------------------
# Python 3
AS_VAR_SET([ptpm_py3majorver], [0])
AS_UNSET([PY3_BIN])
AS_UNSET([PY3_CFG])
AS_UNSET([PY3_VER])
AS_UNSET([PY3_SITE])
AS_UNSET([PY3_INC_FLAGS])
AS_UNSET([PY3_LIB_FLAGS])
AS_UNSET([PY3_EXT])

AC_ARG_WITH([python3],
            [AS_HELP_STRING([--without-python3], [Disabling Python 3])]
            [AS_HELP_STRING([--with-python3=path],
                            [Set location of Python 3 executable])],
            [AS_VAR_COPY([ptpm_py3bin], [withval])],
            [AS_VAR_COPY([ptpm_py3bin], [ptpm_alllang_default])])

AS_VAR_IF([ptpm_py3bin], ['no'],
  [AC_MSG_NOTICE([Disabling python3 probe])],
  [AS_VAR_IF([ptpm_py3bin], ['yes'],
     [AS_VAR_SET([ptpm_py_vers], [3])
      PTPM_LOOP_RANGE([d], [15], [-1], [2],
                      [AS_VAR_APPEND([ptpm_py_vers], [" 3.$d"])])
      PTPM_FOR([d], [$ptpm_py_vers ''],
        [AC_CHECK_TOOLS([PY3_BIN], [python$d])
         PTPM_VAR_SET_IF([PY3_BIN],
               [AC_CHECK_TOOLS([PY3_CFG], [$PY3_BIN-config])
                PTPM_VAR_SET_IF([PY3_CFG], [PTPM_FOR_BREAK])])])],
     [AS_IF([$ptpm_py3bin --version | $GREP -i Python PTPM_DUMP_ALL],
            [AS_VAR_COPY([PY3_BIN], [ptpm_py3bin])
             AC_CHECK_TOOLS([PY3_CFG], [$PY3_BIN-config])],
            [AC_MSG_ERROR([$ptpm_py3bin python tool is unavailable])])])

   PTPM_VAR_SET_IF([PY3_BIN],
         [AC_MSG_CHECKING([for $PY3_BIN major version number])
          AS_VAR_SET([ptpm_py3majorver],
                     ["`$PY3_BIN -c "import sys;\
                        print(sys.version_info.major)" PTPM_DUMP_ERR`"])
          AC_MSG_RESULT([$ptpm_py3majorver])])
   PTPM_VAR_EMPTY_IF([ptpm_py3majorver], [AS_VAR_SET([ptpm_py3majorver], [0])])

   AS_IF([PTPM_GREATER_OR_EQUAL_TEST([ptpm_py3majorver], [3]) &&\
          PTPM_VAR_SET_TEST([PY3_CFG])],
     [AC_MSG_CHECKING([for python3 version])
      AS_VAR_SET([PY3_VER],
                 ["`$PY3_BIN -c "import sys;print('%d.%d' %\
                    (sys.version_info.major, sys.version_info.minor))"\
                    PTPM_DUMP_ERR`"])
      PTPM_MSG_RESULT([PY3_VER])
      AS_VAR_SET([ptpm_py3verkey], ["python$PY3_VER"])

      AS_VAR_SET([ptpm_dirs],
                 ["`$PY3_BIN -c "import site;print('\n'.join(\
                    site.getsitepackages()))" PTPM_DUMP_ERR`"])
      PTPM_CROSS([ptpm_dirs])
      AC_MSG_CHECKING([for python3 site folder])
      PTPM_FOR([d], [$ptpm_dirs],
        [AS_IF([PTPM_GREP_TEST([d], ["^$ptpm_libdir"]) &&\
                PTPM_GREP_TEST([d], [$ptpm_py3verkey])],
               [AS_VAR_COPY([PY3_SITE], [d])
                PTPM_FOR_BREAK])])
      PTPM_VAR_EMPTY_IF([PY3_SITE],
            [PTPM_FOR([d], [$ptpm_dirs],
               [AS_IF([PTPM_NOT_GREP_TEST([d], ['/local/']) &&\
                       PTPM_GREP_TEST([d], [$ptpm_py3verkey])],
                      [AS_VAR_COPY([PY3_SITE], [d])
                       PTPM_FOR_BREAK])])])
      PTPM_VAR_EMPTY_IF([PY3_SITE],
            [PTPM_FOR([d], [$ptpm_dirs],
               [AS_IF([PTPM_GREP_TEST([d], ["^$ptpm_libdir"])],
                      [AS_VAR_COPY([PY3_SITE], [d])
                       PTPM_FOR_BREAK])])])
      PTPM_MSG_RESULT([PY3_SITE])

      AC_MSG_CHECKING([for python3 header files])
      PTPM_LIB_PKG_CONFIG([PY3], [python3])
      PTPM_VAR_EMPTY_IF([RUBY_INC_FLAGS],
            [AS_VAR_SET([PY3_INC_FLAGS],
                        ["`$PY3_CFG --includes PTPM_DUMP_ERR`"])])
      PTPM_CROSS([PY3_INC_FLAGS])
      PTPM_MSG_RESULT([PY3_INC_FLAGS])

      AC_MSG_CHECKING([for python3 linking])
      AS_VAR_SET([PY3_LIB_FLAGS], ["`$PY3_CFG --libs PTPM_DUMP_ERR`"])
      PTPM_CROSS([PY3_LIB_FLAGS])
      PTPM_MSG_RESULT([PY3_LIB_FLAGS])

      AC_MSG_CHECKING([for python3 extension suffix])
      AS_VAR_SET([PY3_EXT], ["`$PY3_CFG --extension-suffix PTPM_DUMP_ERR`"])
      PTPM_CROSS([PY3_EXT])
      PTPM_MSG_RESULT([PY3_EXT])])])

AC_SUBST([PY3_BIN])
AC_SUBST([PY3_CFG])
AC_SUBST([PY3_VER])
AC_SUBST([PY3_SITE])
AC_SUBST([PY3_INC_FLAGS])
AC_SUBST([PY3_LIB_FLAGS])
AC_SUBST([PY3_EXT])

#----------------------------------------------------------------
# Perl5
AS_UNSET([ptpm_perlbin])
AS_UNSET([PERL])
AS_UNSET([PERL5_VER])
AS_UNSET([PERL5_SITE])
AS_UNSET([PERL5_INC_FLAGS])
AS_UNSET([PERL5_HAVE_TEST])
AS_UNSET([PERL5_HAVE_TOUCH])

AC_ARG_WITH([perl5],
            [AS_HELP_STRING([--without-perl5], [Disable Perl5])]
            [AS_HELP_STRING([--with-perl5=path],
                            [Set location of Perl5 executable])],
            [AS_VAR_COPY([ptpm_perlbin], [withval])],
            [AS_VAR_SET([ptpm_perlbin], ['yes'])])

AS_VAR_IF([ptpm_perlbin], ['no'],
  [AC_MSG_NOTICE([Disabling Perl5])],
  [AS_VAR_IF([ptpm_perlbin], ['yes'],
         [AC_CHECK_TOOLS([PERL], [perl perl5])],
         [AS_VAR_COPY([PERL], [ptpm_perlbin])])
   AC_MSG_CHECKING([for Perl5 confiuration])
   PTPM_VAR_SET_IF([PERL],
     [AS_VAR_SET([PERL5_VER],
                 ["`$PERL -e 'print "$^V"' | $SED 's%^v%%' PTPM_DUMP_ERR`"])
      AS_VAR_SET([ptpm_perl_dir],
                 ["`$PERL -MConfig -le 'print $Config{archlibexp}'\
                    PTPM_DUMP_ERR`"])
      AS_IF([PTPM_SYMBOLIC_LINK_TEST(["$ptpm_perl_dir"]) &&\
             PTPM_VAR_SET_TEST([READLINK])],
            [AS_VAR_SET([PERL5_SITE],
                        ["`$READLINK -f $ptpm_perl_dir PTPM_DUMP_ERR`"])
             PTPM_VAR_EMPTY_IF([PERL5_SITE],
                   [AS_VAR_SET([PERL5_SITE],["$ptpm_perl_dir"])])],
            [AS_VAR_SET([PERL5_SITE],["$ptpm_perl_dir"])])
      PTPM_CROSS([PERL5_SITE])
      PTPM_MSG_RESULT([PERL5_SITE])

      AC_MSG_CHECKING([for Perl5 compilation flags])
      AS_IF([$PERL -e 'use ExtUtils::Embed' PTPM_DUMP_ALL],
            [PTPM_FOR([d], [`$PERL -MExtUtils::Embed -e ccopts PTPM_DUMP_ERR`],
                      [AS_CASE([$d],
                               [-D*],
                               [AS_VAR_APPEND([PERL5_INC_FLAGS], [" $d"])],
                               [-I*],
                               [AS_VAR_APPEND([PERL5_INC_FLAGS], [" $d"])])])])
      AS_IF([PTPM_VAR_EMPTY_TEST([PERL5_INC_FLAGS]) &&\
             PTPM_VAR_SET_TEST([PERL5_SITE])],
            [PTPM_FOR([d], ["$PERL5_SITE" "$PERL5_SITE/CORE"],
               [PTPM_FILE_IF(["$d/perl.h"],
                      [AS_VAR_SET([PERL5_INC_FLAGS],
                                  ["-I$d"])
                       PTPM_FOR_BREAK])])])
      PTPM_CROSS([PERL5_INC_FLAGS])
      PTPM_MSG_RESULT([PERL5_INC_FLAGS])

      AS_IF([$PERL -e 'use Test::Class;Test::More' PTPM_DUMP_ALL],
            [AS_VAR_SET([PERL5_HAVE_TEST], [1])])
      AS_IF([$PERL -e 'use File::Touch' PTPM_DUMP_ALL],
            [AS_VAR_SET([PERL5_HAVE_TOUCH], [1])])],
     [AC_MSG_RESULT([not available])])])

AC_SUBST([PERL])
AC_SUBST([PERL5_VER])
AC_SUBST([PERL5_SITE])
AC_SUBST([PERL5_INC_FLAGS])
AC_SUBST([PERL5_HAVE_TEST])
AC_SUBST([PERL5_HAVE_TOUCH])

#----------------------------------------------------------------
# Ruby
AS_UNSET([ptpm_rubybin])
AS_UNSET([RUBY])
AS_UNSET([RUBY_VER])
AS_UNSET([RUBY_SITE])
AS_UNSET([RUBY_INC_FLAGS])
AS_UNSET([RUBY_LIB_FLAGS])

AC_ARG_WITH([ruby],
            [AS_HELP_STRING([--without-ruby], [Disable Ruby])]
            [AS_HELP_STRING([--with-ruby=path],
                            [Set location of Ruby executable])],
            [AS_VAR_COPY([ptpm_rubybin], [withval])],
            [AS_VAR_COPY([ptpm_rubybin], [ptpm_alllang_default])])

dnl configuration comes from rbconfig.rb
AS_VAR_IF([ptpm_rubybin], ['no'],
  [AC_MSG_NOTICE([Disabling Ruby])],
  [AS_VAR_IF([ptpm_rubybin], ['yes'],
         [AC_CHECK_TOOLS([RUBY], [ruby])],
         [AS_VAR_COPY([RUBY], [ptpm_rubybin])])
   AC_MSG_CHECKING([for Ruby version])
   PTPM_VAR_SET_IF([RUBY],
     [AS_VAR_SET([RUBY_VER],
                 ["`$RUBY -e 'v=RUBY_VERSION;print v' PTPM_DUMP_ERR`"])
      PTPM_MSG_RESULT([RUBY_VER])
      AS_VAR_SET([ptpm_ruby_mver],
                 ["`$RUBY -e 'v=RUBY_VERSION.match %r%^(\d+\.\d+)%'\
                          -e 'print v[[1]]' PTPM_DUMP_ERR`"])
      AC_MSG_CHECKING([for Ruby site])
      PTPM_VAR_PKG_CONFIG([RUBY_SITE], [vendorarchdir], [ruby-$ptpm_ruby_mver])
      PTPM_VAR_EMPTY_IF([RUBY_SITE],
                [PTPM_VAR_PKG_CONFIG([RUBY_SITE], [vendorarchdir], [ruby])])
      PTPM_VAR_EMPTY_IF([RUBY_SITE],
                [AS_VAR_SET([RUBY_SITE],
                            ["`$RUBY -rrbconfig -e\
                              'print RbConfig::CONFIG[["vendorarchdir"]]'\
                               PTPM_DUMP_ERR`"])])
      PTPM_CROSS([RUBY_SITE])
      PTPM_MSG_RESULT([RUBY_SITE])

      dnl try using pkg-config,
      PTPM_LIB_PKG_CONFIG([RUBY], [ruby-$ptpm_ruby_mver])
      AS_IF([PTPM_VAR_EMPTY_TEST([RUBY_INC_FLAGS]) &&\
             PTPM_VAR_EMPTY_TEST([RUBY_LIB_FLAGS])],
            [PTPM_LIB_PKG_CONFIG([RUBY], [ruby])])

      dnl If we do not have pkg config, fall back
      AS_IF([PTPM_VAR_EMPTY_TEST([RUBY_INC_FLAGS]) &&\
             PTPM_VAR_EMPTY_TEST([RUBY_LIB_FLAGS])],
            [AS_VAR_SET([ptpm_rubyhdrdir],
                        ["`$RUBY -rrbconfig -e\
                          'print RbConfig::CONFIG[["rubyhdrdir"]]'\
                          PTPM_DUMP_ERR`"])
             AS_IF([PTPM_VAR_SET_TEST([ptpm_rubyhdrdir]) &&\
                    PTPM_FILE_TEST(["$ptpm_rubyhdrdir/ruby.h"])],
                   [AS_VAR_SET([RUBY_INC_FLAGS],
                               ["-I$ptpm_rubyhdrdir"])])
             AS_VAR_SET([ptpm_rubyarchhdrdir],
                        ["`$RUBY -rrbconfig -e\
                          'print RbConfig::CONFIG[["rubyarchhdrdir"]]'\
                           PTPM_DUMP_ERR`"])
             PTPM_CROSS([ptpm_rubyarchhdrdir])
             AS_IF([PTPM_VAR_SET_TEST([ptpm_rubyarchhdrdir]) &&\
                    PTPM_VAR_SET_TEST([RUBY_INC_FLAGS])],
                   [AS_VAR_SET([RUBY_INC_FLAGS],
                               ["-I$ptpm_rubyarchhdrdir $RUBY_INC_FLAGS"])])
             dnl Set Library flags
             AS_VAR_SET([RUBY_LIB_FLAGS],
                        ["`$RUBY -rrbconfig -e\
                          'print RbConfig::CONFIG[["LIBRUBYARG_SHARED"]]'\
                           PTPM_DUMP_ERR`"])
             AS_VAR_APPEND([RUBY_LIB_FLAGS],
                        [" `$RUBY -rrbconfig -e\
                           'print RbConfig::CONFIG[["LIBS"]]' PTPM_DUMP_ERR`"])])
      PTPM_CROSS([RUBY_INC_FLAGS])
      PTPM_CROSS([RUBY_LIB_FLAGS])
      AS_VAR_APPEND([CPPFLAGS], [" $RUBY_INC_FLAGS"])
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([$ptpm_c_include <ruby.h>])], [],
                        [AS_UNSET([RUBY_INC_FLAGS])
                         AS_UNSET([RUBY_LIB_FLAGS])])
      AS_VAR_COPY([CPPFLAGS], [ptpm_keep_cppflags])
      AC_MSG_CHECKING([for Ruby header files])
      PTPM_MSG_RESULT([RUBY_INC_FLAGS])
      AC_MSG_CHECKING([for Ruby link flags])
      PTPM_MSG_RESULT([RUBY_LIB_FLAGS])],
     [AC_MSG_RESULT([ruby is not installed])])])

AC_SUBST([RUBY])
AC_SUBST([RUBY_VER])
AC_SUBST([RUBY_SITE])
AC_SUBST([RUBY_INC_FLAGS])
AC_SUBST([RUBY_LIB_FLAGS])

#----------------------------------------------------------------
# PHP
AS_UNSET([ptpm_phpbin])
AS_UNSET([ptpm_phpdir])
AS_UNSET([PHP])
AS_UNSET([PHP_CFG])
AS_UNSET([PHP_VER])
AS_UNSET([PHP_SITE])
AS_UNSET([PHP_INCDIR])
AS_UNSET([PHP_INC_FLAGS])
AS_UNSET([PHP_UNIT])

AC_ARG_WITH([php],
            [AS_HELP_STRING([--without-php], [Disable PHP])]
            [AS_HELP_STRING([--with-php=path], [Set location of PHP executable])],
            [AS_VAR_COPY([ptpm_phpbin], [withval])],
            [AS_VAR_COPY([ptpm_phpbin], [ptpm_alllang_default])])

AS_VAR_IF([ptpm_phpbin], ['no'],
  [AC_MSG_NOTICE([Disabling PHP])],
  [AS_VAR_IF([ptpm_phpbin], ['yes'],
         [AS_UNSET([ptpm_php_vers])
          PTPM_LOOP_RANGE([d], [4], [-1], [0],
                          [AS_VAR_APPEND([ptpm_php_vers], [" php8.$d"])])
          AS_VAR_APPEND([ptpm_php_vers],
                        [" php7.4 php7.3 php7.2 php7.1 php7.0 php"])
          AC_CHECK_TOOLS([PHP], [$ptpm_php_vers])],
         [AS_VAR_COPY([PHP], [ptpm_phpbin])])
   PTPM_VAR_SET_IF([PHP],
     [AS_CASE([$PHP],
              [php7.*|php8.*],
              [PTPM_VAR_SED_COPY([ptpm_php_cfgver], [PHP],
                                 ['s%[[78]]\..*$%-config&%'])],
              [AS_VAR_SET([ptpm_php_cfgver], ["$PHP-config"])])

      AC_MSG_CHECKING([for PHP include folder])
      AS_VAR_SET([ptpm_dirs],
                 ["`$PHP -r 'print get_include_path();' | $SED 's%:% %g'\
                    PTPM_DUMP_ERR`"])
      PTPM_FOR([d], [$ptpm_dirs],
        [PTPM_VAR_ELSE_IF([d], ['.'],
               [AS_VAR_COPY([ptpm_phpdir], [d])
                PTPM_FOR_BREAK])])
      PTPM_MSG_RESULT([ptpm_phpdir])

      AC_CHECK_TOOLS([PHP_CFG], [$ptpm_php_cfgver php-config])
      PTPM_VAR_SET_IF([PHP_CFG],
        [AC_MSG_CHECKING([for PHP version])
         AS_VAR_SET([PHP_VER], ["`$PHP_CFG --version PTPM_DUMP_ERR`"])
         PTPM_MSG_RESULT([PHP_VER])

         AC_MSG_CHECKING([for PHP extensions])
         AS_VAR_SET([PHP_SITE], ["`$PHP_CFG --extension-dir PTPM_DUMP_ERR`"])
         PTPM_MSG_RESULT([PHP_SITE])

         AC_MSG_CHECKING([for PHP headers base folder])
         AS_VAR_SET([PHP_INCDIR], ["`$PHP_CFG --include-dir PTPM_DUMP_ERR`"])
         PTPM_MSG_RESULT([PHP_INCDIR])

         AC_MSG_CHECKING([for PHP header files])
         AS_CASE([$PHP_VER],
                 [7.*|8.*],
                 [AS_VAR_SET([ptpm_phpinc_flags],
                             ["`$PHP_CFG --includes PTPM_DUMP_ERR`"])
                  AS_VAR_SET([ptpm_sub], ['${PHP_INCDIR}'])
                  PTPM_VAR_SET_IF([ptpm_phpinc_flags],
                        [PTPM_VAR_SED_COPY([PHP_INC_FLAGS], [ptpm_phpinc_flags],
                                           ["s%$PHP_INCDIR%$ptpm_sub%g"])])
                  PTPM_MSG_RESULT([PHP_INC_FLAGS])],
                 [''],
                 [AS_VAR_SET([msg], ["could not find $PHP_CFG or obtain"])
                  AC_MSG_RESULT([$msg PHP version from it])],
                 [AC_MSG_RESULT([found PHP $PHP_VER - not PHP 7 or 8])])])

      AC_CHECK_TOOLS([PHP_UNIT], [phpunit])])])

AC_SUBST([PHP])
AC_SUBST([PHP_CFG])
AC_SUBST([PHP_VER])
AC_SUBST([PHP_SITE])
AC_SUBST([PHP_INCDIR])
AC_SUBST([PHP_INC_FLAGS])
AC_SUBST([PHP_UNIT])

#----------------------------------------------------------------
# Lua
AS_UNSET([ptpm_luabin])
AS_UNSET([ptpm_luainclude])
AS_UNSET([ptpm_lualib])
AS_UNSET([LUA_BIN])
AS_UNSET([LUA_VER])
AS_UNSET([LUA_INC_FLAGS])
AS_UNSET([LUA_LIB_FLAGS])
AS_UNSET([LUA_UNIT])
AS_UNSET([LUA_VERS])
AS_UNSET([LUA_5_1_INC_FLAGS])
AS_UNSET([LUA_5_2_INC_FLAGS])
AS_UNSET([LUA_5_3_INC_FLAGS])
AS_UNSET([LUA_5_4_INC_FLAGS])
AS_UNSET([LUA_5_5_INC_FLAGS])
AS_UNSET([LUA_5_1_LIB_FLAGS])
AS_UNSET([LUA_5_2_LIB_FLAGS])
AS_UNSET([LUA_5_3_LIB_FLAGS])
AS_UNSET([LUA_5_4_LIB_FLAGS])
AS_UNSET([LUA_5_5_LIB_FLAGS])
AS_UNSET([LUA_UNIT_VERS])

AC_ARG_WITH([lua],
            [AS_HELP_STRING([--without-lua], [Disable Lua])]
            [AS_HELP_STRING([--with-lua=path],
                            [Set location of Lua executable])],
            [AS_VAR_COPY([ptpm_luabin], [withval])],
            [AS_VAR_COPY([ptpm_luabin], [ptpm_alllang_default])])
AC_ARG_WITH([luaincl],
            [AS_HELP_STRING([--with-luaincl=path],
                            [Set location of Lua include directory])],
            [AS_VAR_COPY([ptpm_luainclude], [withval])])
AC_ARG_WITH([lualib],
            [AS_HELP_STRING([--with-lualib=path],
                            [Set location of Lua library directory])],
            [AS_VAR_COPY([ptpm_lualib], [withval])])

AS_VAR_IF([ptpm_luabin], ['no'],
  [AC_MSG_NOTICE([Disabling Lua])],
  [AS_VAR_IF([ptpm_luabin], ['yes'],
     [AS_UNSET([ptpm_lua_vers])
      PTPM_LOOP_RANGE([d], [5], [-1], [1],
                      [AS_VAR_APPEND([ptpm_lua_vers], [" lua5.$d"])])
      AC_CHECK_TOOLS([LUA_BIN], [$ptpm_lua_vers lua])],
     [AS_IF([$ptpm_luabin -v | $GREP -i Lua PTPM_DUMP_ALL],
            [AS_VAR_COPY([LUA_BIN], [ptpm_luabin])],
            [AC_MSG_ERROR([$ptpm_luabin lua tool is unavailable])])])

   dnl Try to probe all Lua versions
   dnl Some systems like Debian have several
   AS_IF([PTPM_VAR_EMPTY_TEST([ptpm_luainclude]) &&\
          PTPM_VAR_EMPTY_TEST([ptpm_lualib])],
         [dnl First try package config
          PTPM_LOOP_RANGE([d], [1], [1], [5],
               [AS_UNSET([ptpm_tmp_INC_FLAGS])
                AS_UNSET([ptpm_tmp_LIB_FLAGS])
                PTPM_LIB_PKG_CONFIG([ptpm_tmp], [lua5.$d])
                AS_IF([PTPM_VAR_SET_TEST([ptpm_tmp_INC_FLAGS]) ||\
                       PTPM_VAR_SET_TEST([ptpm_tmp_LIB_FLAGS])],
                      [AS_VAR_APPEND([LUA_VERS], [" 5.$d"])
                       AS_VAR_COPY([LUA_5_${d}_INC_FLAGS],
                                   [ptpm_tmp_INC_FLAGS])
                       AS_VAR_COPY([LUA_5_${d}_LIB_FLAGS],
                                   [ptpm_tmp_LIB_FLAGS])])])
          dnl Search for headers, in case we do not have a package config
          PTPM_FOR([d], [$ptpm_include_dirs],
            [PTPM_FOR([i], [$d/lua5.*],
               [PTPM_FILE_IF(["$i/lua.h"],
                  [PTPM_VAR_SED_COPY([n], [i], ["s%$d/lua%%"])
                   AS_IF([PTPM_NOT_IN_LIST_TEST([n], [LUA_VERS])],
                                  [PTPM_VAR_SED_COPY([j], [n], ['s%\.%_%'])
                                   AS_VAR_SET([LUA_${j}_INC_FLAGS], ["-I$i"])
                                   AS_VAR_SET([LUA_${j}_LIB_FLAGS], ["-llua$n"])
                                   AS_VAR_APPEND([LUA_VERS], [" $n"])])])])])
          dnl Find luaunit for all lua found
          PTPM_FOR([d], [$LUA_VERS],
                   [AS_IF([lua$d -e "require 'luaunit'" PTPM_DUMP_ALL],
                          [AS_VAR_APPEND([LUA_UNIT_VERS], [" $d"])])])])

   PTPM_VAR_SET_IF([LUA_BIN],
     [AS_VAR_SET([lua_version],
                 ["`$LUA_BIN -e 'print(string.match(_VERSION, "%d+[.]%d+"))'\
                     PTPM_DUMP_ERR`"])
      PTPM_VAR_SET_IF([LUA_VERS],
        [AS_IF([PTPM_NOT_IN_LIST_TEST([lua_version], [LUA_VERS])],
               [PTPM_VAR_SED_COPY([j], [lua_version], ['s%\.%_%'])
                AS_UNSET([ptpm_tmp_INC_FLAGS])
                AS_UNSET([ptpm_tmp_LIB_FLAGS])
                AS_UNSET([ptpm_lua_found])
                PTPM_LIB_PKG_CONFIG([ptpm_tmp], [lua])
                dnl Search for headers, in case we do not have a package config
                AS_IF([PTPM_VAR_SET_TEST([ptpm_tmp_INC_FLAGS]) ||\
                       PTPM_VAR_SET_TEST([ptpm_tmp_LIB_FLAGS])],
                      [AS_VAR_COPY([LUA_${j}_INC_FLAGS],
                                   [ptpm_tmp_INC_FLAGS])
                       AS_VAR_COPY([LUA_${j}_LIB_FLAGS],
                                   [ptpm_tmp_LIB_FLAGS])
                       AS_VAR_APPEND([LUA_VERS], [" $lua_version"])
                       AS_VAR_SET([ptpm_lua_found], [1])],
                      dnl Search for headers
                      dnl  in case we do not have a package config
                      [PTPM_FOR([d], [$ptpm_include_dirs],
                                [PTPM_FILE_IF(["$d/lua.h"],
                                   [AS_VAR_SET([LUA_${j}_INC_FLAGS], ["-I$d"])
                                    AS_VAR_SET([LUA_${j}_LIB_FLAGS], ['-llua'])
                                    AS_VAR_APPEND([LUA_VERS], [" $lua_version"])
                                    AS_VAR_SET([ptpm_lua_found], [1])
                                    PTPM_FOR_BREAK])])])
                AS_IF([PTPM_VAR_SET_TEST([ptpm_lua_found]) &&\
                       $LUA_BIN -e "require 'luaunit'" PTPM_DUMP_ALL],
                      [AS_VAR_APPEND([LUA_UNIT_VERS], [" $lua_version"])])])],
        [AC_MSG_CHECKING([Lua version])
         AS_VAR_SET([LUA_VER],["$lua_version"])
         PTPM_MSG_RESULT([LUA_VER])

         AS_CASE([$LUA_VER],
                 [5.*], [],
                 [AS_VAR_SET([msg], ['Unsupport Lua version'])
                  AC_MSG_WARN([$msg this version of Lua])
                  AS_UNSET([LUA_BIN])])

         dnl Set Cflags
         PTPM_VAR_SET_IF([LUA_BIN],
            [PTPM_LIB_PKG_CONFIG([LUA], [lua])
             AC_MSG_CHECKING([for lua.h location])
             PTPM_VAR_SET_IF([ptpm_luainclude],
                [AC_CHECK_FILE([$ptpm_luainclude/lua.h],
                   [AS_VAR_SET([LUA_INC_FLAGS], ["-I$ptpm_luainclude"])
                    AC_MSG_RESULT([$ptpm_luainclude/lua.h])],
                   [AS_UNSET([LUA_BIN])
                    AC_MSG_RESULT([configured 'luaincl' path does not exist])])],
                [PTPM_VAR_EMPTY_IF([LUA_INC_FLAGS],
                   [AC_MSG_RESULT([Search header on system])
                    AC_CHECK_HEADER([lua.h],
                      [AC_MSG_NOTICE([Header on system])],
                      dnl Some systems like Debian
                      dnl  use /usr/include/lua5.n/lua.h
                      [AC_MSG_CHECKING([Search lua.h on all folsers])
                       AS_UNSET([ptpm_dirs])
                       PTPM_FOR([d], [$ptpm_include_dirs],
                         [AS_VAR_APPEND([ptpm_dirs],
                                        [" $d $d/lua$LUA_VER"])])
                       PTPM_FOR([d], [$ptpm_dirs],
                         [PTPM_FILE_IF(["$d/lua.h"],
                                [AC_MSG_RESULT([$d/lua.h])
                                 AS_VAR_SET([LUA_INC_FLAGS],
                                            ["-I$d"])
                                 PTPM_FOR_BREAK])])
                       PTPM_VAR_EMPTY_IF([LUA_INC_FLAGS],
                             [AC_MSG_RESULT([not found])
                              AS_UNSET([LUA_BIN])])])])])])

         dnl Set libs flags
         PTPM_VAR_SET_IF([LUA_BIN],
           [PTPM_VAR_SET_IF([ptpm_lualib],
                  [AC_CHECK_FILE([$ptpm_lualib/liblua.so],
                                 [LUA_LIB_FLAGS="-L$ptpm_lualib -llua"],
                                 [AS_UNSET([LUA_BIN])])],
                  [PTPM_VAR_EMPTY_IF([LUA_LIB_FLAGS],
                             [AC_SEARCH_LIBS([lua_close],
                                             [lua lua$LUA_VER],
                                             [LUA_LIB_FLAGS="-l$ac_lib"],
                                             [AS_UNSET([LUA_BIN])])])])])

         PTPM_VAR_SET_IF([LUA_BIN],
                         dnl Check if we have Lua unit test
                         [AS_IF([$LUA_BIN -e "require 'luaunit'"\
                                 PTPM_DUMP_ALL],
                                [AS_VAR_SET([LUA_UNIT], [1])])],
                         dnl Clear flags on any error
                         [AS_UNSET([LUA_INC_FLAGS])
                          AS_UNSET([LUA_LIB_FLAGS])])])])])

AC_SUBST([LUA_BIN])
AC_SUBST([LUA_VER])
AC_SUBST([LUA_INC_FLAGS])
AC_SUBST([LUA_LIB_FLAGS])
AC_SUBST([LUA_UNIT])
AC_SUBST([LUA_VERS])
AC_SUBST([LUA_5_1_INC_FLAGS])
AC_SUBST([LUA_5_2_INC_FLAGS])
AC_SUBST([LUA_5_3_INC_FLAGS])
AC_SUBST([LUA_5_4_INC_FLAGS])
AC_SUBST([LUA_5_5_INC_FLAGS])
AC_SUBST([LUA_5_1_LIB_FLAGS])
AC_SUBST([LUA_5_2_LIB_FLAGS])
AC_SUBST([LUA_5_3_LIB_FLAGS])
AC_SUBST([LUA_5_4_LIB_FLAGS])
AC_SUBST([LUA_5_5_LIB_FLAGS])
AC_SUBST([LUA_UNIT_VERS])

#----------------------------------------------------------------
# golang
AS_UNSET([ptpm_go])
AS_UNSET([ptpm_gover])
AS_UNSET([GO_BIN])
AS_UNSET([GO_MINVER])
AS_UNSET([GOROOT])
AS_UNSET([GOARCH])

AC_ARG_WITH([go],
            [AS_HELP_STRING([--without-go], [Disable Go])]
            [AS_HELP_STRING([--with-go=path],
                            [Set location of Go executable])],
            [AS_VAR_COPY([ptpm_go], [withval])],
            [AS_VAR_COPY([ptpm_go], [ptpm_alllang_default])])

AS_VAR_IF([ptpm_go], ['no'],
  [AC_MSG_NOTICE([Disabling Go])],
  [AS_VAR_IF([ptpm_go], ['yes'],
         [AC_CHECK_TOOL([GO_BIN], [go])],
         [AS_VAR_COPY([GO_BIN], [ptpm_go])])
   PTPM_VAR_SET_IF([GO_BIN],
     [AS_VAR_SET([GOROOT],
                 ["`$GO_BIN env GOROOT`"])
      AS_VAR_SET([ptpm_gover],
                 ["`$GO_BIN version |\
                    $SED 's%^go version go%%;s% .*%%' PTPM_DUMP_ERR`"])
      AS_VERSION_COMPARE([1.18], [$ptpm_gover],
                         [AS_VAR_COPY([GO_MINVER], [ptpm_gover])], dnl above 1.18
                         [AS_VAR_COPY([GO_MINVER], [ptpm_gover])]) dnl equal 1.18

      PTPM_VAR_SET_IF([USE_CROSS_COMPILE],
            [AS_VAR_SET([GOARCH], ["`./tools/goarch.sh "$host_cpu"`"])])])])

AC_SUBST([GO_BIN])
AC_SUBST([GO_MINVER])
AC_SUBST([GOROOT])
AC_SUBST([GOARCH])

#----------------------------------------------------------------
# Report wrapper languages
AS_UNSET([SKIP_TCL])
AS_UNSET([SKIP_PYTHON3])
AS_UNSET([SKIP_PERL5])
AS_UNSET([SKIP_RUBY])
AS_UNSET([SKIP_PHP])
AS_UNSET([SKIP_LUA])
AS_UNSET([SKIP_GO])

PTPM_VAR_SET_IF([SWIG_MINVER],
  [AS_IF([PTPM_VAR_EMPTY_TEST([TCL_INC_FLAGS]) ||\
          PTPM_VAR_EMPTY_TEST([TCL_MINVER]) ||\
          PTPM_VAR_EMPTY_TEST([TCL_SITE])],
         [AS_VAR_SET([SKIP_TCL], [1])])
   AS_IF([PTPM_VAR_EMPTY_TEST([PY3_SITE]) ||\
          PTPM_VAR_EMPTY_TEST([PY3_INC_FLAGS]) ||\
          PTPM_VAR_EMPTY_TEST([PY3_LIB_FLAGS]) ||\
          PTPM_VAR_EMPTY_TEST([PY3_EXT])],
         [AS_VAR_SET([SKIP_PYTHON3], [1])])
   AS_IF([PTPM_VAR_EMPTY_TEST([PERL5_SITE]) ||\
          PTPM_VAR_EMPTY_TEST([PERL5_INC_FLAGS])],
         [AS_VAR_SET([SKIP_PERL5], [1])])
   AS_IF([PTPM_VAR_EMPTY_TEST([RUBY_INC_FLAGS]) &&\
          PTPM_VAR_EMPTY_TEST([RUBY_LIB_FLAGS])],
         [AS_VAR_SET([SKIP_RUBY], [1])])
   AS_IF([PTPM_VAR_EMPTY_TEST([PHP_INCDIR]) ||\
          PTPM_VAR_EMPTY_TEST([PHP_INC_FLAGS]) ||\
          PTPM_VAR_EMPTY_TEST([ptpm_phpdir])],
         [AS_VAR_SET([SKIP_PHP], [1])])
   AS_IF([PTPM_VAR_EMPTY_TEST([LUA_BIN]) &&\
          PTPM_VAR_EMPTY_TEST([LUA_VERS])],
         [AS_VAR_SET([SKIP_LUA], [1])])
   PTPM_VAR_EMPTY_IF([GO_MINVER], [AS_VAR_SET([SKIP_GO], [1])])],
  [AS_VAR_SET([SKIP_TCL], [1])
   AS_VAR_SET([SKIP_PYTHON3], [1])
   AS_VAR_SET([SKIP_PERL5], [1])
   AS_VAR_SET([SKIP_RUBY], [1])
   AS_VAR_SET([SKIP_PHP], [1])
   AS_VAR_SET([SKIP_LUA], [1])
   AS_VAR_SET([SKIP_GO], [1])])

AC_SUBST([SKIP_TCL])
AC_SUBST([SKIP_PYTHON3])
AC_SUBST([SKIP_PERL5])
AC_SUBST([SKIP_RUBY])
AC_SUBST([SKIP_PHP])
AC_SUBST([SKIP_LUA])
AC_SUBST([SKIP_GO])

#----------------------------------------------------------------
# End
AC_OUTPUT
