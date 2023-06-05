# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for Python libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

PY_BASE:=python/$(SWIG_NAME)
PY_LIB_NAME:=_ptpmgmt
python_SFLAGS+=-python
ifeq ($(PY_USE_S_THRD),)
python_SFLAGS+=-threads -DSWIG_USE_MULTITHREADS
endif
PY_BASE_3:=python/3/$(SWIG_NAME)
PY_SO_3:=python/3/$(PY_LIB_NAME).so
$(PY_SO_3)_LDLIBS:=$(PY3LDLIBS)
PY_INC_BASE_3:=$(subst -I,,$(firstword $(PY3INCLUDE)))
$(PY_BASE_3).o: $(PY_BASE).cpp $(HEADERS)
	$Q$(MKDIR_P) python/3
	$(call LLC,$(CXXFLAGS_PY) $(PY3INCLUDE))
	$(call D_INC,PY3INCDIR)
ifneq ($(PY3INCDIR),$(PY3PLATINCDIR))
	$(call D_INC,PY3PLATINCDIR)
endif
	$(SWIG_DEP)
$(PY_SO_3): $(PY_BASE_3).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(PY_SO_3)
ifneq ($(UVGD),$(SP)) # VALGRIND
PYUVGD:=PYTHONMALLOC=malloc$(UVGD)
endif
utest_python3: $(LIB_NAME_SO) $(PY_SO_3)
	$(call Q_UTEST,Python3)LD_PRELOAD=./$< PYTHONPATH=python/3\
	  $(PYUVGD)python/utest.py $(PY_FILTERS)

install_python3:
	$Q$(INSTALL_LIB) -D python/3/$(PY_LIB_NAME).so\
	  $(DESTDIR)$(PY3SITE_DIR)/$(PY_LIB_NAME)$(PY3EXT)
	$(INSTALL_LIB) python/$(SWIG_LNAME).py $(DESTDIR)$(PY3SITE_DIR)
