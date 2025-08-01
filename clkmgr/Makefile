# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# Makefile for Clock Manager
#
# @author Christopher Hall <christopher.s.hall@@intel.com>
# @copyright © 2024 Intel Corporation.
#
###############################################################################

CLKMGR_COMMON_DIR:=$(CLKMGR_DIR)/common
CLKMGR_CLIENT_DIR:=$(CLKMGR_DIR)/client
CLKMGR_PROXY_DIR:=$(CLKMGR_DIR)/proxy
CLKMGR_UTEST_DIR:=$(CLKMGR_DIR)/utest
CLKMGR_PUB_DIR:=$(CLKMGR_DIR)/pub

CLKMGR_INIT_NAME:=$(CLKMGR_PROXY_DIR)/clkmgr-proxy
CLKMGR_LIB:=lib$(CLKMGR_NAME)
CLKMGR_LIB_SO:=$(LIB_D)/$(CLKMGR_LIB).so
CLKMGR_LIB_A:=$(LIB_D)/$(CLKMGR_LIB).a
CLKMGR_LIB_LA:=$(CLKMGR_LIB).la
CLKMGR_PROXY:=$(CLKMGR_PROXY_DIR)/clkmgr_proxy
CLKMGR_GEN_CPP=$(Q_GEN)$(M4) -I $(CLKMGR_COMMON_DIR) -D lang=cpp $< > $@
CLKMGR_GEN_C=$(Q_GEN)$(M4) -I $(CLKMGR_COMMON_DIR) -D lang=c $< > $@

SPDXCY_INTEL:=$(SPDXCY_BASE) Intel Corporation.
SPDXBSD3:=BSD-3-Clause

CLKMGR_LDLIBS:=$(foreach lib,pthread rt rtpi,-l$(lib))
CLKMGR_CXXFLAGS:=-I$(CLKMGR_DIR)
ifdef HAVE_LIBCHRONY_HEADER
CLKMGR_CXXFLAGS+=-DHAVE_LIBCHRONY $(CHRONY_INC_FLAGS)
endif

