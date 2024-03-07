# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile Create libptpmgmt and pmc for testing
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
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
#   utest            Build and run the unit test                               #
#                                                                              #
#   utest <filter>   Build and run the unit test with filer                    #
#                                                                              #
#   deb              Build Debian packages.                                    #
#                                                                              #
#   deb_arc          Build Debian packages for other architecture.             #
#                    Use DEB_ARC to specify architecture.                      #
#                                                                              #
#   deb_arc <arc>    Build Debian packages for other architecture.             #
#                    With the architecture, skip using DEB_ARC                 #
#                                                                              #
#   deb_clean        Clean Debian intermediate files.                          #
#                                                                              #
#   srcpkg           Create source tar with library code only.                 #
#                                                                              #
#   rpm              Build Red Hat packages.                                   #
#                                                                              #
#   pkg              Build Arch Linux packages.                                #
#                                                                              #
#   gentoo           Build on gentoo target.                                   #
#                                                                              #
#   config           Configure using system default configuration              #
#                                                                              #
################################################################################
#  Make file parameters                                                        #
################################################################################
#                                                                              #
#   V=1              Verbose, show running commands                            #
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
#                    For example: make USE_COL=1 | aha > out.html              #
#                                                                              #
#   DEB_ARC          Specify Debian architectue to build                       #
#                                                                              #
#   VGD              Use valgrid with unit test                                #
#                                                                              #
#   VGD_PY           Use valgrid with Python unit test                         #
#                                                                              #
#   VGD_OPTIONS      Specify more options for valgrid                          #
#                                                                              #
#   PY_USE_S_THRD    Use python with 'Global Interpreter Lock',                #
#                    Which use mutex on all library functions.                 #
#                    So poll() and tpoll() can block other threads.            #
#                    In this case, users may prefer Python native select.      #
#                                                                              #
#   NOPARALLEL       Prevent parallel build                                    #
#                                                                              #
################################################################################

endef

###############################################################################
### General macroes
which=$(shell which $1 2>/dev/null)
NOP:=@:
define depend
$1: $2

endef
define phony
.PHONY: $1
$1:
	$(NOP)

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

ifdef USE_COL
GTEST_NO_COL:=--gtest_color=yes
RUBY_NO_COL:=--use-color=yes
PHP_NO_COL:=--colors=always
else
ifdef NO_COL # gtest handles terminal by itself
GTEST_NO_COL:=--gtest_color=no
RUBY_NO_COL:=--no-use-color
PHP_NO_COL:=--colors=never
endif
endif # USE_COL

# Use tput to check if we have ANSI Colour code
# tput works only if TERM is set
ifndef NO_COL
ifneq ($(and $(TERM),$(call which,tput)),)
ifeq ($(shell tput setaf 1),)
NO_COL:=1
endif
endif # which tput
endif # NO_COL

# Detect output is not device (terminal), it must be a pipe or a file
ifndef USE_COL
ifndef MAKE_TERMOUT
NO_COL:=1
endif
endif # USE_COL

# Terminal colours
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
GTEST_NO_COL?=--gtest_color=auto
RUBY_NO_COL?=--use-color=auto
PHP_NO_COL?=--colors=auto
endif

ifneq ($(V),1)
COLOR_WARNING:=$(COLOR_RED)
endif

###############################################################################
### Generic definitions
.ONESHELL: # Run rules in a single shell
include tools/version
# ver_maj=PACKAGE_VERSION_MAJ
# ver_min=PACKAGE_VERSION_MIN

SRC:=src
PUB:=pub
PUB_C:=$(PUB)/c
PMC_DIR:=ptp-tools
JSON_SRC:=json
OBJ_DIR:=objs

