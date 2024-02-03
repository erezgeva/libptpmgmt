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
$(LUA_D)/$(SWIG_NAME).cpp: $(LUA_D)/msgCall.i
define lua
LUA_FLIB_$1:=liblua$1-$(SWIG_LIB_NAME)
LUA_LIB_$1:=$(LUA_D)/$1/$(SWIG_LIB_NAME)
$$(LUA_LIB_$1)_LDLIBS:=-Wl,-soname,$$(LUA_FLIB_$1)$(SONAME)\
  $$(LUA_$$(subst .,_,$1)_LINK)
$(LUA_D)/$1/$(SWIG_NAME).o: $(LUA_D)/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(MKDIR_P) $(LUA_D)/$1
	$$(call LLC,$$(CXXFLAGS_LUA) -I$$(LUA_$$(subst .,_,$1)_INC))
	$$(call D_INC,LUA_$$(subst .,_,$1)_INC)
	$$(SWIG_DEP)
$$(LUA_LIB_$1): $(LUA_D)/$1/$(SWIG_NAME).o $(LIB_NAME_SO)
	$$(SWIG_LD)
SWIG_ALL+=$$(LUA_LIB_$1)
utest_lua_$1: $(LIB_NAME_SO) $$(LUA_LIB_$1)
	$$(call Q_UTEST,Lua$1)LD_PRELOAD=$(ASAN_PRE)./$$< LUA_CPATH="$(LUA_D)/$1/?.so;;"\
	  $(UVGD)lua$1 $(LUA_D)/utest.lua $(LUA_FILTERS)
.PHONY: utest_lua_$1

endef
# Build multiple Lua versions
$(eval $(foreach n,$(LUAVERSIONS),$(call lua,$n)))

# Build single Lua version
ifneq ($(LUA_VERSION),)
LUA_LIB:=$(LUA_D)/$(SWIG_LIB_NAME)
ifneq ($(LUA_INC),)
CXXFLAGS_LUA+=-I$(LUA_INC)
endif
$(LUA_LIB)_LDLIBS:=-Wl,-soname,$(SWIG_LIB_NAME) $(LUALINK)

$(LUA_D)/$(SWIG_NAME).o: $(LUA_D)/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_LUA))
	$(call D_INC,LUA_INC)
	$(SWIG_DEP)
$(LUA_LIB): $(LUA_D)/$(SWIG_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(LUA_LIB)
ifneq ($(LUAUNIT),)
utest_lua_a: $(LIB_NAME_SO) $(LUA_LIB)
	$(call Q_UTEST,Lua)LD_PRELOAD=$(ASAN_PRE)./$< LUA_CPATH="$(LUA_D)/?.so;;"\
	  $(LUABIN) $(UVGD)$(LUA_D)/utest.lua $(LUA_FILTERS)
endif # LUAUNIT
endif # LUA_VERSION
utest_lua: utest_lua_a $(addprefix utest_lua_,$(LUAVERSIONS_UNIT))

install_lua:
	$Q$(foreach n,$(LUAVERSIONS),\
	  $(INSTALL_LIB) -D $(LUA_LIB_$n)\
	  $(DLIBDIR)/$(LUA_FLIB_$n).$(PACKAGE_VERSION);\
	  $(LN) $(LUA_FLIB_$n).$(PACKAGE_VERSION)\
	  $(DLIBDIR)/$(LUA_FLIB_$n)$(SONAME);\
	  $(INSTALL_FOLDER) $(DLIBDIR)/lua/$n;\
	  $(LN) ../../$(LUA_FLIB_$n).$(PACKAGE_VERSION)\
	  $(DLIBDIR)/lua/$n/$(SWIG_LIB_NAME);)
ifneq ($(LUA_VERSION),)
	$(INSTALL_FOLDER) $(DLIBDIR)/lua/$(LUA_VERSION)
	$(INSTALL_LIB) -D $(LUA_LIB) $(DLIBDIR)/lua/$(LUA_VERSION)/$(SWIG_LIB_NAME)
endif # LUA_VERSION
