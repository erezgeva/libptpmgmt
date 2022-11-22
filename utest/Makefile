# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2022 Erez Geva
#
# Makefile for unit test
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2022 Erez Geva
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
endif # NONPHONY_TGT
endif # filter utest,$(MAKECMDGOALS)

utest: $(UTEST_TGT)

ifneq ($(GTEST_LIB_FLAGS),)
UTEST:=$(OBJ_DIR)/utest
UTEST_SYS:=$(OBJ_DIR)/utest_sys
UTEST_JSON_LOAD:=$(OBJ_DIR)/utest_json_load
UTEST_SYS_SRCS:=sock ptp init
TEST_OBJS:=$(patsubst %.cpp,%.o,$(filter-out\
  $(foreach n,json2msg jsonLoad libsys $(UTEST_SYS_SRCS),utest/$n.cpp)\
  ,$(wildcard utest/*.cpp)))
TEST_SYS_OBJS:=$(foreach n,$(UTEST_SYS_SRCS),utest/$n.o)
TEST_LIBSYS:=objs/libsys.so
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
$(TEST_SYS_OBJS): override CXXFLAGS+=-include utest/libsys.h
utest/%.o: utest/%.cpp | $(COMP_DEPS)
	$(Q_CC)$(CXX) $(CXXFLAGS) $(GTEST_INC_FLAGS)\
	  -include $(HAVE_GTEST_HEADER) -c -o $@ $<
$(TEST_LIBSYS): utest/libsys.cpp
	$(Q_CC)$(CXX) -shared -fPIC\
	  -include utest/libsys.h -o $@ $^ -ldl
$(UTEST): $(OBJ_DIR)/utest_m.o $(TEST_OBJS) $(LIB_NAME_A)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_cpp: $(HEADERS_GEN_COMP) $(UTEST)
	$(call Q_UTEST,C++)$(UTEST) $(GTEST_NO_COL) $(GTEST_FILTERS)
$(UTEST_SYS): $(OBJ_DIR)/utest_sys.o $(TEST_SYS_OBJS) $(LIB_NAME_A) $(TEST_LIBSYS)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_sys: $(HEADERS_GEN_COMP) $(UTEST_SYS)
	$(call Q_UTEST,C++-SYS)LD_PRELOAD=./$(TEST_LIBSYS) $(UTEST_SYS)\
	  $(GTEST_NO_COL) $(GTEST_FILTERS)
$(UTEST_JSON_LOAD): $(OBJ_DIR)/utest_m.o utest/jsonLoad.o $(LIB_NAME_SO) |\
  $(JSONC_FLIB) $(FJSON_FLIB)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@ -ldl
utest_json_load: $(HEADERS_GEN_COMP) $(UTEST_JSON_LOAD)
	$(call Q_UTEST,JSON_LOAD)LD_PRELOAD=./$(LIB_NAME_SO) LD_LIBRARY_PATH=.\
	  $(UTEST_JSON_LOAD) $(GTEST_NO_COL) $(GTEST_FILTERS)
endif # GTEST_LIB_FLAGS
