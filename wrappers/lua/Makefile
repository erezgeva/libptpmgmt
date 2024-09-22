# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for Lua libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

lua_SFLAGS+=-lua
LUA_D:=wrappers/lua
define lua
LUA_FLIB_$1:=liblua$1-$(SWIG_LIB_NAME)
LUA_LIB_$1:=$(LUA_D)/$1/$(SWIG_LIB_NAME)
$$(LUA_LIB_$1)_LDLIBS:=-Wl,-soname,$$(LUA_FLIB_$1)$(SONAME)\
  $$(LUA_$$(subst .,_,$1)_LIB_FLAGS)
$(LUA_D)/$1/$(SWIG_NAME).o: $(LUA_D)/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(MKDIR_P) $(LUA_D)/$1
	$$(call LLC,$$(CXXFLAGS_LUA) $$(LUA_$$(subst .,_,$1)_INC_FLAGS))
$$(LUA_LIB_$1): $(LUA_D)/$1/$(SWIG_NAME).o $(LIB_NAME_SO)
	$$(SWIG_LD)
SWIG_ALL+=$$(LUA_LIB_$1)
ifndef SKIP_CLKMGR
$(LUA_D)/$1/$(CLKMGR_NAME).o: $(LUA_D)/$(CLKMGR_NAME).cpp
	$Q$(MKDIR_P) $(LUA_D)/$1
	$$(call LLC,$$(CXXFLAGS_LUA) $$(LUA_$$(subst .,_,$1)_INC_FLAGS) $(CLKMGR_CXXFLAGS))
$(LUA_D)/$1/$(CLKMGR_NAME).so: $(LUA_D)/$1/$(CLKMGR_NAME).o $(CLKMGR_LIB_SO)
	$$(SWIG_LD)
SWIG_ALL+=$(LUA_D)/$1/$(CLKMGR_NAME).so
endif # SKIP_CLKMGR
utest_lua_$1: $(LIB_NAME_SO) $$(LUA_LIB_$1)
	$$(call Q_UTEST,Lua$1)LD_PRELOAD=$(ASAN_PRE)./$$< LUA_CPATH="$(LUA_D)/$1/?.so;;"\
	  $(UVGD)lua$1 $(LUA_D)/utest.lua $(LUA_FILTERS)
.PHONY: utest_lua_$1

endef
# Build multiple Lua versions
$(eval $(foreach n,$(LUA_VERS),$(call lua,$n)))

# Build single Lua version
ifdef LUA_VER
LUA_LIB:=$(LUA_D)/$(SWIG_LIB_NAME)
CXXFLAGS_LUA+=$(LUA_INC_FLAGS)
$(LUA_LIB)_LDLIBS:=-Wl,-soname,$(SWIG_LIB_NAME) $(LUA_LIB_FLAGS)

$(LUA_D)/$(SWIG_NAME).o: $(LUA_D)/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_LUA))
$(LUA_LIB): $(LUA_D)/$(SWIG_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(LUA_LIB)
ifndef SKIP_CLKMGR
$(LUA_D)/$(CLKMGR_NAME).o: $(LUA_D)/$(CLKMGR_NAME).cpp
	$Q$(MKDIR_P) $(LUA_D)
	$(call LLC,$(CXXFLAGS_LUA) -I$(LUA_D) $(CLKMGR_CXXFLAGS))
$(LUA_D)/$(CLKMGR_NAME).so: $(LUA_D)/$(CLKMGR_NAME).o $(CLKMGR_LIB_SO)
	$(SWIG_LD)
SWIG_ALL+=$(LUA_D)/$(CLKMGR_NAME).so
endif # SKIP_CLKMGR
ifdef LUA_UNIT
utest_lua_a: $(LIB_NAME_SO) $(LUA_LIB)
	$(call Q_UTEST,Lua)LD_PRELOAD=$(ASAN_PRE)./$< LUA_CPATH="$(LUA_D)/?.so;;"\
	  $(LUA_BIN) $(UVGD)$(LUA_D)/utest.lua $(LUA_FILTERS)
endif # LUA_UNIT
endif # LUA_VER
utest_lua: utest_lua_a $(addprefix utest_lua_,$(LUA_UNIT_VERS))

install_lua:
	$Q$(foreach n,$(LUA_VERS),\
	  $(INSTALL_LIB) -D $(LUA_LIB_$n)\
	  $(DLIBDIR)/$(LUA_FLIB_$n).$(PACKAGE_VERSION);\
	  $(LN) $(LUA_FLIB_$n).$(PACKAGE_VERSION)\
	  $(DLIBDIR)/$(LUA_FLIB_$n)$(SONAME);\
	  $(INSTALL_FOLDER) $(DLIBDIR)/lua/$n;\
	  $(LN) ../../$(LUA_FLIB_$n).$(PACKAGE_VERSION)\
	  $(DLIBDIR)/lua/$n/$(SWIG_LIB_NAME);)
ifdef LUA_VER
	$(INSTALL_FOLDER) $(DLIBDIR)/lua/$(LUA_VER)
	$(INSTALL_LIB) -D $(LUA_LIB) $(DLIBDIR)/lua/$(LUA_VER)/$(SWIG_LIB_NAME)
endif # LUA_VER
