# SPDX-License-Identifier: GPL-3.0-or-later

# Makefile Create libpmc and pmc for testing
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva

PMC_USE_LIB?=a # 'a' for static and 'so' for dynamic

which=$(shell which $1 2>/dev/null)
libname=$(subst lib,,$(basename $(notdir $(lastword $(wildcard /usr/lib/$1)))))
define depend
$1: $2

endef
verCheckDo=$(shell test $1 -eq $3 && a=$2 b=$4 || a=$1 b=$3; \
             test $$a -lt $$b && echo l)
verCheck=$(call verCheckDo,$(firstword $(subst ., ,$1 0 0)),$(word 2,\
         $(subst ., ,$1 0 0)),$(firstword $(subst ., ,$2 0)),$(word 2,\
         $(subst ., ,$2 0)))

# Use tput to check if we have ANSI Color code
# tput works only if TERM is set
ifneq ($(and $(TERM),$(call which,tput)),)
ifeq ($(shell tput setaf 1),)
NO_COL:=1
endif
endif
# Detect output is not device (terminal), it must be a pipe or a file
# In case of using 'aha' just call: $ make -j USE_COL=1 | aha > out.html
ifndef USE_COL
ifeq ($(MAKE_TERMOUT),)
NO_COL:=1
endif
endif

# Terminal colors
ifndef NO_COL
ESC:=$(shell printf '\033[')
COLOR_BLACK:=      $(ESC)30m
COLOR_RED:=        $(ESC)31m
COLOR_GREEN:=      $(ESC)32m
COLOR_YELLOW:=     $(ESC)33m
COLOR_BLUE:=       $(ESC)34m
COLOR_MAGENTA:=    $(ESC)35m
COLOR_CYAN:=       $(ESC)36m
COLOR_LTGRAY:=     $(ESC)37m
COLOR_DRKGRAY:=    $(ESC)1;30m
COLOR_NORM:=       $(ESC)00m
COLOR_BACKGROUND:= $(ESC)07m
COLOR_BRIGHTEN:=   $(ESC)01m
COLOR_UNDERLINE:=  $(ESC)04m
COLOR_BLINK:=      $(ESC)05m
endif

ifneq ($(V),1)
Q:=@
Q_DOXY=$(info $(COLOR_MAGENTA)Doxygen$(COLOR_NORM))
Q_FRMT=$(info $(COLOR_MAGENTA)Format$(COLOR_NORM))
Q_TAGS=$(info $(COLOR_MAGENTA)[TAGS]$(COLOR_NORM))
Q_GEN=$(info $(COLOR_MAGENTA)[GEN] $@$(COLOR_NORM))
Q_SWIG=$(info $(COLOR_MAGENTA)[SWIG] $@$(COLOR_NORM))
Q_CLEAN=$(info $(COLOR_MAGENTA)Cleaning$(COLOR_NORM))
Q_DISTCLEAN=$(info $(COLOR_MAGENTA)Cleaning all$(COLOR_NORM))
Q_LD=$(info $(COLOR_MAGENTA)[LD] $@$(COLOR_NORM))
Q_AR=$(info $(COLOR_MAGENTA)[AR] $@$(COLOR_NORM))
Q_LCC=$(info $(COLOR_MAGENTA)[LCC] $(basename $@).cpp$(COLOR_NORM))
Q_CC=$(info $(COLOR_MAGENTA)[CC] $(basename $@).cpp$(COLOR_NORM))
LIBTOOL_QUIET:=--quiet
MAKE_NO_DIRS:=--no-print-directory
endif

include version
# Ensure linker use C++
CC:=g++
RL:=ranlib
LN:=ln -fs
CPPFLAGS_OPT?=-Og
CPPFLAGS+=-Wdate-time -Wall -std=c++11 -g $(CPPFLAGS_OPT)
CPPFLAGS_SWIG:=-Wno-maybe-uninitialized -Wno-stringop-overflow
CPPFLAGS+= -MT $@ -MMD -MP -MF $(basename $@).d
CPPFLAGS_SO:=-fPIC -DPIC -I.
LIBTOOL_CC=$(Q_LCC)$(Q)libtool --mode=compile --tag=CXX $(LIBTOOL_QUIET)
ifneq ($(SONAME),)
LDFLAGS_NM=-Wl,--version-script,pkg/lib.ver\
           -Wl,-soname,$@.$(SONAME)
