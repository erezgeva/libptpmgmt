# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# Makefile for jclklib common
#
# @author Christopher Hall <christopher.s.hall@intel.com>
# @copyright © 2024 Intel Corporation.
# @license BSD-3-Clause
#
###############################################################################

JCLKLIB_COMMON_DIR := $(shell pwd)
JCLKLIB_TOPLEVEL_DIR := $(JCLKLIB_COMMON_DIR)/..
LIBPTPMGMT_PUB_DIR = $(JCLKLIB_TOPLEVEL_DIR)/../pub/
LIBS = pthread rt rtpi
LIBS_FLAGS = $(foreach lib,$(LIBS),-l$(lib))
OBJ = jclk_subscription message msgq_tport notification_msg null_msg connect_msg print sighandler subscribe_msg transport
OBJ_FILES = $(foreach f,$(OBJ),$(f).o)
DEPENDS = $(foreach f,$(OBJ),$(f).d)
OBJ_DIR = $(JCLKLIB_COMMON_DIR)/obj

.PHONY: default
default:
	$(MAKE) -s -C $(JCLKLIB_COMMON_DIR) build-all 2>&1

.PHONY: build-prep
build-prep:
	echo "Running" $@
	$(MAKE) -C $(JCLKLIB_COMMON_DIR) $(DEPENDS) 2>&1


%.d : %.cpp
	echo "[MAKEDEP]" $@
	g++ -I $(JCLKLIB_TOPLEVEL_DIR) -I $(LIBPTPMGMT_PUB_DIR) -MM -MF $@ $<

.PHONY: clean
clean:
	$(RM) -rf *.o *.d $(OBJ_DIR)

ifneq ($(MAKECMDGOALS),)
ifeq ($(findstring build-prep,$(MAKECMDGOALS)),)
ifeq ($(findstring build-all,$(MAKECMDGOALS)),)
ifeq ($(findstring clean,$(MAKECMDGOALS)),)
ifneq ($(suffix $(MAKECMDGOALS)),.d)
-include $(DEPENDS)
endif
endif
endif
endif
endif

.PHONY: build-all
build-all: build-prep
	echo "Running" $@
	$(MAKE) -C $(JCLKLIB_COMMON_DIR) common-obj

.PHONY: common-obj
common-obj: $(OBJ_FILES)
	echo "[LINK]" $@
	install -d $(OBJ_DIR)
	install -t $(OBJ_DIR) $?

%.o : %.cpp
	echo "[COMPILE]" $<
	g++ -c $< -I $(JCLKLIB_TOPLEVEL_DIR) -I $(LIBPTPMGMT_PUB_DIR) -fPIC -Werror -Wall -Wextra -Wno-unused-parameter -fdiagnostics-color=always
