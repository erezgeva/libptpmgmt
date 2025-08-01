#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2024 Erez Geva
#
# Multiple scripts:
# - ci_address:     Run AddressSanitizer in GitHub
# - ci_coverity:    Configure for coverity scan
# - ci_pages:       Build doxygen in GitHub
# - def_params:     Create default GitHub and GitLab parameters files
# - github_docker:  Logging into GitHub Docker Server
# - gitlab_docker:  Logging into GitLab Docker Server
# - ci_build:       Build and install packages
# - ci_pkgs:        Test with linuxptp
# - ci_pkgs_no_dev: Test with linuxptp without the development package
# - ci_abi:         Compart ABI of current library with last version
# - ci_abi_err:     Report ABI error
# - ci_cross:       CI cross compilation
# - add_doxy_spdx:  Add SPDX header to doxygen generated files
# - utest_address:  Run unit tests with Address Sanitizer
# - utest_valgrid:  Run unit tests with valgrind tool
# - cp_license:     Follow FSF RESUSE Specification
#                   https://reuse.software/spec/
#                   Provide folders with all licenses
# - new_version:    Update to a new version
#                   Increase the minor version.
#                   Updating major version is done with the '-m' flag.
# - config_report:  Show configuration summary
# - sim_ptp4l:      Run ptp4l with a dummy clock and call the testing
# - tag_release:    Tag new release
###############################################################################
emk()
{
 make $@ > /dev/null
}
s_mv()
{
 if [[ -f "$1" ]]; then
   mv -f "$1" $2
 fi
}
apt_install()
{
 export DEBIAN_FRONTEND=noninteractive
 sudo apt-get update
 sudo apt-get install -y --no-install-recommends $@
}
config_ubuntu()
{
 autoreconf -i
 ./configure $@
 config_report
}
install_libchrony_ubuntu()
{
 local -r prefix=/usr
 local -r libdir="$prefix/lib/$(dpkg-architecture -qDEB_TARGET_MULTIARCH)"
 echo "libdir=$libdir"
 local -r dir_chrony=/tmp/libchrony
 git clone https://github.com/mlichvar/libchrony $dir_chrony
 cd $dir_chrony
 sudo make install prefix=$prefix libdir="$libdir"
 cd "$base_dir"
 sudo rm -R $dir_chrony
}
install_librtpi_ubuntu()
{
 # GitHub uses: Ubuntu noble 24.04 LTS
 # Packages: https://packages.ubuntu.com/
 local -r url=http://de.archive.ubuntu.com/ubuntu/pool/universe/libr/librtpi
 local -r aver=_1.0.0-3_amd64.deb
 # 'librtpi' is add in oracular 24.10
 local -r dir_rtpi=/tmp/librtpi
 mkdir -p $dir_rtpi
 cd $dir_chron
 wget -c $url/librtpi1$aver
 wget -c $url/librtpi-dev$aver
 sudo dpkg -i *.deb
 cd "$base_dir"
 rm -R $dir_rtpi
}
build_prepare_ubuntu()
{
 apt_install libtool-bin nettle-dev libgnutls28-dev libgcrypt20-dev\
   chrpath
 install_librtpi_ubuntu
 install_libchrony_ubuntu
 config_ubuntu "$@"
}
###############################################################################
# Configure for coverity scan
ci_coverity()
{
 local add_cfg
 case $1 in
   clang)
     add_cfg='CC=clang CXX=clang++'
     ;;
 esac
 build_prepare_ubuntu --without-swig $add_cfg
}
###############################################################################
# Script to run AddressSanitizer in GitHub
ci_address()
{
 build_prepare_ubuntu
 sim_ptp4l -at
}
###############################################################################
# Script to build doxygen in GitHub
ci_pages()
{
 apt_install doxygen graphviz
 config_ubuntu
 make doxygen
 mv doc/html _site
 mv clkmgr/doc/html _site2
 rm -f _site*/*.md5 _site*/*.map
 add_doxy_spdx _site _site
 add_doxy_spdx _site2 _site2 intel
 mv _site2 _site/clkmgr
}
###############################################################################
# Create default GitHub and GitLab parameters files
def_params()
{
  local -r def_namespace=erezgeva
  local -r def_project=libptpmgmt
  if ! [[ -f tools/github_params ]]; then
    cat << EOF > tools/github_params
server=ghcr.io
namespace=$def_namespace
EOF
  fi
  if ! [[ -f tools/gitlab_params ]]; then
    cat << EOF > tools/gitlab_params
server=registry.gitlab.com
namespace=$def_namespace
project=$def_project
EOF
  fi
  # TODO remove the follow
  if ! grep 'project=' tools/gitlab_params >> /dev/null; then
    echo "project=$def_project" >> tools/gitlab_params
  fi
}
docker_dlog()
{
 echo "$2" | docker login $server -u $1 --password-stdin
}
docker_server()
{
 if [[ -n "$1" ]] && [[ -n "$2" ]] && [[ -n "$3" ]]; then
   local server="$1"
   docker_dlog "$2" "$3"
 else
   echo "No token to login"
 fi
}
docker_server_prm()
{
 def_params
 if [[ -f "$1" ]]; then
   local server namespace project
   . "$2"
   local -r a="$(cat "$1")"
   docker_dlog "${a/:*/}" "${a/*:/}"
 else
   echo "No token to login"
 fi
}
# Logging into GitHub Docker Server
github_docker()
{
 docker_server_prm ~/.gh.token tools/github_params
# ~/.gh.token file contain:
# <user in github>:<personal access token>
}
# Logging into GitLab Docker Server
gitlab_docker()
{
 docker_server_prm ~/.gl.token tools/gitlab_params
 # ~/.gh.token file contain:
 # <user in gitlab>:<personal access token>
}
###############################################################################
# Build and install packages
ci_build()
{
 local dist dname
 distribution
 case $dist in
   debian)
     make deb
     sudo dpkg -i ../*.deb
     make config
     ;;
   fedora|redhat)
     make rpm
     sudo rpm -i rpm/RPMS/*/*.rpm
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
###############################################################################
# Test with linuxptp
ci_pkgs()
{
 sim_ptp4l -snt
 # Check C development using the development package
 gcc -Wall sample/check_ver.c -o check_ver -lptpmgmt
 ./check_ver
}
# Test with linuxptp without the development package
ci_pkgs_no_dev()
{
 local dist dname
 distribution
 case $dist in
   debian)
     export DEBIAN_FRONTEND=noninteractive
     sudo apt-get remove -y --no-install-recommends libptpmgmt-dev
     ;;
   fedora|redhat)
     sudo dnf remove -y libptpmgmt-devel
     ;;
   arch)
     sudo pacman -R --noconfirm libptpmgmt-dev
     ;;
   gentoo)
     # Gentoo do not use packages :-)
     return
     ;;
 esac
 # Run test without the development package
 sim_ptp4l -snt
}
###############################################################################
# Compart ABI of current library with last version
ci_abi()
{
 local -r cur_hash="$(git rev-parse HEAD)"
 local -r version="$ver_maj.$ver_min"
 if ! git rev-list $version > /dev/null 2>&1; then
   echo "== Tag $version is missing we can not do the test! =="
   return
 fi
 echo "== Build current version =="
 emk config
 emk libptpmgmt.la
 echo "== Dump current version =="
 # Filter headers which change when adding new TLVs ID
 # The changes should be ABI backward compatible
 local -r hdr_lst=$(mktemp /tmp/headers.XXXX.list)
 for n in pub/* pub/c/*; do
   if [[ -f "$n" ]] && ! [[ $n =~ mngIds.h ]] && ! [[ $n =~ callDef.h ]];then
     echo "$n" >> $hdr_lst
   fi
 done
 abi-dumper .libs/libptpmgmt.so -o cur.dump -lver 1 -public-headers $hdr_lst
 make distclean
 echo "== Build last tag $version =="
 git checkout $version
 emk config
 emk
 # TODO Use new make: emk libptpmgmt.la
 git checkout "$cur_hash"
 echo "== Dump last tag version =="
 local file=".libs/libptpmgmt.so"
 if ! [[ -f "$file" ]]; then
   # TODO Use new location
   file="libptpmgmt.so"
 fi
 abi-dumper "$file" -o old.dump -lver 0 -public-headers $hdr_lst
 echo "== Compare ABI =="
 if ! abi-compliance-checker -l ptpmgmt -old old.dump -new cur.dump; then
   echo "== Found errors =="
   echo 1 > abi_error
 fi
 if [[ -d compat_reports ]]; then
   cd compat_reports
   for n in */*/*.htm* */*.htm*; do
     s_mv "$n" .
   done
   rmdir -p */* || true
   cd ..
 fi
}
ci_abi_err()
{
 if [[ -f abi_error ]]; then
   echo "== Report ABI error =="
   exit $(cat abi_error)
 fi
}
###############################################################################
# CI cross compilation
ci_cross()
{
 sudo debian/inst_arc.sh arm64
 make deb_arc arm64
 config_report
}
###############################################################################
# Add SPDX header to doxygen generated files
add_doxy_spdx()
{
 local -r src="$1"
 local -r tgt="$2"
 case $3 in
   intel)
     local -r hcpy='Intel Corporation.'
     ;;
   *)
     local -r hcpy='Erez Geva <ErezGeva2@gmail.com>'
     ;;
 esac
 local -r gfdl='SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later'
 if [[ -n "$(which date 2> /dev/null)" ]]; then
     local -r year="$(date "+%Y")"
 else
    local -r year=2025
 fi
 local -r cpy="   SPDX-FileCopyrightText: Copyright © $year"
 local m
 for n in $src/*.html $src/*/*.html
 do
   if test -f "$n"; then
     if [[ "$src" = "$tgt" ]]; then
       m="$n"
     else
       m="$(printf $n | sed "s@$src/@$tgt/@")"
     fi
     sed -i "1 i<!-- $gfdl\n  $cpy $hcpy -->" $m
   fi
 done
 for n in $src/search/*_*.js $src/search/searchdata.js
 do
   if test -f "$n"; then

     if [[ "$src" = "$tgt" ]]; then
       m="$n"
     else
       m="$(printf $n | sed "s@$src/@$tgt/@")"
     fi
    sed -i "1 i/* $gfdl\n$cpy $hcpy */\n" $m
   fi
 done
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
   config_report
 elif ! [[ -f defs.mk ]]; then
   echo "You must configure before you can compile!"
   return
 fi
 ASAN_OPTIONS='verbosity=1:strict_string_checks=1'
 ASAN_OPTIONS+=':detect_stack_use_after_return=1'
 ASAN_OPTIONS+=':check_initialization_order=1:strict_init_order=1'
 ASAN_OPTIONS+=':detect_invalid_pointer_pairs=2'
 export ASAN_OPTIONS
 make utest_no_sys uctest_no_sys utest_lua utest_tcl USE_ASAN=1
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
   emk config
   config_report
 elif ! [[ -f defs.mk ]]; then
   echo "You must configure before you can compile!"
   return
 fi
 emk utest
 echo "======= Run utest with valgrind ======="
 readarray <<< `make utest VGD=1 2>&1`
 local -r es='ERROR SUMMARY:'
 local -r e0="$es 0 errors from 0 contexts (suppressed: 0 from 0)"
 local -i ret=0
 local lines
 for n in "${MAPFILE[@]}"; do
   if [[ "$n" =~ "$es" ]]; then
     # test fails
     if [[ "$n" =~ "$e0" ]]; then
       echo -E "$n"
     else
       echo -E "$lines$n"
       ret=1
     fi
     lines=''
   elif [[ "$n" =~ ^\[UTEST ]]; then
     echo -En "$n"
     lines=''
   elif [[ "$n" =~ ^==[0-9]+== ]]; then
     lines+="$n"
   fi
 done
 if [[ $ret -ne 0 ]]; then
   exit $ret
 fi
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
 for n in GPL-3.0-or-later LGPL-3.0-or-later BSD-3-Clause LGPL-2.1-or-later\
          GFDL-1.3-no-invariants-or-later GPL-2.0-or-later LGPL-2.1-only
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
 local list='build host TCL_MINVER PERL PY3_VER RUBY_VER PHP_VER
   LUA_VERS LUA_VER USE_ENDIAN PERL5_VER HAVE_LIBCHRONY_HEADER
   GO_MINVER DOTTOOL ASTYLE_MINVER HAVE_GTEST_HEADER HAVE_CRITERION_HEADER
   HAVE_GMOCK_HEADER CPPCHECK SWIG_MINVER DOXYGEN_MINVER CMARK MARKDOWN
   PANDOC PACKAGE_VERSION CXX_VERSION CXX CC_VERSION CC CHRPATH PATCHELF
   HAVE_SSL_HEADER HAVE_GCRYPT_HEADER HAVE_GNUTLS_HEADER HAVE_NETTLE_HEADER'
 local langs='tcl perl5 python3 ruby php lua go'
 local $list $langs
 read_defs $list
 local -A setLang
 setLang[tcl]="@'$TCL_MINVER'"
 setLang[perl5]="@'$PERL5_VER'"
 setLang[python3]="@'$PY3_VER'"
 setLang[ruby]="@'$RUBY_VER'"
 setLang[php]="@'$PHP_VER'"
 if [[ -n "$LUA_VERS" ]]; then
   setLang[lua]="@'$LUA_VERS'"
 else
   setLang[lua]="@'$LUA_VER'"
 fi
 setLang[go]="@'$GO_MINVER'"
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
 [[ "$build" = "$host" ]] && local -r bon='native' || local -r bon='cross'
 [[ -n "$ASTYLE_MINVER" ]] && local -r astyle="$ASTYLE_MINVER" ||\
   local -r astyle='x'
 if [[ -n "$CHRPATH" ]]; then
   local -r rpath="$CHRPATH"
 elif [[ -n "$PATCHELF" ]]; then
   local -r rpath="$PATCHELF"
 else
   local -r rpath='x'
 fi
 [[ -n "$HAVE_SSL_HEADER" ]] && local -r ssl='v' || local -r ssl='x'
 [[ -n "$HAVE_GCRYPT_HEADER" ]] && local -r gcrypt='v' || local -r gcrypt='x'
 [[ -n "$HAVE_GNUTLS_HEADER" ]] && local -r gnutls='v' || local -r gnutls='x'
 [[ -n "$HAVE_NETTLE_HEADER" ]] && local -r nettle='v' || local -r nettle='x'
 [[ -n "$HAVE_GTEST_HEADER" ]] && local -r gtest='v' || local -r gtest='x'
 [[ -n "$HAVE_GMOCK_HEADER" ]] && local -r gmock='v' || local -r gmock='x'
 [[ -n "$HAVE_CRITERION_HEADER" ]] && local -r crtest='v' || local -r crtest='x'
 [[ -n "$CPPCHECK" ]] && local -r cppcheck='v' || local -r cppcheck='x'
 [[ -n "$SWIG_MINVER" ]] && local -r swig="$SWIG_MINVER" || local -r swig='x'
 [[ -n "$DOXYGEN_MINVER" ]] && local -r doxy="$DOXYGEN_MINVER" || local -r doxy='x'
 [[ -n "$HAVE_LIBCHRONY_HEADER" ]] && local -r chrony='v' || local -r chrony='x'
 [[ -n "$MARKDOWN" ]] && local -r markdown='v' || local -r markdown='x'
 [[ -n "$CMARK" ]] && local -r cmark="$CMARK" || local -r cmark='x'
 [[ -n "$PANDOC" ]] && local -r pandoc="$PANDOC" || local -r pandoc='x'

 cat << EOF
========================== Config ==========================
Version '$PACKAGE_VERSION' build $bon endian $USE_ENDIAN
compilers $CXX $CXX_VERSION, $CC $CC_VERSION
libchrony '$chrony' markdown '$markdown' rpath '$rpath' cmark '$cmark'
ssl '$ssl' gcrypt '$gcrypt' gnutls '$gnutls' nettle '$nettle'
Doxygen '$doxy' dot '$dver' Pandoc '$pandoc'
cppcheck '$cppcheck' astyle '$astyle'
Google test '$gtest' Google test mock '$gmock' Criterion test '$crtest'
swig '$swig' Python '$python3' Ruby '$ruby' PHP '$php'
Perl '$perl5' go '$go' tcl '$tcl' Lua '$lua'
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
   local -r sa_file=/tmp/sa_file.cfg
 else
   local tmp=`mktemp -d`
   local -r cfg=$tmp/ptp4l.conf
   local -r sa_file=$tmp/sa_file.cfg
   local -r uds=$tmp/ptp4l
 fi
 cat << EOF > $cfg
[global]
clientOnly 1
use_syslog 0
userDescription testing with ptp4l
spp 1
active_key_id 1
sa_file $sa_file
EOF
 cat << EOF > $sa_file
[security_association]
spp 1
1 AES128 16 B64:IWnCCVk5+TcRExXy7OI7Sw
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
# Tag new release
tag_release()
{
 local -r version="$ver_maj.$ver_min"
 if git rev-list $version > /dev/null 2>&1; then
   return
 fi
 git tag -a -s -u DA12690F5898E40E -m "Version $version" $version
}
###############################################################################
main()
{
 local -r me1="$(realpath -s "$0")"
 local -r base_dir="$(realpath "$(dirname "$0")/..")"
 cd "$base_dir"
 local ver_maj ver_min
 source tools/util.sh
 source tools/version
 local -r dr1="$(dirname "$me1")"
 local -r me="${me1//$dr1\//}"
 local n
 case "$me" in
  ci_address.sh)     ci_address "$@";;
  ci_coverity.sh)    ci_coverity "$@";;
  ci_pages.sh)       ci_pages "$@";;
  def_params.sh)     def_params "$@";;
  github_docker.sh)  github_docker "$@";;
  gitlab_docker.sh)  gitlab_docker "$@";;
  ci_build.sh)       ci_build "$@";;
  ci_pkgs.sh)        ci_pkgs "$@";;
  ci_pkgs_no_dev.sh) ci_pkgs_no_dev "$@";;
  ci_abi.sh)         ci_abi "$@";;
  ci_abi_err.sh)     ci_abi_err "$@";;
  ci_cross.sh)       ci_cross "$@";;
  add_doxy_spdx.sh)  add_doxy_spdx "$@";;
  utest_address.sh)  utest_address "$@";;
  utest_valgrid.sh)  utest_valgrid "$@";;
  cp_license.sh)     cp_license "$@";;
  new_version.sh)    new_version "$@";;
  config_report.sh)  config_report "$@";;
  sim_ptp4l.sh)      sim_ptp4l "$@";;
  tag_release.sh)    tag_release "$@";;
 esac
}
main "$@"
