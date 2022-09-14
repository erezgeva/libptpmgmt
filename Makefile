# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2021 Erez Geva
#
# Makefile Create libptpmgmt and pmc for testing
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
###############################################################################

PMC_USE_LIB?=a # 'a' for static and 'so' for dynamic

define help
################################################################################
#  Make file targets                                                           #
################################################################################
#                                                                              #
#   all              Build all targets.                                        #
#                                                                              #
#   clean            Clean build files.                                        #
#                                                                              #
#   distclean        Perform full clean.                                       #
#                                                                              #
#   format           Format source code and warn of format issues.             #
#                                                                              #
#   doxygen          Create library documentation.                             #
#                                                                              #
#   checkall         Call doxygen and format targets.                          #
#                                                                              #
#   help             See this help.                                            #
#                                                                              #
#   install          Install application, libraries, and headers in system.    #
#                                                                              #
#   deb              Build Debian packages.                                    #
#                                                                              #
#   deb_arc          Build Debian packages for other architecture.             #
#                    Use DEB_ARC to specify architecture.                      #
#                                                                              #
#   deb_clean        Clean Debian intermediate files.                          #
#                                                                              #
#   rpm              Build Red Hat packages.                                   #
#                                                                              #
#   rpmsrc           Create source tar for Red Hat build.                      #
#                                                                              #
#   pkg              Build Arch Linux packages.                                #
#                                                                              #
#   pkgsrc           Create source tar for Arch Linux build.                   #
#                                                                              #
################################################################################
#  Make file parameters                                                        #
################################################################################
#                                                                              #
#   PMC_USE_LIB      Select the pmc tool library link,                         #
#                    use 'a' for static library or 'so' for shared library.    #
#                                                                              #
#   DESTDIR          Destination folder for install target.                    #
#                    Installation prefix.                                      #
#                                                                              #
#   DEV_PKG          Development package name, default libptpmgmt-dev.         #
#                                                                              #
#   USE_ASAN         Use the AddressSanitizer, for testing!                    #
#                                                                              #
#   NO_COL           Prevent colour output.                                    #
#                                                                              #
#   USE_COL          Force colour when using pipe for tools like 'aha'.        #
#                    For example: make -j USE_COL=1 | aha > out.html           #
#                                                                              #
#   DEB_ARC          Specify Debian architectue to build                       #
#                                                                              #
#   PY_USE_S_THRD    Use python with 'Global Interpreter Lock',                #
#                    Which use mutex on all library functions.                 #
#                    So poll() and tpoll() can block other threads.            #
#                    In this case, users may prefer Python native select.      #
#                                                                              #
################################################################################

endef

###############################################################################
### General macroes
which=$(shell which $1 2>/dev/null)
define depend
$1: $2

endef
SP:=$(subst X, ,X)
verCheckDo=$(shell if [ $1 -eq $4 ];then test $2 -eq $5 && a=$3 b=$6 ||\
  a=$2 b=$5; else a=$1 b=$4;fi;test $$a -lt $$b && echo l)
verCheck=$(call verCheckDo,$(firstword $(subst ., ,$1 0 0 0)),$(word 2,\
  $(subst ., ,$1 0 0 0)),$(word 3,$(subst ., ,$1 0 0 0)),$(firstword\
  $(subst ., ,$2 0)),$(word 2,$(subst ., ,$2 0)),$(word 3,$(subst ., ,$2 0)))

# Make support new file function
ifeq ($(call cmp,$(MAKE_VERSION),4.2),)
USE_FILE_OP:=1
endif
define lbase
A

endef
line=$(subst A,,$(lbase))
space=$(subst A,,A A)

###############################################################################
### output shaper

# Use tput to check if we have ANSI Color code
# tput works only if TERM is set
ifneq ($(and $(TERM),$(call which,tput)),)
ifeq ($(shell tput setaf 1),)
NO_COL:=1
endif
endif # which tput
# Detect output is not device (terminal), it must be a pipe or a file
# In case of using 'aha' just call: $ make -j USE_COL=1 | aha > out.html
ifndef USE_COL
ifndef MAKE_TERMOUT
NO_COL:=1
endif
endif # USE_COL

# Terminal colors
ifndef NO_COL
ESC!=printf '\e['
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
COLOR_WARNING:=$(COLOR_RED)
COLOR_BUILD:=$(COLOR_MAGENTA)
Q_CLEAN=$Q$(info $(COLOR_BUILD)Cleaning$(COLOR_NORM))
Q_DISTCLEAN=$Q$(info $(COLOR_BUILD)Cleaning all$(COLOR_NORM))
Q_TAR=$Q$(info $(COLOR_BUILD)[TAR] $@$(COLOR_NORM))
MAKE_NO_DIRS:=--no-print-directory
endif

