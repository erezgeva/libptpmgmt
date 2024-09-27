# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for Perl libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

perl_SFLAGS+=-perl5
PERL_D:=wrappers/perl
$(PERL_D)/$(SWIG_NAME).cpp: perl_SFLAGS+=-module $(SWIG_NAME)
$(PERL_D)/$(SWIG_NAME).o: $(PERL_D)/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_PERL) -I$(PERL_D) $(PERL5_INC_FLAGS))
PERL_SO_DIR:=$(PERL_D)/auto/$(SWIG_NAME)
$(PERL_SO_DIR):
	$Q$(MKDIR_P) $@
$(PERL_SO_DIR)/$(SWIG_NAME).so: $(PERL_D)/$(SWIG_NAME).o $(LIB_NAME_SO) |\
	  $(PERL_SO_DIR)
	$(SWIG_LD)
SWIG_ALL+=$(PERL_SO_DIR)/$(SWIG_NAME).so
ifndef SKIP_CLKMGR
PERL_CLKMGR:=$(PERL_D)/$(CLKMGR_NAME)
PERL_CLKMGR_NAME:=ClkMgrLib
PERL_CLKMGR_SO_DIR:=$(PERL_D)/auto/$(PERL_CLKMGR_NAME)
$(PERL_CLKMGR_SO_DIR):
	$Q$(MKDIR_P) $@
$(PERL_CLKMGR).cpp: perl_SFLAGS+=-module $(PERL_CLKMGR_NAME)
$(PERL_CLKMGR).o: $(PERL_CLKMGR).cpp
	$Q$(MKDIR_P) $(PERL_D)
	$(call LLC,$(CXXFLAGS_PERL) -I$(PERL_D) $(PERL5_INC_FLAGS) $(CLKMGR_CXXFLAGS))
$(PERL_CLKMGR_SO_DIR)/$(PERL_CLKMGR_NAME).so: $(PERL_CLKMGR).o $(CLKMGR_LIB_SO) |\
	  $(PERL_CLKMGR_SO_DIR)
	$(SWIG_LD)
SWIG_ALL+=$(PERL_CLKMGR_SO_DIR)/$(PERL_CLKMGR_NAME).so
endif # SKIP_CLKMGR
ifdef PERL5_HAVE_TEST
utest_perl5: $(LIB_NAME_SO) $(PERL_SO_DIR)/$(SWIG_NAME).so
	$(call Q_UTEST,Perl5)LD_PRELOAD=$(ASAN_PRE)./$< PERL5LIB=$(PERL_D)\
	  $(PERL_D)/utest.pl
endif

install_perl5:
	$(Q)$(INSTALL_FOLDER) $(DESTDIR)$(PERL5_SITE)
	cp -a $(PERL_D)/auto $(DESTDIR)$(PERL5_SITE)
	$(INSTALL_LIB) $(PERL_D)/*.pm $(DESTDIR)$(PERL5_SITE)
	$(SED) -i "1 i$(hash) $(SPDXLI) $(SPDXLGPL)\n$(hash) $(SPDXCY)\n"\
	  $(DESTDIR)$(PERL5_SITE)/$(SWIG_NAME).pm
ifndef SKIP_CLKMGR
	$(SED) -i "1 i$(hash) $(SPDXLI) $(SPDXBSD3)\n$(hash) $(SPDXCY_INTEL)\n"\
	  $(DESTDIR)$(PERL5_SITE)/$(PERL_CLKMGR_NAME).pm
endif
