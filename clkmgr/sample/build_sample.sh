#!/bin/bash -e
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation.
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2025 Intel Corporation.
#
# Create sample using the installed clock manager development package
#
###############################################################################

main()
{
  local -r base_dir="$(realpath "$(dirname "$0")")"
  local n
  # probe C compiler
  for n in cc gcc clang; do
    if [[ -n "$CC" ]]; then
        break
    fi
    if [[ -x "$(which $n)" ]]; then
      CC=$n
      break
    fi
  done
  # probe C++ compiler
  for n in c++ g++ clang++; do
    if [[ -n "$CXX" ]]; then
        break
    fi
    if [[ -x "$(which $n)" ]]; then
      CXX=$n
      break
    fi
  done
  # Ensure we have both compilers
  if [[ -z "$CC" ]] || [[ -z "$CXX" ]]; then
    echo "Lack compilers CC=$CC CXX=$CXX"
    exit 255
    return
  fi
  # change to sample directory
  cd $base_dir
  # Build sample code
  $CXX -Wall clkmgr_test.cpp -o clkmgr_test -lclkmgr -lrtpi
  $CC -Wall clkmgr_c_test.c -o clkmgr_c_test -lclkmgr -lrtpi
}
main "$@"
