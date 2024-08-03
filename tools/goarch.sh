# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Convert GNU target to GOARCH
###############################################################################
main()
{
  [[ -n "$1" ]] || return
  local GOARCH
  case "$1" in
    i686) GOARCH='386';;
    x86_64) GOARCH='amd64';;
    aarch64) GOARCH='arm64';;
    powerpc64) GOARCH='ppc64';;
    arm|mips|mips64|riscv64|s390|s390x|sparc|sparc64) GOARCH="$1";;
    *)
      if [[ -n "`which dpkg-architecture 2> /dev/null`" ]]; then
        GOARCH="`dpkg-architecture -qDEB_TARGET_GNU_CPU -a$1 2> /dev/null`"
      else
        GOARCH="$1"
      fi
      ;;
  esac
  printf "$GOARCH"
}
main "$@"
