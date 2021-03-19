# SPDX-License-Identifier: GPL-3.0-or-later

# Makefile Create libpmc and pmc for testing
#
# Authors: Erez Geva <ErezGeva2@gmail.com>

PMC_USE_LIB?=a # 'a' for static and 'so' for dynamic

which=$(shell which $1 2>/dev/null)
define depend
$(1): $(2)

endef
verCheckDo=$(shell test $1 -eq $3 && a=$2 b=$4 || a=$1 b=$3; \
             test $$a -lt $$b && echo l)
verCheck=$(call verCheckDo,$(firstword $(subst ., ,$1 0 0)),$(word 2,\
         $(subst ., ,$1 0 0)),$(firstword $(subst ., ,$2 0)),$(word 2,\
         $(subst ., ,$2 0)))

include version
# Ensure linker link using C++
CC:=g++
RL:=ranlib
CPPFLAGS+=-Wall -std=c++11 -g
CPPFLAGS+= -MT $@ -MMD -MP -MF $(basename $@).d
LIBTOOL_CC:=libtool --mode=compile --tag=CXX
PMC_OBJS:=$(patsubst %.cpp,%.o,$(wildcard pmc*.cpp))
LIB_OBJS:=$(filter-out $(PMC_OBJS),$(patsubst %.cpp,%.o,$(wildcard *.cpp)))
LIB_NAME:=libpmc
PMC_NAME:=pmc
msg.o: CPPFLAGS+=-DVER_MAJ=$(ver_maj) -DVER_MIN=$(ver_min)

ALL:=$(PMC_NAME) $(LIB_NAME).so $(LIB_NAME).a
main: all

# Compile library source code
$(LIB_OBJS):
	$(LIBTOOL_CC) $(CXX) -c $(CPPFLAGS) $(basename $@).cpp -o $@
# Depened shared library objects on the static library to ensure build
$(eval $(foreach obj,$(LIB_OBJS), $(call depend,.libs/$(obj),$(obj))))

$(LIB_NAME).a: $(LIB_OBJS)
	$(AR) rcs $@ $^
	$(RL) $@
$(LIB_NAME).so: $(foreach obj,$(LIB_OBJS),.libs/$(obj))
	$(CXX) $(LDFLAGS) $(LDFLAGS_SO) -shared $^ $(LOADLIBES) $(LDLIBS) -o $@

$(PMC_NAME): $(PMC_OBJS) $(LIB_NAME).$(PMC_USE_LIB)

include $(wildcard *.d)

CLEAN:=*.o *.lo *.d .libs/*
DISTCLEAN:=$(ALL)
DISTCLEAN_DIRS:=.libs

clean:
	$(RM) -f $(CLEAN)
distclean: deb_clean clean
	$(RM) -f $(DISTCLEAN)
	$(RM) -Rf $(DISTCLEAN_DIRS)

ifneq ($(call which,astyle),)
astyle_ver:=$(lastword $(shell astyle -V))
ifeq ($(call verCheck,$(astyle_ver),3.1),)
FORMAT_SRC:=$(wildcard *.h *.cpp)
FORMAT_ASTYLE:=$(filter-out pmc_dump.cpp,$(FORMAT_SRC))
format:
	@astyle --project=none --options=astyle.opt $(FORMAT_ASTYLE)
endif
endif # which astyle

ifneq ($(call which,dpkg-architecture),)
DEB_HOST_MULTIARCH:=$(shell dpkg-architecture -qDEB_HOST_MULTIARCH)
endif # dpkg-architecture

SWIG:=swig
ifneq ($(call which,$(SWIG)),)
swig_ver=$(lastword $(shell swig -version | grep Version))
ifeq ($(call verCheck,$(swig_ver),3.0),)
SWIG_ALL:=
SWIG_NAME:=PmcLib
ifneq ($(call which,perl),)
PERL_VER:=$(shell perl -e '$$_=$$^V;s/^v//;print')
ifneq ($(DEB_HOST_MULTIARCH),)
PERL_INC:=/usr/lib/$(DEB_HOST_MULTIARCH)/perl/$(PERL_VER)/CORE
ifeq ($(realpath $(PERL_INC)),)
PERL_INC=/usr/lib/perl/$(PERL_VER)/CORE
endif
else
PERL_INC=/usr/lib/perl/$(PERL_VER)/CORE
endif # realpath $(PERL_INC)
PERL_NAME:=perl/$(SWIG_NAME)
$(PERL_NAME).cpp: $(LIB_NAME).i $(wildcard *.h)
	ln -fs ids.h ids2.h
	$(SWIG) -c++ -I. -outdir perl -o $@ -perl5 $<
$(PERL_NAME).o: $(PERL_NAME).cpp
	$(CXX) $(CPPFLAGS) -fPIC -DPIC -I. -I$(PERL_INC) -c $< -o $@
	sed -i 's#$(PERL_INC)#\$$(PERL_INC)#' $(PERL_NAME).d
$(PERL_NAME).so: $(PERL_NAME).o $(LIB_NAME).so
	$(CXX) $(LDFLAGS) -shared $^ $(LOADLIBES) $(LDLIBS) -o $@
SWIG_ALL+=$(PERL_NAME).so
CLEAN+=$(foreach e,d o,$(PERL_NAME).$e)
DISTCLEAN+=$(foreach e,cpp pm so,$(PERL_NAME).$e) ids2.h
include $(wildcard perl/*.d)
endif # which perl

ALL+=$(SWIG_ALL)
endif # swig 3.0
endif # which swig

ifneq ($(call which,dpkg-buildpackage),)
deb_src:
	dpkg-source -b .
deb:
	dpkg-buildpackage -b -us -uc
deb_clean:
	@$(MAKE) -f debian/rules deb_clean
endif # Debian

ifneq ($(call which,doxygen),)
ifeq ($(call verCheck,$(shell doxygen -v),1.8),)
doxygen:
	doxygen doxygen.cfg
DISTCLEAN_DIRS+=doc
endif
endif # which doxygen

ifneq ($(call which,ctags),)
tags: $(wildcard *.h *.cpp)
	ctags -R
ALL+=tags
DISTCLEAN+=tags
endif # which ctags

all: $(ALL)

.PHONY: all main clean distclean format deb_src deb deb_clean doxygen
