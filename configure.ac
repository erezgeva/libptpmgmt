# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */
#
# @file
# @brief configuration
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#

AC_COPYRIGHT(Copyright © 2022 Erez Geva)
AC_PREREQ([2.69])
m4_define([ptpm_VER], m4_esyscmd_s([. ./tools/version && echo "$ver_maj.$ver_min"]))
AC_INIT([libptpmgmt], m4_defn([ptpm_VER]), [],
        [libptpmgmt-]m4_defn([ptpm_VER]).txz,
        [https://libptpmgmt.nwtime.org])
AC_CONFIG_FILES([defs.mk])
AC_CONFIG_SRCDIR([src/mngIds.m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_LANG([C++])
AC_PROG_CXX
AC_PROG_GREP
AC_PROG_EGREP
AC_PROG_FGREP
AC_PROG_RANLIB
AC_PROG_SED
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_LN_S

#----------------------------------------------------------------
# globals
#----------------------------------------------------------------
AS_VAR_SET([ptpm_include_dirs],
           ["$includedir /usr/local/include /usr/include /opt/local/include"])

#----------------------------------------------------------------
# Macroes
#----------------------------------------------------------------
m4_define([ptpm_result],
          [AS_IF([test -z "${$1}"], [AC_MSG_RESULT([not found])],
          [AC_MSG_RESULT([${$1}])])])
m4_define([ptpm_basename],
          [AS_VAR_SET([$1], [`expr "X$2" : 'X.*/\(.*\)'`])])

#----------------------------------------------------------------
# Tools
#----------------------------------------------------------------
AS_UNSET([ASTYLEVER])
AS_UNSET([ASTYLEMINVER])
AS_UNSET([DOXYGENVER])
AS_UNSET([DOXYGENMINVER])

AC_CHECK_TOOLS([M4], [m4 autom4te])
AC_CHECK_TOOLS([CTAGS], [ctags ctags-exuberant])
AC_CHECK_TOOL([PKG_CONFIG], [pkg-config])
AC_CHECK_TOOL([ASTYLE], [astyle])
AC_MSG_CHECKING([for astyle version])
AS_IF([test -n "$ASTYLE"],
      [AS_VAR_SET([astylever], [`$ASTYLE -V`])
       for d in `$ASTYLE -V`; do :;done
       AS_VAR_SET([ASTYLEVER], ["$d"])])
ptpm_result([ASTYLEVER])
AC_MSG_CHECKING([for astyle minimum version])
AS_VERSION_COMPARE([3.1], [$ASTYLEVER],
                   [AS_VAR_SET([ASTYLEMINVER], [$ASTYLEVER])], # above 3.1
                   [AS_VAR_SET([ASTYLEMINVER], [$ASTYLEVER])]) # equal 3.1
ptpm_result([ASTYLEMINVER])
AC_CHECK_TOOL([CPPCHECK], [cppcheck])
AC_CHECK_TOOL([DOXYGEN], [doxygen])
AC_MSG_CHECKING([for doxygen version])
AS_IF([test -n "$DOXYGEN"],
      [AS_VAR_SET([DOXYGENVER], [`$DOXYGEN -v`])])
ptpm_result([DOXYGENVER])
AC_MSG_CHECKING([for doxygen minimum version])
AS_VERSION_COMPARE([1.8], [$DOXYGENVER],
                   [AS_VAR_SET([DOXYGENMINVER], [$DOXYGENVER])], # above 1.8
                   [AS_VAR_SET([DOXYGENMINVER], [$DOXYGENVER])]) # equal 1.8
ptpm_result([DOXYGENMINVER])
AC_CHECK_TOOL([DOTTOOL], [dot])
AC_CHECK_TOOL([VALGRIND], [valgrind])

AC_SUBST([ASTYLEVER])
AC_SUBST([ASTYLEMINVER])
AC_SUBST([DOXYGENVER])
AC_SUBST([DOXYGENMINVER])

#----------------------------------------------------------------
# cross compilation
#----------------------------------------------------------------
AS_UNSET([USE_CROSS_COMPILE])
AC_MSG_CHECKING([for cross compilation])
AS_IF([test "x$cross_compiling" = xyes],
      [AS_VAR_SET([USE_CROSS_COMPILE], [true])])
ptpm_result([USE_CROSS_COMPILE])
AC_SUBST([USE_CROSS_COMPILE])

m4_define([ptpm_cross],
          [AS_IF([test -n "$USE_CROSS_COMPILE" &&\
                  (echo ${$1} | $GREP "$build_cpu" >/dev/null)],
          [AS_VAR_SET([$1],
                      [`echo ${$1} | $SED "s%$build_cpu%$host_cpu%"`])])])

#----------------------------------------------------------------
# Miscellaneous
#----------------------------------------------------------------

# Root directory
AC_MSG_CHECKING([for project root folder])
AS_VAR_SET([ROOT_DIR], [`pwd`])
ptpm_result([ROOT_DIR])
AC_SUBST([ROOT_DIR])

AS_UNSET([CXXFLAGS_PMC])
AC_ARG_WITH([pmc-flags],
            [AS_HELP_STRING([--with-pmc-flags=flags],
                            [compilation flags for pmc tool])],
            [AS_VAR_SET([CXXFLAGS_PMC], ["$withval"])])
AC_SUBST([CXXFLAGS_PMC])

AS_UNSET([USE_FULL_PATH_LINK])
AC_ARG_ENABLE([full-link],
              [AS_HELP_STRING([--enable-full-link],
                              [Use full path in symbolic link])],
              [AS_VAR_SET([USE_FULL_PATH_LINK], ["true"])])
AC_SUBST([USE_FULL_PATH_LINK])

AS_UNSET([HAVE_METHODS_COMPARE])
AS_VAR_SET([ptpm_method_warn], [pmf-conversions])
AS_VAR_SET([METHODS_COMPARE_FLAGS], ["-W$ptpm_method_warn"])
AS_VAR_SET([ptpm_keep_cppflags], ["$CPPFLAGS"])
AS_VAR_SET([CPPFLAGS], ["$CPPFLAGS -Wno-$ptpm_method_warn"])

AC_MSG_CHECKING([whether compiler can compare inherited class methods])
AC_RUN_IFELSE(
   [AC_LANG_PROGRAM(
    [struct B{virtual void f1(void){} virtual void f2(void){}};
     struct C:public B{virtual void f1(void){}};],
    [C c; return !((void*)(&B::f1) == (void*)(c.*(&B::f1))) &&
     ((void*)(&B::f2) == (void*)(c.*(&B::f2))) ? 0 : -1])],
   [AS_VAR_SET([HAVE_METHODS_COMPARE],[yes]) # success run
    AC_MSG_RESULT([$HAVE_METHODS_COMPARE])],
   [AC_MSG_RESULT([no])],                    # failure
   [AS_VAR_SET([HAVE_METHODS_COMPARE],[cross]) # cross compilation
    AC_MSG_RESULT([$HAVE_METHODS_COMPARE])])

AS_VAR_SET([CPPFLAGS], [$ptpm_keep_cppflags])
AC_SUBST([HAVE_METHODS_COMPARE])
AC_SUBST([METHODS_COMPARE_FLAGS])

#----------------------------------------------------------------
# Testing tools
#----------------------------------------------------------------
AS_UNSET([HAVE_GTEST_HEADER])
AS_UNSET([GTEST_INC_FLAGS])
AS_UNSET([GTEST_LIB_FLAGS])

AC_CHECK_HEADER([gtest/gtest.h],
                [AS_VAR_SET([HAVE_GTEST_HEADER], [gtest/gtest.h])])
AS_IF([test -n "$HAVE_GTEST_HEADER" && test -n "$PKG_CONFIG"],
      [AS_VAR_SET([GTEST_INC_FLAGS], [`$PKG_CONFIG --cflags gtest`])
       AS_VAR_SET([GTEST_LIB_FLAGS], [`$PKG_CONFIG --libs gtest`])
       AC_CHECK_LIB([gtest], [main],
                    [AS_IF([test -z "$GTEST_LIB_FLAGS"],
                           [AS_VAR_SET([GTEST_LIB_FLAGS],
                                       ['-lgtest -lpthread'])])],
                    [AS_UNSET([GTEST_LIB_FLAGS])],
                    ["$GTEST_LIB_FLAGS"])])

AC_SUBST([HAVE_GTEST_HEADER])
AC_SUBST([GTEST_INC_FLAGS])
AC_SUBST([GTEST_LIB_FLAGS])

#----------------------------------------------------------------
# Find Address Sanitizer libraries
#----------------------------------------------------------------
AS_UNSET([ASAN_LIBS])

AS_VAR_SET([ptpm_keep_cppflags], [$CPPFLAGS])
AS_VAR_SET([ptpm_keep_ldflags], [$LDFLAGS])

for n in address pointer-compare pointer-subtract undefined leak; do
   AS_VAR_SET([CPPFLAGS], ["$CPPFLAGS -fsanitize=$n"])
   AS_VAR_SET([LDFLAGS], ["$LDFLAGS -fsanitize=$n"])
done

AC_MSG_CHECKING([for Address Sanitizer libraries])
AC_LINK_IFELSE(
   [AC_LANG_PROGRAM([],[])],
   [AS_VAR_SET([ASAN_LIBS],
               [`ldd conftest$EXEEXT | $GREP 'san.so' | $SED 's/=>.*//;s/\n//' |\
                 tr '\n' ' ' `])
    AC_MSG_RESULT([$ASAN_LIBS])],
   [AC_MSG_RESULT([no])])

AS_VAR_SET([CPPFLAGS], [$ptpm_keep_cppflags])
AS_VAR_SET([LDFLAGS], [$ptpm_keep_ldflags])

AC_SUBST([ASAN_LIBS])

#----------------------------------------------------------------
# JSON libraries
#----------------------------------------------------------------

AS_UNSET([HAVE_JSONC_LIB])
AS_UNSET([JSONC_LIB_FLAGS])
AS_UNSET([HAVE_FJSON_LIB])
AS_UNSET([FJSON_LIB_FLAGS])

AC_ARG_WITH([json],
            [AS_HELP_STRING([--without-json],
                            [Skip using all json libraries])],
            [AS_VAR_SET([skip_json], ["$withval"])])
AC_ARG_WITH([jsonc],
            [AS_HELP_STRING([--without-jsonc], [Skip using jsonc library])],
            [AS_VAR_SET([skip_jsonc], ["$withval"])])
AC_ARG_WITH([fjson],
            [AS_HELP_STRING([--without-fjson],
                            [Skip using libfastjson library])],
            [AS_VAR_SET([skip_fsonc], ["$withval"])])

AS_IF([test "x$skip_json" = xno],
      [AS_VAR_SET([skip_jsonc], ["no"])
       AS_VAR_SET([skip_fsonc], ["no"])])

AS_IF([test "x$skip_jsonc" != xno],
      [AC_CHECK_HEADER([json-c/json.h],
                       [AS_VAR_SET([HAVE_JSONC_LIB], [json-c/json.h])])
       AS_IF([test -n "$HAVE_JSONC_LIB"],
             [AC_CHECK_LIB([json-c], [json_c_version],
                           [AS_VAR_SET([JSONC_LIB_FLAGS], ["-ljson-c"])],
                           [AS_UNSET([HAVE_JSONC_LIB])])])])

AS_IF([test "x$skip_fsonc" != xno],
      [AC_CHECK_HEADER([libfastjson/json.h],
                       [AS_VAR_SET([HAVE_FJSON_LIB], [libfastjson/json.h])])
       AS_VAR_SET([FJSON_LIB_FLAGS], ["-lfastjson"])])

AC_SUBST([HAVE_JSONC_LIB])
AC_SUBST([JSONC_LIB_FLAGS])
AC_SUBST([HAVE_FJSON_LIB])
AC_SUBST([FJSON_LIB_FLAGS])

#----------------------------------------------------------------
# Version
#----------------------------------------------------------------
AS_UNSET([PACKAGE_VERSION_MAJ])
AS_UNSET([PACKAGE_VERSION_MIN])
AS_UNSET([PACKAGE_VERSION_VAL])

. ./tools/version
AC_MSG_CHECKING([for package version string])
ptpm_result([PACKAGE_VERSION])
AC_MSG_CHECKING([for package version major])
AS_VAR_SET([PACKAGE_VERSION_MAJ], [$ver_maj])
ptpm_result([PACKAGE_VERSION_MAJ])
AC_MSG_CHECKING([for package version minor])
AS_VAR_SET([PACKAGE_VERSION_MIN], [$ver_min])
ptpm_result([PACKAGE_VERSION_MIN])
AC_MSG_CHECKING([for package version value])
AS_VAR_SET([PACKAGE_VERSION_VAL], [`printf '0x%.2x%.2x' $ver_maj $ver_min`])
ptpm_result([PACKAGE_VERSION_VAL])
AC_SUBST([PACKAGE_VERSION_MAJ])
AC_SUBST([PACKAGE_VERSION_MIN])
AC_SUBST([PACKAGE_VERSION_VAL])

#----------------------------------------------------------------
# Ignore automake flags passed by Debian
#----------------------------------------------------------------
AC_ARG_ENABLE([dependency-tracking],
              [AS_HELP_STRING([--disable-dependency-tracking], [ignored])])
AC_ARG_ENABLE([maintainer-mode],
              [AS_HELP_STRING([--enable-maintainer-mode], [ignored])])
AC_ARG_ENABLE([silent-rules],
              [AS_HELP_STRING([--disable-silent-rules], [ignored])])

#----------------------------------------------------------------
# Look for swig
#----------------------------------------------------------------
AS_UNSET([SWIG])
AS_UNSET([SWIGBIN])
AS_UNSET([SWIGVER])
AS_UNSET([SWIGMINVER])
AS_UNSET([SWIGLIB])
AS_UNSET([SWIGARGCARGV])
AS_UNSET([SWIGARGCARGV_GO])

AC_ARG_WITH([swig],
            [AS_HELP_STRING([--without-swig], [Disable swig])]
[AS_HELP_STRING([--with-swig=path], [Set location of swig executable])],
            [AS_VAR_SET([SWIGBIN], ["$withval"])],
            [AS_VAR_SET([SWIGBIN], [yes])])

if test "x$SWIGBIN" = xno; then
  AC_MSG_NOTICE([Disabling swig])
else
  AS_IF([test "x$SWIGBIN" = xyes],
        [AC_CHECK_TOOL([SWIG], [swig])],
        [AS_VAR_SET([SWIG], ["$SWIGBIN"])])
  AC_MSG_CHECKING([for swig version])
  AS_VAR_SET([SWIGVER], [`$SWIG -version | $GREP Version | $SED 's%.* %%'`])
  ptpm_result([SWIGVER])
  AC_MSG_CHECKING([for swig minimum version])
  AS_VERSION_COMPARE([3.0], [$SWIGVER],
                     [AS_VAR_SET([SWIGMINVER], [$SWIGVER])], # above 3.0
                     [AS_VAR_SET([SWIGMINVER], [$SWIGVER])]) # equal 3.0

  ptpm_result([SWIGMINVER])
  AC_MSG_CHECKING([for swig library])
  AS_VAR_SET([SWIGLIB], [`$SWIG -swiglib`])
  ptpm_result([SWIGLIB])

  AS_IF([test -n "$SWIGLIB"],
        [AC_MSG_CHECKING([for swig argcargv])
         for n in lua perl5 php tcl; do
           AS_IF([test ! -r $SWIGLIB/$n/argcargv.i],
                 [AS_VAR_SET([SWIGARGCARGV], [true])
                  break])
         done
         ptpm_result([SWIGARGCARGV])])
  AS_IF([test -n "$SWIGLIB"],
        [AC_MSG_CHECKING([for swig argcargv])
         AS_IF([test ! -r $SWIGLIB/go/argcargv.i],
               [AS_VAR_SET([SWIGARGCARGV_GO], [true])])
         ptpm_result([SWIGARGCARGV_GO])])
fi

AC_SUBST([SWIG])
AC_SUBST([SWIGBIN])
AC_SUBST([SWIGVER])
AC_SUBST([SWIGMINVER])
AC_SUBST([SWIGLIB])
AC_SUBST([SWIGARGCARGV])
AC_SUBST([SWIGARGCARGV_GO])

#--------------------------------------------------------------------
# Target languages
#--------------------------------------------------------------------
AC_MSG_CHECKING([for build wrapper])
if test -z "$SWIGMINVER"; then
  AS_VAR_SET([alllang_default], [no])
  AC_MSG_RESULT([no proper swig])
else
  AC_ARG_WITH([alllang],
              [AS_HELP_STRING([--without-alllang], [Disable all languages])],
              [AS_VAR_SET([with_alllang], ["$withval"])])
  AS_IF([test "x$with_alllang" = "xno"],
        [AS_VAR_SET([alllang_default], [no])],
        [AS_VAR_SET([alllang_default], [yes])])
  ptpm_result([alllang_default])
fi

#--------------------------------------------------------------------
# Look for Tcl
#--------------------------------------------------------------------
AS_UNSET([TCLPACKAGE])
AS_UNSET([TCLCONFIG])
AS_UNSET([TCLINCDIR])
AS_UNSET([TCLINCLUDE])
AS_UNSET([TCLLIB])
AS_UNSET([TCLVER])
AS_UNSET([TCLMINVER])
AS_UNSET([TCL_PKG_DIR])

AC_ARG_WITH([tclconfig],
            [AS_HELP_STRING([--without-tcl], [Disable Tcl])]
[AS_HELP_STRING([--with-tclconfig=path], [Set location of tclConfig.sh])],
            [AS_VAR_SET([with_tclconfig],["$withval"])],
            [AS_UNSET([with_tclconfig])])
AC_ARG_WITH([tcl],
            [AS_HELP_STRING([--with-tcl=path],
                            [Set location of Tcl package])],
            [AS_VAR_SET([TCLPACKAGE], ["$withval"])],
            [AS_VAR_SET([TCLPACKAGE], ["$alllang_default"])])
AC_ARG_WITH([tclincl],
            [AS_HELP_STRING([--with-tclincl=path],
                            [Set location of Tcl include directory])],
            [AS_VAR_SET([TCLINCDIR], ["$withval"])])
AC_ARG_WITH([tcllib],
            [AS_HELP_STRING([--with-tcllib=path],
                            [Set location of Tcl library directory])],
            [AS_VAR_SET([TCLLIB], ["-L$withval"])])

# First, check for "--without-tcl" or "--with-tcl=no".
if test "x$TCLPACKAGE" = xno; then
  AC_MSG_NOTICE([Disabling Tcl])
  AS_UNSET([TCLLIB])
else
  AC_MSG_CHECKING([for Tcl configuration])

  # First check to see if --with-tclconfig was specified.
  if test "x$with_tclconfig" != x; then
    AS_IF([test -f "$with_tclconfig/tclConfig.sh"],
          [AS_VAR_SET([TCLCONFIG], [`(cd $with_tclconfig; pwd)`])],
          [AC_MSG_ERROR([$with_tcl directory does not contain tclConfig.sh])])
  fi
  # check in a few common install locations
  AS_VAR_SET([dirs], ["/usr/lib*/ /usr/lib*/tcl*/ /usr/local/lib*/"])
  AS_VAR_SET([dirs], ["$dirs /usr/local/lib*/tcl*/"])
  AS_IF([test "x$TCLCONFIG" = x],
        [for d in $dirs; do
           for i in `ls -d -r $d 2>/dev/null`; do
             AS_IF([test -f $i"tclConfig.sh"],
                   [AS_VAR_SET([TCLCONFIG], [`(cd $i; pwd)`])
                    break])
           done
         done])

  if test "x$TCLCONFIG" = x; then
    AC_MSG_RESULT([no])
  else
    AC_MSG_RESULT([found $TCLCONFIG/tclConfig.sh])
    . $TCLCONFIG/tclConfig.sh
    AS_IF([test -z "$TCLINCDIR"],
          [AS_VAR_SET([TCLINCDIR],
                      [`echo $TCL_INCLUDE_SPEC | $SED 's%^-I%%'`])])
    AS_IF([test -z "$TCLLIB"], [AS_VAR_SET([TCLLIB], [$TCL_LIB_SPEC])])
    if test -n "$TCL_VERSION"; then
      AC_MSG_CHECKING([for Tcl version])
      AS_VAR_SET([TCLVER], [$TCL_VERSION])
      AS_VERSION_COMPARE([8.0], [$TCL_VERSION],
                         [AS_VAR_SET([TCLMINVER], [$TCLVER])], # above 8.0
                         [AS_VAR_SET([TCLMINVER], [$TCLVER])]) # equal 8.0
      ptpm_result([TCLMINVER])
    fi
    if test -n "$TCL_PACKAGE_PATH"; then
      AC_MSG_CHECKING([for Tcl package path])
      for d in $TCL_PACKAGE_PATH; do
        AS_IF([echo $d | $GREP "$host_cpu" >/dev/null],
              [AS_VAR_SET([TCL_PKG_DIR], [$d])
               break])
      done
      AS_IF([test -z "$TCL_PKG_DIR"],
            [for d in $TCL_PACKAGE_PATH; do
               AS_IF([test "x$d" = "x$TCLCONFIG"],
                     [AS_VAR_SET([TCL_PKG_DIR], [$d])
                      break])
             done])
      AS_IF([test -z "$TCL_PKG_DIR"],
            [AS_VAR_SET([i], [0])
             for d in $TCL_PACKAGE_PATH; do
               AS_VAR_ARITH([i], [$i + 1])
             done
             AS_IF([test $i -eq 1],
                   [AS_VAR_SET([TCL_PKG_DIR],
                               [$TCL_PACKAGE_PATH])])])
      ptpm_result([TCL_PKG_DIR])
    fi
  fi
  AS_IF([test "x$TCLPACKAGE" != xyes],
        [AS_IF([test -z "$TCLINCDIR"],
               [AS_VAR_SET([TCLINCDIR], ["$TCLPACKAGE/include"])])
         AS_IF([test -z "$TCLLIB"],
               [AS_VAR_SET([TCLLIB], ["-L$TCLPACKAGE/lib -ltcl"])])])
  AC_MSG_CHECKING([for Tcl header files])
  AS_IF([test -z "$TCLINCDIR"],
        [for i in $ptpm_include_dirs; do
           AS_IF([test -r $i/tcl.h],
                 [AC_MSG_RESULT([$i])
                  AS_VAR_SET([TCLINCDIR], ["$i"])
                  break])
         done])
  AS_IF([test -n "$TCLINCDIR"],
        [AS_VAR_SET([TCLINCLUDE], ["-I$TCLINCDIR"])])
  ptpm_result([TCLINCLUDE])

  AC_MSG_CHECKING([for Tcl library])
  AS_IF([test -z "$TCLLIB"],
        [AS_VAR_SET([dirs],
         ["/usr/local/lib /usr/lib /opt/local/lib /opt/freeware/lib"])
         for i in $dirs; do
           AS_IF([test -r $i/libtcl.a],
                 [AS_VAR_SET([TCLLIB], ["-L$i -ltcl"])
                  break])
         done])
  ptpm_result([TCLLIB])
fi

AC_SUBST([TCLCONFIG])
AC_SUBST([TCLINCDIR])
AC_SUBST([TCLINCLUDE])
AC_SUBST([TCLLIB])
AC_SUBST([TCLVER])
AC_SUBST([TCLMINVER])
AC_SUBST([TCL_PKG_DIR])

#----------------------------------------------------------------
# Look for Python 3.x
#----------------------------------------------------------------
AS_UNSET([PY3VER])
AS_UNSET([PY3VERSION])
AS_UNSET([PY3INCLUDE])
AS_UNSET([PY3INCDIR])
AS_UNSET([PY3PLATINCDIR])
AS_UNSET([PY3SITE_DIR])
AS_UNSET([PY3LINK])
AS_UNSET([PY3EXT])
AS_UNSET([PY3LDLIBS])

AC_ARG_WITH([python3],
            [AS_HELP_STRING([--without-python3],
                            [Don't probe for Python 3.x])]
dnl '
[AS_HELP_STRING([--with-python3=path],
                [Set location of Python 3.x executable])],
            [AS_VAR_SET([PY3BIN], ["$withval"])],
            [AS_VAR_SET([PY3BIN], ["$alllang_default"])])

# First, check for "--without-python3" or "--with-python3=no".
if test "x$PY3BIN" = xno; then
  AC_MSG_NOTICE([Disabling Python 3.x probe])
  AS_UNSET([PY3LINK])
else
  if test "x$PY3BIN" = xyes; then
    for py_ver in 3 3.10 3.9 3.8 3.7 3.6 3.5 3.4 3.3 3.2 ""; do
      AC_CHECK_PROGS([PYTHON3], [python$py_ver])
      AS_IF([test -n "$PYTHON3"],
            [AC_CHECK_PROGS([PY3CONFIG], [$PYTHON3-config])
             AS_IF([test -n "$PY3CONFIG"], [break])])
    done
  else
    AS_VAR_SET([PYTHON3], ["$PY3BIN"])
    AC_CHECK_PROGS([PY3CONFIG], [$PYTHON3-config])
  fi

  AS_IF([test -n "$PYTHON3"],
        [AC_MSG_CHECKING([for $PYTHON3 major version number])
         AS_VAR_SET([PY3VER],
                    [`($PYTHON3 -c "import sys;\
                       sys.stdout.write(sys.version[[0]])") 2>/dev/null`])
         AC_MSG_RESULT([$PY3VER])])
  AS_IF([test -z "$PY3VER"], [AS_VAR_SET([PY3VER], [0])])

  if test $PY3VER -ge 3 && test -n "$PY3CONFIG"; then

    # Note: I could not think of a standard way to get the version string
    # from different versions.
    # This trick pulls it out of the file location
    # for a standard library file.
    AC_MSG_CHECKING([for Python 3.x version])
    AS_VAR_SET([PY3VERSION],
               [`($PYTHON3 -c "import string,operator,os.path;\
                 print(operator.getitem(os.path.split(\
                 operator.getitem(os.path.split(string.__@&t@file__),\
                 0)),1))") 2>/dev/null`])
    ptpm_result([PY3VERSION])

    # Set the include directory
    AC_MSG_CHECKING([for Python 3.x header files])
    AS_VAR_SET([PY3INCLUDE], [`($PY3CONFIG --includes) 2>/dev/null`])
    ptpm_result([PY3INCLUDE])

    AS_IF([test -n "$PY3INCLUDE"],
          [AS_VAR_SET([var], [PY3INCDIR])
           for d in $PY3INCLUDE; do
             AS_VAR_SET([${var}], [`echo $d | $SED 's/^-I//'`])
             AS_VAR_SET([var], [PY3PLATINCDIR])
           done])

    AS_VAR_SET([dirs],
               [`($PYTHON3 -c "import site;print('\n'.join(\
                  site.getsitepackages()))") 2>/dev/null`])
    AC_MSG_CHECKING([for Python 3.x site folder])
    for d in $dirs; do
      AS_VAR_SET([have_lib], [`echo $d | $GREP '^$libdir'`])
      AS_VAR_SET([have_ver], [`echo $d | $GREP $PY3VERSION`])
      AS_IF([test "x$have_lib" != x && test "x$have_ver" != x],
            [AS_VAR_SET([PY3SITE_DIR], [$d])
             break])
    done
    AS_IF([test -z "$PY3SITE_DIR"],
          [for d in $dirs; do
             AS_VAR_SET([not_local], [`echo $d | $GREP '/local/'`])
             AS_VAR_SET([have_ver], [`echo $d | $GREP $PY3VERSION`])
             AS_IF([test "x$not_local" = x && test "x$have_ver" != x],
                   [AS_VAR_SET([PY3SITE_DIR], [$d])
                    break])
           done])
    AS_IF([test -z "$PY3SITE_DIR"],
          [for d in $dirs; do
             AS_IF([echo $d | $GREP "^$libdir" >/dev/null],
                   [AS_VAR_SET([PY3SITE_DIR], [$d])
                    break])
           done])
    ptpm_result([PY3SITE_DIR])

    AC_MSG_CHECKING([for Python 3.x library])
    AS_VAR_SET([PY3LINK], ["-l$PY3VERSION"])
    ptpm_result([PY3LINK])

    AC_MSG_CHECKING([for Python 3.x extension suffix])
    AS_VAR_SET([PY3EXT], [`($PY3CONFIG --extension-suffix) 2>/dev/null`])
    ptpm_cross([PY3EXT])
    ptpm_result([PY3EXT])

    AC_MSG_CHECKING([for Python 3.x linking])
    AS_VAR_SET([PY3LDLIBS], [`($PY3CONFIG --libs) 2>/dev/null`])
    ptpm_result([PY3LDLIBS])
  fi
fi

AC_SUBST([PY3VER])
AC_SUBST([PY3VERSION])
AC_SUBST([PY3INCLUDE])
AC_SUBST([PY3INCDIR])
AC_SUBST([PY3PLATINCDIR])
AC_SUBST([PY3SITE_DIR])
AC_SUBST([PY3LINK])
AC_SUBST([PY3EXT])
AC_SUBST([PY3LDLIBS])

#----------------------------------------------------------------
# Look for Perl5
#----------------------------------------------------------------
AS_UNSET([PERL])
AS_UNSET([PERLBIN])
AS_UNSET([PERL5DIR])
AS_UNSET([PERL5EXT])
AS_UNSET([PERL5TEST])

AC_ARG_WITH([perl5],
            [AS_HELP_STRING([--without-perl5],
                            [Disable Perl5])]
[AS_HELP_STRING([--with-perl5=path],
                [Set location of Perl5 executable])],
            [AS_VAR_SET([PERLBIN], ["$withval"])],
            [AS_VAR_SET([PERLBIN], [yes])])

# First, check for "--without-perl5" or "--with-perl5=no".
if test "x$PERLBIN" = xno; then
  AC_MSG_NOTICE([Disabling Perl5])
else
  # First figure out what the name of Perl5 is
  AS_IF([test "x$PERLBIN" = xyes],
        [AC_CHECK_PROGS([PERL], [perl perl5])],
        [AS_VAR_SET([PERL], ["$PERLBIN"])])
  # This could probably be simplified as for all platforms and
  # all versions of Perl the following apparently
  # should be run to get the compilation options:
  # perl -MExtUtils::Embed -e ccopts
  AC_MSG_CHECKING([for Perl5 confiuration])
  if test -n "$PERL"; then
    AS_VAR_SET([PERL5DIR],
               [`($PERL -MConfig -le 'print $Config{archlibexp}')\
                  2>/dev/null`])
    ptpm_cross([PERL5DIR])
    AS_IF([test -n "$PERL5DIR"],
          [AC_MSG_RESULT([$PERL5DIR])
           AC_MSG_CHECKING([for Perl5 header files])
           for i in $PERL5DIR $PERL5DIR/CORE; do
             AS_IF([test -r $i/perl.h],
                   [AS_VAR_SET([PERL5EXT], ["$i"])
                    break])
           done
           ptpm_result(PERL5EXT)],
          [AC_MSG_RESULT([unable to determine perl5 configuration])])
    AS_IF([$PERL -e 'use Test::Class;Test::More;' 2>/dev/null],
          [AS_VAR_SET([PERL5TEST], [have])])
  else
    AC_MSG_RESULT([could not figure out how to run perl5])
  fi
fi
AC_SUBST([PERL])
AC_SUBST([PERL5DIR])
AC_SUBST([PERL5EXT])
AC_SUBST([PERL5TEST])

#----------------------------------------------------------------
# Look for Ruby
#----------------------------------------------------------------
AS_UNSET([RUBY])
AS_UNSET([RUBYBIN])
AS_UNSET([RUBYHDRDIR])
AS_UNSET([RUBYARCHHDRDIR])
AS_UNSET([RUBYINCLUDE])
AS_UNSET([RUBYVER])
AS_UNSET([RUBYLIB])
AS_UNSET([RUBYLINK])
AS_UNSET([RUBYSITE])

AC_ARG_WITH([ruby],
            [AS_HELP_STRING([--without-ruby], [Disable Ruby])]
[AS_HELP_STRING([--with-ruby=path], [Set location of Ruby executable])],
            [AS_VAR_SET([RUBYBIN], ["$withval"])],
            [AS_VAR_SET([RUBYBIN], ["$alllang_default"])])

# First, check for "--without-ruby" or "--with-ruby=no".
if test "x$RUBYBIN" = xno; then
  AC_MSG_NOTICE([Disabling Ruby])
else
  # configuration comes from /usr/lib/*/ruby/*/rbconfig.rb
  # First figure out what the name of Ruby is
  AS_IF([test "x$RUBYBIN" = xyes],
        [AC_CHECK_PROGS([RUBY], [ruby])],
        [AS_VAR_SET([RUBY], ["$RUBYBIN"])])
  AC_MSG_CHECKING([for Ruby header files])
  if test -n "$RUBY"; then
    # Try Ruby1.9+ first
    AS_VAR_SET([RUBYHDRDIR],
               [`($RUBY -rrbconfig -e\
                  'print RbConfig::CONFIG[["rubyhdrdir"]]') 2>/dev/null`])
    AS_VAR_SET([RUBYARCHHDRDIR],
               [`($RUBY -rrbconfig -e\
                  'print RbConfig::CONFIG[["rubyarchhdrdir"]]') 2>/dev/null`])
    ptpm_cross([RUBYARCHHDRDIR])
    if test "x$RUBYHDRDIR" != x; then
      AS_IF([test -r $RUBYHDRDIR/ruby.h && test -n "$RUBYARCHHDRDIR"],
            [AS_VAR_SET([RUBYINCLUDE], ["-I$RUBYHDRDIR -I$RUBYARCHHDRDIR"])])
      ptpm_result([RUBYINCLUDE])

      AC_MSG_CHECKING([for Ruby header folder])
      ptpm_result([RUBYHDRDIR])
      AC_MSG_CHECKING([for Ruby header architecture folder])
      ptpm_result([RUBYARCHHDRDIR])

      AC_MSG_CHECKING([for Ruby version])
      AS_VAR_SET([RUBYVER],
                 [`$RUBY -e 'print "#{RUBY_VERSION}"'`])
      ptpm_result([RUBYVER])
      AS_VAR_SET([RUBYLINK],
                 [`($RUBY -rrbconfig -e\
                  'n=RbConfig::CONFIG[["RUBY_SO_NAME"]];print "-l#{n}"')
                  2>/dev/null`])

      # Find library and path for linking.
      AC_MSG_CHECKING([for Ruby library])
      AS_VAR_SET([rb_archlibdir],
                 [`($RUBY -rrbconfig -e\
                    'print RbConfig::CONFIG[["archlibdir"]]') 2>/dev/null`])
      AS_VAR_SET([rb_libdir],
                 [`($RUBY -rrbconfig -e 'print RbConfig::CONFIG[["libdir"]]')\
                    2>/dev/null`])
      AS_VAR_SET([rb_bindir],
                 [`($RUBY -rrbconfig -e 'print RbConfig::CONFIG[["bindir"]]')\
                    2>/dev/null`])
      AS_VAR_SET([dirs], ["$rb_archlibdir $rb_libdir $rb_bindir"])
      AS_VAR_SET([rb_libruby],
                 [`($RUBY -rrbconfig -e\
                    'print RbConfig::CONFIG[["LIBRUBY_SO"]]') 2>/dev/null`])
      AS_IF([test "x$rb_libruby" != x],
            [for i in $dirs; do
               AS_IF([test -r $i/$rb_libruby],
                     [AS_VAR_SET([RUBYLIB], ["$i"])
                      break])
             done])
      ptpm_cross([RUBYLIB])
      ptpm_result([RUBYLIB])
    else
      AC_MSG_RESULT([unable to determine ruby configuration])
    fi
    AC_MSG_CHECKING([for Ruby link flags])
    AS_VAR_SET([RUBYLINK],
               ["$RUBYLINK `($RUBY -rrbconfig -e\
                 'print RbConfig::CONFIG[["LIBS"]]') 2>/dev/null`"])
    ptpm_result([RUBYLINK])

    AC_MSG_CHECKING([for Ruby site])
    AS_VAR_SET([RUBYSITE],
               [`($RUBY -rrbconfig -e\
                  'print RbConfig::CONFIG[["vendorarchdir"]]') 2>/dev/null`])
    ptpm_cross([RUBYSITE])
    ptpm_result([RUBYSITE])
  else
    AC_MSG_RESULT([could not figure out how to run ruby])
  fi
fi

AC_SUBST([RUBY])
AC_SUBST([RUBYHDRDIR])
AC_SUBST([RUBYARCHHDRDIR])
AC_SUBST([RUBYINCLUDE])
AC_SUBST([RUBYVER])
AC_SUBST([RUBYLIB])
AC_SUBST([RUBYLINK])
AC_SUBST([RUBYSITE])

#-------------------------------------------------------------------------
# Look for PHP
#-------------------------------------------------------------------------
AS_UNSET([PHPBIN])
AS_UNSET([PHP])
AS_UNSET([PHPCONFIG])
AS_UNSET([PHPINCDIR])
AS_UNSET([PHPVER])
AS_UNSET([PHPMINVER])
AS_UNSET([PHPEXT])
AS_UNSET([PHPINC])
AS_UNSET([PHPINC_FLAGS])
AS_UNSET([PHPUNIT])

AC_ARG_WITH([php],
            [AS_HELP_STRING([--without-php], [Disable PHP])
AS_HELP_STRING([--with-php=path], [Set location of PHP executable])],
            [AS_VAR_SET([PHPBIN], ["$withval"])],
            [AS_VAR_SET([PHPBIN], ["$alllang_default"])])

AS_IF([test -z "$SWIGMINVER"],
      [AS_VAR_SET([PHPBIN], ["no"])],
      [AS_VERSION_COMPARE([$SWIGMINVER], [3.0.12],
                          [AS_VAR_SET([PHPBIN], ["no"])],   # below 3.0.12
                          [AS_VAR_SET([PHPBIN], ["no"])])]) # equal 3.0.12

# First, check for "--without-php" or "--with-php=no".
if test "x$PHPBIN" = xno; then
  AC_MSG_NOTICE([Disabling PHP])
else
  AS_IF([test "x$PHPBIN" = xyes],
        [AC_CHECK_PROGS([PHP],
                        [php8.1 php8.0 php7.4 php7.3 php7.2\
                         php7.1 php7.0 php])],
        [AS_VAR_SET([PHP], [$PHPBIN])])
  if test -n "$PHP"; then
    # /usr/bin/php7.0 -> /usr/bin/php-config7.0
    AS_CASE([$PHP],
            [*7.*],
            [AS_VAR_SET([phpconfig],
                        [`echo "$PHP" | $SED 's%7\...*$%-config&%'`])],
            [*8.*],
            [AS_VAR_SET([phpconfig],
                        [`echo "$PHP" | $SED 's%8\...*$%-config&%'`])],
            [AS_VAR_SET([phpconfig], [$PHP-config])])

    AC_CHECK_PROGS([PHPCONFIG], [$phpconfig php-config])

    AC_MSG_CHECKING([for PHP include folder])
    AS_VAR_SET([dirs],
               [`$PHP -r 'echo get_include_path();' | $SED 's%:% %g'`])
    for d in $dirs; do
      AS_IF([test "x$d" != 'x.'],
            [AS_VAR_SET([PHPINCDIR], ["$d"])
             break])
    done
    ptpm_result([PHPINCDIR])

    if test -n "$PHPCONFIG"; then
      AC_MSG_CHECKING([for PHP version])
      AS_VAR_SET([PHPVER], [`$PHPCONFIG --version 2>/dev/null`])
      ptpm_result([PHPVER])

      AC_MSG_CHECKING([for PHP extensions])
      AS_VAR_SET([PHPEXT], [`$PHPCONFIG --extension-dir 2>/dev/null`])
      ptpm_result([PHPEXT])

      AC_MSG_CHECKING([for PHP headers base folder])
      AS_VAR_SET([PHPINC], [`$PHPCONFIG --include-dir 2>/dev/null`])
      ptpm_result([PHPINC])

      AC_MSG_CHECKING([for PHP header files])

      AS_CASE([$PHPVER],
              [7.*|8.*],
              [AS_VAR_SET([phpinc_flags],
                          [`$PHPCONFIG --includes 2>/dev/null`])
               AS_VAR_SET([sub], ['${PHPINC}'])
               AS_IF([test -n "$phpinc_flags"],
                     [AS_VAR_SET([PHPINC_FLAGS],
                                 [`echo "$phpinc_flags" |\
                                  $SED "s%$PHPINC%$sub%g"`])])
               ptpm_result([PHPINC_FLAGS])],
              [""],
              [AS_VAR_SET([msg], ["could not find $PHPCONFIG or obtain"])
               AC_MSG_RESULT([$msg PHP version from it])],
              [AC_MSG_RESULT([found PHP $PHPVER - not PHP 7 or 8])])

    fi
    AS_VERSION_COMPARE([$SWIGVER], [4.1],
                       # Swig bellow 4.1
                       [AS_VERSION_COMPARE([$PHPVER], [8.0],
                                           # Require PHP below 8.0
                                           [AS_VAR_SET([PHPMINVER],
                                                       [$PHPVER])])],
                       # Swig 4.1
                       [AS_VAR_SET([PHPMINVER], [$PHPVER])],
                       # Above swig 4.1
                       [AS_VAR_SET([PHPMINVER], [$PHPVER])])
    AC_CHECK_PROGS([PHPUNIT], [phpunit])
  fi
fi

AC_SUBST([PHP])
AC_SUBST([PHPCONFIG])
AC_SUBST([PHPINCDIR])
AC_SUBST([PHPVER])
AC_SUBST([PHPMINVER])
AC_SUBST([PHPEXT])
AC_SUBST([PHPINC])
AC_SUBST([PHPINC_FLAGS])
AC_SUBST([PHPUNIT])

#----------------------------------------------------------------
# Look for Lua
#----------------------------------------------------------------
AS_UNSET([LUABIN])
AS_UNSET([LUA_VERSION])
AS_UNSET([LUABIN_VERSION])
AS_UNSET([LUAINCLUDE])
AS_UNSET([LUALIB])
AS_UNSET([LUA_INC])
AS_UNSET([LUALINK])
AS_UNSET([LUAVERSIONS])
AS_UNSET([LUAVERSIONS_UNIT])
AS_UNSET([LUAUNIT])
AS_UNSET([LUA_5_1_INC])
AS_UNSET([LUA_5_2_INC])
AS_UNSET([LUA_5_3_INC])
AS_UNSET([LUA_5_4_INC])
AS_UNSET([LUA_5_5_INC])
AS_UNSET([LUA_5_6_INC])
AS_UNSET([LUA_5_7_INC])
AS_UNSET([LUA_5_1_LINK])
AS_UNSET([LUA_5_2_LINK])
AS_UNSET([LUA_5_3_LINK])
AS_UNSET([LUA_5_4_LINK])
AS_UNSET([LUA_5_5_LINK])
AS_UNSET([LUA_5_6_LINK])
AS_UNSET([LUA_5_7_LINK])

AC_ARG_WITH([lua],
            [AS_HELP_STRING([--without-lua], [Disable Lua])]
[AS_HELP_STRING([--with-lua=path], [Set location of Lua executable])],
            [AS_VAR_SET([LUABIN], ["$withval"])],
            [AS_VAR_SET([LUABIN], ["$alllang_default"])])
AC_ARG_WITH([luaincl],
            [AS_HELP_STRING([--with-luaincl=path],
                            [Set location of Lua include directory])],
            [AS_VAR_SET([LUAINCLUDE], ["$withval"])])
AC_ARG_WITH([lualib],
            [AS_HELP_STRING([--with-lualib=path],
                            [Set location of Lua library directory])],
            [AS_VAR_SET([LUALIB], ["$withval"])])

# First, check for "--without-lua" or "--with-lua=no".
if test "x$LUABIN" = xno; then
  AC_MSG_NOTICE([Disabling Lua])
  AS_UNSET([LUABIN])
else
  # Try to probe all Lua version on Debian and similar
  AS_IF([test -z "$LUAINCLUDE" && test "x$LUABIN" = xyes &&\
         test -z "$LUALIB"],
        [for i in $ptpm_include_dirs; do
           for v in $i/lua5.*; do
             AS_IF([test -r $v/lua.h],
                   [AS_VAR_SET([name], [`echo $v | $SED "s%$i/lua%%"`])
                    AS_VAR_SET([tag], [`echo $name | $SED 's%\.%_%'`])
                    AS_VAR_SET([LUA_${tag}_INC], ["$v"])
                    AS_VAR_SET([LUA_${tag}_LINK], ["-llua$name"])
                    AS_VAR_SET([LUAVERSIONS], ["$LUAVERSIONS $name"])
                    AS_IF([lua$name -e "require 'luaunit'" 2>/dev/null],
                          [AS_VAR_SET([LUAVERSIONS_UNIT],
                                      ["$LUAVERSIONS_UNIT $name"])])])
           done
         done])

  # can we find lua?
  AS_IF([test "x$LUABIN" = xyes],
    # We look for a versioned Lua binary first, as there can be
    # multiple versions of Lua installed on some systems (like Debian).
        [AC_PATH_PROGS([LUABIN],
         [lua5.7 lua5.6 lua5.5 lua5.4 lua5.3 lua5.2 lua5.1 lua])])

  if test -z "$LUAVERSIONS" && test -n "$LUABIN"; then
    # check version: we need Lua 5.x
    AC_MSG_CHECKING([Lua version])
    AS_VAR_SET([LUA_VERSION],
               [`$LUABIN -e 'print(string.match(_VERSION, "%d+[.]%d+"))'`])
    ptpm_result([LUA_VERSION])

    AS_CASE([$LUA_VERSION],
            [5.*], [],
            [AS_VAR_SET([msg], ["Not Lua 5.x, SWIG does not support"])
             AC_MSG_WARN([$msg this version of Lua])
             AS_UNSET([LUABIN])])

    if test -n "$LUABIN"; then
      # look for the header files & set LUA_INC accordingly
      # will clear LUABIN if not present
      if test -n "$LUAINCLUDE"; then
        AC_CHECK_FILE([$LUAINCLUDE/lua.h],
                      [AS_VAR_SET([LUA_INC], ["$LUAINCLUDE"])],
                      [AS_UNSET([LUABIN])])
      else
        AS_UNSET([found_lua_h])
        # Use empty CFLAGS to avoid failure: "present but cannot be compiled"
        AS_VAR_SET([cflags_saved], [$CFLAGS])
        AS_UNSET([CFLAGS])
        AC_CHECK_HEADER([lua.h], [AS_VAR_SET([found_lua_h], [1])])
        AS_VAR_SET([CFLAGS], [$cflags_saved])
        # if we didn't get it, going to have to look elsewhere (the hard way)
        if test -z "$found_lua_h"; then
          AC_MSG_CHECKING([for lua.h in other locations])
          # note: Debian/Ubuntu seem to like /usr/include/lua5.1/lua.h
          # The ordering of the include directories to search should match
          # the ordering of libraries to search in the library test below.
          AS_UNSET([dirs])
          for d in $ptpm_include_dirs; do
            AS_VAR_SET([dirs], ["$dirs $d $d/lua$LUA_VERSION"])
          done
          for i in $dirs; do
            AS_IF([test -r $i/lua.h],
                  [AC_MSG_RESULT([$i/lua.h])
                   AS_VAR_SET([LUA_INC], ["$i"])
                   break])
          done
          AS_IF([test -z "$LUA_INC"],
                [AC_MSG_RESULT([not found])
                 AS_UNSET([LUABIN])]) # clear the bin
        fi
      fi
      # Does Lua use versions for libraries on this system?
      if test -n "$LUABIN"; then
        ptpm_basename([bin], ["$LUABIN"])
        AS_IF([test "x$bin" != "xlua"],
              [AS_VAR_SET([LUABIN_VERSION], ["$LUA_VERSION"])])

        AS_IF([test -n "$LUALIB"],
              [AC_CHECK_FILE([$LUALIB/liblua.so],
                             [LUALINK="-L$LUALIB -llua"],
                             [AS_UNSET([LUABIN])])],
              [AC_SEARCH_LIBS([lua_close],
                              [lua lua$LUA_VERSION],
                              [LUALINK="-l$ac_lib"],
                              [AS_UNSET([LUABIN])])])
      fi
    fi
    AS_IF([test -n "$LUABIN" && $LUABIN -e "require 'luaunit'" 2>/dev/null],
          [AS_VAR_SET([LUAUNIT], [have])])
  fi # test -z "$LUAVERSIONS"; then
fi # if not disabled

AC_SUBST([LUABIN])
AC_SUBST([LUA_VERSION])
AC_SUBST([LUABIN_VERSION])
AC_SUBST([LUA_INC])
AC_SUBST([LUALINK])
AC_SUBST([LUAVERSIONS])
AC_SUBST([LUAVERSIONS_UNIT])
AC_SUBST([LUAUNIT])
AC_SUBST([LUA_5_1_INC])
AC_SUBST([LUA_5_2_INC])
AC_SUBST([LUA_5_3_INC])
AC_SUBST([LUA_5_4_INC])
AC_SUBST([LUA_5_5_INC])
AC_SUBST([LUA_5_6_INC])
AC_SUBST([LUA_5_7_INC])
AC_SUBST([LUA_5_1_LINK])
AC_SUBST([LUA_5_2_LINK])
AC_SUBST([LUA_5_3_LINK])
AC_SUBST([LUA_5_4_LINK])
AC_SUBST([LUA_5_5_LINK])
AC_SUBST([LUA_5_6_LINK])
AC_SUBST([LUA_5_7_LINK])

#----------------------------------------------------------------
# Look for go
#----------------------------------------------------------------
AS_UNSET([GO])
AS_UNSET([GOBIN])
AS_UNSET([GOVER])
AS_UNSET([GOMINVER])
AS_UNSET([GOROOT])
AS_UNSET([GOVCS])
AS_UNSET([SWIG_INTGOSIZE])

AC_ARG_WITH([go],
            [AS_HELP_STRING([--without-go], [Disable Go])]
[AS_HELP_STRING([--with-go=path], [Set location of Go executable])],
            [AS_VAR_SET([GO], ["$withval"])],
            [AS_VAR_SET([GO], ["$alllang_default"])])

# First, check for "--without-go" or "--with-go=no".
if test "x$GO" = xno; then
  AC_MSG_NOTICE([Disabling Go])
else
  # figure out what is the name of Go application
  AS_IF([test "x$GO" = xyes],
        [AC_CHECK_TOOL([GOBIN], [go])],
        [AS_VAR_SET([GOBIN], ["$GO"])])
  if test -n "$GOBIN"; then
    AS_VAR_SET([GOROOT],
               [`$GOBIN env GOROOT`])
    AS_VERSION_COMPARE([$SWIGVER], [4.1],
                       # Swig bellow 4.1
                       [AS_VAR_SET([SWIG_INTGOSIZE], ["$SWIGVER"])])
    AS_VAR_SET([GOVER],
               [`$GOBIN version | $SED 's/^go version go//;s/ .*//'`])
    AS_VERSION_COMPARE([$GOVER], [1.14],
                       # Go bellow 1.14
                       [AS_UNSET([GOMINVER])],
                       # Go 1.14
                       [AS_VAR_SET([GOMINVER], [$GOVER])],
                       # Above go 1.14
                       [AS_VAR_SET([GOMINVER], [$GOVER])])
    AS_VERSION_COMPARE([$GOVER], [1.18],
                       # Go bellow 1.18
                       [AS_UNSET([GOVCS])],
                       # Go 1.18
                       [AS_VAR_SET([GOVCS], [true])],
                       # Above go 1.18
                       [AS_VAR_SET([GOVCS], [true])])
  fi # GOBIN
fi # if not disabled

AC_SUBST([GO])
AC_SUBST([GOBIN])
AC_SUBST([GOVER])
AC_SUBST([GOMINVER])
AC_SUBST([GOROOT])
AC_SUBST([GOVCS])
AC_SUBST([SWIG_INTGOSIZE])

#----------------------------------------------------------------
# Determine which languages to use for examples/test-suite
#----------------------------------------------------------------

if test -n "$SWIGMINVER"; then
  AS_UNSET([SKIP_TCL])
  AS_UNSET([SKIP_PYTHON3])
  AS_UNSET([SKIP_PERL5])
  AS_UNSET([SKIP_RUBY])
  AS_UNSET([SKIP_PHP])
  AS_UNSET([SKIP_LUA])
  AS_UNSET([SKIP_GO])

  AS_IF([test -z "$TCLLIB" || test -z "$TCLINCDIR" || test -z "$TCLMINVER"],
        [AS_VAR_SET([SKIP_TCL], [1])])
  AS_IF([test -z "$PY3LINK" || test -z "$PY3INCLUDE"],
        [AS_VAR_SET([SKIP_PYTHON3], [1])])
  AS_IF([test -z "$PERL5EXT"], [AS_VAR_SET([SKIP_PERL5], [1])])
  AS_IF([test -z "$RUBYLINK" || test -z "$RUBYINCLUDE" || test -z "$RUBYLIB"],
        [AS_VAR_SET([SKIP_RUBY], [1])])
  AS_IF([test -z "$PHPINC" || test -z "$PHPINC_FLAGS" ||\
         test -z "$PHPINCDIR" || test -z "$PHPMINVER"],
        [AS_VAR_SET([SKIP_PHP], [1])])
  AS_IF([test -z "$LUABIN"], [AS_VAR_SET([SKIP_LUA], [1])])
  AS_IF([test -z "$GOBIN" || test -z "$GOMINVER"],
        [AS_VAR_SET([SKIP_GO], [1])])
else
  AS_VAR_SET([SKIP_TCL], [1])
  AS_VAR_SET([SKIP_PYTHON3], [1])
  AS_VAR_SET([SKIP_PERL5], [1])
  AS_VAR_SET([SKIP_RUBY], [1])
  AS_VAR_SET([SKIP_PHP], [1])
  AS_VAR_SET([SKIP_LUA], [1])
  AS_VAR_SET([SKIP_GO], [1])
fi

AC_SUBST([SKIP_TCL])
AC_SUBST([SKIP_PYTHON3])
AC_SUBST([SKIP_PERL5])
AC_SUBST([SKIP_RUBY])
AC_SUBST([SKIP_PHP])
AC_SUBST([SKIP_LUA])
AC_SUBST([SKIP_GO])

#----------------------------------------------------------------
# End
#----------------------------------------------------------------
AC_OUTPUT
