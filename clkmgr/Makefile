# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# Makefile for clkmgr
#
# @author Christopher Hall <christopher.s.hall@intel.com>
# @copyright © 2024 Intel Corporation.
#
###############################################################################

CLKMGR_COMMON_DIR:=$(CLKMGR_DIR)/common
CLKMGR_CLIENT_DIR:=$(CLKMGR_DIR)/client
CLKMGR_PROXY_DIR:=$(CLKMGR_DIR)/proxy
CLKMGR_UTEST_DIR:=$(CLKMGR_DIR)/utest
CLKMGR_PUB_DIR:=$(CLKMGR_DIR)/pub

CLKMGR_LIB:=lib$(CLKMGR_NAME)
CLKMGR_LIB_SO:=$(LIB_D)/$(CLKMGR_LIB).so
CLKMGR_LIB_A:=$(LIB_D)/$(CLKMGR_LIB).a
CLKMGR_LIB_LA:=$(CLKMGR_LIB).la
CLKMGR_PROXY:=$(CLKMGR_PROXY_DIR)/clkmgr_proxy
CLKMGR_GEN_CPP=$(Q_GEN)$(M4) -I $(CLKMGR_COMMON_DIR) -D lang=cpp $< > $@
CLKMGR_GEN_C=$(Q_GEN)$(M4) -I $(CLKMGR_COMMON_DIR) -D lang=c $< > $@

CLKMGR_LDLIBS:=$(foreach lib,pthread rt rtpi,-l$(lib))
CLKMGR_CXXFLAGS:=-I$(CLKMGR_DIR)
ifdef HAVE_LIBCHRONY_HEADER
CLKMGR_CXXFLAGS+=-DHAVE_LIBCHRONY $(CHRONY_INC_FLAGS)
endif