endif
LDLIBS_LIB:=-lm
PMC_OBJS:=$(patsubst %.cpp,%.o,$(wildcard pmc*.cpp))
LIB_OBJS:=$(filter-out $(PMC_OBJS),$(patsubst %.cpp,%.o,$(wildcard *.cpp)))
LIB_NAME:=libpmc
LIB_NAME_SO:=$(LIB_NAME).so
PMC_NAME:=pmc
msg.o: CPPFLAGS+=-DVER_MAJ=$(ver_maj) -DVER_MIN=$(ver_min)

ifeq ($(call verCheck,$(shell $(CXX) -dumpversion),4.9),)
# GCC output colors
ifndef NO_COL
CPPFLAGS_COLOR:=-fdiagnostics-color=always
else
CPPFLAGS_COLOR:=-fdiagnostics-color=never
endif
endif
CPPFLAGS+=$(CPPFLAGS_COLOR)

ALL:=$(PMC_NAME) $(LIB_NAME_SO) $(LIB_NAME).a

# Compile library source code
$(LIB_OBJS):
	$(LIBTOOL_CC) $(CXX) -c $(CPPFLAGS) $(basename $@).cpp -o $@
# Depened shared library objects on the static library to ensure build
$(eval $(foreach obj,$(LIB_OBJS), $(call depend,.libs/$(obj),$(obj))))

$(LIB_NAME).a: $(LIB_OBJS)
	$(Q_AR)
	$Q$(AR) rcs $@ $^
	$Q$(RL) $@
$(LIB_NAME_SO): $(foreach obj,$(LIB_OBJS),.libs/$(obj))
	$(Q_LD)
	$Q$(CXX) $(LDFLAGS) $(LDFLAGS_NM) -shared $^ $(LOADLIBES) \
	$(LDLIBS_LIB) $(LDLIBS) -o $@

# pmc tool
pm%.o: pm%.cpp
	$(Q_CC)
	$Q$(CXX) $(CPPFLAGS) -c -o $@ $<
$(PMC_NAME): $(PMC_OBJS) $(LIB_NAME).$(PMC_USE_LIB)
	$(Q_LD)
	$Q$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

include $(wildcard *.d)

CLEAN:=*.o *.lo *.d .libs/*
DISTCLEAN:=$(ALL)
DISTCLEAN_DIRS:=.libs

clean:
	$Q$(Q_CLEAN)
	$Q$(RM) $(CLEAN)
distclean: deb_clean clean
	$(Q_DISTCLEAN)
	$Q$(RM) $(DISTCLEAN)
	$Q$(RM) -R $(DISTCLEAN_DIRS)

HEADERS:=$(filter-out mngIds.h,$(wildcard *.h))
# MAP for  mngIds.cc:
#  %@ => '/'        - Use next to star
#  %! => '%'        - Self escape
#  %# => '#'        - Use on line start to define a preproccesor in final
#  %_ =>            - Place marker, retain empty lines
#  %- => ' '        - When need 2 spaces or more, use with spaces between
#  %^ => '\n'       - Add new line in preprocessor only
mngIds.h: mngIds.cc
	$(Q_GEN)
	$Q$(CXX) -E $< | sed 's/^#.*//;/^\s*$$/d;s#%@#/#g' > $@
	$(Q)sed -i 's/^%#/#/;s/%-/ /g;s/%^/\n/g;s/%_//;s/%!/%/g' $@

DISTCLEAN+=mngIds.h

