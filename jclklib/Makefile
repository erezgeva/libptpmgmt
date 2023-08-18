##! \file Makefile
#   \brief top level makefile
#
#   (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
#   Author: Christopher Hall <christopher.s.hall@intel.com>
#

CWD := $(shell pwd)

.PHONY: all
all:
	$(MAKE) -C $(CWD)/common default
	$(MAKE) -C $(CWD)/proxy default
	$(MAKE) -C $(CWD)/client default

.PHONY: clean
clean:
	$(MAKE) -C $(CWD)/client clean
	$(MAKE) -C $(CWD)/proxy clean
	$(MAKE) -C $(CWD)/common clean
