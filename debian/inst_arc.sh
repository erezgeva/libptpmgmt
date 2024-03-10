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
 if [[ -n "$1" ]]; then
  local n
  local -r a="$1"
  local -a l
  for n in libgcrypt20-dev
  do l+=($n:$a);done
  apt-get update
  apt-get install -y --no-install-recommends ${l[@]}
 fi
}
main "$@"
