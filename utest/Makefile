# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for unit tests
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

ifneq ($(filter utest $(UTEST_TGT),$(MAKECMDGOALS)),)
ifneq ($(NONPHONY_TGT),)
$(eval $(call phony,$(NONPHONY_TGT)))
GTEST_FILTERS:=--gtest_filter=*$(NONPHONY_TGT)*
RUBY_FILTERS:=-v -n '/$(NONPHONY_TGT)/i'
PY_FILTERS:=-v -k $(NONPHONY_TGT)
PHP_FILTERS:=--testdox --filter $(NONPHONY_TGT)
LUA_FILTERS:=-v -p $(NONPHONY_TGT)
TCL_FILTERS:=-verbose "pass body erro" -match "*$(NONPHONY_TGT)*"
CRTEST_FILTERS:=--filter $(NONPHONY_TGT)
endif # NONPHONY_TGT
endif # filter utest,$(MAKECMDGOALS)

utest_lang: $(UTEST_TGT_LNG)
utest: utest_cpp utest_lang utest_c utest_clkmgr

TEST_LIBSYS:=$(OBJ_DIR)/libsys.so
$(TEST_LIBSYS): utest/libsys.cpp | $(OBJ_DIR)
	$(Q_CC)$(CXX) -shared -fPIC -include utest/libsys.h -o $@ $^ -ldl

ifdef GTEST_LIB_FLAGS
UTEST:=$(OBJ_DIR)/utest
UTEST_SYS:=$(OBJ_DIR)/utest_sys
UTEST_AUTH:=$(OBJ_DIR)/utest_auth
UTEST_JSON_LOAD:=$(OBJ_DIR)/utest_json_load
UTEST_SRCS:=bin buf cfg err mngIds msg2json msgCall msg opt proc sig types ver\
  jsonParser json2msg
TEST_OBJS:=$(foreach n,$(UTEST_SRCS),utest/$n.o)
UTEST_SYS_SRCS:=sock ptp init
TEST_SYS_OBJS:=$(foreach n,$(UTEST_SYS_SRCS),utest/$n.o)
# Main for gtest
$(OBJ_DIR)/utest_m.o: | $(OBJ_DIR)
	$(Q)printf 'int main(int argc,char**argv)%s'\
	  '{::testing::InitGoogleTest(&argc,argv);return RUN_ALL_TESTS();}' |\
	  $(CXX) -include $(HAVE_GTEST_HEADER) $(GTEST_INC_FLAGS)\
	  -c -x c++ - -o $@
$(OBJ_DIR)/utest_sys.o: | $(OBJ_DIR)
	$(Q)printf 'int main(int argc,char**argv)%s%s'\
	  '{initLibSys();::testing::InitGoogleTest(&argc,argv);'\
	  'return RUN_ALL_TESTS();}' |\
	  $(CXX) -include $(HAVE_GTEST_HEADER) -include utest/libsys.h\
	  $(GTEST_INC_FLAGS) -c -x c++ - -o $@
CXXFLAGS_UTEST=$(filter-out -std=%,$(CXXFLAGS)) -std=c++14
$(TEST_SYS_OBJS): CXXFLAGS_UTEST+=-include utest/libsys.h
utest/%.o: utest/%.cpp | $(COMP_DEPS)
	$(Q_CC)$(CXX) $(CXXFLAGS_UTEST) $(GTEST_INC_FLAGS)\
	  -include $(HAVE_GTEST_HEADER) -c -o $@ $<
$(UTEST): $(OBJ_DIR)/utest_m.o $(TEST_OBJS) $(LIB_NAME_A)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_no_sys: $(HEADERS_GEN_COMP) $(UTEST)
	$(call Q_UTEST,C++)$(UVGD)$(UTEST) $(GTEST_NO_COL) $(GTEST_FILTERS)
$(UTEST_SYS): $(OBJ_DIR)/utest_sys.o $(TEST_SYS_OBJS) $(LIB_NAME_A) $(TEST_LIBSYS)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
ifndef SKIP_LIBSYS
utest_sys: $(HEADERS_GEN_COMP) $(UTEST_SYS)
	$(call Q_UTEST,C++-SYS)LD_PRELOAD=./$(TEST_LIBSYS) $(UTEST_SYS)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)
endif
ifdef HMAC_ALIB
$(UTEST_AUTH): $(OBJ_DIR)/utest_m.o utest/auth.o $(LIB_NAME_A) $(HMAC_ALIB)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(HMAC_LIBA_FLAGS)\
	  $(HMAC_ALIB_FLAGS) $(GTEST_LIB_FLAGS) -o $@
utest_auth: $(HEADERS_GEN_COMP) $(UTEST_AUTH)
	$(call Q_UTEST,C++AUTH)$(UVGD)$(UTEST_AUTH) $(GTEST_NO_COL) $(GTEST_FILTERS)
endif
$(UTEST_JSON_LOAD): $(OBJ_DIR)/utest_m.o utest/jsonLoad.o $(LIB_NAME_SO)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@ -ldl
utest_json_load: $(HEADERS_GEN_COMP) $(UTEST_JSON_LOAD)
	$(call Q_UTEST,JSON_LOAD)LD_PRELOAD=./$(LIB_NAME_SO)\
	  $(UVGD)$(UTEST_JSON_LOAD) $(GTEST_NO_COL) $(GTEST_FILTERS)
TEST_LIBSYS_PMC:=$(OBJ_DIR)/libsys_pmc.so
UTEST_PMC:=$(OBJ_DIR)/utest_pmc
$(TEST_LIBSYS_PMC): utest/libsys_pmc.cpp | $(OBJ_DIR)
	$(Q_CC)$(CXX) -shared -fPIC -include utest/libsys.h -o $@ $^ -ldl
TEST_PMC_OBJS:=$(foreach n,pmc_dump pmc_build,utest/$n.o)
$(TEST_PMC_OBJS): CXXFLAGS_UTEST+=-include utest/libsys.h -Iptp-tools
$(UTEST_PMC): $(OBJ_DIR)/utest_sys.o $(LIB_NAME_A) $(TEST_PMC_OBJS)\
  $(OBJ_DIR)/pmc_dump.o $(TEST_LIBSYS_PMC)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(LIB_NAME_A)\
	  $(GTEST_LIB_FLAGS) -o $@
ifndef SKIP_LIBSYS
utest_pmc: $(HEADERS_GEN_COMP) $(UTEST_PMC)
	$(call Q_UTEST,PMC)LD_PRELOAD=./$(TEST_LIBSYS_PMC)$(UVGD)$(UTEST_PMC)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)
endif
utest_cpp: $(UTEST_CPP_TGT)
endif # GTEST_LIB_FLAGS
PTP4L_SIM:=$(OBJ_DIR)/ptp4l_sim.so
$(PTP4L_SIM): utest/ptp4l_sim.cpp | $(OBJ_DIR)
	$(Q_CC)$(CXX) -shared -fPIC -o $@ $^ -ldl
ptp4l_sim: $(PTP4L_SIM)
