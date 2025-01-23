#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# CI (Continous integration) script for containers
###############################################################################
ecmd()
{
 set +e
 out="`$@`"
 last_ret=$?
 set -e
}
eacmd()
{
 set +e
 out="`$@ 2>&1`"
 last_ret=$?
 set -e
}
e_mk()
{
 set +e
 out="`CXXFLAGS=-Werror make $@`"
 last_ret=$?
 set -e
}
ea_mk()
{
 set +e
 out="`CXXFLAGS=-Werror make $@ 2>&1`"
 last_ret=$?
 set -e
}
mquit()
{
 if [[ -n "$out" ]]; then
   echo "$out"
 fi
 echo "$color_red$color_red$@$color_norm"
 exit 1
}
equit()
{
 if [[ $last_ret -ne 0 ]]; then
   mquit "$@"
 fi
}
check_clean()
{
 local left=''
 local name=$1
 shift
 readarray <<< `git clean -fxdn`
 for n in "${MAPFILE[@]}"; do
   if [[ -z "${n%$'\n'}" ]] ||\
      [[ $n =~ \/\.[^\/]+\.swp ]] ||\
      [[ $n =~ ^$would\.[^\/]+\.swp ]]; then
     continue
   fi
   for m in "$@"; do
     if [[ $n =~ ^$would$m ]]; then
       continue 2
     fi
   done
   if [[ -n "${n%$'\n'}" ]]; then
     left+="$n"
   fi
 done
 if [[ -n "$left" ]]; then
   printf "$color_red$left"
   mquit "Make $name left unused files"
 fi
 gstatus="$(git status -s)"
 if [[ -n "$gstatus" ]]; then
   printf "$color_red$gstatus"
   mquit "Make $name change files"
 fi
}
test_clean()
{
 if $have_git; then
   out=''
   ecmd make clean $mk_noc
   equit "'make clean' fails"
   check_clean clean $clean_list $distclean_list $dist_clean_more
   ecmd make distclean $mk_noc
   equit "'make distclean' fails"
   check_clean distclean $distclean_list
 elif [[ "$1" = "clean" ]]; then
   make clean $mk_noc
 fi
}
main()
{
 cd "$(realpath "$(dirname "$0")/..")"
 source tools/util.sh
 local -i jobs=1 # Number of Make parallel jobs
 local nocolor gstatus
 if [[ "$GITHUB_ACTIONS" = "true" ]] && [[ `id -u` -ne 0 ]]; then
   nocolor=yes
 fi
 while getopts 'oj:' opt; do
   case $opt in
     o)
       nocolor=yes
       ;;
     j)
       jobs="0 + $OPTARG"
       ;;
   esac
 done
 if [[ $jobs -le 0 ]]; then
   jobs=1
 fi
 local n m out distclean_list dist dname dist_clean_more
 local -i last_ret
 distribution
 if [[ "$dist" != "gentoo" ]]; then
   local -r not_gentoo=true
 else
   local -r not_gentoo=false
 fi
 # Make sure we output to STDOUT directly, no pipes
 # check our teminal support coulors
 if [[ -n "$nocolor" ]]; then
   local -r mk_noc='NO_COL=1'
 elif [[ -t 1 ]] && tput setaf 1; then
   local -r esc=`printf '\e['`
   local -r color_red=${esc}31m
   local -r color_blue=${esc}34m
   local -r color_norm=${esc}00m
   printf "$color_norm"
 fi
 local clean_list="autom4te.cache/ m4/ config.log config.status"
 clean_list+=" config.guess config.sub ltmain.sh configure defs.mk"
 clean_list+=" src/config.h src/config.h.in install-sh aclocal.m4 libtool"
 for n in */.upgrade_cockie; do
   ! [[ -f "$n" ]] || distclean_list+=" $n"
 done
 ##########################################################
 ######                 Test start                   ######
 ##########################################################
 ### clean ###
 eacmd git rev-parse --is-inside-work-tree
 if [[ $last_ret -eq 0 ]]; then
   local -r have_git=true
   local -r would='Would[[:space:]]remove[[:space:]]'
   test_clean
 else
   local -r have_git=false
   # Make sure we do not have leftovers
   ecmd make clean $mk_noc
   equit "'make clean' fails"
 fi
 ### run reuse lint ###
 if $have_git && [[ -n "$(which reuse 2> /dev/null)" ]]; then
   local -ri reuse_ver="$(reuse --version | head -1 |\
     sed 's/version//;s/,//;s/^reuse\s*//;s/\..*//')"
   echo " * Check files licenses with 'reuse'"
   if [[ $reuse_ver -ge 2 ]]; then
     # reuse 2.1 can detect copyright better, suppress warnings
     ecmd reuse --suppress-deprecation lint
   else
     ecmd reuse lint
   fi
   equit "'reuse' detect missing SPDX tags"
 fi
 ### Configure ###
 echo " * Configure"
 autoreconf -i
 # Were is Gentoo defualt configure setting?
 # This is after the build flags, we use 64 bits container.
 local -r gentoo_cfg='--prefix=/usr --sysconfdir=/etc --localstatedir=/var/lib
   --libdir=/usr/lib64'
 if $not_gentoo; then
   ecmd make config $mk_noc
 else
   ecmd ./configure $gentoo_cfg
 fi
 equit "Configuratation fails"
 tools/config_report.sh
 make checkall -j$jobs $mk_noc
 ### unit test ###
 echo " * Run unit test"
 ea_mk utest -j$jobs $mk_noc
 equit "Unit test fails"
 echo " * Build"
 e_mk -j$jobs $mk_noc
 equit "Build fails"
 test_clean clean
 ### Configure clang ###
 echo " * Configure with clang"
 autoreconf -i
 # Use clang
 if $not_gentoo; then
   ecmd make config $mk_noc CC=clang CXX=clang++
 else
   # Add clang to path
   local -r lvm_p="$(ls -d /usr/lib/llvm/*/bin 2> /dev/null | sort -n | tail -1)"
   export PATH+=":$lvm_p"
   ecmd ./configure $gentoo_cfg CC=clang CXX=clang++
 fi
 if [[ "`grep CC: defs.mk`" != "CC:=clang" ]]; then
   mquit "Compiler $CC is not 'clang'"
 fi
 if [[ "`grep ^CXX: defs.mk`" != "CXX:=clang++" ]]; then
   mquit "Compiler $CXX is not 'clang++'"
 fi
 equit "Configuratation with clang fails"
 tools/config_report.sh
 ### unit test clang ###
 echo " * Run unit test with clang"
 ea_mk utest -j$jobs $mk_noc
 equit "Unit test with clang fails"
 echo " * Build with clang"
 e_mk -j$jobs $mk_noc
 equit "Build with clang fails"
 test_clean
 # mk_packages() # We test packaging in another test, we can skip here.
 echo "$color_blue * CI test done$color_norm"
}
mk_packages()
{
 ### make packages ###
 case $dist in
   debian)
     if [[ -n "$(which dpkg-buildpackage 2> /dev/null)" ]]; then
       echo " * Build $dname packages"
       ea_mk deb -j$jobs $mk_noc
       equit "Build $dname packages fails"
     fi
     ;;
   fedora|redhat)
     if [[ -n "$(which rpmbuild 2> /dev/null)" ]]; then
       echo " * Build Fedora RPM packages"
       eacmd make rpm -j$jobs $mk_noc
       equit "Build Fedora RPM packages fails"
     fi
     ;;
   arch)
     if [[ -n "$(which makepkg 2> /dev/null)" ]]; then
       echo " * Build Arch Linux packages"
       ea_mk pkg -j$jobs MAKEFLAGS="-j$jobs" $mk_noc
       equit "Build Arch Linux packages fails"
       for n in archlinux/libptpmgmt-*.txz; do
         dist_clean_more+=" $n"
       done
     fi
     ;;
   gentoo)
     if [[ -n "$(which ebuild 2> /dev/null)" ]]; then
       echo " * Build Gentoo packages"
       ea_mk gentoo -j$jobs $mk_noc
       equit "Build Gentoo fails"
     fi
     ;;
 esac
 test_clean
}
main "$@"
