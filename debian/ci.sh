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
    if [[ -n "$(which reuse)" ]]; then
        echo " * Check files licenses with 'reuse'"
        ecmd reuse lint
        equit "'reuse' detect missing SPDX tags"
    fi
    # We run Debian package first, as it create 'def.mk' we need
    echo " * Build Debian packages"
    eacmd make deb
    equit "Build Debian packages fails"
    make checkall -j
    echo " * Run unit test"
    ecmd make utest -j
    equit "Unit test fails"
    eacmd git rev-parse --is-inside-work-tree
    if [[ $last_ret -eq 0 ]]; then
        out=''
        make clean
        local n m left=''
        local clean_left='autom4te.cache/ config.log
                          config.status configure defs.mk'
        readarray <<< `git clean -fxdn`
        for n in "${MAPFILE[@]}"; do
            if [[ -z "${n%$'\n'}" ]]; then
                continue
            fi
            for m in $clean_left; do
                if [[ $n =~ ^Would[[:space:]]remove[[:space:]]$m ]]; then
                    continue 2
                fi
            done
            if [[ -n "${n%$'\n'}" ]]; then
                left+="$n"
            fi
        done
        if [[ -n "$left" ]]; then
            printf "$color_red|$left|"
            mquit "Make clean left unused files"
        fi
        make distclean
        left=`git clean -fxdn`
        if [[ -n "$left" ]]; then
            echo "$color_red$left"
            mquit "Make clean left unused files"
        fi
    fi
    echo "$color_blue * CI test done$color_norm"
}
main "$@"
