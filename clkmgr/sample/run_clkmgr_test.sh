#!/bin/bash -e
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# @author Christopher Hall <christopher.s.hall@@intel.com>
# @copyright © 2024 Intel Corporation.
#
###############################################################################

main()
{
 local -r script_path="$(dirname "$(realpath "$0")")"
 local -r script_name="$(basename "$0")"
 local -r b="${script_name/run_/}"
 local -r sample_name="${b/.sh/}"
 local -r test_path="${script_path}/../../.libs/:/usr/local/lib/"
 if  [[ $(id -u) -eq 0 ]] && command -v chrt > /dev/null; then
  local -r use_chrt='chrt -f 99'
 fi
 LD_LIBRARY_PATH=$test_path $use_chrt $script_path/$sample_name "$@"
}
main "$@"