CLKMGR_COMMON_SRCS:=$(wildcard $(CLKMGR_COMMON_DIR)/*.cpp)
CLKMGR_COMMON_LOBJS:=$(CLKMGR_COMMON_SRCS:.cpp=.lo)
CLKMGR_COMMON_OBJS:=$(addprefix $(CLKMGR_COMMON_DIR)/$(LIB_D)/,\
  $(notdir $(CLKMGR_COMMON_SRCS:.cpp=.o)))
CLKMGR_CLIENT_SRCS:=$(wildcard $(CLKMGR_CLIENT_DIR)/*.cpp)
CLKMGR_CLIENT_LOBJS:=$(CLKMGR_CLIENT_SRCS:.cpp=.lo)
CLKMGR_PROXY_SRCS:=$(wildcard $(CLKMGR_PROXY_DIR)/*.cpp)
CLKMGR_PROXY_OBJS:=$(CLKMGR_PROXY_SRCS:.cpp=.o)
ifndef HAVE_LIBCHRONY_HEADER
CLKMGR_PROXY_OBJS:=$(filter-out $(CLKMGR_PROXY_DIR)/connect_chrony.o,\
  $(CLKMGR_PROXY_OBJS))
endif

$(CLKMGR_PUB_DIR)/clkmgr/types.h: $(CLKMGR_COMMON_DIR)/types.m4
	$(CLKMGR_GEN_CPP)
$(CLKMGR_PUB_DIR)/clkmgr/types_c.h: $(CLKMGR_COMMON_DIR)/types.m4
	$(CLKMGR_GEN_C)

$(CLKMGR_COMMON_LOBJS): $(CLKMGR_COMMON_DIR)/%.lo: $(CLKMGR_COMMON_DIR)/%.cpp |\
	$(CLKMGR_HEADERS_GEN)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(CLKMGR_CXXFLAGS) $< -o $@
$(CLKMGR_COMMON_DIR)/$(LIB_D)/%.o: $(CLKMGR_COMMON_DIR)/%.lo | $(CLKMGR_HEADERS_GEN)
	@:
$(CLKMGR_CLIENT_LOBJS): $(CLKMGR_CLIENT_DIR)/%.lo: $(CLKMGR_CLIENT_DIR)/%.cpp |\
	$(CLKMGR_HEADERS_GEN)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(CLKMGR_CXXFLAGS) $< -o $@
$(CLKMGR_PROXY_OBJS): $(CLKMGR_PROXY_DIR)/%.o: $(CLKMGR_PROXY_DIR)/%.cpp |\
	$(COMP_DEPS) $(CLKMGR_HEADERS_GEN)
	$(Q_CC)$(CXX) $(CXXFLAGS) $(CLKMGR_CXXFLAGS) -c -o $@ $<

$(CLKMGR_LIB_LA)_LDLIBS:=$(CLKMGR_LDLIBS)
$(CLKMGR_LIB_LA): $(CLKMGR_COMMON_LOBJS) $(CLKMGR_CLIENT_LOBJS)
$(CLKMGR_LIB_SO): $(CLKMGR_LIB_LA)
	@:
$(CLKMGR_LIB_A): $(CLKMGR_LIB_LA)
	@:

$(CLKMGR_PROXY): $(CLKMGR_COMMON_OBJS) $(CLKMGR_PROXY_OBJS) $(LIB_NAME_SO) |\
	$(CLKMGR_COMMON_LOBJS)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  -o $@ $(CHRONY_LIB_FLAGS)

ALL+=$(CLKMGR_LIB_LA) $(CLKMGR_PROXY)

ifdef GTEST_LIB_FLAGS

CLKMGR_CLIENT_UTEST:=$(CLKMGR_UTEST_DIR)/utest_client
CLKMGR_CLIENT_SRC:=subscription client_state
CLKMGR_CLIENT_OBJS:=$(foreach n,$(CLKMGR_CLIENT_SRC),$(CLKMGR_UTEST_DIR)/$n.o)

$(CLKMGR_UTEST_DIR)/%.o: $(CLKMGR_UTEST_DIR)/%.cpp | $(COMP_DEPS)
	$(Q_CC)$(CXX) $(CXXFLAGS_UTEST) $(CLKMGR_CXXFLAGS) $(GTEST_INC_FLAGS)\
	  -include $(HAVE_GTEST_HEADER) -c -o $@ $<
$(CLKMGR_CLIENT_UTEST): $(OBJ_DIR)/utest_m.o $(CLKMGR_CLIENT_OBJS) \
	$(CLKMGR_LIB_A) | $(CLKMGR_LIB_LA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)\
	  $(GTEST_LIB_FLAGS) -o $@
utest_clkmgr_client: $(HEADERS_GEN_COMP) $(CLKMGR_CLIENT_UTEST)
	$(call Q_UTEST,C++)$(UVGD)$(CLKMGR_CLIENT_UTEST) $(GTEST_NO_COL) $(GTEST_FILTERS)
endif #GTEST_LIB_FLAGS

define clkmgr_pkgconfig
$(hash) $(SPDXLI) $(SPDXBSD3)
$(hash) $(SPDXCY_INTEL)

Name: $(CLKMGR_LIB) library
Description: $(CLKMGR_LIB) Clock Manager library

URL: $(PACKAGE_URL)
Version: $(PACKAGE_VERSION)
Cflags:
Libs: -l$(CLKMGR_NAME)
endef

install_clkmgr:
	$(Q)$(INSTALL_FOLDER) $(DESTDIR)$(includedir) $(PKGCFGDIR)\
	  $(DESTDIR)$(sbindir)
	$(INSTALL_PROGRAM) -D $(CLKMGR_PROXY) -t $(DESTDIR)$(sbindir)
ifdef PKG_CONFIG_DIR
	echo "$(clkmgr_pkgconfig)" > $(PKGCFGDIR)/$(CLKMGR_NAME).pc
	for pf in $(CLKMGR_NAME)$(PACKAGE_VERSION) $(CLKMGR_LIB)\
	  $(CLKMGR_LIB)$(PACKAGE_VERSION)
	do $(LN) $(CLKMGR_NAME).pc $(PKGCFGDIR)/$$pf.pc;done
endif
	cp -a $(CLKMGR_PUB_DIR)/* $(DESTDIR)$(includedir)
	$(SED) -i 's!$(c_inc)\s*\"pub/\([^"]\+\)\"!$(c_inc) <\1>!'\
	  $(DESTDIR)$(includedir)/clockmanager.h\
	  $(DESTDIR)$(includedir)/$(CLKMGR_NAME)/*.h
