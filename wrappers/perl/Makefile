# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for Perl libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

perl_SFLAGS+=-perl5 -module PtpMgmtLib
PERL_D:=wrappers/perl
$(PERL_D)/$(SWIG_NAME).o: $(PERL_D)/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_PERL) -Iwrappers/perl $(PERL5_INC_FLAGS))
PERL_SO_DIR:=$(PERL_D)/auto/$(SWIG_NAME)
$(PERL_SO_DIR):
	$Q$(MKDIR_P) $@
$(PERL_SO_DIR)/$(SWIG_NAME).so: $(PERL_D)/$(SWIG_NAME).o $(LIB_NAME_SO) |\
	  $(PERL_SO_DIR)
	$(SWIG_LD)
SWIG_ALL+=$(PERL_SO_DIR)/$(SWIG_NAME).so
ifdef PERL5_HAVE_TEST
utest_perl5: $(LIB_NAME_SO) $(PERL_SO_DIR)/$(SWIG_NAME).so
	$(call Q_UTEST,Perl5)LD_PRELOAD=$(ASAN_PRE)./$< PERL5LIB=$(PERL_D)\
	  $(PERL_D)/utest.pl
endif

install_perl5:
	$Q$(INSTALL_LIB) -D $(PERL_D)/auto/*/*.so -t\
	  $(DESTDIR)$(PERL5_SITE)/auto/$(SWIG_NAME)
	$(INSTALL_LIB) $(PERL_D)/*.pm $(DESTDIR)$(PERL5_SITE)
	$(SED) -i "1 i# $(SPDXLI) LGPL-3.0-or-later\n# $(SPDXCY)\n"\
	  $(DESTDIR)$(PERL5_SITE)/*.pm
