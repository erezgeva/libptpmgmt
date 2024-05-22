#!/bin/bash
# SPDX-License-Identifier: LGPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
#
# @author Christopher Hall <christopher.s.hall@intel.com>
# @copyright © 2024 Intel Corporation. All rights reserved.
# @license LGPL-3.0-or-later
#
###############################################################################

SCRIPT_PATH="$(dirname "$0")"
TEST_PATH="${SCRIPT_PATH}/../client"

LD_LIBRARY_PATH=$TEST_PATH chrt -f 99 $SCRIPT_PATH/jclk_test "$@"