ifneq ($(call which,astyle),)
astyle_ver:=$(lastword $(shell astyle -V))
ifeq ($(call verCheck,$(astyle_ver),3.1),)
format:
	$(Q_FRMT)
	$(Q)astyle --project=none --options=astyle.opt $(wildcard *.h *.cpp)
	$Q./format.pl
endif
endif # which astyle

ifeq ($(DEB_HOST_MULTIARCH),)
ifneq ($(call which,dpkg-architecture),)
DEB_HOST_MULTIARCH:=$(shell dpkg-architecture -qDEB_HOST_MULTIARCH)
endif # dpkg-architecture
endif
HOST_MULTIARCH:=$(DEB_HOST_MULTIARCH)
LIB_ARCH:=/usr/lib/$(HOST_MULTIARCH)

SWIG:=swig
ifneq ($(call which,$(SWIG)),)
swig_ver=$(lastword $(shell swig -version | grep Version))
ifeq ($(call verCheck,$(swig_ver),3.0),)
SWIG_ALL:=
SWIG_NAME:=PmcLib

ifneq ($(call which,perl),)
PERL_VER:=$(shell perl -e '$$_=$$^V;s/^v//;print')
PERLDIR:=$(DESTDIR)$(LIB_ARCH)/perl/$(PERL_VER)
ifneq ($(HOST_MULTIARCH),)
PERL_INC:=$(LIB_ARCH)/perl/$(PERL_VER)/CORE
ifeq ($(realpath $(PERL_INC)),)
PERL_INC=/usr/lib/perl/$(PERL_VER)/CORE
endif
else
PERL_INC=/usr/lib/perl/$(PERL_VER)/CORE
endif # realpath $(PERL_INC)
PERL_NAME:=perl/$(SWIG_NAME)
$(PERL_NAME).cpp: $(LIB_NAME).i $(HEADERS) mngIds.h
	$(Q_SWIG)
	$Q$(SWIG) -c++ -I. -outdir perl -o $@ -perl5 $<
$(PERL_NAME).o: $(PERL_NAME).cpp
	$(Q_LCC)
	$Q$(CXX) $(CPPFLAGS) $(CPPFLAGS_SO) $(CPPFLAGS_SWIG) -I$(PERL_INC) -c $< -o $@
	$(Q)sed -i 's#$(PERL_INC)#\$$(PERL_INC)#' $(PERL_NAME).d
$(PERL_NAME).so: $(PERL_NAME).o $(LIB_NAME_SO)
	$(Q_LD)
	$Q$(CXX) $(LDFLAGS) -shared $^ $(LOADLIBES) $(LDLIBS) -o $@
