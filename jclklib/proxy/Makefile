##! \file Makefile
#   \brief Proxy makefile
#
#   (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
#   Author: Christopher Hall <christopher.s.hall@intel.com>
#

JCLKLIB_PROXY_DIR := $(shell pwd)
JCLKLIB_TOPLEVEL_DIR = $(JCLKLIB_PROXY_DIR)/..
JCLKLIB_COMMON_DIR = $(JCLKLIB_TOPLEVEL_DIR)/common
LIBS = pthread rt ptpmgmt
LIBS_FLAGS = $(foreach lib,$(LIBS),-l$(lib))
OBJ = client main message msgq_tport notification_msg null_tport connect_msg subscribe_msg transport connect
COMMON_OBJ = print sighandler transport msgq_tport jclklib_import message connect_msg
OBJ_FILES = $(foreach f,$(OBJ),$(f).o)
DEPENDS = $(foreach f,$(OBJ),$(f).d)
COMMON_OBJ_DIR = $(JCLKLIB_COMMON_DIR)/obj
COMMON_OBJ_FILES = $(foreach obj,$(COMMON_OBJ),$(COMMON_OBJ_DIR)/$(obj).o)
REAL_TARGETS = jclklib_proxy

.PHONY: default
default:
	$(MAKE) -s -C $(JCLKLIB_PROXY_DIR) build-all 2>&1

.PHONY: build-prep
build-prep:
	echo "Running" $@
	$(MAKE) -C $(JCLKLIB_PROXY_DIR) $(DEPENDS) 2>&1


%.d : %.cpp
	echo "[MAKEDEP]" $@
	g++ -I $(JCLKLIB_TOPLEVEL_DIR) -MM -MF $@ $<

.PHONY: clean
clean:
	$(RM) -rf *.o *.d jclklib_proxy

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
	$(MAKE) -C $(JCLKLIB_PROXY_DIR) $(REAL_TARGETS)

jclklib_proxy: $(OBJ_FILES) $(COMMON_OBJ_FILES)
	echo "[LINK]" $@ "{" $(call pathof_relative_to,$^,$(JCLKLIB_TOPLEVEL_DIR)) "}" | fold -s
	g++ -o $@ $^ $(LIBS_FLAGS) -fdiagnostics-color=always

%.o : %.cpp
	echo "[COMPILE]" $<
	g++ -c $< -g -I $(JCLKLIB_TOPLEVEL_DIR) -fdiagnostics-color=always

