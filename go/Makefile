# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for go libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
###############################################################################

# Go environment
CGO_CXXFLAGS!=$(GOBIN) env CGO_CXXFLAGS
CGO_CXXFLAGS+=$(CXXFLAGS_GO) -I $(PWD)/src
CGO_LDFLAGS!=$(GOBIN) env CGO_LDFLAGS
CGO_LDFLAGS+=$(LDFLAGS) -lm $(PWD)/$(LIB_NAME_SO)
GOFLAGS!=$(GOBIN) env GOFLAGS
GOFLAGS+=-buildvcs=false
export GOFLAGS CGO_CXXFLAGS CGO_LDFLAGS
export GO111MODULE=on
ifneq ($(GOARCH),)
# Cross compilation
export CXX CC GOARCH
export CGO_ENABLED=1
endif
go_SFLAGS+=-go

GO_SRC:=go/$(SWIG_LNAME)/$(SWIG_NAME).cpp
$(GO_SRC): go/$(SWIG_NAME).cpp $(LIB_NAME_FSO)
	$Q$(MKDIR_P) $(@D)
	cp $^ go/$(SWIG_LNAME).go $(@D)
go/$(SWIG_LNAME)/go.mod:
	$(Q_GEN)if [ -f $@ ]; then touch $@; else $(MKDIR_P) $(@D);\
	cd $(@D) && $(GOBIN) mod init $(SWIG_LNAME) $(Q_ERR);fi
go/gtest/go.mod go/unit_test/go.mod: go/$(SWIG_LNAME)/go.mod $(GO_SRC)
	$(Q_GEN)cd $(@D);if [ -f $(@F) ]; then touch $(@F);\
	else $(GOBIN) mod init $(subst go/,,$(@D)) $(Q_ERR);\
	$(GOBIN) mod edit -replace $(SWIG_LNAME)=../$(SWIG_LNAME);fi;\
	$(GOBIN) mod tidy $(Q_ERR)
go/gtest/gtest: go/gtest/go.mod go/gtest/gtest.go $(GO_SRC)
	$(Q_LD)cd $(@D) && $(GOBIN) build .
SWIG_ALL+=go/gtest/gtest

utest_go: go/unit_test/go.mod
	$(call Q_UTEST,GO)export LD_PRELOAD="$(PWD)/$(LIB_NAME_SO)"
	cd go/unit_test && $(GOBIN) test .

install_go:
	$Q$(INSTALL_FOLDER) $(DESTDIR)$(GOROOT)/src
	cp -a go/$(SWIG_LNAME) $(DESTDIR)$(GOROOT)/src
	$(SED) -i 's@^\s*#include "\([^.]\+\.h\)"@#include <$(SWIG_LNAME)/\1>@'\
	  $(DESTDIR)$(GOROOT)/src/$(SWIG_LNAME)/$(SWIG_NAME).cpp
	rm $(DESTDIR)$(GOROOT)/src/$(SWIG_LNAME)/$(LIB_NAME_SO)*
