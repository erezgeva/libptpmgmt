##! \file Makefile
#   \brief Client makefile
#
#   (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
#   Author: Christopher Hall <christopher.s.hall@intel.com>
#

JCLKLIB_CLIENT_DIR := $(shell pwd)
JCLKLIB_TOPLEVEL_DIR = $(JCLKLIB_CLIENT_DIR)/..
JCLKLIB_COMMON_DIR = $(JCLKLIB_TOPLEVEL_DIR)/common
LIBS = pthread rt
LIBS_FLAGS = $(foreach lib,$(LIBS),-l$(lib))
OBJ = init msgq_tport message null_tport transport msgq_tport notification_msg connect_msg client_state subscribe_msg
COMMON_OBJ = print sighandler transport msgq_tport message connect_msg subscribe_msg notification_msg jclklib_import
OBJ_FILES = $(foreach f,$(OBJ),$(f).o)
DEPENDS = $(foreach f,$(OBJ),$(f).d)
COMMON_OBJ_DIR = $(JCLKLIB_COMMON_DIR)/obj
COMMON_OBJ_FILES = $(foreach obj,$(COMMON_OBJ),$(COMMON_OBJ_DIR)/$(obj).o)
REAL_TARGETS = jclklib.so test

.PHONY: default
default:
	$(MAKE) -s -C $(JCLKLIB_CLIENT_DIR) build-all 2>&1

.PHONY: build-prep
build-prep:
	echo "Running" $@
	$(MAKE) -C $(JCLKLIB_CLIENT_DIR) $(DEPENDS) 2>&1


%.d : %.cpp
	echo "[MAKEDEP]" $@
	g++ -I $(JCLKLIB_TOPLEVEL_DIR) -MM -MF $@ $<

.PHONY: clean
clean:
	$(RM) -rf *.o *.d $(REAL_TARGETS)

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

pathof_relative_to = $(foreach abpth,$(foreach dep,$1,$(shell realpath $(dep))), \
$(shell realpath --relative-to="$2" $(abpth)))

.PHONY: build-all
build-all: build-prep
	echo "Running" $@
	$(MAKE) -C $(JCLKLIB_CLIENT_DIR) $(REAL_TARGETS)

jclklib.so: $(OBJ_FILES) $(COMMON_OBJ_FILES)
	echo "[LINK]" $@ "{" $(call pathof_relative_to,$^,$(JCLKLIB_TOPLEVEL_DIR)) "}" | fold -s
	g++ -shared -o $@ $^ $(LIBS_FLAGS) -fdiagnostics-color=always

test: test.o jclklib.so
	g++ -o test test.o -L $(JCLKLIB_CLIENT_DIR) -l:jclklib.so

%.o : %.cpp
	echo "[COMPILE]" $<
	g++ -c $< -g -I $(JCLKLIB_TOPLEVEL_DIR) -fPIC -fdiagnostics-color=always
