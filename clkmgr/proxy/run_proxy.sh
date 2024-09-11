#!/bin/bash
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# @author Christopher Hall <christopher.s.hall@intel.com>
# @copyright © 2024 Intel Corporation.
# @license BSD-3-Clause
#
###############################################################################

SCRIPT_PATH="$(dirname "$0")"
TEST_PATH="${SCRIPT_PATH}/../../.libs"

LD_LIBRARY_PATH=$TEST_PATH $SCRIPT_PATH/clkmgr_proxy "$@"
