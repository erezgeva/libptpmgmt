# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# Makefile for clkmgr proxy
#
# @author Christopher Hall <christopher.s.hall@intel.com>
# @copyright © 2024 Intel Corporation.
# @license BSD-3-Clause
#
###############################################################################

CLKMGR_PROXY_DIR := $(shell pwd)
CLKMGR_TOPLEVEL_DIR = $(CLKMGR_PROXY_DIR)/..
CLKMGR_COMMON_DIR = $(CLKMGR_TOPLEVEL_DIR)/common
LIBPTPMGMT_PUB_DIR = $(CLKMGR_TOPLEVEL_DIR)/../pub/
PTPMGMT_DIR := $(CLKMGR_TOPLEVEL_DIR)/../.libs
LIBS = pthread rt ptpmgmt rtpi
LIBS_FLAGS = $(foreach lib,$(LIBS),-l$(lib)) -L$(PTPMGMT_DIR)
OBJ = client main message msgq_tport notification_msg null_tport connect_msg subscribe_msg transport connect_ptp4l thread
COMMON_OBJ = print sighandler transport msgq_tport clkmgr_subscription message connect_msg subscribe_msg notification_msg
OBJ_FILES = $(foreach f,$(OBJ),$(f).o)
DEPENDS = $(foreach f,$(OBJ),$(f).d)
COMMON_OBJ_DIR = $(CLKMGR_COMMON_DIR)/obj
COMMON_OBJ_FILES = $(foreach obj,$(COMMON_OBJ),$(COMMON_OBJ_DIR)/$(obj).o)
REAL_TARGETS = clkmgr_proxy

.PHONY: default
default:
	$(MAKE) -s -C $(CLKMGR_PROXY_DIR) build-all 2>&1

.PHONY: build-prep
build-prep:
	echo "Running" $@
	$(MAKE) -C $(CLKMGR_PROXY_DIR) $(DEPENDS) 2>&1


%.d : %.cpp
	echo "[MAKEDEP]" $@
	g++ -I $(CLKMGR_TOPLEVEL_DIR) -I $(LIBPTPMGMT_PUB_DIR) -MM -MF $@ $<

.PHONY: clean
clean:
	$(RM) -rf *.o *.d clkmgr_proxy

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

pathof_relative_to = $(foreach abpth,$(foreach dep,$1,$(shell realpath $(dep))),\
$(shell realpath --relative-to="$2" $(abpth)))

.PHONY: build-all
build-all: build-prep
	echo "Running" $@
	$(MAKE) -C $(CLKMGR_PROXY_DIR) $(REAL_TARGETS)

clkmgr_proxy: $(OBJ_FILES) $(COMMON_OBJ_FILES)
	echo "[LINK]" $@ "{" $(call pathof_relative_to,$^,$(CLKMGR_TOPLEVEL_DIR)) "}" | fold -s
	g++ -o $@ $^ $(LIBS_FLAGS) -fdiagnostics-color=always

%.o : %.cpp
	echo "[COMPILE]" $<
	g++ -c $< -I $(CLKMGR_TOPLEVEL_DIR) -I $(LIBPTPMGMT_PUB_DIR) -fdiagnostics-color=always

