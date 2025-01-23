#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2024 Erez Geva
#
# Install Architecture packages needed for cross debian build
#  which do not have the 'Multi-Arch: same' flag
###############################################################################
main()
{
 local -r a="$1"
 if [[ -n "$a" ]]; then
  local n
  local -a l
  for n in libgcrypt20-dev
  do
   local p="$n:$a"
   if ! dpkg-query -s $p 2>/dev/null |\
    grep -q '^Status: install ok installed'; then
    l+=($p)
   fi
  done
  if [[ ${#l[@]} -gt 0 ]]; then
   export DEBIAN_FRONTEND=noninteractive
   apt-get update
   apt-get install -y --no-install-recommends ${l[@]}
  fi
 fi
}
main "$@"
