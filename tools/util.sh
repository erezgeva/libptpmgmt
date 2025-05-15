#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2024 Erez Geva
#
# Utils for scripts
###############################################################################
# Find distrubution
# [out] dist dname
distribution()
{
 if [[ -f '/etc/os-release' ]]; then
   dname="$(grep ^NAME /etc/os-release | sed 's/^NAME=//;s/"//g')"
 fi
 if [[ -f /etc/debian_version ]]; then
   # Also Ubuntu and other Debian based distributions
   dist=debian
 else
   local n m
   for n in /etc/*-release; do
     if [[ -f "$n" ]]; then
       case $n in
         /etc/os-release) ;;
         /etc/system-release) ;;
         *)
            m="${n%-release}"
            dist="${m#/etc/}"
            break
            ;;
       esac
     fi
   done
 fi
}
###############################################################################
# Read defs
# [out] -A R
read_defs()
{
 [[ -f defs.mk ]] || return
 local -r export_list="$@"
 local var val n
 mapfile < defs.mk
 for n in "${MAPFILE[@]}"; do
  if [[ $n =~ ^# ]]; then
    continue
  fi
  if [[ $n =~ (.*):=(.*) ]]; then
    var="${BASH_REMATCH[1]}"
    val="${BASH_REMATCH[2]/$'\n'/}"
    # remove leading whitespace characters
    val="${val#"${val%%[![:space:]]*}"}"
    # remove trailing whitespace characters
    val="${val%"${val##*[![:space:]]}"}"
    if [[ -n "$val" ]]; then
      R["$var"]="$val"
    fi
  fi
 done
 if [[ -n "$export_list" ]]; then
  for n in $export_list; do
   val="$(eval "echo '${R["$n"]}'")"
   eval "$n=\"$val\""
  done
 fi
}
