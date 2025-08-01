# SPDX-License-Identifier: BSD-3-Clause
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
CLKMGR_CLIENT_UTEST_SRCS:=timebase_configs subscription client_state event
CLKMGR_CLIENT_UTEST_OBJS:=$(foreach n,$(CLKMGR_CLIENT_UTEST_SRCS),\
  $(CLKMGR_UTEST_DIR)/$n.o)

CLKMGR_COMMON_UTEST:=$(CLKMGR_UTEST_DIR)/utest_common
CLKMGR_COMMON_UTEST_SRCS:=buffer
CLKMGR_COMMON_UTEST_OBJS:=$(foreach n,$(CLKMGR_COMMON_UTEST_SRCS),\
  $(CLKMGR_UTEST_DIR)/$n.o)

CLKMGR_MSG_UTEST:=$(CLKMGR_UTEST_DIR)/utest_message
CLKMGR_MSG_UTEST_SRCS:=connect_msg subscribe_msg notification_msg message
CLKMGR_MSG_UTEST_OBJS:=$(foreach n,$(CLKMGR_MSG_UTEST_SRCS),\
  $(CLKMGR_UTEST_DIR)/$n.o)
CLKMGR_MSG_SRCS:=$(wildcard $(CLKMGR_DIR)/*/*_msg.cpp)
CLKMGR_MSG_OBJS:=$(CLKMGR_MSG_SRCS:.cpp=.o) $(addsuffix .o,\
  $(addprefix $(CLKMGR_COMMON_DIR)/,message msgq_tport print termin sighandler)\
  $(addprefix $(CLKMGR_CLIENT_DIR)/,subscription))

CLKMGR_PROXY_UTEST:=$(CLKMGR_UTEST_DIR)/utest_proxy
CLKMGR_PROXY_UTEST_SRCS:=config_parser client
CLKMGR_PROXY_UTEST_OBJS:=$(foreach n,\
  $(CLKMGR_PROXY_UTEST_SRCS),$(CLKMGR_UTEST_DIR)/$n.o)
CLKMGR_PROXY_OBJS:=$(addsuffix .o,\
  $(addprefix $(CLKMGR_PROXY_DIR)/,subscribe_msg notification_msg connect_msg\
    config_parser client)\
  $(addprefix $(CLKMGR_COMMON_DIR)/,subscribe_msg notification_msg connect_msg\
    message sighandler msgq_tport print termin))

CLKMGR_API_UTEST:=$(CLKMGR_UTEST_DIR)/utest_api
CLKMGR_API_UTEST_SRCS:=clockmanager
CLKMGR_API_UTEST_OBJS:=$(foreach n,$(CLKMGR_API_UTEST_SRCS),\
  $(CLKMGR_UTEST_DIR)/$n.o)
CLKMGR_API_OBJS+=$(addsuffix .o,\
  $(CLKMGR_CLIENT_DIR)/clockmanager\
  $(addprefix $(CLKMGR_COMMON_DIR)/,print termin)\
  $(addprefix $(CLKMGR_CLIENT_DIR)/,timebase_configs subscription clock_event))

$(CLKMGR_UTEST_DIR)/%.o: $(CLKMGR_UTEST_DIR)/%.cpp | $(CLKMGR_HEADERS_GEN)
	$(Q_CC)$(CXX) $(CXXFLAGS_UTEST) $(CLKMGR_CXXFLAGS) $(GTEST_INC_FLAGS)\
	  -include $(HAVE_GTEST_HEADER) -c -o $@ $<

$(CLKMGR_CLIENT_UTEST): $(OBJ_DIR)/utest_m.o $(CLKMGR_CLIENT_UTEST_OBJS) \
	$(CLKMGR_LIB_A) | $(CLKMGR_LIB_LA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_clkmgr_client: $(CLKMGR_CLIENT_UTEST)
	$(call Q_UTEST,ClkMgr client C++)$(UVGD)$(CLKMGR_CLIENT_UTEST)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)

$(CLKMGR_COMMON_UTEST): $(OBJ_DIR)/utest_m.o $(CLKMGR_COMMON_UTEST_OBJS) \
	$(CLKMGR_LIB_A) | $(CLKMGR_LIB_LA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_clkmgr_common: $(CLKMGR_COMMON_UTEST)
	$(call Q_UTEST,ClkMgr common C++)$(UVGD)$(CLKMGR_COMMON_UTEST)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)

$(CLKMGR_MSG_UTEST): $(OBJ_DIR)/utest_m.o $(CLKMGR_MSG_UTEST_OBJS)\
	$(CLKMGR_MSG_OBJS)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_clkmgr_message: $(CLKMGR_MSG_UTEST)
	$(call Q_UTEST,ClkMgr message C++)$(UVGD)$(CLKMGR_MSG_UTEST)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)

$(CLKMGR_PROXY_UTEST): $(OBJ_DIR)/utest_m.o\
	$(CLKMGR_PROXY_UTEST_OBJS) $(CLKMGR_PROXY_OBJS)\
	$(LIB_NAME_A)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_clkmgr_proxy: $(CLKMGR_PROXY_UTEST)
	$(call Q_UTEST,ClkMgr proxy C++)$(UVGD)$(CLKMGR_PROXY_UTEST)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)

$(CLKMGR_API_UTEST): $(OBJ_DIR)/utest_m.o $(CLKMGR_API_UTEST_OBJS)\
	$(CLKMGR_API_OBJS)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_clkmgr_api: $(CLKMGR_API_UTEST)
	$(call Q_UTEST,ClkMgr API C++)$(UVGD)$(CLKMGR_API_UTEST)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)

endif #GTEST_LIB_FLAGS

CLKMGR_UTESTS:=$(addprefix utest_clkmgr_,client common message proxy api)
.PHONY: $(CLKMGR_UTESTS)
utest_clkmgr: $(CLKMGR_GEN_SRC) $(CLKMGR_UTESTS)
