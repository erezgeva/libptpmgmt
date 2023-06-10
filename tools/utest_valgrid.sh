#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Run unit tests with valgrind tool
###############################################################################
main()
{
 cd "$(dirname "$(realpath "$0")")/.."
 [[ -f defs.mk ]] || make config
 make utest
 echo "======= Run utest with valgrind ======="
 local n
 readarray <<< `make utest VGD=1 2>&1`
 local -r es='ERROR SUMMARY:'
 local -r e0="$es 0 errors from 0 contexts (suppressed: 0 from 0)"
 for n in "${MAPFILE[@]}"; do
   if [[ "$n" =~ "$es" ]]; then
     echo -En "$n"
     # test fails
     [[ "$n" =~ "$e0" ]] || exit 1
   elif [[ "$n" =~ ^\[UTEST ]]; then
     echo -En "$n"
   fi
 done
}
main "$@"
