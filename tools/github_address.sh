#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2024 Erez Geva
#
# Multiple scripts:
# - Run AddressSanitizer in GitHub
# - Build doxygen in GitHub
# - Log into github docker
# - Test with linuxptp and installed packages
# - Run unit tests with Address Sanitizer
# - Run unit tests with valgrind tool
# - Follow FSF RESUSE Specification https://reuse.software/spec/
#   Provide folders with all licenses
# - Update to a new version
#   Increase the minor version.
#   Updating major version is done with the '-m' flag.
# - Show configuration summary
# - Run ptp4l with a dummy clock and call the testing
###############################################################################
# Script to run AddressSanitizer in GitHub
github_address()
{
 sudo apt-get install -y --no-install-recommends libtool libtool-bin
 autoconf
 ./configure
 sim_ptp4l -at
}
###############################################################################
# Script to build doxygen in GitHub
github_pages()
{
 sudo apt-get install -y --no-install-recommends doxygen graphviz
 autoconf
 ./configure
 make doxygen
 mv doc/html _site
}
###############################################################################
# Log into github docker
dlog()
{
 echo $2 | docker login $server -u $1 --password-stdin
}
github_docker()
{
 local -r f=~/.gh.token
 if [[ -f $f ]]; then
   local server namespace
   . tools/github_params
   local -r a="$(cat $f)"
   dlog "${a/:*/}" "${a/*:/}"
 elif [[ -n "$1" ]] && [[ -n "$2" ]]; then
   dlog "$1" "$2"
 else
   echo "No token for github"
 fi
}
###############################################################################
# Test with linuxptp and installed packages
github_build()
{
 local dist
 distribution
 case $dist in
   debian)
     make deb
     sudo dpkg -i ../*.deb
     make config
     ;;
   fedora)
     make rpm
     sudo dnf install -y rpm/RPMS/*/*.rpm
     make config
     ;;
   arch)
     make pkg
     sudo pacman -U --noconfirm archlinux/*pkg.tar.zst
     make config
     ;;
   gentoo)
     make gentoo
     gentoo/install.sh
     ;;
 esac
 make clean
}
github_pkgs()
{
 tools/sim_ptp4l.sh -snt
 # Check development package
 gcc -Wall sample/check_ver.c -o check_ver -lptpmgmt
 ./check_ver
}
###############################################################################
# Run unit tests with Address Sanitizer
utest_address()
{
 while getopts 'c' opt; do
   case $opt in
     c) local do_config=true ;;
   esac
 done
 if [[ -n "$do_config" ]]; then
   make config
 elif ! [[ -f defs.mk ]]; then
   echo "You must configure before you can compile!"
   return
 fi
 ASAN_OPTIONS='verbosity=1:strict_string_checks=1'
 ASAN_OPTIONS+=':detect_stack_use_after_return=1'
 ASAN_OPTIONS+=':check_initialization_order=1:strict_init_order=1'
 ASAN_OPTIONS+=':detect_invalid_pointer_pairs=2'
 export ASAN_OPTIONS
 make utest_no_sys utest_lua utest_tcl USE_ASAN=1
}
###############################################################################
# Run unit tests with valgrind tool
utest_valgrid()
{
 while getopts 'c' opt; do
   case $opt in
     c) local do_config=true ;;
   esac
 done
 if [[ -n "$do_config" ]]; then
   make config
 elif ! [[ -f defs.mk ]]; then
   echo "You must configure before you can compile!"
   return
 fi
 make utest
 echo "======= Run utest with valgrind ======="
 readarray <<< `make utest VGD=1 2>&1`
 local -r es='ERROR SUMMARY:'
 local -r e0="$es 0 errors from 0 contexts (suppressed: 0 from 0)"
 for n in "${MAPFILE[@]}"; do
   if [[ "$n" =~ "$es" ]]; then
     echo -En "$n"
     # test fails
     [[ "$n" =~ "$e0" ]] || exit 1
   elif [[ "$n" =~ ^\[UTEST ]]; then
     echo -En "$n"
   fi
 done
}
###############################################################################
# Follow FSF RESUSE Specification https://reuse.software/spec/
# Provide folders with all licenses
cp_license()
{
 if [[ -z "$(which reuse 2> /dev/null)" ]]; then
   return
 fi
 mkdir -p LICENSES
 for n in GPL-3.0-or-later LGPL-3.0-or-later BSD-3-Clause\
          GFDL-1.3-no-invariants-or-later GPL-2.0-or-later MIT
 do
   if ! [[ -f "LICENSES/$n.txt" ]]; then
     reuse download $n
   fi
 done
}
###############################################################################
# Update to a new version
# Increase the minor version.
# Updating major version is done with the '-m' flag.
new_version()
{
 [[ -f rpm/libptpmgmt.spec ]] || [[ -f archlinux/PKGBUILD.org ]] || return
 local -r email=$(git config --get user.email)
 local -r cur_ver=$(dpkg-parsechangelog --show-field Version)
 local -r maj_ver=${cur_ver%.*} min_ver=${cur_ver##*.}
 case $1 in
   -u)
     local -ri next_min_ver=min_ver
     local -ri next_maj_ver=maj_ver
     local -r update_only=true
     ;;
   -m)
     local -ri next_min_ver=0
     local -ri next_maj_ver=maj_ver+1
     ;;
   *)
     local -ri next_min_ver=min_ver+1
     local -ri next_maj_ver=maj_ver
     ;;
 esac
 local -r next_ver=$next_maj_ver.$next_min_ver
 sed -i -e "s/^\s*ver_min=.*/ver_min=$next_min_ver/"\
        -e "s/^\s*ver_maj=.*/ver_maj=$next_maj_ver/" tools/version
 sed -i -e "s/^\(Version:\s*\).*/\1$next_ver/"\
        -e "s/\.so\.[0-9]*{/.so.$next_maj_ver{/" rpm/libptpmgmt.spec
 sed -i "s/^pkgver=.*/pkgver=$next_ver/" archlinux/PKGBUILD.org
 if [[ "$update_only" != "true" ]]; then
   local -r hash=$(git blame debian/changelog  | head -n1 | sed 's/ .*//')
   local -a log
   mapfile -t log < <(git log $hash..HEAD | grep -v '^Author:.*' |\
     grep -v '^\s*$')
   local l add_line=false
   for l in "${log[@]}"; do
    if [[ $l =~ ^Date|^commit ]]; then
     if $add_line; then
      DEBEMAIL="$email" debchange -v $next_ver "==============="
     fi
     DEBEMAIL="$email" debchange -v $next_ver "** $l"
     add_line=false
    else
     DEBEMAIL="$email" debchange -v $next_ver "$l"
     add_line=true
    fi
   done
   DEBEMAIL="$email" debchange -r bookworm
   gvim debian/changelog
   gitk &
 fi
}
###############################################################################
# Show configuration summary
config_report()
{
 local -A R
 local list='TCLVER PERL PY3VERSION RUBYVER PHPVER LUAVERSIONS LUA_VERSION
   GOVER DOTTOOL ASTYLEMINVER HAVE_GTEST_HEADER HAVE_CRITERION_HEADER
   CPPCHECK HAVE_JSONC_LIB HAVE_FJSON_LIB SWIGMINVER DOXYGENMINVER
   PACKAGE_VERSION'
 local langs='tcl perl python ruby php lua go'
 local $list $langs
 read_defs $list
 local -A setLang
 setLang[tcl]="@'$TCLVER'"
 local -r p2=';@"${perl#v}"'
 setLang[perl]="@'$($PERL -e 'print "$^V"')'$p2"
 setLang[python]="@'${PY3VERSION#python}'"
 setLang[ruby]="@'$RUBYVER'"
 setLang[php]="@'$PHPVER'"
 if [[ -n "$LUAVERSIONS" ]]; then
   setLang[lua]="@'$LUAVERSIONS'"
 else
   setLang[lua]="@'$LUA_VERSION'"
 fi
 setLang[go]="@'$GOVER'"
 for n in $langs; do
   [[ -n "${R["SKIP_${n^^}"]}" ]] && local $n='x' ||\
     eval "${setLang[$n]//@/local $n=}"
 done
 if [[ -n "$DOTTOOL" ]]; then
   local dver="$($DOTTOOL -V 2>&1)"
   dver="${dver#*version }"
   dver="${dver% (0)}"
 else
   local -r dver='x'
 fi
 local -r gccver=$(g++ -v 2>&1 | tail -1 | sed 's/.* version //;s/ .*//')
 [[ "$build" = "$host" ]] && local -r bon='native' || local -r bon='cross'
 [[ -n "$ASTYLEMINVER" ]] && local -r astyle="$ASTYLEMINVER" || local -r astyle='x'
 [[ -n "$HAVE_GTEST_HEADER" ]] && local -r gtest='v' || local -r gtest='x'
 [[ -n "$HAVE_CRITERION_HEADER" ]] && local -r crtest='v' || local -r crtest='x'
 [[ -n "$CPPCHECK" ]] && local -r cppcheck='v' || local -r cppcheck='x'
 [[ -n "$HAVE_JSONC_LIB" ]] && local -r jsonc='v' || local -r jsonc='x'
 [[ -n "$HAVE_FJSON_LIB" ]] && local -r fjson='v' || local -r fjson='x'
 [[ -n "$SWIGMINVER" ]] && local -r swig="$SWIGMINVER" || local -r swig='x'
 [[ -n "$DOXYGENMINVER" ]] && local -r doxy="$DOXYGENMINVER" || local -r doxy='x'
 cat << EOF
========================== Config ==========================
Version '$PACKAGE_VERSION' build $bon gcc '$gccver' astyle '$astyle'
Jsonc '$jsonc' Fjson '$fjson'
Doxygen '$doxy' dot '$dver' cppcheck '$cppcheck'
Google test '$gtest' Criterion test '$crtest'
swig '$swig' Python '$python' Ruby '$ruby' PHP '$php'
Perl '$perl' go '$go' tcl '$tcl' Lua '$lua'
============================================================
EOF
}
###############################################################################
# Run ptp4l with a dummy clock and call the testing
sim_ptp4l()
{
 # Default values
 local -r def_linuxptpLoc=../linuxptp
 # environment variables
 ##############################################################################
 local pass_args
 while getopts 'cl:asnt' opt; do
   case $opt in
     c) local do_config=true ;;
     l) local -r linuxptpLoc="$OPTARG" ;;
     a) pass_args+="a" ;; # Use AddressSanitizer
     s) pass_args+="s" ;; # Use system libraries
     n) pass_args+="n" ;; # Do not build, yield error if need to
     t) pass_args+="t" ;; # query each tlv by itself
   esac
 done
 if [[ -z "$linuxptpLoc" ]]; then
   local -n env='PTP_PATH'
   if [[ -n "$env" ]]; then
     local -r linuxptpLoc="$env"
   else
     local -r linuxptpLoc="$def_linuxptpLoc"
   fi
 fi
 ##############################################################################
 if ! [[ -f $linuxptpLoc/makefile ]]; then
   echo "You must provide linuxptp source folder!"
   return
 fi
 if [[ -n "$do_config" ]]; then
   make config
 elif ! [[ -f defs.mk ]]; then
   echo "You must configure before you can compile!"
   return
 fi
 make -C $linuxptpLoc
 make ptp4l_sim
 if [[ $(id -u) -eq 0 ]]; then
   local -r cfg=/tmp/ptp4l.conf
 else
   local tmp=`mktemp -d`
   local -r cfg=$tmp/ptp4l.conf
   local -r uds=$tmp/ptp4l
 fi
 cat << EOF > $cfg
