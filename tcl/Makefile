# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for TCL libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

TCL_NAME:=tcl/$(SWIG_NAME).cpp
TCL_LNAME:=tcl/$(SWIG_LNAME)
tcl_SFLAGS+=-tcl8 -namespace -pkgversion $(PACKAGE_VERSION)
$(TCL_LNAME).o: $(TCL_NAME) $(HEADERS)
	$Q$(call LLC,$(TCLINCLUDE))
	$(call D_INC,TCLINCDIR)
	$(SWIG_DEP)
$(TCL_LNAME).so_LDLIBS:=-Wl,-soname,$(SWIG_LIB_NAME)$(SONAME)
$(TCL_LNAME).so: $(TCL_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(TCL_LNAME).so
TCLDIR:=$(DESTDIR)$(TCL_PKG_DIR)/$(SWIG_LNAME)
pkgIndex:=if {![package vsatisfies [package provide Tcl] $(TCLVER)]} {return}\n
pkgIndex+=package ifneeded $(SWIG_LNAME) $(ver_maj)
pkgIndex+=[list load [file join $$dir $(SWIG_LIB_NAME)]]\n
tcl/pkgIndex.tcl:
	$(Q)tcl/pkgIndex_tcl.sh tcl $(ver_maj)
utest_tcl: $(LIB_NAME_SO) $(TCL_LNAME).so tcl/pkgIndex.tcl
	$(call Q_UTEST,TCL)LD_PRELOAD=$(ASAN_PRE)./$< TCLLIBPATH=tcl$(UVGD)tcl/utest.tcl\
	  $(TCL_FILTERS)

install_tcl:
	$Q$(INSTALL_LIB) -D $(TCL_LNAME).so -t $(TCLDIR)
	printf '$(pkgIndex)' > $(TCLDIR)/pkgIndex.tcl
