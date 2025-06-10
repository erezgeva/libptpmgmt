# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation.
#
# Makefile for Clock Manager unit tests
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2025 Intel Corporation.
#
###############################################################################

ifdef GTEST_LIB_FLAGS

CLKMGR_CLIENT_UTEST:=$(CLKMGR_UTEST_DIR)/utest_client
CLKMGR_CLIENT_SRC:=timebase_configs subscription client_state event
CLKMGR_CLIENT_OBJS:=$(foreach n,$(CLKMGR_CLIENT_SRC),$(CLKMGR_UTEST_DIR)/$n.o)

CLKMGR_COMMON_UTEST:=$(CLKMGR_UTEST_DIR)/utest_common
CLKMGR_COMMON_SRC:=buffer
CLKMGR_COMMON_OBJS:=$(foreach n,$(CLKMGR_COMMON_SRC),$(CLKMGR_UTEST_DIR)/$n.o)

CLKMGR_MSG_UTEST:=$(CLKMGR_UTEST_DIR)/utest_message
CLKMGR_MSG_SRC:=connect_msg subscribe_msg notification_msg
CLKMGR_MSG_UTEST_OBJS:=$(foreach n,$(CLKMGR_MSG_SRC),$(CLKMGR_UTEST_DIR)/$n.o)
CLKMGR_MSG_SRCS:=$(wildcard $(CLKMGR_DIR)/*/*_msg.cpp)
CLKMGR_MSG_OBJS:=$(CLKMGR_MSG_SRCS:.cpp=.o)
CLKMGR_MSG_OBJS+=$(foreach n,message msgq_tport print termin sighandler,\
  $(CLKMGR_COMMON_DIR)/$n.o)

CLKMGR_API_UTEST:=$(CLKMGR_UTEST_DIR)/utest_api
CLKMGR_API_SRC:=clockmanager
CLKMGR_API_UTEST_OBJS:=$(foreach n,$(CLKMGR_API_SRC),$(CLKMGR_UTEST_DIR)/$n.o)
CLKMGR_API_SRCS:=$(CLKMGR_CLIENT_DIR)/clockmanager.cpp
CLKMGR_API_OBJS:=$(CLKMGR_API_SRCS:.cpp=.o)
CLKMGR_API_OBJS+=$(foreach n,print termin,$(CLKMGR_COMMON_DIR)/$n.o)
CLKMGR_API_OBJS+=$(foreach n,timebase_configs subscription clock_event,\
  $(CLKMGR_CLIENT_DIR)/$n.o)

$(CLKMGR_UTEST_DIR)/%.o: $(CLKMGR_UTEST_DIR)/%.cpp | $(COMP_DEPS)
	$(Q_CC)$(CXX) $(CXXFLAGS_UTEST) $(CLKMGR_CXXFLAGS) $(GTEST_INC_FLAGS)\
	  -include $(HAVE_GTEST_HEADER) -c -o $@ $<
$(CLKMGR_CLIENT_UTEST): $(OBJ_DIR)/utest_m.o $(CLKMGR_CLIENT_OBJS) \
	$(CLKMGR_LIB_A) | $(CLKMGR_LIB_LA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_clkmgr_client: $(HEADERS_GEN_COMP) $(CLKMGR_CLIENT_UTEST)
	$(call Q_UTEST,ClkMgr client C++)$(UVGD)$(CLKMGR_CLIENT_UTEST)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)
$(CLKMGR_COMMON_UTEST): $(OBJ_DIR)/utest_m.o $(CLKMGR_COMMON_OBJS) \
	$(CLKMGR_LIB_A) | $(CLKMGR_LIB_LA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_clkmgr_common: $(HEADERS_GEN_COMP) $(CLKMGR_COMMON_UTEST)
	$(call Q_UTEST,ClkMgr common C++)$(UVGD)$(CLKMGR_COMMON_UTEST)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)
$(CLKMGR_MSG_UTEST): $(OBJ_DIR)/utest_m.o $(CLKMGR_MSG_UTEST_OBJS)\
	$(CLKMGR_MSG_OBJS)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_clkmgr_message: $(HEADERS_GEN_COMP) $(CLKMGR_MSG_UTEST)
	$(call Q_UTEST,ClkMgr message C++)$(UVGD)$(CLKMGR_MSG_UTEST)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)
$(CLKMGR_API_UTEST): $(OBJ_DIR)/utest_m.o $(CLKMGR_API_UTEST_OBJS)\
	$(CLKMGR_API_OBJS)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_clkmgr_api: $(HEADERS_GEN_COMP) $(CLKMGR_API_UTEST)
	$(call Q_UTEST,ClkMgr api C++)$(UVGD)$(CLKMGR_API_UTEST)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)

endif #GTEST_LIB_FLAGS

CLKMGR_UTESTS:=$(addprefix utest_clkmgr_,client common message api)
.PHONY: $(CLKMGR_UTESTS)
utest_clkmgr: $(CLKMGR_GEN_SRC) $(CLKMGR_UTESTS)
