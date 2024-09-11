# SPDX-License-Identifier: LGPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
#
# Makefile for clkmgr
#
# @author Christopher Hall <christopher.s.hall@intel.com>
# @copyright © 2024 Intel Corporation. All rights reserved.
# @license LGPL-3.0-or-later
#
###############################################################################

CWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

.PHONY: clkmgr
clkmgr:
	$(MAKE) -C $(CWD)/common default
	$(MAKE) -C $(CWD)/proxy default
	$(MAKE) -C $(CWD)/client default

.PHONY: clkmgr_clean
clkmgr_clean:
	$(MAKE) -C $(CWD)/client clean
	$(MAKE) -C $(CWD)/proxy clean
	$(MAKE) -C $(CWD)/common clean