[global]
clientOnly 1
use_syslog 0
userDescription testing with ptp4l
EOF
 if [[ -n "$uds" ]]; then
   printf "uds_address $uds\nuds_ro_address $uds.ro\n" >> $cfg
 fi
 # The interface is ignored in the ptp4l_sim.so wrapper.
 # But ptp4l and phc_ctl except an interface.
 local -r opts="-f $cfg -i eth0"
 LD_PRELOAD=./objs/ptp4l_sim.so $linuxptpLoc/ptp4l $opts &
 #pid=$!
 sleep 1
 tools/testing.sh $opts -p$pass_args -l $linuxptpLoc
 sleep 1
 kill %1
 wait
}
###############################################################################
main()
{
 local -r me1="$(realpath -s "$0")"
 cd "$(dirname "$(realpath "$0")")/.."
 source tools/util.sh
 local -r dr1="$(dirname "$me1")"
 local -r me="${me1//$dr1\//}"
 local n
 case "$me" in
  github_address.sh) github_address "$@";;
  github_pages.sh)   github_pages "$@";;
  github_docker.sh)  github_docker "$@";;
  github_build.sh)   github_build "$@";;
  github_pkgs.sh)    github_pkgs "$@";;
  utest_address.sh)  utest_address "$@";;
  utest_valgrid.sh)  utest_valgrid "$@";;
  cp_license.sh)     cp_license "$@";;
  new_version.sh)    new_version "$@";;
  config_report.sh)  config_report "$@";;
  sim_ptp4l.sh)      sim_ptp4l "$@";;
 esac
}
main "$@"