SWIG_ALL+=$(PERL_NAME).so
CLEAN+=$(foreach e,d o,$(PERL_NAME).$e)
DISTCLEAN+=$(foreach e,cpp pm so,$(PERL_NAME).$e)
include $(wildcard perl/*.d)
endif # which perl

ifneq ($(call which,lua),)
LUA_FLAGS:=$(foreach n,$(LUA_WARNS),-w$n)
LUADIR:=$(DESTDIR)$(LIB_ARCH)
LUA_LIB_NAME:=pmc.so
lua/$(SWIG_NAME).cpp: $(LIB_NAME).i $(HEADERS) mngIds.h
	$(Q_SWIG)
	$Q$(SWIG) -c++ -I. $(LUA_FLAGS) -outdir lua -o $@ -lua $<
define lua
LUA_LIB_$1:=lua/5.$1/$(LUA_LIB_NAME)
lua/5.$1/$(SWIG_NAME).o: lua/$(SWIG_NAME).cpp
	$$(Q_LCC)
	$Q$(CXX) $$(CPPFLAGS) $(CPPFLAGS_SO) $(CPPFLAGS_SWIG) -I/usr/include/lua5.$1 \
	-c $$< -o $$@
$$(LUA_LIB_$1): lua/5.$1/$(SWIG_NAME).o $(LIB_NAME_SO)
	$$(Q_LD)
	$Q$(CXX) $(LDFLAGS) -shared $$^ $(LOADLIBES) $(LDLIBS) \
	$$(LD_LUA_$1) -o $$@
SWIG_ALL+=$$(LUA_LIB_$1)
CLEAN+=$$(foreach n,o d,lua/5.$1/$(SWIG_NAME).$$n)
DISTCLEAN+=$$(LUA_LIB_$1)

endef
define lua_fname
LUA_FLIB_$1:=liblua5.$1-$(LUA_LIB_NAME)

endef
define lua_soname
LD_LUA_$1:=-Wl,-soname,$(LUA_FLIB_$1).$(SONAME)

endef
DISTCLEAN+=lua/$(SWIG_NAME).cpp lua/$(LUA_LIB_NAME)
LUA_VERSIONS:=1 2 3
$(eval $(foreach n,$(LUA_VERSIONS),$(call lua_fname,$n)))
ifneq ($(SONAME),)
$(eval $(foreach n,$(LUA_VERSIONS),$(call lua_soname,$n)))
endif
$(eval $(foreach n,$(LUA_VERSIONS),$(call lua,$n)))
endif # which lua

ifneq ($(call which,python),)
PY_BASE:=python/$(SWIG_NAME)
$(PY_BASE).cpp: $(LIB_NAME).i $(HEADERS) mngIds.h
	$(Q_SWIG)
	$Q$(SWIG) -c++ -I. -outdir python -o $@ -python $<
PYV2=$(call libname,*/libpython2*.a)
ifeq ($(PYV2),)
PYV2=$(call libname,libpython2*.a)
endif
PYV3=$(call libname,*/libpython3*.a)
ifeq ($(PYV3),)
PYV3=$(call libname,libpython3*.a)
endif
PY_LIB_NAME:=_pmc
PY3_VER:=$(subst .,,$(subst python,,$(PYV3)))
PY_SO_2:=python/2/$(PY_LIB_NAME).so
PY_SO_3:=python/3/$(PY_LIB_NAME).cpython-$(PY3_VER)-$(HOST_MULTIARCH).so
PY2DIR:=$(DESTDIR)/usr/lib/$(PYV2)/dist-packages
PY3DIR:=$(DESTDIR)/usr/lib/python3/dist-packages
define python
PY_BASE_$1:=python/$1/$(SWIG_NAME)
PY_INC_$1:=/usr/include/$$(PYV$1)
$$(PY_BASE_$1).o: $(PY_BASE).cpp
	$$(Q_LCC)
	$Q$(CXX) $$(CPPFLAGS) $(CPPFLAGS_SO) $(CPPFLAGS_SWIG) -I$$(PY_INC_$1) -c $$< -o $$@
$$(PY_SO_$1): $$(PY_BASE_$1).o $(LIB_NAME_SO)
	$$(Q_LD)
	$Q$(CXX) $(LDFLAGS) -shared $$^ $(LOADLIBES) $(LDLIBS) \
	-l$$(PYV$1) -lm -ldl -o $$@
SWIG_ALL+=$$(PY_SO_$1)
CLEAN+=$$(foreach n,o d,$$(PY_BASE_$1).$$n)
DISTCLEAN+=$$(PY_SO_$1)