CONF_FILES:=configure src/config.h.in
SONAME:=.$(ver_maj)
LIB_NAME:=libptpmgmt
LIB_NAME_SO:=$(LIB_NAME).so
LIB_NAME_A:=$(LIB_NAME).a
LIB_NAME_FSO:=$(LIB_NAME_SO)$(SONAME)
PMC_NAME:=$(PMC_DIR)/pmc
SWIG_NAME:=PtpMgmtLib
SWIG_LNAME:=ptpmgmt
SWIG_LIB_NAME:=$(SWIG_LNAME).so
D_FILES:=$(wildcard *.d */*.d */*/*.d)
PHP_LNAME:=wrappers/php/$(SWIG_LNAME)
HDR_BTH:=mngIds types proc sig callDef
HEADERS_GEN_PUB:=$(foreach n,ver name $(HDR_BTH),$(PUB)/$n.h)
HEADERS_PUB:=$(filter-out $(HEADERS_GEN_PUB),$(wildcard $(PUB)/*.h))
HEADERS_GEN_PUB_C:=$(foreach n,$(HDR_BTH),$(PUB_C)/$n.h)
HEADERS_PUB_C:=$(filter-out $(HEADERS_GEN_PUB_C),$(wildcard $(PUB_C)/*.h))
HEADERS_GEN_COMP:=$(HEADERS_GEN_PUB) $(HEADERS_GEN_PUB_C) $(SRC)/ids.h
HEADERS_SRCS:=$(HEADERS_PUB) $(HEADERS_PUB_C) $(SRC)/comp.h
HEADERS:=$(HEADERS_SRCS) $(HEADERS_GEN_COMP)
HEADERS_GEN:=$(HEADERS_GEN_COMP) $(addprefix $(SRC)/,vecDef.h cnvFunc.h)
HEADERS_INST:=$(HEADERS_PUB) $(HEADERS_GEN_PUB)
HEADERS_INST_C:=$(HEADERS_PUB_C) $(HEADERS_GEN_PUB_C)
SRCS:=$(wildcard $(SRC)/*.cpp)
SRCS_JSON:=$(wildcard $(JSON_SRC)/*.cpp)
COMP_DEPS:=$(OBJ_DIR) $(HEADERS_GEN_COMP)
# json-c
JSONC_LIB:=$(LIB_NAME)_jsonc.so
JSONC_LIBA:=$(LIB_NAME)_jsonc.a
JSONC_FLIB:=$(JSONC_LIB)$(SONAME)
# fastjson
FJSON_LIB:=$(LIB_NAME)_fastjson.so
FJSON_LIBA:=$(LIB_NAME)_fastjson.a
FJSON_FLIB:=$(FJSON_LIB)$(SONAME)
TGT_LNG:=perl5 lua python3 ruby php tcl go
UTEST_CPP_TGT:=$(addprefix utest_,no_sys json sys json_load pmc)
UTEST_C_TGT:=$(addprefix uctest_,no_sys json sys)
UTEST_TGT_LNG:=$(addprefix utest_,$(TGT_LNG))
UTEST_TGT:=utest_cpp utest_lang utest_c $(UTEST_CPP_TGT) $(UTEST_TGT_LNG)\
  $(UTEST_C_TGT)
INS_TGT:=install_main $(addprefix install_,$(TGT_LNG))
PHONY_TGT:=all clean distclean format install deb deb_arc deb_clean\
  doxygen checkall help srcpkg rpm pkg gentoo utest config\
  $(UTEST_TGT) $(INS_TGT) utest_lua_a uctest
.PHONY: $(PHONY_TGT)
NONPHONY_TGT_ALL:=$(filter-out $(PHONY_TGT),$(MAKECMDGOALS))
NONPHONY_TGT:=$(firstword $(NONPHONY_TGT_ALL))

####### Source tar file #######
TAR:=tar cfJ
SRC_NAME:=$(LIB_NAME)-$(ver_maj).$(ver_min)
ifneq ($(call which,git),)
INSIDE_GIT!=git rev-parse --is-inside-work-tree 2>/dev/null
endif
SRC_FILES_DIR:=$(wildcard scripts/* *.md t*/*.pl */*/*.m4 .reuse/*\
  */github* */*.opt config.guess config.sub configure.ac install-sh */*.m4\
  t*/*.sh */*/*.sh swig/*.md swig/*/* */*.i */*/msgCall.i */*/warn.i man/*\
  $(PMC_DIR)/phc_ctl $(PMC_DIR)/*.[ch]* $(JSON_SRC)/* */Makefile w*/*/Makefile\
  */*/*test*/*.go LICENSES/* *.in tools/*.in) src/ver.h.in src/name.h.in\
  $(SRCS) $(HEADERS_SRCS) LICENSE $(MAKEFILE_LIST) credits
ifeq ($(INSIDE_GIT),true)
SRC_FILES!=git ls-files $(foreach n,archlinux debian rpm sample gentoo\
  utest/*.[ch]* uctest/*.[ch]* .github/workflows/*,':!/:$n') ':!:*.gitignore'\
  ':!*/*/test.*' ':!*/*/utest.*'
GIT_ROOT!=git rev-parse --show-toplevel
ifeq ($(GIT_ROOT),$(CURDIR))
# compare manual source list to git based:
diff1:=$(filter-out $(SRC_FILES_DIR),$(SRC_FILES))
diff2:=$(filter-out $(SRC_FILES),$(SRC_FILES_DIR))
ifneq ($(diff1),)
$(info $(COLOR_WARNING)source files missed in SRC_FILES_DIR: $(diff1)$(COLOR_NORM))
endif
ifneq ($(diff2),)
$(info $(COLOR_WARNING)source files present only in SRC_FILES_DIR: $(diff2))
endif
endif # ($(GIT_ROOT),$(CURDIR))
else # ($(INSIDE_GIT),true)
SRC_FILES:=$(SRC_FILES_DIR)
endif # ($(INSIDE_GIT),true)

###############################################################################
### Configure area
ifeq ($(wildcard defs.mk),)
ifeq ($(MAKECMDGOALS),)
$(info defs.mk is missing, please run ./configure)
endif
all: $(CONF_FILES)
	$(NOP)

###############################################################################
### Build area
else # wildcard defs.mk
include defs.mk

ifndef V
ifeq ($(DO_VERBOSE),true)
V:=1
undefine COLOR_WARNING
endif
endif # V
ifneq ($(V),1)
Q:=@
MAKE_NO_DIRS:=--no-print-directory
COLOR_BUILD:=$(COLOR_MAGENTA)
Q_ERR:=2>/dev/null
Q_CLEAN=$Q$(info $(COLOR_BUILD)Cleaning$(COLOR_NORM))
Q_DISTCLEAN=$Q$(info $(COLOR_BUILD)Cleaning all$(COLOR_NORM))
Q_TAR=$Q$(info $(COLOR_BUILD)[TAR] $@$(COLOR_NORM))
Q_DOXY=$Q$(info $(COLOR_BUILD)Doxygen$(COLOR_NORM))
Q_FRMT=$Q$(info $(COLOR_BUILD)Format$(COLOR_NORM))
Q_TAGS=$Q$(info $(COLOR_BUILD)[TAGS]$(COLOR_NORM))
Q_GEN=$Q$(info $(COLOR_BUILD)[GEN] $@$(COLOR_NORM))
Q_SWIG=$Q$(info $(COLOR_BUILD)[SWIG] $@$(COLOR_NORM))
Q_LD=$Q$(info $(COLOR_BUILD)[LD] $@$(COLOR_NORM))
Q_AR=$Q$(info $(COLOR_BUILD)[AR] $@$(COLOR_NORM))
Q_LCC=$(info $(COLOR_BUILD)[LCC] $<$(COLOR_NORM))
Q_CC=$Q$(info $(COLOR_BUILD)[CC] $<$(COLOR_NORM))
Q_UTEST=$Q$(info $(COLOR_BUILD)[UTEST $1]$(COLOR_NORM))
LIBTOOL_QUIET:=--quiet
endif

LN:=$(LN_S) -f
ifeq ($(findstring -O,$(CXXFLAGS)),)
# Add debug optimization, unless we already have an optimization :-)
override CXXFLAGS+=-Og
endif # find '-O'
override CXXFLAGS+=-Wdate-time -Wall -std=c++11 -g -I$(SRC) -I$(PUB)
# Add warnings from -Wextra
override CXXFLAGS+=-Wtype-limits -Wdeprecated-copy -Wundef
CXXFLAGS_SWIG+=-Wno-tautological-type-limit-compare -Wno-undef
CXXFLAGS_RUBY+=-Wno-deprecated-copy
CXXFLAGS_GO:=$(filter-out -I%,$(CXXFLAGS))
# Ignore deprecated functions in pub/opt.h and pub/init.h
CXXFLAGS_SWIG+=-Wno-deprecated-declarations
CXXFLAGS_GO+=-Wno-deprecated-declarations
ifdef USE_DEPS
# Add dependencies during compilation
override CXXFLAGS+=-MT $@ -MMD -MP -MF $(basename $@).d
endif # USE_DEPS
ifdef USE_CLANG_CPP_COMPILE
override CXXFLAGS+=-Wno-c99-designator
endif
ifneq ($(USE_ASAN),)
# Use https://github.com/google/sanitizers/wiki/AddressSanitizer
ASAN_FLAGS:=$(addprefix -fsanitize=,address pointer-compare pointer-subtract\
  undefined leak)
override CXXFLAGS+=$(ASAN_FLAGS) -fno-omit-frame-pointer
override LDFLAGS+=$(ASAN_FLAGS)
ASAN_PRE:=$(subst $(SP),,$(foreach n,$(ASAN_LIBS),$n:))
endif # USE_ASAN
LIBTOOL_CC=$Q$(Q_LCC)libtool --mode=compile --tag=CXX $(LIBTOOL_QUIET)
LDFLAGS_NM=-Wl,--version-script,scripts/lib.ver -Wl,-soname,$@$(SONAME)
$(LIB_NAME_SO)_LDLIBS:=-lm -ldl
LIB_OBJS:=$(subst $(SRC)/,$(OBJ_DIR)/,$(SRCS:.cpp=.o))
PMC_OBJS:=$(subst $(PMC_DIR)/,$(OBJ_DIR)/,$(patsubst %.cpp,%.o,\
  $(wildcard $(PMC_DIR)/*.cpp)))
$(OBJ_DIR)/ver.o: override CXXFLAGS+=-DVER_MAJ=$(ver_maj)\
  -DVER_MIN=$(ver_min) -DVER_VAL=$(PACKAGE_VERSION_VAL)
ifdef USE_DEPS
D_INC=$(if $($1),$(SED) -i 's@$($1)@\$$($1)@g' $(basename $@).d)
endif # USE_DEPS
LLC=$(Q_LCC)$(CXX) $(CXXFLAGS) $(CXXFLAGS_SWIG) -fPIC -DPIC -I. $1 -c $< -o $@
LLA=$(Q_AR)$(AR) rcs $@ $^;$(RANLIB) $@

ifneq ($(CXX_COLOR_USE),)
# GCC output colours
ifndef NO_COL
override CXXFLAGS+=$(CXX_COLOR_USE)
else
override CXXFLAGS+=$(CXX_COLOR_NONE)
endif
endif # CXX_COLOR_USE

ALL:=$(PMC_NAME) $(LIB_NAME_FSO) $(LIB_NAME_A)

%.so:
	$(Q_LD)$(CXX) $(LDFLAGS) $(LDFLAGS_NM) -shared $^ $(LOADLIBES)\
	  $($@_LDLIBS) $(LDLIBS) -o $@

UVGD:=$(SP)
ifneq ($(VALGRIND),)
ifdef VGD
UVGD+=$(VALGRIND) --read-inline-info=yes $(VGD_OPTIONS)$(SP)
# failing: utest_ruby utest_go utest_sys
endif # VGD
ifdef VGD_PY
PYUVGD:=PYTHONMALLOC=malloc $(VALGRIND) --read-inline-info=yes $(VGD_OPTIONS)$(SP)
endif # VGD_PY
endif # VALGRIND

# JSON libraries
include json/Makefile

# Compile library source code
$(LIB_OBJS): $(OBJ_DIR)/%.o: $(SRC)/%.cpp | $(COMP_DEPS)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $< -o $@

# Depened shared library objects on the static library to ensure build
$(eval $(foreach obj,$(notdir $(LIB_OBJS)),\
  $(call depend,$(OBJ_DIR)/.libs/$(obj),$(OBJ_DIR)/$(obj))))

$(LIB_NAME_A): $(LIB_OBJS)
	$(LLA)
$(LIB_NAME_FSO): $(LIB_NAME_SO)
	$Q$(LN) $^ $@
$(LIB_NAME_SO): $(addprefix $(OBJ_DIR)/.libs/,$(notdir $(LIB_OBJS)))

include utest/Makefile
ifneq ($(CRITERION_LIB_FLAGS),)
include uctest/Makefile
endif

# pmc tool
$(PMC_OBJS): $(OBJ_DIR)/%.o: $(PMC_DIR)/%.cpp | $(COMP_DEPS)
	$(Q_CC)$(CXX) $(CXXFLAGS) $(CXXFLAGS_PMC) -c -o $@ $<
$(PMC_NAME): $(PMC_OBJS) $(LIB_NAME).$(PMC_USE_LIB)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(SRC)/%.h: $(SRC)/%.m4 $(SRC)/ids_base.m4 $(SRC)/cpp.m4
	$(Q_GEN)$(M4) -I $(SRC) -D lang=cpp $< > $@
$(PUB)/%.h: $(SRC)/%.m4 $(SRC)/ids_base.m4 $(SRC)/cpp.m4
	$(Q_GEN)$(M4) -I $(SRC) -D lang=cpp $< > $@
$(PUB_C)/%.h: $(SRC)/%.m4 $(SRC)/c.m4
	$(Q_GEN)$(M4) -I $(SRC) -D lang=c $< > $@
# This is basically what configure does.
# Yet, I prefer configure create only the def.mk,
# and forward the version parameters here :-)
$(PUB)/ver.h: $(SRC)/ver.h.in
	$(Q_GEN)$(SED) $(foreach n,PACKAGE_VERSION_MAJ PACKAGE_VERSION_MIN\
	  PACKAGE_VERSION_VAL PACKAGE_VERSION,-e 's/@$n@/$($n)/') $< > $@

HAVE_LIST:=$(foreach n,NET_IF_H NETINET_IN_H SYS_UN_H GETOPT_H SYS_TYPES_H\
	  UNISTD_H,$(if $(HAVE_$(n)), HAVE_$n))
$(PUB)/name.h: $(SRC)/name.h.in
	$(Q_GEN)$(SED) $(foreach n,$(HAVE_LIST),\
	  -e 's/undef __PTPMGMT_$(n)$$/define __PTPMGMT_$(n) 1/') $< > $@

ifneq ($(and $(ASTYLEMINVER),$(PERL5TOUCH)),)
EXTRA_C_SRCS:=$(wildcard uctest/*.c)
EXTRA_SRCS:=$(wildcard $(foreach n,sample utest uctest,$n/*.cpp $n/*.h))
EXTRA_SRCS+=$(EXTRA_C_SRCS)
format: $(HEADERS_GEN) $(HEADERS_SRCS) $(SRCS) $(EXTRA_SRCS) $(SRCS_JSON)
	$(Q_FRMT)
	r=`$(ASTYLE) --project=none --options=tools/astyle.opt $^`
	test -z "$$r" || echo "$$r";./tools/format.pl $^
	if test $$? -ne 0 || test -n "$$r"; then echo '';exit 1;fi
ifneq ($(CPPCHECK),)
	$(SED) -i 's@^#error@// #error@' $(PUB)/*.h
	$(CPPCHECK) --quiet --force --language=c++ --error-exitcode=-1\
	  $(filter-out $(EXTRA_C_SRCS) $(addprefix $(SRC)/,ids.h proc.cpp),$^)
	$(SED) -i 's@^// #error@#error@' $(PUB)/*.h
endif
endif # ASTYLEMINVER && PERL5TOUCH

ifneq ($(SWIGMINVER),)
SWIG_ALL:=
ifneq ($(SWIGARGCARGV_GO),)
go_SFLAGS+=-Iswig/go
endif #SWIGARGCARGV_GO

# SWIG warnings
# a label defined but not used
CXXFLAGS_PHP+=-Wno-unused-label
# variable defined but not used
CXXFLAGS_PHP+=-Wno-unused-variable
# function defined but not used
CXXFLAGS_GO+=-Wno-unused-function
# dereferencing type-punned pointer will break strict-aliasing rules
CXXFLAGS_GO+=-Wno-strict-aliasing
# variable is used uninitialized in this function
CXXFLAGS_GO+=-Wno-uninitialized
# ANYARGS is deprecated (seems related to ruby headers)
CXXFLAGS_RUBY+=-Wno-deprecated-declarations
# comparison integer of different signedness
CXXFLAGS_RUBY+=-Wno-sign-compare
ifdef USE_CLANG_CPP_COMPILE
CXXFLAGS_SWIG+=-Wno-sometimes-uninitialized
CXXFLAGS_PERL+=-Wno-implicit-const-int-float-conversion
CXXFLAGS_TCL+=-Wno-missing-braces
endif # USE_CLANG_CPP_COMPILE

wrappers/%/$(SWIG_NAME).cpp: $(SRC)/$(LIB_NAME).i $(HEADERS) wrappers/%/warn.i
	$(Q_SWIG)$(SWIG) -c++ -I$(SRC) -I$(PUB) -I$(@D) -outdir $(@D) -Wextra\
	  $($(subst wrappers/,,$(@D))_SFLAGS) -o $@ $<
ifdef USE_DEPS
# As SWIG does not create a dependencies file
# We create it during compilation from the compilation dependencies file
SWIG_DEP=$(SED) -e '1 a\ $(SRC)/$(LIB_NAME).i $(PUB)/mngIds.h \\'\
  $(foreach n,$(wildcard $(<D)/*.i),-e '1 a\ $n \\')\
  -e 's@.*\.o:\s*@@;s@\.cpp\s*@.cpp: @' $*.d > $*_i.d
endif # USE_DEPS
SWIG_LD=$(Q_LD)$(CXX) $(LDFLAGS) -shared $^ $(LOADLIBES) $(LDLIBS)\
  $($@_LDLIBS) -o $@

ifdef DATE
CYEAR!=$(DATE) "+%Y"
else
CYEAR=2024
endif
SPDXLI:=SPDX-License-Identifier:
SPDXCY:=SPDX-FileCopyrightText:
SPDXCY+=Copyright © $(CYEAR) Erez Geva <ErezGeva2@gmail.com>
SPDXGFDL:=GFDL-1.3-no-invariants-or-later
SPDXHTML:=<!-- $(SPDXLI) $(SPDXGFDL)\n     $(SPDXCY) -->

ifeq ($(SKIP_PERL5),)
include wrappers/perl/Makefile
endif
ifeq ($(SKIP_LUA),)
include wrappers/lua/Makefile
endif
ifeq ($(SKIP_PYTHON3),)
include wrappers/python/Makefile
endif
ifeq ($(SKIP_RUBY),)
include wrappers/ruby/Makefile
endif
ifeq ($(SKIP_PHP),)
include wrappers/php/Makefile
endif
ifeq ($(SKIP_TCL),)
include wrappers/tcl/Makefile
endif
ifeq ($(SKIP_GO),)
include wrappers/go/Makefile
endif

ALL+=$(SWIG_ALL)
endif # SWIGMINVER

tools/doxygen.cfg: tools/doxygen.cfg.in
	$(Q_GEN)$(SED) $(foreach n, PACKAGE_VERSION,-e 's/@$n@/$($n)/') $< > $@

ifneq ($(DOXYGENMINVER),)
doxygen: $(HEADERS_GEN) $(HEADERS) tools/doxygen.cfg
ifeq ($(DOTTOOL),)
	$Q$(info $(COLOR_WARNING)You miss the 'dot' application.$(COLOR_NORM))
	$Q$(SED) -i 's/^\#HAVE_DOT\s.*/HAVE_DOT               = NO/' tools/doxygen.cfg
endif
ifdef Q_DOXY
# doxygen fails with cairo 1.17.6, use workaround
# https://github.com/doxygen/doxygen/issues/9319
# TODO The bug should be fixed in doxygen version 1.9.7
	$(Q_DOXY)CAIRO_DEBUG_PDF=1 $(DOXYGEN) tools/doxygen.cfg >/dev/null
else
	$(DOXYGEN) tools/doxygen.cfg
endif
ifeq ($(DOTTOOL),)
	$Q$(SED) -i 's/^HAVE_DOT\s.*/\#HAVE_DOT               = YES/' tools/doxygen.cfg
endif
endif # DOXYGENMINVER

checkall: format doxygen

ifneq ($(CTAGS),)
tags: $(filter-out $(SRC)/ids.h,$(HEADERS_GEN_COMP)) $(HEADERS_SRCS) $(SRCS)\
	$(SRCS_JSON)
	$(Q_TAGS)$(CTAGS) -R $^
ALL+=tags
endif # CTAGS

.DEFAULT_GOAL=all
all: $(COMP_DEPS) $(ALL)
	$(NOP)

####### installation #######
URL:=html/index.html
REDIR:=$(SPDXHTML)\n<meta
REDIR+=http-equiv="refresh" charset="utf-8" content="0; url=$(URL)"/>\n
INSTALL_FOLDER:=$(INSTALL) -d
INSTALL_LIB:=$(INSTALL_DATA)
TOOLS_EXT:=-$(SWIG_LNAME)
DEV_PKG?=$(LIB_NAME)-dev
DLIBDIR:=$(DESTDIR)$(libdir)
DOCDIR:=$(DESTDIR)$(datarootdir)/doc/$(LIB_NAME)-doc
MANDIR:=$(DESTDIR)$(mandir)/man8
# 1=Dir 2=file 3=link
ifeq ($(USE_FULL_PATH_LINK),)
mkln=$(LN) $2 $(DESTDIR)$1/$3
else
mkln=$(LN) $1/$2 $(DESTDIR)$1/$3
endif

install: $(INS_TGT)
install_main:
	$(Q)for lib in $(LIB_NAME)*.so
	  do $(INSTALL_LIB) -D $$lib $(DLIBDIR)/$$lib.$(PACKAGE_VERSION)
	  $(call mkln,$(libdir),$$lib.$(PACKAGE_VERSION),$$lib$(SONAME))
	  $(call mkln,$(libdir),$$lib$(SONAME),$$lib);done
	$(INSTALL_LIB) $(LIB_NAME)*.a $(DLIBDIR)
	$(INSTALL_DATA) -D $(HEADERS_INST_C) -t $(DESTDIR)$(includedir)/$(SWIG_LNAME)/c
	$(INSTALL_DATA) -D $(HEADERS_INST) -t $(DESTDIR)$(includedir)/$(SWIG_LNAME)
	$(foreach f,$(notdir $(HEADERS_INST)),$(SED) -i\
	  's!#include\s*\"\([^"]\+\)\"!#include <$(SWIG_LNAME)/\1>!'\
	  $(DESTDIR)$(includedir)/$(SWIG_LNAME)/$f;)
	$(foreach f,$(notdir $(HEADERS_INST_C)),$(SED) -i\
	  's!#include\s*\"\([^"]\+\)\"!#include <$(SWIG_LNAME)/\1>!'\
	  $(DESTDIR)$(includedir)/$(SWIG_LNAME)/c/$f;)
	$(INSTALL_DATA) -D scripts/*.mk -t $(DESTDIR)$(datarootdir)/$(DEV_PKG)
	$(INSTALL_PROGRAM) -D $(PMC_NAME) $(DESTDIR)$(sbindir)/pmc$(TOOLS_EXT)
	$(INSTALL_DATA) -D man/pmc.8 $(MANDIR)/pmc$(TOOLS_EXT).8
	$(INSTALL_PROGRAM) -D $(PMC_DIR)/phc_ctl\
	  $(DESTDIR)$(sbindir)/phc_ctl$(TOOLS_EXT)
	$(INSTALL_DATA) -D man/phc_ctl.8 $(MANDIR)/phc_ctl$(TOOLS_EXT).8
	$(INSTALL_FOLDER) $(DOCDIR)
	cp *.md $(DOCDIR)
ifneq ($(DOXYGENMINVER),)
	$(MKDIR_P) "doc/html"
	$(RM) doc/html/*.md5 doc/html/*.map
	cp -a doc/html $(DOCDIR)
	printf '$(REDIR)' > $(DOCDIR)/index.html
	$(SED) -i '1 i$(SPDXHTML)' $(DOCDIR)/html/*.html\
	  $(subst doc/html/,$(DOCDIR)/html/,$(wildcard doc/html/*/*.html))
	$(SED) -i '1 i/* $(SPDXLI) $(SPDXGFDL)\n   $(SPDXCY) */\n'\
	  $(DOCDIR)/html/search/*_*.js $(DOCDIR)/html/search/searchdata.js
endif # DOXYGENMINVER

ifeq ($(filter distclean clean,$(MAKECMDGOALS)),)
include $(D_FILES)
endif

$(OBJ_DIR):
	$Q$(MKDIR_P) "$@"

endif # wildcard defs.mk
###############################################################################

####### Debain build #######
ifneq ($(and $(wildcard debian/rules),$(call which,dpkg-buildpackage)),)
ifneq ($(filter deb_arc,$(MAKECMDGOALS)),)
ifeq ($(DEB_ARC),)
ifneq ($(NONPHONY_TGT),)
ifneq ($(shell dpkg-architecture -qDEB_TARGET_ARCH -a$(NONPHONY_TGT) 2>/dev/null),)
$(eval $(call phony,$(NONPHONY_TGT)))
DEB_ARC:=$(NONPHONY_TGT)
endif # dpkg-architecture -qDEB_TARGET_ARCH
endif # $(NONPHONY_TGT)
endif # $(DEB_ARC)
endif # filter deb_arc,$(MAKECMDGOALS)
deb:
	$(Q)MAKEFLAGS=$(MAKE_NO_DIRS) Q=$Q dpkg-buildpackage -b --no-sign
ifneq ($(DEB_ARC),)
deb_arc:
	$(Q)MAKEFLAGS=$(MAKE_NO_DIRS) Q=$Q DEB_BUILD_OPTIONS=nocheck\
	  dpkg-buildpackage -B --no-sign -a$(DEB_ARC)
endif
deb_clean:
	$Q$(MAKE) $(MAKE_NO_DIRS) -f debian/rules deb_clean Q=$Q
endif # and wildcard debian/rules, which dpkg-buildpackage

####### library code only #######
LIB_SRC:=$(SRC_NAME).txz
$(LIB_SRC): $(SRC_FILES)
	$(Q_TAR)$(TAR) $@ $^ --transform "s#^#$(SRC_NAME)/#S"
srcpkg: $(LIB_SRC)

####### RPM build #######
ifneq ($(call which,rpmbuild),)
rpm/SOURCES:
	$(Q)mkdir -p "$@"
rpm: $(LIB_SRC) rpm/SOURCES
	$(Q)cp $(LIB_SRC) rpm/SOURCES/
	$(Q)rpmbuild --define "_topdir $(PWD)/rpm" -bb rpm/$(LIB_NAME).spec
endif # which rpmbuild

####### Arch Linux build #######
ifneq ($(call which,makepkg),)
ARCHL_BLD:=archlinux/PKGBUILD
$(ARCHL_BLD): $(ARCHL_BLD).org | $(LIB_SRC)
	$(Q)cp $^ $@
	cp $(LIB_SRC) archlinux/
	printf "sha256sums=('%s')\n"\
	  $(firstword $(shell sha256sum $(LIB_SRC))) >> $@
pkg: $(ARCHL_BLD)
	$(Q)cd archlinux && makepkg
endif # which makepkg

####### Gentoo build #######
ifneq ($(call which,ebuild),)
gentoo: $(LIB_SRC)
	$(Q)gentoo/build.sh
endif # which ebuild

####### Generic rules #######

ifeq ($(filter distclean,$(MAKECMDGOALS)),)
src/config.h.in: configure.ac
	$(Q)autoheader
configure: configure.ac
	$(Q)autoconf
ifneq ($(filter config,$(MAKECMDGOALS)),)
ifneq ($(NONPHONY_TGT_ALL),)
$(eval $(call phony,$(NONPHONY_TGT_ALL)))
MORE_CONFIG:=$(foreach n,$(NONPHONY_TGT_ALL),--$n)
endif # $(NONPHONY_TGT_ALL)
endif # filter config,$(MAKECMDGOALS)
# Debian default configuration
ifneq ($(call which,dh_auto_configure),)
HAVE_CONFIG_GAOL:=1
config: $(CONF_FILES)
	$(Q)dh_auto_configure -- --enable-silent-rules\
	  --enable-dependency-tracking $(MORE_CONFIG)
endif # which,dh_auto_configure
ifeq ($(HAVE_CONFIG_GAOL),)
ifneq ($(call which,rpm),)
rpm_list!=rpm -qa 2>/dev/null
ifneq ($(rpm_list),)
# Default configuration on RPM based distributions
HAVE_CONFIG_GAOL:=1
config: FCFG!=rpm --eval %configure | sed -ne '/^\s*.\/configure/,$$ p' |\
	  sed 's@\\$$@@' | sed 's/disable-dependency/enable-dependency/'
config: $(CONF_FILES)
	$(Q)$(FCFG) $(MORE_CONFIG)
endif # rpm_list
endif # which rpm
endif # HAVE_CONFIG_GAOL
ifeq ($(HAVE_CONFIG_GAOL),)
ifneq ($(wildcard /usr/share/pacman/PKGBUILD.proto),)
# Default configuration on Arch Linux
HAVE_CONFIG_GAOL:=1
config: $(CONF_FILES)
	$(Q)`grep configure /usr/share/pacman/PKGBUILD.proto` $(MORE_CONFIG)
endif # wildcard pacman/PKGBUILD.proto
endif # HAVE_CONFIG_GAOL
endif # filter distclean,MAKECMDGOALS

ifeq ($(filter help distclean clean,$(MAKECMDGOALS)),)
ifneq ($(wildcard config.status),)
config.status: $(CONF_FILES)
	$(Q)./config.status --recheck

defs.mk: defs.mk.in config.status
	$(Q)./config.status
endif # config.status
endif # MAKECMDGOALS

CLEAN:=$(wildcard */*.o */*/*.o archlinux/*.pkg.tar.zst\
  $(LIB_NAME)*.so $(LIB_NAME)*.a $(LIB_NAME)*.so.$(ver_maj) */*.so */*/*.so\
  wrappers/python/*.pyc wrappers/php/*.h wrappers/php/*.ini wrappers/perl/*.pm\
  wrappers/go/*/go.mod */$(LIB_SRC) wrappers/*/$(SWIG_NAME).cpp\
  wrappers/*/$(SWIG_NAME).h\
  */*/$(LIB_SRC)) $(D_FILES) $(LIB_SRC) tools/doxygen.cfg\
  $(ARCHL_BLD) tags wrappers/python/$(SWIG_LNAME).py $(PHP_LNAME).php $(PMC_NAME)\
  wrappers/tcl/pkgIndex.tcl wrappers/php/.phpunit.result.cache\
  .phpunit.result.cache wrappers/go/allocTlv.i\
  wrappers/go/$(SWIG_LNAME).go $(HEADERS_GEN) wrappers/go/gtest/gtest .null
CLEAN_DIRS:=$(filter %/, $(wildcard wrappers/lua/*/ wrappers/python/*/ rpm/*/\
  archlinux/*/ obj-*/)) doc _site $(OBJ_DIR) wrappers/perl/auto\
  wrappers/go/$(SWIG_LNAME)
DISTCLEAN:=$(addprefix config.,log status) configure configure~ defs.mk\
  $(wildcard src/config.h*)
DISTCLEAN_DIRS:=autom4te.cache

clean: deb_clean
	$(Q_CLEAN)$(RM) $(CLEAN)
	$(RM) -R $(CLEAN_DIRS)
distclean: deb_clean
	$(Q_DISTCLEAN)$(RM) $(CLEAN) $(DISTCLEAN)
	$(RM) -R $(CLEAN_DIRS) $(DISTCLEAN_DIRS)

help:
	$(NOP)$(info $(help))
ifdef NOPARALLEL
# Use to prevent parallel building
.NOTPARALLEL: all
endif
