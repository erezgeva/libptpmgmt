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
PERL_SO:=$(PERL_SO_DIR)/$(SWIG_NAME).so
$(PERL_SO_DIR):
	$Q$(MKDIR_P) $@
$(PERL_SO): $(PERL_D)/$(SWIG_NAME).o $(LIB_NAME_SO) |\
	  $(PERL_SO_DIR)
	$(SWIG_LD)
SWIG_ALL+=$(PERL_SO)
ifndef SKIP_CLKMGR
PERL_CLKMGR:=$(PERL_D)/$(CLKMGR_NAME)
PERL_CLKMGR_NAME:=ClkMgrLib
PERL_CLKMGR_SO_DIR:=$(PERL_D)/auto/$(PERL_CLKMGR_NAME)
$(PERL_CLKMGR_SO_DIR):
	$Q$(MKDIR_P) $@
$(PERL_CLKMGR).cpp: perl_SFLAGS+=-module $(PERL_CLKMGR_NAME)
$(PERL_CLKMGR).o: $(PERL_CLKMGR).cpp
	$Q$(call LLC,$(CXXFLAGS_PERL) -I$(PERL_D) $(PERL5_INC_FLAGS) $(CLKMGR_CXXFLAGS))
$(PERL_CLKMGR_SO_DIR)/$(PERL_CLKMGR_NAME).so: $(PERL_CLKMGR).o $(CLKMGR_LIB_SO) |\
	  $(PERL_CLKMGR_SO_DIR)
	$(SWIG_LD)
SWIG_ALL+=$(PERL_CLKMGR_SO_DIR)/$(PERL_CLKMGR_NAME).so
endif # SKIP_CLKMGR
ifdef PERL5_HAVE_TEST
PERL_UTEST:=$(PERL_D)/utest
PERL_UTEST_NAME:=UtestLib
PERL_UTEST_SO_DIR:=$(PERL_D)/auto/$(PERL_UTEST_NAME)
PERL_UTEST_SO:=$(PERL_UTEST_SO_DIR)/$(PERL_UTEST_NAME).so
$(PERL_UTEST_SO_DIR):
	$Q$(MKDIR_P) $@
$(PERL_UTEST).cpp: perl_SFLAGS+=-module $(PERL_UTEST_NAME)
$(PERL_UTEST).o: $(PERL_UTEST).cpp
	$Q$(call LLC,$(CXXFLAGS_PERL) -Iutest $(PERL5_INC_FLAGS))
$(PERL_UTEST_SO): $(PERL_UTEST).o $(LIBUTEST) |\
	  $(PERL_UTEST_SO_DIR)
	$(SWIG_LD)
utest_perl5: $(LIB_NAME_SO) $(PERL_SO) $(PERL_UTEST_SO)
	$(call Q_UTEST,Perl5)LD_PRELOAD=$(ASAN_PRE)./$< PERL5LIB=$(PERL_D)\
	  $(PERL_D)/utest.pl
	LD_PRELOAD=$(ASAN_PRE)./$< PERL5LIB=$(PERL_D)\
	  $(PERL_D)/utestSig.pl
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