endef
DISTCLEAN+=$(PY_BASE).cpp $(wildcard python/*.so) python/pmc.py python/pmc.pyc
DISTCLEAN_DIRS+=python/__pycache__
$(eval $(call python,2))
$(eval $(call python,3))
endif # which python

ALL+=$(SWIG_ALL)
endif # swig 3.0
endif # which swig

ifneq ($(call which,dpkg-buildpackage),)
deb_src: distclean
	$(Q)dpkg-source -b .
deb:
	$(Q)MAKEFLAGS=$(MAKE_NO_DIRS) Q=$Q dpkg-buildpackage -b -us -uc
	$Q$(RM) $(PMC_NAME) $(LIB_NAME_SO) $(PERL_NAME).so $(wildcard */*/*.so)
deb_clean:
	$Q$(MAKE) $(MAKE_NO_DIRS) -f debian/rules deb_clean Q=$Q
endif # Debian

ifneq ($(call which,doxygen),)
ifeq ($(call verCheck,$(shell doxygen -v),1.8),)
doxygen: $(HEADERS) mngIds.h
	$(Q_DOXY)
	$(Q)doxygen doxygen.cfg 2>&1 >/dev/null
DISTCLEAN_DIRS+=doc
endif
endif # which doxygen

ifneq ($(call which,ctags),)
tags: $(filter-out $(wildcard ids*.h),$(wildcard *.h *.cpp))
	$(Q_TAGS)
	$(Q)ctags -R $^
ALL+=tags
DISTCLEAN+=tags
endif # which ctags

all: $(ALL)
	@:
.DEFAULT_GOAL=all

LIB_VER:=$(ver_maj).$(ver_min)
LIB_FNAME_SO:=$(LIB_NAME_SO).$(LIB_VER)
LIB_SNAME_SO:=$(LIB_NAME_SO).$(SONAME)
DOCDIR:=$(DESTDIR)/usr/share/doc/libpmc-doc
URL:=html/index.html
REDIR:="<meta http-equiv=\"refresh\" charset=\"utf-8\" content=\"0; url=$(URL)\"/>"
NINST:=install -m 644
DINST:=install -d
BINST:=install

install: $(ALL)
	$Q$(NINST) -D $(LIB_NAME_SO) $(DESTDIR)$(LIB_ARCH)/$(LIB_FNAME_SO)
	$Q$(LN) $(LIB_FNAME_SO) $(DESTDIR)$(LIB_ARCH)/$(LIB_SNAME_SO)
	$Q$(LN) $(LIB_SNAME_SO) $(DESTDIR)$(LIB_ARCH)/$(LIB_NAME_SO)
	$Q$(NINST) libpmc.a $(DESTDIR)$(LIB_ARCH)
	$Q$(NINST) -D $(HEADERS) -t $(DESTDIR)/usr/include/pmc
	$Q$(NINST) -D pkg/*.mk -t $(DESTDIR)/usr/share/libpmc-dev
	$Q$(BINST) -D pmc $(DESTDIR)/usr/sbin/pmc.lib
	$Q$(RM) doc/html/*.md5
	$Q$(DINST) $(DOCDIR)
	$(Q)cp -a doc/html $(DOCDIR)
	$(Q)printf $(REDIR) > $(DOCDIR)/index.html
ifneq ($(call which,perl),)
	$Q$(NINST) -D perl/$(SWIG_NAME).so -t $(PERLDIR)/auto/$(SWIG_NAME)
	$Q$(NINST) perl/$(SWIG_NAME).pm $(PERLDIR)
endif # which perl
ifneq ($(call which,lua),)
	$Q$(foreach n,$(LUA_VERSIONS),\
	  $(NINST) -D $(LUA_LIB_$n) $(LUADIR)/$(LUA_FLIB_$n).$(LIB_VER);\
	  $(LN) $(LUA_FLIB_$n).$(LIB_VER) $(LUADIR)/$(LUA_FLIB_$n).$(SONAME);\
	  $(DINST) $(LUADIR)/lua/5.$n;\
	  $(LN) ../../$(LUA_FLIB_$n).$(LIB_VER) $(LUADIR)/lua/5.$n/$(LUA_LIB_NAME);)
endif # which lua
ifneq ($(call which,python),)
	$Q$(NINST) -D python/2/$(PY_LIB_NAME).so\
	  $(PY2DIR)/$(PY_LIB_NAME).$(HOST_MULTIARCH).so
	$Q$(NINST) -D python/3/$(PY_LIB_NAME).*.so -t $(PY3DIR)
	$Q$(NINST) python/pmc.py $(PY2DIR)
	$Q$(NINST) python/pmc.py $(PY3DIR)
endif # which python

check: format doxygen

.PHONY: all clean distclean format deb_src deb deb_clean doxygen check
