# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2022 Erez Geva
#
# Makefile for PHP libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2022 Erez Geva
#
###############################################################################

PHP_NAME:=php/$(SWIG_NAME).cpp
php_SFLAGS+=-php7
$(PHP_LNAME).o: $(PHP_NAME) $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_PHP) -Iphp $(PHPINC_FLAGS))
	$(call D_INC,PHPINC)
	$(SWIG_DEP)
$(PHP_LNAME).so: $(PHP_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(PHP_LNAME).so
php/php.ini:
	$(Q)php/php_ini.sh php/
utest_php: $(LIB_NAME_SO) $(PHP_LNAME).so php/php.ini
	$(call Q_UTEST,PHP)LD_PRELOAD=./$< PHPRC=php phpunit php/utest.php\
	  $(PHP_NO_COL) $(PHP_FILTERS)

install_php:
	$Q$(INSTALL_PROGRAM) -D $(PHP_LNAME).so -t $(DESTDIR)$(PHPEXT)
	$(INSTALL_DATA) -D $(PHP_LNAME).php -t $(DESTDIR)$(PHPINCDIR)
