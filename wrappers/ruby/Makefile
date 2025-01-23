# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for Ruby libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

RUBY_D:=wrappers/ruby
RUBY_NAME:=$(RUBY_D)/$(SWIG_NAME).cpp
RUBY_LNAME:=$(RUBY_D)/$(SWIG_LNAME)
ruby_SFLAGS:=-ruby
$(RUBY_LNAME).so_LDLIBS:=$(RUBY_LIB_FLAGS)
$(RUBY_LNAME).o: $(RUBY_NAME) $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_RUBY) -I$(RUBY_D) $(RUBY_INC_FLAGS))
$(RUBY_LNAME).so: $(RUBY_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(RUBY_LNAME).so
utest_ruby: $(LIB_NAME_SO) $(RUBY_LNAME).so
	$(call Q_UTEST,Ruby)LD_PRELOAD=$(ASAN_PRE)./$< RUBYLIB=$(RUBY_D)\
	  $(RUBY_D)/utest.rb $(RUBY_NO_COL) $(RUBY_FILTERS)

install_ruby:
	$Q$(INSTALL_LIB) -D $(RUBY_LNAME).so -t $(DESTDIR)$(RUBY_SITE)
