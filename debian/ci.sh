#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# CI (Continous integration) script for containers
###############################################################################
ecmd()
{
    set +e
    out=`$@`
    last_ret=$?
    set -e
}
eacmd()
{
    set +e
    out=`$@ 2>&1`
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
}
test_clean()
{
    if $have_git; then
        out=''
        make clean
        check_clean clean $clean_list $distclean_list
        make distclean
        check_clean distclean $distclean_list
    fi
}
main()
{
    local -r jobs=3  # Number of Make parallel jobs
    local n m last_ret out distclean_list
    # Make sure we output to STDOUT directly, no pipes
    # check our teminal support coulors
    if [[ -t 1 ]] && tput setaf 1; then
        local -r esc=`printf '\e['`
        local -r color_red=${esc}31m
        local -r color_blue=${esc}34m
        local -r color_norm=${esc}00m
        printf "$color_norm"
    fi
    local clean_list="autom4te.cache/ config.log config.status"
    clean_list+=" configure defs.mk"
    for n in archlinux debian gentoo rpm; do
      distclean_list+=" $n/.upgrade_cockie"
    done
    cd $(dirname $(realpath $0))/..
    eacmd git rev-parse --is-inside-work-tree
    if [[ $last_ret -eq 0 ]]; then
        local -r have_git=true
        local -r would='Would[[:space:]]remove[[:space:]]'
        test_clean
    else
        local -r have_git=false
        # Make sure we do not have leftovers
        ecmd make clean
    fi
    if $have_git && [[ -n "$(which reuse)" ]]; then
        echo " * Check files licenses with 'reuse'"
        ecmd reuse lint
        equit "'reuse' detect missing SPDX tags"
    fi
    echo " * Configure"
    autoconf
    ecmd dh_auto_configure
    equit "Configuratation fails"
    # Run syntax checking
    make checkall -j$jobs
    echo " * Run unit test"
    eacmd make utest -j$jobs
    equit "Unit test fails"
    echo " * Build"
    ecmd make -j$jobs
    equit "Build fails"
    test_clean
    echo " * Build Debian packages"
    eacmd make deb
    equit "Build Debian packages fails"
    test_clean
    echo "$color_blue * CI test done$color_norm"
}
main "$@"
