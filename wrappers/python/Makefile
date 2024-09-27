# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for Python libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

PY_D:=wrappers/python
PY_BASE:=$(PY_D)/$(SWIG_NAME)
PY_LIB_NAME:=_$(SWIG_LNAME)
python_SFLAGS+=-python
ifeq ($(PY_USE_S_THRD),)
python_SFLAGS+=-threads -DSWIG_USE_MULTITHREADS
endif
PY_BASE_3:=$(PY_D)/3/$(SWIG_NAME)
PY_SO_3:=$(PY_D)/3/$(PY_LIB_NAME).so
$(PY_SO_3)_LDLIBS:=$(PY3_LIB_FLAGS)
PY_INC_BASE_3:=$(subst -I,,$(firstword $(PY3_INC_FLAGS)))
$(PY_BASE_3).o: $(PY_BASE).cpp $(HEADERS)
	$Q$(MKDIR_P) $(PY_D)/3
	$(call LLC,$(CXXFLAGS_PY) -I$(PY_D) $(PY3_INC_FLAGS))
$(PY_SO_3): $(PY_BASE_3).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(PY_SO_3)
ifndef SKIP_CLKMGR
PY_CLKMGR:=$(PY_D)/$(CLKMGR_NAME)
PY_CLKMGRLIB_NAME:=_$(CLKMGR_NAME)
PY_CLKMGR_3:=$(PY_D)/3/$(CLKMGR_NAME)
PY_CLKMGR_SO_3:=$(PY_D)/3/$(PY_CLKMGRLIB_NAME).so
$(PY_CLKMGR_SO_3)_LDLIBS:=$(PY3_LIB_FLAGS)
$(PY_CLKMGR_3).o: $(PY_CLKMGR).cpp
	$Q$(MKDIR_P) $(PY_D)/3
	$(call LLC,$(CXXFLAGS_PY) -I$(PY_D) $(PY3_INC_FLAGS) $(CLKMGR_CXXFLAGS))
$(PY_CLKMGR_SO_3): $(PY_CLKMGR_3).o $(CLKMGR_LIB_SO)
	$(SWIG_LD)
SWIG_ALL+=$(PY_CLKMGR_SO_3)
endif # SKIP_CLKMGR
utest_python3: $(LIB_NAME_SO) $(PY_SO_3)
	$(call Q_UTEST,Python3)LD_PRELOAD=$(ASAN_PRE)./$< PYTHONPATH=$(PY_D)/3\
	  $(PYUVGD)$(PY_D)/utest.py $(PY_FILTERS)

install_python3:
	$Q$(INSTALL_LIB) -D $(PY_D)/3/$(PY_LIB_NAME).so\
	  $(DESTDIR)$(PY3_SITE)/$(PY_LIB_NAME)$(PY3_EXT)
	$(INSTALL_LIB) $(PY_D)/$(SWIG_LNAME).py $(DESTDIR)$(PY3_SITE)
	$(SED) -i "1 i$(hash) $(SPDXLI) $(SPDXLGPL)\n$(hash) $(SPDXCY)\n"\
	  $(DESTDIR)$(PY3_SITE)/$(SWIG_LNAME).py
ifndef SKIP_CLKMGR
	$(INSTALL_LIB) -D $(PY_CLKMGR_SO_3)\
	  $(DESTDIR)$(PY3_SITE)/$(PY_CLKMGRLIB_NAME)$(PY3_EXT)
	$(INSTALL_LIB) $(PY_D)/$(CLKMGR_NAME).py $(DESTDIR)$(PY3_SITE)
	$(SED) -i "1 i$(hash) $(SPDXLI) $(SPDXBSD3)\n$(hash) $(SPDXCY_INTEL)\n"\
	  $(DESTDIR)$(PY3_SITE)/$(CLKMGR_NAME).py
endif
# Used by gentoo to byte-compile
compile_py3:
	$Q$(PY3_BIN) -m compileall -q -f -d "$(PY3_SITE)" "$(DESTDIR)$(PY3_SITE)"
	$(PY3_BIN) -OO -m compileall -q -f -d "$(PY3_SITE)" "$(DESTDIR)$(PY3_SITE)"
	$(PY3_BIN) -O -m compileall -q -f -d "$(PY3_SITE)" "$(DESTDIR)$(PY3_SITE)"
