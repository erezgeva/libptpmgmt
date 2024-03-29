# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for libptpmgmt jsonFrom libraries
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

# List of libraries of jsonFrom
JSON_C:=

UTEST_JSONC:=$(OBJ_DIR)/utest_json_c
UTEST_FJSON:=$(OBJ_DIR)/utest_json_f
UCTEST_JSONC:=$(OBJ_DIR)/uctest_json_c
UCTEST_FJSON:=$(OBJ_DIR)/uctest_json_f
# Using json-c
JSON_C+=\"$(JSONC_FLIB)\",
ifneq ($(HAVE_JSONC_LIB),)
JSONC_CFLAGS:=-include $(HAVE_JSONC_LIB) -DJLIB_NAME=\"$(JSONC_LIBA)\"
$(OBJ_DIR)/jsonFromJc.o: $(JSON_SRC)/jsonFrom.cpp | $(COMP_DEPS)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(JSONC_CFLAGS) $< -o $@
$(OBJ_DIR)/.libs/jsonFromJc.o: $(OBJ_DIR)/jsonFromJc.o
$(JSONC_LIB)_LDLIBS:=$(JSONC_LIB_FLAGS)
$(JSONC_FLIB): $(JSONC_LIB)
	$Q$(LN) $^ $@
$(JSONC_LIB): $(OBJ_DIR)/.libs/jsonFromJc.o $(LIB_NAME).so
$(JSONC_LIBA): $(OBJ_DIR)/jsonFromJc.o
	$(LLA)
ALL+=$(JSONC_FLIB) $(JSONC_LIBA)
ifneq ($(GTEST_LIB_FLAGS),)
$(UTEST_JSONC): $(OBJ_DIR)/utest_m.o utest/json2msg.o $(LIB_NAME_A) $(JSONC_LIBA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(JSONC_LIB_FLAGS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_jsonc: $(HEADERS_GEN_COMP) $(UTEST_JSONC)
	$(call Q_UTEST,JSON-C)$(UVGD)$(UTEST_JSONC) $(GTEST_NO_COL) $(GTEST_FILTERS)
endif # GTEST_LIB_FLAGS
ifneq ($(CRITERION_LIB_FLAGS),)
$(UCTEST_JSONC): uctest/json2msg.o $(LIB_NAME_A) $(JSONC_LIBA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(JSONC_LIB_FLAGS)\
	  $(CRITERION_LIB_FLAGS) -o $@
uctest_jsonc: $(HEADERS_GEN_COMP) $(UCTEST_JSONC)
	$(call Q_UTEST,C-JSON-C)$(UVGD)$(UCTEST_JSONC) $(CRTEST_FILTERS)
endif # CRITERION_LIB_FLAGS
endif # HAVE_JSONC_LIB

# Using fastjson
JSON_C+=\"$(FJSON_FLIB)\",
ifneq ($(HAVE_FJSON_LIB),)
FJSON_CFLAGS:=-include $(HAVE_FJSON_LIB) -DJLIB_NAME=\"$(FJSON_LIBA)\"
$(OBJ_DIR)/jsonFromFj.o: $(JSON_SRC)/jsonFrom.cpp | $(COMP_DEPS)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(FJSON_CFLAGS) $< -o $@
$(OBJ_DIR)/.libs/jsonFromFj.o: $(OBJ_DIR)/jsonFromFj.o
$(FJSON_LIB)_LDLIBS:=$(FJSON_LIB_FLAGS)
$(FJSON_FLIB): $(FJSON_LIB)
	$Q$(LN) $^ $@
$(FJSON_LIB): $(OBJ_DIR)/.libs/jsonFromFj.o $(LIB_NAME).so
$(FJSON_LIBA): $(OBJ_DIR)/jsonFromFj.o
	$(LLA)
ALL+=$(FJSON_FLIB) $(FJSON_LIBA)
ifneq ($(GTEST_LIB_FLAGS),)
$(UTEST_FJSON): $(OBJ_DIR)/utest_m.o utest/json2msg.o $(LIB_NAME_A) $(FJSON_LIBA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(FJSON_LIB_FLAGS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_fjson: $(HEADERS_GEN_COMP) $(UTEST_FJSON)
	$(call Q_UTEST,JSON-F)$(UVGD)$(UTEST_FJSON) $(GTEST_NO_COL) $(GTEST_FILTERS)
endif # GTEST_LIB_FLAGS
ifneq ($(CRITERION_LIB_FLAGS),)
$(UCTEST_FJSON): uctest/json2msg.o $(LIB_NAME_A) $(FJSON_LIBA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(FJSON_LIB_FLAGS)\
	  $(CRITERION_LIB_FLAGS) -o $@
uctest_fjson: $(HEADERS_GEN_COMP) $(UCTEST_FJSON)
	$(call Q_UTEST,C-JSON-F)$(UVGD)$(UCTEST_FJSON) $(CRTEST_FILTERS)
endif # CRITERION_LIB_FLAGS
endif # HAVE_FJSON_LIB

.PHONY: utest_jsonc utest_fjson uctest_jsonc uctest_fjson
utest_json: utest_jsonc utest_fjson
uctest_json: uctest_jsonc uctest_fjson
# Add jsonFrom libraries to search
$(OBJ_DIR)/jsonDef.o: override CXXFLAGS+=-DJSON_C="$(JSON_C)"
