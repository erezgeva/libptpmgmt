#!/bin/bash
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# @author Christopher Hall <christopher.s.hall@intel.com>
# @copyright © 2024 Intel Corporation.
#
###############################################################################

SCRIPT_PATH="$(dirname "$0")"
TEST_PATH="${SCRIPT_PATH}/../client:/usr/local/lib/"

LD_LIBRARY_PATH=$TEST_PATH chrt -f 99 $SCRIPT_PATH/clkmgr_test "$@"
