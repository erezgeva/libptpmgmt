# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for go libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
###############################################################################

GO_D:=wrappers/go
# Go environment
CGO_CXXFLAGS!=$(GO_BIN) env CGO_CXXFLAGS
CGO_CXXFLAGS+=$(CXXFLAGS_GO) -I$(PWD)/$(SRC) -I$(PWD)/$(PUB)
CGO_CXXFLAGS+=-I$(PWD)/$(GO_D)
CGO_LDFLAGS!=$(GO_BIN) env CGO_LDFLAGS
CGO_LDFLAGS+=$(LDFLAGS) -lm $(PWD)/$(LIB_NAME_SO)
GOFLAGS!=$(GO_BIN) env GOFLAGS
GOFLAGS+=-buildvcs=false
export GOFLAGS CGO_CXXFLAGS CGO_LDFLAGS
export GO111MODULE=on
ifdef GOARCH
# Cross compilation
export CXX CC GOARCH
export CGO_ENABLED=1
endif
go_SFLAGS+=-go

GO_SRC:=$(GO_D)/$(SWIG_LNAME)/$(SWIG_NAME).cpp
$(GO_D)/allocTlv.i: $(GO_D)/allocTlv.i.m4
	$(Q_GEN)$(M4) -I $(SRC) $< > $@
$(GO_D)/$(SWIG_NAME).cpp: $(GO_D)/allocTlv.i
$(GO_SRC): $(GO_D)/$(SWIG_NAME).cpp | $(LIB_NAME_LA)
	$Q$(MKDIR_P) $(@D)
	cp $^ $(GO_D)/$(SWIG_LNAME).go $(@D)
$(GO_D)/$(SWIG_LNAME)/go.mod:
	$(Q_GEN)if [ -f $@ ]; then touch $@; else $(MKDIR_P) $(@D);\
	cd $(@D) && $(GO_BIN) mod init $(SWIG_LNAME) $(Q_ERR);fi
$(GO_D)/gtest/go.mod $(GO_D)/unit_test/go.mod: $(GO_D)/$(SWIG_LNAME)/go.mod\
	  $(GO_SRC)
	$(Q_GEN)cd $(@D);if [ -f $(@F) ]; then touch $(@F);\
	else $(GO_BIN) mod init $(subst $(GO_D)/,,$(@D)) $(Q_ERR);\
	$(GO_BIN) mod edit -replace $(SWIG_LNAME)=../$(SWIG_LNAME);fi;\
	$(GO_BIN) mod tidy $(Q_ERR)
$(GO_D)/gtest/gtest: $(GO_D)/gtest/go.mod $(GO_D)/gtest/gtest.go $(GO_SRC)
	$(Q_LD)cd $(@D) && $(GO_BIN) build .
SWIG_ALL+=$(GO_D)/gtest/gtest
ifndef SKIP_CLKMGR
GO_CLKMGR_DIR:=$(GO_D)/$(CLKMGR_NAME)
$(GO_CLKMGR_DIR)/$(CLKMGR_NAME).cpp: $(GO_D)/$(CLKMGR_NAME).cpp | $(CLKMGR_LIB_LA)
	$Q$(MKDIR_P) $(@D)
	cp $^ $(GO_D)/$(CLKMGR_NAME).go $(@D)
$(GO_CLKMGR_DIR)/go.mod:
	$(Q_GEN)if [ -f $@ ]; then touch $@; else $(MKDIR_P) $(@D);\
	cd $(@D) && $(GO_BIN) mod init $(CLKMGR_NAME) $(Q_ERR);fi
SWIG_ALL+=$(GO_CLKMGR_DIR)/$(CLKMGR_NAME).cpp $(GO_CLKMGR_DIR)/go.mod
endif # SKIP_CLKMGR

utest_go: $(GO_D)/unit_test/go.mod
	$(call Q_UTEST,GO)export LD_PRELOAD="$(PWD)/$(LIB_NAME_SO)"
	cd $(GO_D)/unit_test && $(GO_BIN) test .

install_go:
	$Q$(INSTALL_FOLDER) $(DESTDIR)$(GOROOT)/src
	cp -a $(GO_D)/$(SWIG_LNAME) $(DESTDIR)$(GOROOT)/src
	$(SED) -i 's@^\s*$(c_inc) "\([^.]\+\.h\)"@$(c_inc) <$(SWIG_LNAME)/\1>@'\
	  $(DESTDIR)$(GOROOT)/src/$(SWIG_LNAME)/$(SWIG_NAME).cpp
	$(SED) -i '1 i/* $(SPDXLI) MIT\n   $(SPDXCY) */\n'\
	  $(DESTDIR)$(GOROOT)/src/$(SWIG_LNAME)/*
