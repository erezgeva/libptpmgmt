##! \file Makefile
#   \brief Common makefile
#
#   (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
#   Author: Christopher Hall <christopher.s.hall@intel.com>
#

JCLKLIB_COMMON_DIR := $(shell pwd)
JCLKLIB_TOPLEVEL_DIR := $(JCLKLIB_COMMON_DIR)/..
LIBS = pthread rt
LIBS_FLAGS = $(foreach lib,$(LIBS),-l$(lib))
OBJ = jclklib_import message msgq_tport notification_msg null_msg connect_msg print sighandler subscribe_msg transport util
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
	g++ -I $(JCLKLIB_TOPLEVEL_DIR) -MM -MF $@ $<

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
	g++ -c $< -g -I $(JCLKLIB_TOPLEVEL_DIR) -fPIC -Werror -Wall -fdiagnostics-color=always

