#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2021 Erez Geva
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
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
  local n m left=''
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
main()
{
    local last_ret out
    # Make sure we output to STDOUT directly, no pipes
    # check our teminal support coulors
    if [[ -t 1 ]] && tput setaf 1; then
        local -r esc=`printf '\e['`
        local -r color_red=${esc}31m
        local -r color_blue=${esc}34m
        local -r color_norm=${esc}00m
        printf "$color_norm"
    fi
    cd $(dirname $(realpath $0))/..
    eacmd git rev-parse --is-inside-work-tree
    if [[ $last_ret -eq 0 ]]; then
        out=''
        local -r would='Would[[:space:]]remove[[:space:]]'
        make clean
        check_clean clean autom4te.cache/ config.log\
                          config.status configure defs.mk
        make distclean
        check_clean distclean
        # Reuse workd in side git
        if [[ -n "$(which reuse)" ]]; then
            echo " * Check files licenses with 'reuse'"
            ecmd reuse lint
            equit "'reuse' detect missing SPDX tags"
        fi
    else
        make clean
    fi
    echo " * Configure"
    autoconf
    ecmd dh_auto_configure
    equit "Configuratation fails"
    # Run syntax checking
    make checkall -j
    echo " * Run unit test"
    eacmd make utest -j
    equit "Unit test fails"
    echo " * Build Debian packages"
    eacmd make deb
    equit "Build Debian packages fails"
    echo "$color_blue * CI test done$color_norm"
}
main "$@"