###############################################################################
### Generic definitions
.ONESHELL: # Run rules in a single shell

SRC:=src
PMC_DIR:=tools
JSON_SRC:=json
OBJ_DIR:=objs

PMC_NAME:=$(PMC_DIR)/pmc
SWIG_NAME:=PtpMgmtLib
SWIG_LNAME:=ptpmgmt
SWIG_LIB_NAME:=$(SWIG_LNAME).so
D_FILES:=$(wildcard */*.d */*/*.d)
PHP_LNAME:=php/$(SWIG_LNAME)
PERL_NAME:=perl/$(SWIG_NAME)
HEADERS_GEN_COMP:=$(addprefix $(SRC)/,ids.h mngIds.h callDef.h ver.h)
HEADERS_GEN:=$(HEADERS_GEN_COMP) $(addprefix $(SRC)/,vecDef.h cnvFunc.h)
HEADERS_SRCS:=$(filter-out $(HEADERS_GEN),$(wildcard $(SRC)/*.h))
HEADERS:=$(HEADERS_SRCS) $(HEADERS_GEN_COMP)
HEADERS_INST:=$(filter-out $(addprefix $(SRC)/,end.h err.h jsonDef.h comp.h\
  msgProc.h ids.h),$(HEADERS))
SRCS:=$(wildcard $(SRC)/*.cpp)
COMP_DEPS:=$(OBJ_DIR) $(HEADERS_GEN_COMP)

####### Source tar file #######
TAR:=tar cfJ
include version
SRC_NAME:=libptpmgmt-$(ver_maj).$(ver_min)
ifneq ($(call which,git),)
INSIDE_GIT!=git rev-parse --is-inside-work-tree 2>/dev/null
endif
ifneq ($(INSIDE_GIT),true)
SRC_FILES:=$(wildcard */test.* scripts/* *.sh *.pl *.md *.cfg *.opt *.in\
  config.guess config.sub configure.ac install-sh $(SRC)/*.in $(SRC)/*.m4\
  php/*.sh swig/*.md swig/*/* */*.i man/* LICENSES/* .reuse/*\
  $(PMC_DIR)/phc_ctl $(PMC_DIR)/*.[ch]* $(JSON_SRC)/*)\
  $(SRCS) $(HEADERS_SRCS) LICENSE $(MAKEFILE_LIST)
else
SRC_FILES!=git ls-files $(foreach n,archlinux debian rpm sample,':!/:$n')\
  ':!:*.gitignore'
endif
# Add configure script for source archive
SRC_FILES+=configure
# To compare manual source list to git based:
# $(foreach n,$(sort $(SRC_FILES)),$(info $n))

###############################################################################
### Configure area
ifeq ($(wildcard defs.mk),)
ifeq ($(MAKECMDGOALS),)
$(info defs.mk is missing, please run ./configure)
endif
all: configure
	@:

###############################################################################
### Build area
else # wildcard defs.mk
include defs.mk

ifneq ($(V),1)
Q_DOXY=$Q$(info $(COLOR_BUILD)Doxygen$(COLOR_NORM))
Q_FRMT=$Q$(info $(COLOR_BUILD)Format$(COLOR_NORM))
Q_TAGS=$Q$(info $(COLOR_BUILD)[TAGS]$(COLOR_NORM))
Q_GEN=$Q$(info $(COLOR_BUILD)[GEN] $@$(COLOR_NORM))
Q_SWIG=$Q$(info $(COLOR_BUILD)[SWIG] $@$(COLOR_NORM))
Q_LD=$Q$(info $(COLOR_BUILD)[LD] $@$(COLOR_NORM))
Q_AR=$Q$(info $(COLOR_BUILD)[AR] $@$(COLOR_NORM))
Q_LCC=$(info $(COLOR_BUILD)[LCC] $<$(COLOR_NORM))
Q_CC=$Q$(info $(COLOR_BUILD)[CC] $<$(COLOR_NORM))
LIBTOOL_QUIET:=--quiet
# Filter normal output, send error output to stdout
QE:=2>&1 >/dev/null | $(SED) 's@^$(ROOT_DIR)/@@'
endif

LN:=$(LN_S) -f
ifeq ($(findstring -O,$(CXXFLAGS)),)
# Add debug optimization, unless we already have an optimization :-)
override CXXFLAGS+=-Og
endif # find '-O'
override CXXFLAGS+=-Wdate-time -Wall -std=c++11 -g -Isrc
override CXXFLAGS+=-MT $@ -MMD -MP -MF $(basename $@).d
ifeq ($(USE_ASAN),)
else
# Use https://github.com/google/sanitizers/wiki/AddressSanitizer
ASAN_FLAGS:=$(addprefix -fsanitize=,address pointer-compare pointer-subtract\
  undefined leak)
override CXXFLAGS+=$(ASAN_FLAGS) -fno-omit-frame-pointer
override LDFLAGS+=$(ASAN_FLAGS)
endif # USE_ASAN
LIBTOOL_CC=$Q$(Q_LCC)libtool --mode=compile --tag=CXX $(LIBTOOL_QUIET)
SONAME:=.$(PACKAGE_VERSION_MAJ)
LIB_NAME:=libptpmgmt
LIB_NAME_SO:=$(LIB_NAME).so
LDFLAGS_NM=-Wl,--version-script,scripts/lib.ver -Wl,-soname,$@$(SONAME)
$(LIB_NAME_SO)_LDLIBS:=-lm -ldl
LIB_OBJS:=$(subst $(SRC)/,$(OBJ_DIR)/,$(patsubst %.cpp,%.o,$(SRCS)))
PMC_OBJS:=$(subst $(PMC_DIR)/,$(OBJ_DIR)/,$(patsubst %.cpp,%.o,\
  $(wildcard $(PMC_DIR)/*.cpp)))
JSON_FROM_OBJS:=$(subst $(JSON_SRC)/,$(OBJ_DIR)/,$(patsubst %.cpp,%.o,\
  $(wildcard $(JSON_SRC)/*.cpp)))
$(OBJ_DIR)/ver.o: override CXXFLAGS+=-DVER_MAJ=$(PACKAGE_VERSION_MAJ)\
  -DVER_MIN=$(PACKAGE_VERSION_MIN) -DVER_VAL=$(PACKAGE_VERSION_VAL)
D_INC=$(if $($1),$(SED) -i 's@$($1)@\$$($1)@g' $(basename $@).d)
LLC=$(Q_LCC)$(CXX) $(CXXFLAGS) $(CXXFLAGS_SWIG) -fPIC -DPIC -I. $1 -c $< -o $@
LLA=$(Q_AR)$(AR) rcs $@ $^;$(RANLIB) $@

ifeq ($(call verCheck,$(shell $(CXX) -dumpversion),4.9),)
# GCC output colors
ifndef NO_COL
CXXFLAGS_COLOR:=-fdiagnostics-color=always
else
CXXFLAGS_COLOR:=-fdiagnostics-color=never
endif
endif # verCheck CXX 4.9
# https://clang.llvm.org/docs/UsersManual.html
# -fcolor-diagnostics
override CXXFLAGS+=$(CXXFLAGS_COLOR)

ALL:=$(PMC_NAME) $(LIB_NAME_SO)$(SONAME) $(LIB_NAME).a

%.so:
	$(Q_LD)$(CXX) $(LDFLAGS) $(LDFLAGS_NM) -shared $^ $(LOADLIBES)\
	  $($@_LDLIBS) $(LDLIBS) -o $@

# JSON libraries
JSON_C:=
# Using json-c
JSONC_LIB:=$(LIB_NAME)_jsonc.so
JSONC_LIBA:=$(LIB_NAME)_jsonc.a
JSONC_FLIB:=$(JSONC_LIB)$(SONAME)
JSON_C+=\"$(JSONC_FLIB)\",
ifneq ($(HAVE_JSONC_LIB),)
JSONC_CFLAGS:=-include $(HAVE_JSONC_LIB) -DJLIB_NAME=\"$(JSONC_LIBA)\"
$(OBJ_DIR)/jsonFromJc.o: $(JSON_SRC)/jsonFrom.cpp | $(COMP_DEPS)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(JSONC_CFLAGS) $< -o $@
$(OBJ_DIR)/.libs/jsonFromJc.o: $(OBJ_DIR)/jsonFromJc.o
$(JSONC_LIB)_LDLIBS:=$(JSONC_LIB_FLAGS)
$(JSONC_LIB)$(SONAME): $(JSONC_LIB)
	$Q$(LN) $^ $@
$(JSONC_LIB): $(OBJ_DIR)/.libs/jsonFromJc.o $(LIB_NAME).so
$(JSONC_LIBA): $(OBJ_DIR)/jsonFromJc.o
	$(LLA)
ALL+=$(JSONC_LIB)$(SONAME) $(JSONC_LIBA)
endif # HAVE_JSONC_LIB

# Using fastjson
FJSON_LIB:=$(LIB_NAME)_fastjson.so
FJSON_LIBA:=$(LIB_NAME)_fastjson.a
FJSON_FLIB:=$(FJSON_LIB)$(SONAME)
JSON_C+=\"$(FJSON_FLIB)\",
ifneq ($(HAVE_FJSON_LIB),)
FJSON_CFLAGS:=-include $(HAVE_FJSON_LIB) -DJLIB_NAME=\"$(FJSON_LIBA)\"
$(OBJ_DIR)/jsonFromFj.o: $(JSON_SRC)/jsonFrom.cpp | $(COMP_DEPS)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(FJSON_CFLAGS) $< -o $@
$(OBJ_DIR)/.libs/jsonFromFj.o: $(OBJ_DIR)/jsonFromFj.o
$(FJSON_LIB)_LDLIBS:=$(FJSON_LIB_FLAGS)
$(FJSON_LIB)$(SONAME): $(FJSON_LIB)
	$Q$(LN) $^ $@
$(FJSON_LIB): $(OBJ_DIR)/.libs/jsonFromFj.o $(LIB_NAME).so
$(FJSON_LIBA): $(OBJ_DIR)/jsonFromFj.o
	$(LLA)
ALL+=$(FJSON_LIB)$(SONAME) $(FJSON_LIBA)
endif # HAVE_FJSON_LIB

# Add jsonFrom libraries to search
$(OBJ_DIR)/jsonDef.o: override CXXFLAGS+=-DJSON_C="$(JSON_C)"

# Compile library source code
$(LIB_OBJS): $(OBJ_DIR)/%.o: $(SRC)/%.cpp | $(COMP_DEPS)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $< -o $@

# Depened shared library objects on the static library to ensure build
$(eval $(foreach obj,$(notdir $(LIB_OBJS)),\
  $(call depend,$(OBJ_DIR)/.libs/$(obj),$(OBJ_DIR)/$(obj))))

$(LIB_NAME).a: $(LIB_OBJS)
	$(LLA)
$(LIB_NAME_SO)$(SONAME): $(LIB_NAME_SO)
	$Q$(LN) $^ $@
$(LIB_NAME_SO): $(addprefix $(OBJ_DIR)/.libs/,$(notdir $(LIB_OBJS)))

# pmc tool
$(PMC_OBJS): $(OBJ_DIR)/%.o: $(PMC_DIR)/%.cpp | $(COMP_DEPS)
	$(Q_CC)$(CXX) $(CXXFLAGS) $(CXXFLAGS_PMC) -c -o $@ $<
$(PMC_NAME): $(PMC_OBJS) $(LIB_NAME).$(PMC_USE_LIB)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(SRC)/%.h: $(SRC)/%.m4 $(SRC)/ids_base.m4
	$(Q_GEN)m4 -I $(SRC) $< > $@
$(SRC)/ver.h: $(SRC)/ver.h.in
	$(Q_GEN)$(SED) $(foreach n,PACKAGE_VERSION_MAJ PACKAGE_VERSION_MIN\
	  PACKAGE_VERSION_VAL PACKAGE_VERSION,-e 's/@$n@/$($n)/') $< > $@

ifneq ($(ASTYLEMINVER),)
format: $(HEADERS_GEN) $(HEADERS_SRCS) $(SRCS) $(wildcard sample/*.cpp)
	$(Q_FRMT)$(ASTYLE) --project=none --options=astyle.opt $^
	./format.pl
ifneq ($(CPPCHECK),)
	$(CPPCHECK) --quiet --language=c++ --error-exitcode=-1 $^
endif
endif # ASTYLEMINVER

ifneq ($(SWIGMINVER),)
SWIG_ALL:=
ifneq ($(SWIGARGCARGV),)
# Only python and ruby have argcargv.i
perl_SFLAGS+=-Iswig/perl5
$(foreach n,lua php tcl,$(eval $(n)_SFLAGS+=-Iswig/$n))
endif #SWIGARGCARGV

# suppress swig compilation warnings for old swig versions
ifneq ($(call verCheck,$(SWIGVER),4.1),)
# SWIG warnings
# comparison integer of different signedness
CXXFLAGS_RUBY+=-Wno-sign-compare
# ANYARGS is deprecated (seems related to ruby headers)
CXXFLAGS_RUBY+=-Wno-deprecated-declarations
# label 'thrown' is not used
CXXFLAGS_PHP+=-Wno-unused-label
# 'result' may be used uninitialized
CXXFLAGS_LUA+=-Wno-maybe-uninitialized
ifeq ($(PY_USE_S_THRD),)
# PyEval_InitThreads is deprecated
CXXFLAGS_PY+=-Wno-deprecated-declarations
endif
ifneq ($(call verCheck,$(SWIGVER),4.0),)
# catching polymorphic type 'class std::out_of_range' by value
CXXFLAGS_RUBY+=-Wno-catch-value
# 'argv[1]' may be used uninitialized
CXXFLAGS_RUBY+=-Wno-maybe-uninitialized
# strncpy() specified bound depends on the length of the source argument
CXXFLAGS_PY+=-Wno-stringop-overflow
endif # ! swig 4.0.0
endif # ! swig 4.1.0

%/$(SWIG_NAME).cpp: $(SRC)/$(LIB_NAME).i $(HEADERS)
	$(Q_SWIG)$(SWIG) -c++ -Isrc -I$(@D) -outdir $(@D) -Wextra\
	  $($(@D)_SFLAGS) -o $@ $<
# As SWIG does not create a dependencies file
# We create it during compilation from the compilation dependencies file
SWIG_DEP=$(SED) -e '1 a\ $(SRC)/$(LIB_NAME).i $(SRC)/mngIds.h \\'\
  $(foreach n,$(wildcard $(<D)/*.i),-e '1 a\ $n \\')\
  -e 's@.*\.o:\s*@@;s@\.cpp\s*@.cpp: @' $*.d > $*_i.d
SWIG_LD=$(Q_LD)$(CXX) $(LDFLAGS) -shared $^ $(LOADLIBES) $(LDLIBS)\
  $($@_LDLIBS) -o $@
ifeq ($(SKIP_PERL5),)
perl_SFLAGS+=-perl5
$(PERL_NAME).o: $(PERL_NAME).cpp $(HEADERS)
	$Q$(call LLC,-I$(PERL5EXT))
	$(call D_INC,PERL5EXT)
	$(SWIG_DEP)
$(PERL_NAME).so: $(PERL_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(PERL_NAME).so
endif # SKIP_PERL5

ifeq ($(SKIP_LUA),)
lua_SFLAGS+=-lua
define lua
LUA_FLIB_$1:=liblua$1-$(SWIG_LIB_NAME)
LUA_LIB_$1:=lua/$1/$(SWIG_LIB_NAME)
$$(LUA_LIB_$1)_LDLIBS:=-Wl,-soname,$$(LUA_FLIB_$1)$(SONAME)\
  $$(LUA_$$(subst .,_,$1)_LINK)
lua/$1/$(SWIG_NAME).o: lua/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(MKDIR_P) lua/$1
	$$(call LLC,$$(CXXFLAGS_LUA) -I$$(LUA_$$(subst .,_,$1)_INC))
	$$(call D_INC,LUA_$$(subst .,_,$1)_INC)
	$$(SWIG_DEP)
$$(LUA_LIB_$1): lua/$1/$(SWIG_NAME).o $(LIB_NAME_SO)
	$$(SWIG_LD)
SWIG_ALL+=$$(LUA_LIB_$1)

endef
# Build multiple Lua versions
$(eval $(foreach n,$(LUAVERSIONS),$(call lua,$n)))

# Build single Lua version
ifneq ($(LUA_VERSION),)
LUA_FLIB:=liblua$(LUABIN_VERSION)-$(SWIG_LIB_NAME)
LUA_LIB:=lua/$(SWIG_LIB_NAME)
ifneq ($(LUA_INC),)
CXXFLAGS_LUA+=-I$(LUA_INC)
endif
$(LUA_LIB)_LDLIBS:=-Wl,-soname,$(LUA_FLIB)$(SONAME) $(LUALINK)
lua/$(SWIG_NAME).o: lua/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_LUA))
	$(call D_INC,LUA_INC)
	$(SWIG_DEP)
$(LUA_LIB): lua/$(SWIG_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(LUA_LIB)
endif # LUA_VERSION
endif # SKIP_LUA

ifeq ($(SKIP_PYTHON3),)
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
endif # SKIP_PYTHON3

ifeq ($(SKIP_RUBY),)
RUBY_NAME:=ruby/$(SWIG_NAME).cpp
RUBY_LNAME:=ruby/$(SWIG_LNAME)
ruby_SFLAGS:=-ruby
$(RUBY_LNAME).so_LDLIBS:=$(RUBYLINK)
$(RUBY_LNAME).o: $(RUBY_NAME) $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_RUBY) $(RUBYINCLUDE))
	$(call D_INC,RUBYHDRDIR)
ifneq ($(RUBYHDRDIR),$(RUBYARCHHDRDIR))
	$(call D_INC,RUBYARCHHDRDIR)
endif
	$(SWIG_DEP)
$(RUBY_LNAME).so: $(RUBY_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(RUBY_LNAME).so
endif # SKIP_RUBY

ifeq ($(SKIP_PHP),)
PHP_NAME:=php/$(SWIG_NAME).cpp
php_SFLAGS+=-php7
$(PHP_LNAME).o: $(PHP_NAME) $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_PHP) -Iphp $(PHPINC_FLAGS))
	$(call D_INC,PHPINC)
	$(SWIG_DEP)
$(PHP_LNAME).so: $(PHP_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(PHP_LNAME).so
endif # SKIP_PHP

ifeq ($(SKIP_TCL),)
TCL_NAME:=tcl/$(SWIG_NAME).cpp
TCL_LNAME:=tcl/$(SWIG_LNAME)
tcl_SFLAGS+=-tcl8 -namespace
$(TCL_LNAME).o: $(TCL_NAME) $(HEADERS)
	$Q$(call LLC,$(TCLINCLUDE))
	$(call D_INC,TCLINCDIR)
	$(SWIG_DEP)
$(TCL_LNAME).so_LDLIBS:=-Wl,-soname,$(SWIG_LIB_NAME)$(SONAME)
$(TCL_LNAME).so: $(TCL_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(TCL_LNAME).so
TCLDIR:=$(DESTDIR)$(TCL_PKG_DIR)/$(SWIG_LNAME)
# TODO how the hell tcl "know" the library version? Why does it think it's 0.0?
# Need to add soname!
pkgIndex:=if {![package vsatisfies [package provide Tcl] $(TCLVER)]} {return}\n
pkgIndex+=package ifneeded $(SWIG_LNAME) $(PACKAGE_VERSION_MAJ)
pkgIndex+=[list load [file join $$dir $(SWIG_LIB_NAME)]]\n
endif # SKIP_TCL

ALL+=$(SWIG_ALL)
endif # SWIGMINVER

ifneq ($(DOXYGENMINVER),)
doxygen: $(HEADERS_GEN) $(HEADERS)
ifeq ($(DOTTOOL),)
	$Q:$(info $(COLOR_WARNING)You miss the 'dot' application.$(COLOR_NORM))
else
	$(Q_DOXY)$(DOXYGEN) doxygen.cfg $(QE)
endif # DOTTOOL
endif # DOXYGENMINVER

checkall: format doxygen

ifneq ($(CTAGS),)
tags: $(filter-out ids.h,$(HEADERS_GEN_COMP)) $(HEADERS_SRCS) $(SRCS)
	$(Q_TAGS)$(CTAGS) -R $^
ALL+=tags
endif # CTAGS

.DEFAULT_GOAL=all
all: $(COMP_DEPS) $(ALL)
	@:

####### installation #######
URL:=html/index.html
REDIR:="<meta http-equiv=\"refresh\" charset=\"utf-8\" content=\"0; url=$(URL)\"/>"
INSTALL_FOLDER:=$(INSTALL) -d
TOOLS_EXT:=-ptpmgmt
DEV_PKG?=libptpmgmt-dev
DLIBDIR:=$(DESTDIR)$(libdir)
DOCDIR:=$(DESTDIR)$(datarootdir)/doc/libptpmgmt-doc
MANDIR:=$(DESTDIR)$(mandir)/man8
# 1=Dir 2=file 3=link
ifeq ($(USE_FULL_PATH_LINK),)
mkln=$(LN) $2 $(DESTDIR)$1/$3
else
mkln=$(LN) $1/$2 $(DESTDIR)$1/$3
endif

install:
	$(Q)for lib in $(LIB_NAME)*.so; do\
	  $(INSTALL_PROGRAM) -D $$lib $(DLIBDIR)/$$lib.$(PACKAGE_VERSION);\
	  $(call mkln,$(libdir),$$lib.$(PACKAGE_VERSION),$$lib$(SONAME));\
	  $(call mkln,$(libdir),$$lib$(SONAME),$$lib);done
	$(INSTALL_DATA) $(LIB_NAME)*.a $(DLIBDIR)
	$(INSTALL_DATA) -D $(HEADERS_INST) -t $(DESTDIR)/usr/include/ptpmgmt
	$(foreach f,$(notdir $(HEADERS_INST)),$(SED) -i\
	  's!#include\s*\"\([^"]\+\)\"!#include <ptpmgmt/\1>!'\
	  $(DESTDIR)/usr/include/ptpmgmt/$f;)
	$(INSTALL_DATA) -D scripts/*.mk -t $(DESTDIR)/usr/share/$(DEV_PKG)
	$(INSTALL_PROGRAM) -D $(PMC_NAME) $(DESTDIR)$(sbindir)/pmc$(TOOLS_EXT)
	if [ ! -f $(MANDIR)/pmc$(TOOLS_EXT).8.gz ]; then\
	  $(INSTALL_DATA) -D man/pmc.8 $(MANDIR)/pmc$(TOOLS_EXT).8;\
	  gzip $(MANDIR)/pmc$(TOOLS_EXT).8;fi
	$(INSTALL_PROGRAM) -D $(PMC_DIR)/phc_ctl\
	  $(DESTDIR)$(sbindir)/phc_ctl$(TOOLS_EXT)
	if [ ! -f $(MANDIR)/phc_ctl$(TOOLS_EXT).8.gz ]; then\
	  $(INSTALL_DATA) -D man/phc_ctl.8 $(MANDIR)/phc_ctl$(TOOLS_EXT).8;\
	  gzip $(MANDIR)/phc_ctl$(TOOLS_EXT).8;fi
	$(RM) doc/html/*.md5
	$(INSTALL_FOLDER) $(DOCDIR)
	cp -a *.md doc/html $(DOCDIR)
	printf $(REDIR) > $(DOCDIR)/index.html
ifeq ($(SKIP_PERL5),)
	$(INSTALL_PROGRAM) -D perl/$(SWIG_NAME).so -t\
	  $(DESTDIR)$(PERL5DIR)/auto/$(SWIG_NAME)
	$(INSTALL_DATA) perl/$(SWIG_NAME).pm $(DESTDIR)$(PERL5DIR)
endif # SKIP_PERL5
ifeq ($(SKIP_LUA),)
	$(foreach n,$(LUAVERSIONS),\
	  $(INSTALL_PROGRAM) -D $(LUA_LIB_$n)\
	  $(DLIBDIR)/$(LUA_FLIB_$n).$(PACKAGE_VERSION);\
	  $(LN) $(LUA_FLIB_$n).$(PACKAGE_VERSION)\
	  $(DLIBDIR)/$(LUA_FLIB_$n)$(SONAME);\
	  $(INSTALL_FOLDER) $(DLIBDIR)/lua/$n;\
	  $(LN) ../../$(LUA_FLIB_$n).$(PACKAGE_VERSION)\
	  $(DLIBDIR)/lua/$n/$(SWIG_LIB_NAME);)
ifneq ($(LUA_VERSION),)
	$(INSTALL_PROGRAM) -D $(LUA_LIB) $(DLIBDIR)/$(LUA_FLIB)
ifneq ($(LUABIN_VERSION),)
	$(INSTALL_FOLDER) $(DLIBDIR)/lua/$(LUABIN_VERSION)
	$(LN) ../../$(LUA_FLIB)\
	$(DLIBDIR)/lua/$(LUABIN_VERSION)/$(SWIG_LIB_NAME)
endif # LUABIN_VERSION
endif # LUA_VERSION
endif # SKIP_LUA
ifeq ($(SKIP_PYTHON3),)
	$(INSTALL_PROGRAM) -D python/3/$(PY_LIB_NAME).so\
	  $(DESTDIR)$(PY3SITE_DIR)/$(PY_LIB_NAME)$(PY3EXT)
	$(INSTALL_DATA) python/$(SWIG_LNAME).py $(DESTDIR)$(PY3SITE_DIR)
endif # SKIP_PYTHON3
ifeq ($(SKIP_RUBY),)
	$(INSTALL_PROGRAM) -D $(RUBY_LNAME).so -t $(DESTDIR)$(RUBYSITE)
endif # SKIP_RUBY
ifeq ($(SKIP_PHP),)
	$(INSTALL_PROGRAM) -D $(PHP_LNAME).so -t $(DESTDIR)$(PHPEXT)
	$(INSTALL_DATA) -D $(PHP_LNAME).php -t $(DESTDIR)$(PHPINCDIR)
endif # SKIP_PHP
ifeq ($(SKIP_TCL),)
	$(INSTALL_PROGRAM) -D $(TCL_LNAME).so -t $(TCLDIR)
	printf '$(pkgIndex)' > $(TCLDIR)/pkgIndex.tcl
endif # SKIP_TCL

include $(D_FILES)

$(OBJ_DIR):
	$Q$(MKDIR_P) $@

endif # wildcard defs.mk
###############################################################################

####### Debain build #######
ifneq ($(and $(wildcard debian/rules),$(call which,dpkg-buildpackage)),)
deb:
	$(Q)MAKEFLAGS=$(MAKE_NO_DIRS) Q=$Q dpkg-buildpackage -b --no-sign
ifneq ($(DEB_ARC),)
deb_arc:
	$(Q)MAKEFLAGS=$(MAKE_NO_DIRS) Q=$Q dpkg-buildpackage -B --no-sign\
	  -a$(DEB_ARC)
endif
deb_clean:
	$Q$(MAKE) $(MAKE_NO_DIRS) -f debian/rules deb_clean Q=$Q
endif # and wildcard debian/rules, which dpkg-buildpackage

####### RPM build #######
RPM_SRC:=rpm/SOURCES/$(SRC_NAME).txz
rpm/SOURCES:
	$(Q)mkdir -p $@
$(RPM_SRC): $(SRC_FILES) | rpm/SOURCES
	$(Q_TAR)$(TAR) $@ $^ --transform "s#^#$(SRC_NAME)/#S"
ifneq ($(call which,rpmbuild),)
rpm: $(RPM_SRC)
	$(Q)rpmbuild --define "_topdir $(PWD)/rpm" -bb rpm/libptpmgmt.spec
endif # which rpmbuild
rpmsrc: $(RPM_SRC)

####### Arch Linux build #######
ARCHL_SRC:=archlinux/$(SRC_NAME).txz
ARCHL_BLD:=archlinux/PKGBUILD
$(ARCHL_SRC): $(SRC_FILES)
	$(Q_TAR)$(TAR) $@ $^
$(ARCHL_BLD): $(ARCHL_BLD).org | $(ARCHL_SRC)
	$(Q)cp $^ $@
	printf "sha256sums=('%s')\n"\
	  $(firstword $(shell sha256sum $(ARCHL_SRC))) >> $@
ifneq ($(call which,makepkg),)
pkg: $(ARCHL_BLD)
	$(Q)cd archlinux && makepkg
endif # which makepkg
pkgsrc: $(ARCHL_BLD)

####### Generic rules #######

ifeq ($(filter distclean,$(MAKECMDGOALS)),)
configure: configure.ac
	$(Q)autoconf
endif # MAKECMDGOALS

ifeq ($(filter help distclean clean,$(MAKECMDGOALS)),)
ifneq ($(wildcard config.status),)
config.status: configure
	$(Q)./config.status --recheck

defs.mk: defs.mk.in config.status
	$(Q)./config.status
endif # config.status
endif # MAKECMDGOALS

CLEAN:=$(wildcard */*.o */*/*.o */$(SWIG_NAME).cpp archlinux/*.pkg.tar.zst\
  $(LIB_NAME)*.so $(LIB_NAME)*.a $(LIB_NAME)*.so.$(ver_maj) */*.so */*/*.so\
  python/*.pyc php/*.h php/*.ini perl/*.pm) $(D_FILES) $(ARCHL_SRC)\
  $(ARCHL_BLD) tags python/ptpmgmt.py $(PHP_LNAME).php $(PMC_NAME)\
  $(HEADERS_GEN)
CLEAN_DIRS:=$(filter %/, $(wildcard lua/*/ python/*/ rpm/*/\
  archlinux/*/)) doc $(OBJ_DIR)
DISTCLEAN:=$(foreach n, log status,config.$n) configure defs.mk
DISTCLEAN_DIRS:=autom4te.cache

clean: deb_clean
	$(Q_CLEAN)$(RM) $(CLEAN)
	$(RM) -R $(CLEAN_DIRS)
distclean: deb_clean
	$(Q_DISTCLEAN)$(RM) $(CLEAN) $(DISTCLEAN)
	$(RM) -R $(CLEAN_DIRS) $(DISTCLEAN_DIRS)

help:
	@$(info $(help))
	:

.PHONY: all clean distclean format install deb deb_arc deb_clean\
  doxygen checkall help rpm rpmsrc pkg pkgsrc
