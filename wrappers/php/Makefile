# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for PHP libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

PHP_D:=wrappers/php
PHP_NAME:=$(PHP_D)/$(SWIG_NAME).cpp
php_SFLAGS+=-php7
$(PHP_LNAME).o: $(PHP_NAME) $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_PHP) -I$(PHP_D) $(PHP_INC_FLAGS))
$(PHP_LNAME).so: $(PHP_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(PHP_LNAME).so
$(PHP_D)/php.ini:
	$(Q)$(PHP_D)/php_ini.sh
ifndef SKIP_CLKMGR
PHP_CLKMGR:=$(PHP_D)/$(CLKMGR_NAME)
$(PHP_CLKMGR).o: $(PHP_CLKMGR).cpp
	$Q$(MKDIR_P) $(PHP_D)
	$(call LLC,$(CXXFLAGS_PHP) -I$(PHP_D) $(PHP_INC_FLAGS) $(CLKMGR_CXXFLAGS))
$(PHP_CLKMGR).so: $(PHP_CLKMGR).o $(CLKMGR_LIB_SO)
	$(SWIG_LD)
SWIG_ALL+=$(PHP_CLKMGR).so
endif # SKIP_CLKMGR
ifdef PHP_UNIT
utest_php: $(LIB_NAME_SO) $(PHP_LNAME).so $(PHP_D)/php.ini
	$(call Q_UTEST,PHP)LD_PRELOAD=$(ASAN_PRE)./$< PHPRC=$(PHP_D) $(PHP_UNIT)\
	  $(PHP_D)/utest.php $(PHP_NO_COL) $(PHP_FILTERS)
endif

install_php:
	$Q$(INSTALL_LIB) -D $(PHP_LNAME).so -t $(DESTDIR)$(PHP_SITE)
ifndef SKIP_CLKMGR
	$(INSTALL_LIB) -D $(PHP_CLKMGR).so -t $(DESTDIR)$(PHP_SITE)
endif
