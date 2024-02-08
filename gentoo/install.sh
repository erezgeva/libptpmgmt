#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2024 Erez Geva
#
# script to install the libptpmgmt on gentoo system
#  and copy defs.mk configuration file for further testing.
# Call this script after build!
###############################################################################
cmd()
{
  echo "$@"
  "$@"
}
main()
{
  local -r me="$(realpath $0)"
  local -r base="$(realpath $(dirname $0)/..)"
  if ! [[ -d /var/db/repos/gentoo/net-libs/libptpmgmt ]]; then
    return
  fi
  if [[ $(id -u) -ne 0 ]]; then
    sudo "$me"
    return
  fi
  if ! [[ -f "$base/defs.mk" ]]; then
    cp /var/tmp/portage/net-libs/libptpmgmt*/work/libptpmgmt*/defs.mk "$base"/
  fi
  cd /var/db/repos/gentoo/net-libs/libptpmgmt
  # merge image into live filesystem
  cmd ebuild libptpmgmt*.ebuild qmerge
}
if [[ -n "$(which ebuild 2> /dev/null)" ]]; then
  main
fi
