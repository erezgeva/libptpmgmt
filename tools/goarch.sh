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
  [[ -n "$2" ]] || return
  local GOARCH bits
  case "$1" in
    i686)
      GOARCH="386"
      bits="32"
      ;;
    x86_64)
      GOARCH="amd64"
      bits="64"
      ;;
    arm)
      GOARCH="arm"
      bits="32"
      ;;
    aarch64)
      GOARCH="arm64"
      bits="64"
      ;;
    powerpc64)
      GOARCH="ppc64"
      bits="64"
      ;;
    mips)
      GOARCH="mips"
      bits="32"
      ;;
    mips64)
      GOARCH="mips64"
      bits="64"
      ;;
    riscv64)
      GOARCH="riscv64"
      bits="64"
      ;;
    s390)
      GOARCH="s390"
      bits="32"
      ;;
    s390x)
      GOARCH="s390x"
      bits="64"
      ;;
    sparc)
      GOARCH="sparc"
      bits="32"
      ;;
    sparc64)
      GOARCH="sparc64"
      bits="64"
      ;;
    *)
      if [[ -n "`which dpkg-architecture 2> /dev/null`" ]]; then
        GOARCH="`dpkg-architecture -qDEB_TARGET_GNU_CPU -a$1 2> /dev/null`"
        bits="`dpkg-architecture -qDEB_TARGET_ARCH_BITS -a$1 2> /dev/null`"
      else
        GOARCH="$1"
        bits=`getconf LONG_BIT`
      fi
      ;;
  esac
  local -n v=$2
  printf "$v"
}
main "$@"
