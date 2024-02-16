# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for C unit tests
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
###############################################################################

UCTEST:=$(OBJ_DIR)/uctest
UCTEST_SYS:=$(OBJ_DIR)/uctest_sys
UCTEST_SRCS:=cfg ver err setErr opt msg mngIds types proc sig msg2json msgCall
UCTEST_SYS_SRCS:=sock ptp init
UCTEST_OBJS:=$(foreach n,$(UCTEST_SRCS),uctest/$n.o)
UCTEST_SYS_OBJS:=$(foreach n,$(UCTEST_SYS_SRCS),uctest/$n.o)
CFLAGS_UCTEST=$(filter-out -std=% -Wdeprecated-copy,$(CXXFLAGS)) -std=c11
CFLAGS_UCTEST+=$(addprefix -include ,$(HAVE_CRITERION_HEADER))
$(UCTEST_SYS_OBJS): CFLAGS_UCTEST+=-include utest/libsys.h
uctest/%.o: uctest/%.c | $(COMP_DEPS)
	$(Q_CC)$(CC) $(CFLAGS_UCTEST) $(CRITERION_INC_FLAGS) -c -o $@ $<
uctest/%.o: uctest/%.cpp | $(COMP_DEPS)
	$(Q_CC)$(CXX) $(CXXFLAGS) $(CRITERION_INC_FLAGS) -c -o $@ $<
$(UCTEST): $(UCTEST_OBJS) $(LIB_NAME_A)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)\
	  $(CRITERION_LIB_FLAGS) -o $@
$(UCTEST_SYS): $(UCTEST_SYS_OBJS) $(LIB_NAME_A) $(TEST_LIBSYS)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)\
	  $(CRITERION_LIB_FLAGS) -o $@
uctest_no_sys: $(HEADERS_GEN_COMP) $(UCTEST)
	$(call Q_UTEST,C)$(UVGD)$(UCTEST) $(CRTEST_FILTERS)
ifndef SKIP_LIBSYS
uctest_sys: $(HEADERS_GEN_COMP) $(UCTEST_SYS)
	$(call Q_UTEST,C-SYS)LD_PRELOAD=./$(TEST_LIBSYS) $(UVGD)$(UCTEST_SYS)\
	  -j1 $(CRTEST_FILTERS)
endif
utest_c: $(UTEST_C_TGT)
