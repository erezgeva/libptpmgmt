# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2022 Erez Geva
#
# Makefile for unit test
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2022 Erez Geva
#
###############################################################################

ifneq ($(GTEST_LIB_FLAGS),)
TEST_OBJS:=$(patsubst %.cpp,%.o,$(filter-out\
  $(foreach n,json2msg,utest/$n.cpp),$(wildcard utest/*.cpp)))
ifneq ($(filter utest $(UTEST_TGT),$(MAKECMDGOALS)),)
ifneq ($(NONPHONY_TGT),)
$(eval $(call phony,$(NONPHONY_TGT)))
GTEST_FILTERS:=--gtest_filter=*$(NONPHONY_TGT)*
RUBY_FILTERS:=-v -n '/$(NONPHONY_TGT)/i'
PY_FILTERS:=-v -k $(NONPHONY_TGT)
PHP_FILTERS:=--testdox --filter $(NONPHONY_TGT)
LUA_FILTERS:=-v -p $(NONPHONY_TGT)
TCL_FILTERS:=-verbose "pass body erro" -match "*$(NONPHONY_TGT)*"
endif # NONPHONY_TGT
endif # filter utest,$(MAKECMDGOALS)

# Main for gtest
$(OBJ_DIR)/utest_m.o: | $(OBJ_DIR)
	$(Q)printf 'int main(int argc,char**argv)%s'\
	  '{::testing::InitGoogleTest(&argc,argv);return RUN_ALL_TESTS();}' |\
	  $(CXX) -include $(HAVE_GTEST_HEADER) $(GTEST_INC_FLAGS)\
	  -c -x c++ - -o $@
utest/%.o: utest/%.cpp | $(COMP_DEPS)
	$(Q_CC)$(CXX) $(CXXFLAGS) $(GTEST_INC_FLAGS)\
	  -include $(HAVE_GTEST_HEADER) -c -o $@ $<

$(UTEST): $(OBJ_DIR)/utest_m.o $(TEST_OBJS) $(LIB_NAME_A)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_cpp: $(HEADERS_GEN_COMP) $(UTEST)
	$(call Q_UTEST,C++)$(UTEST) $(GTEST_NO_COL) $(GTEST_FILTERS)
utest: $(UTEST_TGT)

endif # GTEST_LIB_FLAGS
