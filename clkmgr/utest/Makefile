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
CLKMGR_CLIENT_SRC:=timebase_configs subscription client_state
CLKMGR_CLIENT_OBJS:=$(foreach n,$(CLKMGR_CLIENT_SRC),$(CLKMGR_UTEST_DIR)/$n.o)

$(CLKMGR_UTEST_DIR)/%.o: $(CLKMGR_UTEST_DIR)/%.cpp | $(COMP_DEPS)
	$(Q_CC)$(CXX) $(CXXFLAGS_UTEST) $(CLKMGR_CXXFLAGS) $(GTEST_INC_FLAGS)\
	  -include $(HAVE_GTEST_HEADER) -c -o $@ $<
$(CLKMGR_CLIENT_UTEST): $(OBJ_DIR)/utest_m.o $(CLKMGR_CLIENT_OBJS) \
	$(CLKMGR_LIB_A) | $(CLKMGR_LIB_LA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_clkmgr_client: $(HEADERS_GEN_COMP) $(CLKMGR_CLIENT_UTEST)
	$(call Q_UTEST,ClkMgr C++)$(UVGD)$(CLKMGR_CLIENT_UTEST) $(GTEST_NO_COL) $(GTEST_FILTERS)
endif #GTEST_LIB_FLAGS

CLKMGR_UTESTS:=$(addprefix utest_clkmgr_,client)
.PHONY: $(CLKMGR_UTESTS)
utest_clkmgr: $(CLKMGR_GEN_SRC) $(CLKMGR_UTESTS)
