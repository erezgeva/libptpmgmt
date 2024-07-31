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
	$Q$(call LLC,$(CXXFLAGS_PHP) -Iwrappers/php $(PHPINC_FLAGS))
$(PHP_LNAME).so: $(PHP_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(PHP_LNAME).so
$(PHP_D)/php.ini:
	$(Q)$(PHP_D)/php_ini.sh
ifdef PHPUNIT
utest_php: $(LIB_NAME_SO) $(PHP_LNAME).so $(PHP_D)/php.ini
	$(call Q_UTEST,PHP)LD_PRELOAD=$(ASAN_PRE)./$< PHPRC=$(PHP_D) phpunit\
	  $(PHP_D)/utest.php $(PHP_NO_COL) $(PHP_FILTERS)
endif

install_php:
	$Q$(INSTALL_LIB) -D $(PHP_LNAME).so -t $(DESTDIR)$(PHPEXT)