CLKMGR_COMMON_SRCS:=$(wildcard $(CLKMGR_COMMON_DIR)/*.cpp)
CLKMGR_COMMON_LOBJS:=$(CLKMGR_COMMON_SRCS:.cpp=.lo)
CLKMGR_COMMON_OBJS:=$(CLKMGR_COMMON_SRCS:.cpp=.o)
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

$(CLKMGR_COMMON_DIR)/%.lo: $(CLKMGR_COMMON_DIR)/%.cpp | $(CLKMGR_HEADERS_GEN)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(CLKMGR_CXXFLAGS) $< -o $@
ifeq ($(LIBTOOL_USE_STATIC),yes)
$(CLKMGR_COMMON_DIR)/%.o: $(CLKMGR_COMMON_DIR)/%.lo | $(CLKMGR_HEADERS_GEN)
	@:
else
$(CLKMGR_COMMON_DIR)/%.o: $(CLKMGR_COMMON_DIR)/%.cpp | $(CLKMGR_HEADERS_GEN)
	$(Q_CC)$(CXX) $(CXXFLAGS) $(CLKMGR_CXXFLAGS) -c -o $@ $<
endif
$(CLKMGR_CLIENT_DIR)/%.lo: $(CLKMGR_CLIENT_DIR)/%.cpp | $(CLKMGR_HEADERS_GEN)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(CLKMGR_CXXFLAGS) $< -o $@
$(CLKMGR_PROXY_DIR)/%.o: $(CLKMGR_PROXY_DIR)/%.cpp | $(CLKMGR_HEADERS_GEN)
	$(Q_CC)$(CXX) $(CXXFLAGS) $(CLKMGR_CXXFLAGS) -c -o $@ $<

$(CLKMGR_LIB_LA)_LDLIBS:=$(CLKMGR_LDLIBS)
$(CLKMGR_LIB_LA): $(CLKMGR_COMMON_LOBJS) $(CLKMGR_CLIENT_LOBJS)
$(CLKMGR_LIB_SO): $(CLKMGR_LIB_LA)
	@:
$(CLKMGR_LIB_A): $(CLKMGR_LIB_LA)
	@:

$(CLKMGR_PROXY): $(CLKMGR_PROXY_OBJS) $(CLKMGR_COMMON_OBJS) $(LIB_NAME_SO)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(CLKMGR_LDLIBS)\
	  -o $@ $(CHRONY_LIB_FLAGS)

ALL+=$(CLKMGR_LIB_LA) $(CLKMGR_PROXY)

include $(CLKMGR_DIR)/utest/Makefile

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

rundir:=$(realpath $(runstatedir))
INIT_DIR:=$(DESTDIR)$(sysconfdir)/init.d/clkmgr-proxy
CLKMGR_INIT_FILES:=$(subst .in,,$(wildcard $(CLKMGR_INIT_NAME).*.in))
$(CLKMGR_INIT_NAME).%: $(CLKMGR_INIT_NAME).%.in
	$(Q_GEN)$(SED) $(foreach n,sysconfdir sbindir rundir,-e 's!@$n@!$($n)!') $< > $@

install_clkmgr: $(CLKMGR_INIT_FILES)
	$(Q)$(INSTALL_FOLDER) $(INCDIR) $(PKGCFGDIR)
	$(INSTALL_PROGRAM) -D $(CLKMGR_PROXY) -t $(DESTDIR)$(sbindir)
	$(INSTALL_DATA) -D $(CLKMGR_PROXY_DIR)/proxy_cfg.json -t\
	  $(DESTDIR)$(sysconfdir)/clkmgr
	$(INSTALL_DATA) -D man/clkmgr_proxy.8 -t $(MANDIR)
ifeq ($(USE_SYSTEMD),yes)
	$(INSTALL_DATA) -D $(CLKMGR_INIT_NAME).service -t $(SYSTEMDDIR)
	$(INSTALL_DATA) -D $(CLKMGR_INIT_NAME).socket -t $(SYSTEMDDIR)
endif
ifeq ($(USE_LSB_INIT),yes)
	$(INSTALL_PROGRAM) -D $(CLKMGR_INIT_NAME).lsb $(INIT_DIR)
endif
ifeq ($(USE_OPENRC),yes)
	$(INSTALL_PROGRAM) -D $(CLKMGR_INIT_NAME).openrc $(INIT_DIR)
endif
ifneq ($(USE_RUNIT),)
	$(INSTALL_PROGRAM) -D $(CLKMGR_INIT_NAME).runit\
	  $(DESTDIR)$(USE_RUNIT)/clkmgr-proxy/run
endif
ifdef PKG_CONFIG_DIR
	echo "$(clkmgr_pkgconfig)" > $(PKGCFGDIR)/$(CLKMGR_NAME).pc
	for pf in $(CLKMGR_NAME)$(PACKAGE_VERSION) $(CLKMGR_LIB)\
	  $(CLKMGR_LIB)$(PACKAGE_VERSION)
	do $(LN) $(CLKMGR_NAME).pc $(PKGCFGDIR)/$$pf.pc;done
endif
	cp -a $(CLKMGR_PUB_DIR)/* $(INCDIR)
	$(SED) -i 's!$(c_inc)\s*\"pub/\([^"]\+\)\"!$(c_inc) <\1>!'\
	  $(INCDIR)/clockmanager.h $(INCDIR)/$(CLKMGR_NAME)/*.h
	printf "$(hash) $(SPDXLI) $(SPDXGFDL)\n$(hash) $(SPDXCY_INTEL)\n\n%s\n"\
	  'LDLIBS+=-l$(CLKMGR_NAME)' > $(DEVDOCDIR)/clkmgr_default.mk
	printf "$(hash) $(SPDXLI) $(SPDXGFDL)\n$(hash) $(SPDXCY_INTEL)\n\n%s\n"\
	  'LDLIBS+= -Wl,-static -l$(CLKMGR_NAME) -Wl,-Bdynamic' >\
	  $(DEVDOCDIR)/clkmgr_static.mk
	$(INSTALL_FOLDER) $(DOCDIR)/clkmgr
	cp -a clkmgr/*.md clkmgr/image $(DOCDIR)/clkmgr
ifdef DOXYGEN_MINVER
	$(INSTALL_FOLDER) $(DOCDIR)/clkmgr_html
	$(MKDIR_P) "clkmgr/doc/html"
	$(RM) clkmgr/doc/html/*.md5 clkmgr/doc/html/*.map
	cp -a clkmgr/doc/html/* $(DOCDIR)/clkmgr_html
	tools/add_doxy_spdx.sh clkmgr/doc/html "$(DOCDIR)/clkmgr_html" intel
endif # DOXYGEN_MINVER
ifdef CMARK
	head -2 clkmgr/TEST_clkmgr.md > $(DOCDIR)/clkmgr/TEST_clkmgr.html
	echo "<!doctype html><title>Build and Test Project Document</title>"\
	  >> $(DOCDIR)/clkmgr/TEST_clkmgr.html
	$(CMARK) -t html clkmgr/TEST_clkmgr.md |\
	  $(SED) 's!href="./proxy/!href="$(GITHUB_REP)/blob/master/clkmgr/proxy/!'\
	  >> $(DOCDIR)/clkmgr/TEST_clkmgr.html
endif # CMARK
ifdef MARKDOWN
	$(MARKDOWN) clkmgr/HLD_clkmgr.md |\
	  $(SED) -e "4 i <!doctype html><title>Clock Manager Design Document</title>"\
	  -e 's!"\./doc/!"./!;s!\.md"!.html"!' > $(DOCDIR)/clkmgr/HLD_clkmgr.html
	$(MARKDOWN) clkmgr/README.md |\
	  $(SED) -e "4 i <!doctype html><title>Clock Manager Introduction</title>"\
	  -e 's!"\./doc/!"./!;s!\.md"!.html"!' > $(DOCDIR)/clkmgr/index.html
#ifdef DOXYGEN_MINVER
# TODO Add link to the clkmgr doxygen documents
#       $(SED) -i 's$(REDIR2)' $(DOCDIR)/clkmgr/index.html
#endif
endif # MARKDOWN
ifdef PANDOC
	$(INSTALL_FOLDER) $(DESTDIR)$(mandir)/man5
	$(PANDOC) -s -t man $(CLKMGR_DIR)/clkmgr_proxy_cfg.md\
	  -o $(DESTDIR)$(mandir)/man5/clkmgr_proxy_cfg.5
	$(SED) -i '1i\\\" $(SPDXLI) $(SPDXGFDL)\n\\" $(SPDXCY_INTEL)'\
	  $(DESTDIR)$(mandir)/man5/clkmgr_proxy_cfg.5
	head -2 $(CLKMGR_DIR)/clkmgr_proxy_cfg.md >\
	  $(DOCDIR)/clkmgr/clkmgr_proxy_cfg.5.html
	$(PANDOC) -s -t html $(CLKMGR_DIR)/clkmgr_proxy_cfg.md\
	  >> $(DOCDIR)/clkmgr/clkmgr_proxy_cfg.5.html
endif # PANDOC
