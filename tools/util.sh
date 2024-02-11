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
# [out] dist
distribution()
{
 if [[ -f /etc/debian_version ]]; then
   dist=debian
 else
   for n in /etc/*-release; do
     m="${n%-release}"
     dist="${m#/etc/}"
     break
   done
 fi
}
###############################################################################
# Read defs
# [out] -A R
read_defs()
{
 [[ -f defs.mk ]] || retrurn
 local l="$@"
 local var val
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
  if [[ -n "$l" ]]; then
  local n a
  for n in $l; do
   a="$(eval "echo '${R["$n"]}'")"
   eval "$n=\"$a\""
  done
 fi
}
