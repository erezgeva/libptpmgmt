# SPDX-License-Identifier: LGPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
#
# Makefile for jclklib
#
# @author Christopher Hall <christopher.s.hall@intel.com>
# @copyright © 2024 Intel Corporation. All rights reserved.
# @license LGPL-3.0-or-later
#
###############################################################################

CWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

.PHONY: jclklib
jclklib:
	$(MAKE) -C $(CWD)/common default
	$(MAKE) -C $(CWD)/proxy default
	$(MAKE) -C $(CWD)/client default

.PHONY: jclklib_clean
jclklib_clean:
	$(MAKE) -C $(CWD)/client clean
	$(MAKE) -C $(CWD)/proxy clean
	$(MAKE) -C $(CWD)/common clean
