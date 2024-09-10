# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# Makefile for clkmgr
#
# @author Christopher Hall <christopher.s.hall@intel.com>
# @copyright © 2024 Intel Corporation.
#
###############################################################################

CLKMGR_LIB:=libclkmgr
CLKMGR_LIB_SO:=$(LIB_D)/$(CLKMGR_LIB).so
CLKMGR_LIB_A:=$(LIB_D)/$(CLKMGR_LIB).a
CLKMGR_LIB_LA:=$(CLKMGR_LIB).la

CLKMGR_PUB_DIR = $(CLKMGR_DIR)/pub
CLKMGR_COMMON_DIR:=$(CLKMGR_DIR)/common
CLKMGR_CLIENT_DIR:=$(CLKMGR_DIR)/client
CLKMGR_PROXY_DIR:=$(CLKMGR_DIR)/proxy

CLKMGR_PROXY:=$(CLKMGR_PROXY_DIR)/clkmgr_proxy

CLKMGR_LDLIBS:=$(foreach lib,pthread rt rtpi,-l$(lib))

include $(wildcard $(CLKMGR_DIR)/*/*.d)

CLKMGR_CXXFLAGS:=-I$(CLKMGR_PUB_DIR) -I$(CLKMGR_DIR)

CLKMGR_COMMON_SRCS:=$(wildcard $(CLKMGR_COMMON_DIR)/*.cpp)
CLKMGR_COMMON_LOBJS:=$(CLKMGR_COMMON_SRCS:.cpp=.lo)
CLKMGR_COMMON_OBJS:=$(CLKMGR_COMMON_SRCS:.cpp=.o)
CLKMGR_CLIENT_SRCS:=$(wildcard $(CLKMGR_CLIENT_DIR)/*.cpp)
CLKMGR_CLIENT_LOBJS:=$(CLKMGR_CLIENT_SRCS:.cpp=.lo)
CLKMGR_PROXY_SRCS:=$(wildcard $(CLKMGR_PROXY_DIR)/*.cpp)
CLKMGR_PROXY_OBJS:=$(CLKMGR_PROXY_SRCS:.cpp=.o)

$(CLKMGR_COMMON_LOBJS): $(CLKMGR_COMMON_DIR)/%.lo: $(CLKMGR_COMMON_DIR)/%.cpp
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(CLKMGR_CXXFLAGS) $< -o $@
$(CLKMGR_CLIENT_LOBJS): $(CLKMGR_CLIENT_DIR)/%.lo: $(CLKMGR_CLIENT_DIR)/%.cpp
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(CLKMGR_CXXFLAGS) $< -o $@
$(CLKMGR_PROXY_OBJS): $(CLKMGR_PROXY_DIR)/%.o: $(CLKMGR_PROXY_DIR)/%.cpp |\
	$(COMP_DEPS)
	$(Q_CC)$(CXX) $(CXXFLAGS) $(CLKMGR_CXXFLAGS) -c -o $@ $<

$(CLKMGR_LIB_LA)_LDLIBS:=$(CLKMGR_LDLIBS)
$(CLKMGR_LIB_LA): $(CLKMGR_COMMON_LOBJS) $(CLKMGR_CLIENT_LOBJS)

$(CLKMGR_PROXY): $(CLKMGR_COMMON_OBJS) $(CLKMGR_PROXY_OBJS) $(LIB_NAME_SO) |\
	$(CLKMGR_COMMON_LOBJS)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS) -o $@

ALL+=$(CLKMGR_LIB_LA) $(CLKMGR_PROXY)
