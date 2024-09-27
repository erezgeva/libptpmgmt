# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for TCL libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

TCL_D:=wrappers/tcl
TCL_NAME:=$(TCL_D)/$(SWIG_NAME).cpp
TCL_LNAME:=$(TCL_D)/$(SWIG_LNAME)
tcl_SFLAGS+=-tcl8 -namespace -pkgversion $(PACKAGE_VERSION)
$(TCL_LNAME).o: $(TCL_NAME) $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_TCL) $(TCL_INC_FLAGS))
$(TCL_LNAME).so_LDLIBS:=-Wl,-soname,$(SWIG_LIB_NAME)$(SONAME)
$(TCL_LNAME).so: $(TCL_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(TCL_LNAME).so
ifndef SKIP_CLKMGR
TCL_CLKMGR:=$(TCL_D)/$(CLKMGR_NAME)
$(TCL_CLKMGR).o: $(TCL_CLKMGR).cpp
	$Q$(MKDIR_P) $(TCL_D)
	$(call LLC,$(CXXFLAGS_TCL) -I$(TCL_D) $(TCL_INC_FLAGS) $(CLKMGR_CXXFLAGS))
$(TCL_CLKMGR).so: $(TCL_CLKMGR).o $(CLKMGR_LIB_SO)
	$(SWIG_LD)
SWIG_ALL+=$(TCL_CLKMGR).so
TCLDIR_CLKMGR:=$(DESTDIR)$(TCL_SITE)/$(CLKMGR_NAME)
endif # SKIP_CLKMGR
TCLDIR:=$(DESTDIR)$(TCL_SITE)/$(SWIG_LNAME)
pkgIndex:=if {![package vsatisfies [package provide Tcl] $(TCL_MINVER)]} {return}\n
clkmgr_pkgIndex:=$(pkgIndex)
pkgIndex+=package ifneeded $(SWIG_LNAME) $(PACKAGE_VERSION)
clkmgr_pkgIndex+=package ifneeded $(CLKMGR_NAME) $(PACKAGE_VERSION)
pkgIndex+=[list load [file join $$dir $(SWIG_LIB_NAME)]]\n
clkmgr_pkgIndex+=[list load [file join $$dir $(CLKMGR_NAME).so]]\n
$(TCL_D)/pkgIndex.tcl:
	$(Q)$(TCL_D)/pkgIndex_tcl.sh $(TCL_D)
utest_tcl: $(LIB_NAME_SO) $(TCL_LNAME).so $(TCL_D)/pkgIndex.tcl
	$(call Q_UTEST,TCL)LD_PRELOAD=$(ASAN_PRE)./$<\
	  TCLLIBPATH=$(TCL_D)$(UVGD)$(TCL_D)/utest.tcl $(TCL_FILTERS)

install_tcl:
	$Q$(INSTALL_LIB) -D $(TCL_LNAME).so -t $(TCLDIR)
	printf '$(hash) $(SPDXLI) $(SPDXLGPL)\n$(hash) $(SPDXCY)\n\n$(pkgIndex)' >\
	  $(TCLDIR)/pkgIndex.tcl
ifndef SKIP_CLKMGR
	$(INSTALL_LIB) -D $(TCL_CLKMGR).so -t $(TCLDIR_CLKMGR)
	printf '$(hash) %s %s\n$(hash) %s\n\n$(clkmgr_pkgIndex)'\
	  '$(SPDXLI)' '$(SPDXBSD3)' '$(SPDXCY_INTEL)' >\
	  $(TCLDIR_CLKMGR)/pkgIndex.tcl
endif
