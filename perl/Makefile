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
perl/$(SWIG_NAME).o: perl/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(call LLC,-I$(PERL5EXT))
	$(call D_INC,PERL5EXT)
	$(SWIG_DEP)
PERL_SO_DIR:=perl/auto/$(SWIG_NAME)
$(PERL_SO_DIR):
	$Q$(MKDIR_P) $@
$(PERL_SO_DIR)/$(SWIG_NAME).so: perl/$(SWIG_NAME).o $(LIB_NAME_SO) | $(PERL_SO_DIR)
	$(SWIG_LD)
SWIG_ALL+=$(PERL_SO_DIR)/$(SWIG_NAME).so
utest_perl5: $(LIB_NAME_SO) $(PERL_SO_DIR)/$(SWIG_NAME).so
	$(call Q_UTEST,Perl5)LD_PRELOAD=./$< PERL5LIB=perl perl/utest.pl

install_perl5:
	$Q$(INSTALL_LIB) -D perl/auto/*/*.so -t\
	  $(DESTDIR)$(PERL5DIR)/auto/$(SWIG_NAME)
	$(INSTALL_LIB) perl/*.pm $(DESTDIR)$(PERL5DIR)
