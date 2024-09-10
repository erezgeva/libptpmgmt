# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile Create libptpmgmt and pmc for testing
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
###############################################################################

# 'a' for static and 'so' for dynamic
PMC_USE_LIB?=a

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
#   DOC_PKG          Document package name, default libptpmgmt-doc.            #
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
space=$(subst A,,A )
hash=$(subst A,,A#)
c_inc=$(hash)include

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
LIB_D:=.libs
PUB_C:=$(PUB)/c
PMC_DIR:=ptp-tools
HMAC_SRC:=hmac
CLKMGR_DIR:=clkmgr
OBJ_DIR:=objs

CONF_FILES:=configure src/config.h.in
SONAME:=.$(ver_maj)
LIB_NAME:=libptpmgmt
LIB_NAME_SO:=$(LIB_D)/$(LIB_NAME).so
LIB_NAME_A:=$(LIB_D)/$(LIB_NAME).a
LIB_NAME_LA:=$(LIB_NAME).la
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
HEADERS_SRCS:=$(HEADERS_PUB) $(HEADERS_PUB_C) $(SRC)/comp.h $(SRC)/jsonParser.h
HEADERS:=$(HEADERS_SRCS) $(HEADERS_GEN_COMP)
HEADERS_GEN:=$(HEADERS_GEN_COMP) $(addprefix $(SRC)/,vecDef.h cnvFunc.h)
HEADERS_INST:=$(HEADERS_PUB) $(HEADERS_GEN_PUB)
HEADERS_INST_C:=$(HEADERS_PUB_C) $(HEADERS_GEN_PUB_C)
SRCS:=$(wildcard $(SRC)/*.cpp)
SRCS_HMAC:=$(wildcard $(HMAC_SRC)/*.cpp)
SRCS_CLKMGR:=$(wildcard $(CLKMGR_DIR)/[cip]*/*.[ch]* $(CLKMGR_DIR)/*/*/*.h)
COMP_DEPS:=$(OBJ_DIR) $(HEADERS_GEN_COMP)
# hmac
SSL_NAME:=$(LIB_NAME)_openssl
SSL_LIBA:=$(LIB_D)/$(SSL_NAME).a
SSL_LIB:=$(LIB_D)/$(SSL_NAME).so
SSL_DL:=$(SSL_NAME).so$(SONAME)
SSL_LA:=$(SSL_NAME).la
GCRYPT_NAME:=$(LIB_NAME)_gcrypt
GCRYPT_LIBA:=$(LIB_D)/$(GCRYPT_NAME).a
GCRYPT_LIB:=$(LIB_D)/$(GCRYPT_NAME).so
GCRYPT_DL:=$(GCRYPT_NAME).so$(SONAME)
GCRYPT_LA:=$(GCRYPT_NAME).la
GNUTLS_NAME:=$(LIB_NAME)_gnutls
GNUTLS_LIBA:=$(LIB_D)/$(GNUTLS_NAME).a
GNUTLS_LIB:=$(LIB_D)/$(GNUTLS_NAME).so
GNUTLS_DL:=$(GNUTLS_NAME).so$(SONAME)
GNUTLS_LA:=$(GNUTLS_NAME).la
NETTLE_NAME:=$(LIB_NAME)_nettle
NETTLE_LIBA:=$(LIB_D)/$(NETTLE_NAME).a
NETTLE_LIB:=$(LIB_D)/$(NETTLE_NAME).so
NETTLE_DL:=$(NETTLE_NAME).so$(SONAME)
NETTLE_LA:=$(NETTLE_NAME).la

HMAC_FLIBS:=$(SSL_LIB) $(GNUTLS_LIB) $(NETTLE_LIB) $(GCRYPT_LIB)
TGT_LNG:=perl5 lua python3 ruby php tcl go
UTEST_CPP_TGT:=$(addprefix utest_,no_sys sys auth json_load pmc hmac)
UTEST_C_TGT:=$(addprefix uctest_,no_sys sys auth)
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
SRC_FILES_DIR:=$(wildcard README.md t*/*.pl */*/*.m4 .reuse/* */gitlab*\
  */github* */*.opt configure.ac src/*.m4 */*.md\
  t*/*.sh */*/*.sh swig/*/* */*.i */*/msgCall.i */*/warn.i man/*\
  $(PMC_DIR)/phc_ctl $(PMC_DIR)/*.[ch]* */Makefile */*/Makefile\
  $(CLKMGR_DIR)/*/*.[ch]* $(CLKMGR_DIR)/*/*/*.h $(CLKMGR_DIR)/image/*\
  */*/*test*/*.go LICENSES/* *.in tools/*.in $(HMAC_SRC)/*.cpp)\
  src/ver.h.in src/name.h.in $(SRCS) $(HEADERS_SRCS) LICENSE\
  $(MAKEFILE_LIST) credits
ifeq ($(INSIDE_GIT),true)
SRC_FILES!=git ls-files $(foreach n,archlinux debian rpm sample gentoo\
  utest/*.[chj]* uctest/*.[ch]* .github/workflows/* .gitlab/*,':!/:$n')\
  ':!:*.gitignore' ':!*/*/test.*' ':!*/*/utest.*'
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
override V:=1
undefine COLOR_WARNING
endif
endif # V
ifneq ($(V),1)
Q:=@
MAKE_NO_DIRS:=--no-print-directory
COLOR_BUILD:=$(COLOR_MAGENTA)
Q_OUT:=>/dev/null
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
Q_LCC=$(info $(COLOR_BUILD)[LCC] $<$(COLOR_NORM))
Q_CC=$Q$(info $(COLOR_BUILD)[CC] $<$(COLOR_NORM))
Q_UTEST=$Q$(info $(COLOR_BUILD)[UTEST $1]$(COLOR_NORM))
LIBTOOL_QUIET:=--quiet
Q_CC_STR=\$$(info $(COLOR_BUILD)[CC] $1\$$<$(COLOR_NORM))
Q_LD_STR=\$$(info $(COLOR_BUILD)[LD] $1\$$<$(COLOR_NORM))
else
V:=0
endif
export V

LN:=$(LN_S) -f
ifeq ($(findstring -O,$(CXXFLAGS)),)
# Add debug optimization, unless we already have an optimization :-)
override CXXFLAGS+=-Og
endif # find '-O'
override CXXFLAGS+=-Wdate-time -Wall -std=c++11 -g -I$(SRC) -I$(PUB)
# Add warnings from -Wextra
override CXXFLAGS+=-Wtype-limits -Wdeprecated-copy -Wundef
# Ignore deprecated functions in pub/opt.h and pub/init.h pub/json.h pub/c/json.h
override CXXFLAGS+=-Wno-deprecated-declarations
CXXFLAGS_SWIG+=-Wno-tautological-type-limit-compare -Wno-undef
CXXFLAGS_RUBY+=-Wno-deprecated-copy
CXXFLAGS_GO:=$(filter-out -I%,$(CXXFLAGS))
ifdef USE_DEPS
# Add dependencies during compilation
override CXXFLAGS+=-MT $@ -MMD -MP -MF $(basename $@).d
endif
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
LIBTOOL=./libtool
LIBTOOL_CC=$Q$(Q_LCC)$(LIBTOOL) --mode=compile --tag=CXX $(LIBTOOL_QUIET)
LIBTOOL_LD=$Q$(Q_LD)$(LIBTOOL) --mode=link --tag=CXX $(LIBTOOL_QUIET)
$(LIB_NAME_LA)_LDLIBS:=-lm -ldl
LIB_OBJS:=$(subst $(SRC)/,$(OBJ_DIR)/,$(SRCS:.cpp=.lo))
PMC_OBJS:=$(subst $(PMC_DIR)/,$(OBJ_DIR)/,$(patsubst %.cpp,%.o,\
  $(wildcard $(PMC_DIR)/*.cpp)))
$(OBJ_DIR)/ver.lo: override CXXFLAGS+=-DVER_MAJ=$(ver_maj)\
  -DVER_MIN=$(ver_min) -DVER_VAL=$(PACKAGE_VERSION_VAL)
LLC=$(Q_LCC)$(CXX) $(CXXFLAGS) $(CXXFLAGS_SWIG) -fPIC -DPIC -I. $1 -c $< -o $@
ifdef CHRPATH
RMRPATH=$(CHRPATH) -d $1
else
ifdef PATCHELF
RMRPATH=$(PATCHELF) --remove-rpath $1
endif
endif

ifdef CXX_COLOR_USE
# GCC output colours
ifndef NO_COL
override CXXFLAGS+=$(CXX_COLOR_USE)
else
override CXXFLAGS+=$(CXX_COLOR_NONE)
endif
endif # CXX_COLOR_USE

ALL:=$(PMC_NAME) $(LIB_NAME_LA)

%.so:
	$(Q_LD)$(CXX) $(LDFLAGS) -shared $^ $(LOADLIBES)\
	  $($@_LDLIBS) $(LDLIBS) -o $@

%.la:
	$(LIBTOOL_LD) $(CXX) -version-number $(ver_maj):$(ver_min):0 -Og -g\
	  -o $@ -rpath "$(libdir)" $(LDFLAGS) $^ $(LOADLIBES)\
	  $($@_LDLIBS) $(LDLIBS)

UVGD:=$(SP)
ifdef VALGRIND
ifdef VGD
UVGD+=$(VALGRIND) --read-inline-info=yes $(VGD_OPTIONS)$(SP)
# failing: utest_ruby utest_go utest_sys
endif # VGD
ifdef VGD_PY
PYUVGD:=PYTHONMALLOC=malloc $(VALGRIND) --read-inline-info=yes $(VGD_OPTIONS)$(SP)
endif # VGD_PY
endif # VALGRIND

# HMAC libraries
include $(HMAC_SRC)/Makefile

# CLKMGR libraries
ifndef SKIP_CLKMGR
include $(CLKMGR_DIR)/Makefile
endif

# Compile library source code
$(LIB_OBJS): $(OBJ_DIR)/%.lo: $(SRC)/%.cpp | $(COMP_DEPS)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $< -o $@
$(LIB_NAME_LA): $(LIB_OBJS)
$(LIB_NAME_SO): $(LIB_NAME_LA)
	@:
$(LIB_NAME_A): $(LIB_NAME_LA)
	@:

include utest/Makefile
ifdef CRITERION_LIB_FLAGS
include uctest/Makefile
endif

# pmc tool
$(PMC_OBJS): $(OBJ_DIR)/%.o: $(PMC_DIR)/%.cpp | $(COMP_DEPS)
	$(Q_CC)$(CXX) $(CXXFLAGS) $(CXXFLAGS_PMC) -c -o $@ $<
PMC_LIBS:=
PMC_LDLIBS:=
ifneq ($(and $(HMAC_ALIB), $(filter a,$(PMC_USE_LIB))),)
PMC_LIBS+=$(HMAC_ALIB)
PMC_LDLIBS+=$(HMAC_ALIB_FLAGS) $(HMAC_LIBA_FLAGS)
endif
$(PMC_NAME): $(PMC_OBJS) $(LIB_D)/$(LIB_NAME).$(PMC_USE_LIB) $(PMC_LIBS)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(PMC_LDLIBS) -o $@

$(SRC)/%.h: $(SRC)/%.m4 $(SRC)/ids_base.m4 $(SRC)/cpp.m4
	$(Q_GEN)$(M4) -I $(SRC) -D lang=cpp $< > $@
$(PUB)/%.h: $(SRC)/%.m4 $(SRC)/ids_base.m4 $(SRC)/cpp.m4
	$(Q_GEN)$(M4) -I $(SRC) -D lang=cpp $< > $@
$(PUB_C)/%.h: $(SRC)/%.m4 $(SRC)/ids_base.m4 $(SRC)/c.m4
	$(Q_GEN)$(M4) -I $(SRC) -D lang=c $< > $@
# This is basically what configure does.
# Yet, I prefer configure create only the def.mk,
# and forward the version parameters here :-)
$(PUB)/ver.h: $(SRC)/ver.h.in
	$(Q_GEN)$(SED) $(foreach n,PACKAGE_VERSION_MAJ PACKAGE_VERSION_MIN\
	  PACKAGE_VERSION_VAL PACKAGE_VERSION,-e 's!@$n@!$($n)!') $< > $@

HAVE_LIST:=$(foreach n,UNISTD_H SYS_TYPES_H SYS_UN_H NETINET_IN_H\
	  GETOPT_H,$(if $(HAVE_$(n)), HAVE_$n))
$(PUB)/name.h: $(SRC)/name.h.in
	$(Q_GEN)$(SED) $(foreach n,$(HAVE_LIST),\
	  -e 's!undef __PTPMGMT_$(n)$$!define __PTPMGMT_$(n) 1!') $< > $@

ifneq ($(and $(ASTYLE_MINVER),$(PERL5_HAVE_TOUCH)),)
CPPCHECK_OPT:=--quiet --force --error-exitcode=-1
CPPCHECK_OPT+=$(CPPCHECK_OPT_BASE)
EXTRA_C_SRCS:=$(wildcard uctest/*.c)
EXTRA_SRCS:=$(wildcard $(foreach n,sample utest uctest,$n/*.cpp $n/*.h))
EXTRA_SRCS+=$(EXTRA_C_SRCS)
format: $(HEADERS_GEN) $(HEADERS_SRCS) $(SRCS) $(EXTRA_SRCS) $(SRCS_HMAC)\
	$(SRCS_CLKMGR)
	$(Q_FRMT)
	r=`$(ASTYLE) --project=none --options=tools/astyle.opt $^`
	test -z "$$r" || echo "$$r";./tools/format.pl $^
	if test $$? -ne 0 || test -n "$$r"; then echo '';exit 1;fi
ifdef CPPCHECK
	$(CPPCHECK) $(CPPCHECK_OPT) --language=c++\
	  $(filter-out $(EXTRA_C_SRCS) $(addprefix $(SRC)/,ids.h),$^)
endif
endif # ASTYLE_MINVER && PERL5_HAVE_TOUCH

ifdef SWIG_MINVER
SWIG_ALL:=
ifdef SWIG_ARGCARGV_GO
go_SFLAGS+=-Iswig/go
endif #SWIG_ARGCARGV_GO

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
ifdef USE_DEPS
# Add dependencies for swig files during swig compilation
SWIG_DEPS=-MT $@ -MMD -MP -MF $(basename $@)_i.d
endif

wrappers/%/$(SWIG_NAME).cpp: $(SRC)/$(LIB_NAME).i $(HEADERS) wrappers/%/warn.i
	$(Q_SWIG)$(SWIG) -c++ -I$(SRC) -I$(PUB) -I$(@D) -outdir $(@D) -Wextra\
	  $(SWIG_DEPS) $($(subst wrappers/,,$(@D))_SFLAGS) -o $@ $<
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
SPDXGPL:=GPL-3.0-or-later
SPDXLGPL:=L$(SPDXGPL)
SPDXGFDL:=GFDL-1.3-no-invariants-or-later
SPDXHTML:=<!-- $(SPDXLI) $(SPDXGFDL)\n     $(SPDXCY) -->

ifndef SKIP_PERL5
include wrappers/perl/Makefile
endif
ifndef SKIP_LUA
include wrappers/lua/Makefile
endif
ifndef SKIP_PYTHON3
include wrappers/python/Makefile
endif
ifndef SKIP_RUBY
include wrappers/ruby/Makefile
endif
ifndef SKIP_PHP
include wrappers/php/Makefile
endif
ifndef SKIP_TCL
include wrappers/tcl/Makefile
endif
ifndef SKIP_GO
include wrappers/go/Makefile
endif

ALL+=$(SWIG_ALL)
endif # SWIG_MINVER

tools/doxygen.cfg: tools/doxygen.cfg.in
	$(Q_GEN)$(SED) $(foreach n, PACKAGE_VERSION,-e 's!@$n@!$($n)!') $< > $@

ifdef DOXYGEN_MINVER
doxygen: $(HEADERS_GEN) $(HEADERS) tools/doxygen.cfg
ifndef DOTTOOL
	$Q$(info $(COLOR_WARNING)You miss the 'dot' application.$(COLOR_NORM))
	$(SED) -i 's!^\$(hash)HAVE_DOT\s.*!HAVE_DOT               = NO!' tools/doxygen.cfg
endif
# doxygen fails with cairo 1.17.6, use workaround
# https://github.com/doxygen/doxygen/issues/9319
# TODO The bug should be fixed in doxygen version 1.9.7
	$(Q_DOXY)CAIRO_DEBUG_PDF=1 $(DOXYGEN) tools/doxygen.cfg $(Q_OUT)
ifndef DOTTOOL
	$(SED) -i 's!^HAVE_DOT\s.*!\$(hash)HAVE_DOT               = YES!' tools/doxygen.cfg
endif
endif # DOXYGEN_MINVER

checkall: format doxygen

ifdef CTAGS
tags: $(filter-out $(SRC)/ids.h,$(HEADERS_GEN_COMP)) $(HEADERS_SRCS) $(SRCS)\
	$(SRCS_HMAC) $(SRCS_CLKMGR)
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
REDIR2:=!Library API doxygen!<a href="./html/index.html">Library API doxygen</a>!
INSTALL_FOLDER:=$(INSTALL) -d
INSTALL_LIB:=$(INSTALL_DATA)
TOOLS_EXT:=-$(SWIG_LNAME)
DEV_PKG?=$(LIB_NAME)-dev
DOC_PKG?=$(LIB_NAME)-doc
DEVDOCDIR:=$(DESTDIR)$(datarootdir)/doc/$(DEV_PKG)
DLIBDIR:=$(DESTDIR)$(libdir)
DOCDIR:=$(DESTDIR)$(datarootdir)/doc/$(DOC_PKG)
MANDIR:=$(DESTDIR)$(mandir)/man8
ifdef PKG_CONFIG_DIR
PKGCFGDIR:=$(DESTDIR)$(PKG_CONFIG_DIR)
endif

define pkgconfig
$(hash) $(SPDXLI) $(SPDXGPL)
$(hash) $(SPDXCY)

Name: $(LIB_NAME) library
Description: $(LIB_NAME) library to communicate with IEEE 1558 PTP clocks
URL: $(PACKAGE_URL)
Version: $(PACKAGE_VERSION)
Cflags:
Libs: -l$(SWIG_LNAME)
endef

install: $(INS_TGT)
install_main:
	$(Q)$(INSTALL_FOLDER) $(DLIBDIR) $(PKGCFGDIR)
	cp -a $(LIB_D)/$(LIB_NAME)*.so* $(DLIBDIR)
	$(INSTALL_LIB) $(LIB_D)/$(LIB_NAME)*.so.*.*.* $(DLIBDIR)
	$(call RMRPATH,$(DLIBDIR)/$(LIB_NAME)*.so.*.*.*)
	if test -f "$(LIB_NAME_A)"
	then $(INSTALL_LIB) $(LIB_D)/$(LIB_NAME)*.a $(DLIBDIR); fi
ifdef PKG_CONFIG_DIR
	echo "$(pkgconfig)" > $(PKGCFGDIR)/$(SWIG_LNAME).pc
	for pf in $(SWIG_LNAME)$(PACKAGE_VERSION) $(LIB_NAME)\
	  $(LIB_NAME)$(PACKAGE_VERSION)
	do $(LN) $(SWIG_LNAME).pc $(PKGCFGDIR)/$$pf.pc;done
endif
	$(INSTALL_DATA) -D $(HEADERS_INST_C) -t $(DESTDIR)$(includedir)/$(SWIG_LNAME)/c
	$(INSTALL_DATA) -D $(HEADERS_INST) -t $(DESTDIR)$(includedir)/$(SWIG_LNAME)
	$(foreach f,$(notdir $(HEADERS_INST)),$(SED) -i\
	  's!$(c_inc)\s*\"\([^"]\+\)\"!$(c_inc) <$(SWIG_LNAME)/\1>!'\
	  $(DESTDIR)$(includedir)/$(SWIG_LNAME)/$f;)
	$(foreach f,$(notdir $(HEADERS_INST_C)),$(SED) -i\
	  's!$(c_inc)\s*\"\([^"]\+\)\"!$(c_inc) <$(SWIG_LNAME)/\1>!'\
	  $(DESTDIR)$(includedir)/$(SWIG_LNAME)/c/$f;)
	$(INSTALL_FOLDER) $(DEVDOCDIR)
	printf "$(hash) $(SPDXLI) $(SPDXGFDL)\n$(hash) $(SPDXCY)\n\n%s\n"\
	  'LDLIBS+=-lptpmgmt' > $(DEVDOCDIR)/default.mk
	printf "$(hash) $(SPDXLI) $(SPDXGFDL)\n$(hash) $(SPDXCY)\n\n%s\n"\
	  'LDLIBS+= -Wl,-static -lptpmgmt -Wl,-Bdynamic' > $(DEVDOCDIR)/static.mk
	$(INSTALL_PROGRAM) -D $(PMC_NAME) $(DESTDIR)$(sbindir)/pmc$(TOOLS_EXT)
	$(INSTALL_DATA) -D man/pmc.8 $(MANDIR)/pmc$(TOOLS_EXT).8
	$(INSTALL_PROGRAM) -D $(PMC_DIR)/phc_ctl\
	  $(DESTDIR)$(sbindir)/phc_ctl$(TOOLS_EXT)
	$(INSTALL_DATA) -D man/phc_ctl.8 $(MANDIR)/phc_ctl$(TOOLS_EXT).8
	$(INSTALL_FOLDER) $(DOCDIR)
	cp README.md doc/*.md $(DOCDIR)
	$(SED) -i  's!\./doc/!./!' $(DOCDIR)/README.md
ifdef DOXYGEN_MINVER
	$(MKDIR_P) "doc/html"
	$(RM) doc/html/*.md5 doc/html/*.map
	cp -a doc/html $(DOCDIR)
	printf '$(REDIR)' > $(DOCDIR)/index.html
	for dh in doc/html/*.html doc/html/*/*.html;do if test -f "$$dh"
	then $(SED) -i '1 i$(SPDXHTML)' $(subst doc/html/,$(DOCDIR)/html/,$$dh)
	fi;done
	for dh in doc/html/search/*_*.js doc/html/search/searchdata.js
	do if test -f "$$dh"
	then $(SED) -i '1 i/* $(SPDXLI) $(SPDXGFDL)\n   $(SPDXCY) */\n'\
	  $(subst doc/html/,$(DOCDIR)/html/,$$dh);fi;done
endif # DOXYGEN_MINVER
ifdef MARKDOWN
	for hf in doc/[CBHs]*.md
	do tl=$$($(SED) -n '/^$(hash) /{s!^$(hash) !!;s!<!\&lt;!;s!>!\&gt;!;p;q}' $$hf)
	tf="$(DOCDIR)/$$(basename "$${hf%.md}.html")"
	$(MARKDOWN) $$hf | sed "4 i <!doctype html><title>$$tl</title>" > $$tf
	done
	$(MARKDOWN) doc/FAQs.md |\
	  $(SED) "4 i <!doctype html><title>Frequently asked questions</title>"\
	  > $(DOCDIR)/FAQs.html
	$(MARKDOWN) README.md |\
	$(SED) -e "4 i <!doctype html><title>libptpmgmt library README</title>"\
	  -e 's!"\./doc/!"./!;s!\.md"!.html"!' > $(DOCDIR)/index.html
ifdef DOXYGEN_MINVER
	$(SED) -i 's$(REDIR2)' $(DOCDIR)/index.html
endif
endif # MARKDOWN

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
	$(Q_TAR)$(TAR) $@ $^ --transform "s!^!$(SRC_NAME)/!S"
srcpkg: $(LIB_SRC)

####### RPM build #######
ifneq ($(call which,rpmbuild),)
rpm/SOURCES:
	$(Q)mkdir -p "$@"
rpm: $(LIB_SRC) rpm/SOURCES
	$(Q)cp $(LIB_SRC) rpm/SOURCES/
	rpmbuild --define "_topdir $(PWD)/rpm" -bb rpm/$(LIB_NAME).spec
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
ifneq ($(wildcard configure),)
src/config.h.in: configure.ac
	$(Q)autoheader
endif
configure: configure.ac
	$(Q)autoreconf -i
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
	  sed 's@\\$$@@' | sed 's!disable-dependency!enable-dependency!'
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
  *.la wrappers/*/*.so\
  wrappers/python/*.pyc wrappers/php/*.h wrappers/php/*.ini wrappers/perl/*.pm\
  wrappers/go/*/go.mod */$(LIB_SRC) wrappers/*/$(SWIG_NAME).cpp\
  wrappers/*/$(SWIG_NAME).h\
  */*/$(LIB_SRC) $(CLKMGR_DIR)/*/*.lo $(CLKMGR_DIR)/*/*.d)\
  $(D_FILES) $(LIB_SRC) tools/doxygen.cfg\
  $(ARCHL_BLD) tags wrappers/python/$(SWIG_LNAME).py $(PHP_LNAME).php $(PMC_NAME)\
  wrappers/tcl/pkgIndex.tcl wrappers/php/.phpunit.result.cache\
  .phpunit.result.cache wrappers/go/allocTlv.i $(CLKMGR_PROXY)\
  wrappers/go/$(SWIG_LNAME).go $(HEADERS_GEN) wrappers/go/gtest/gtest .null
CLEAN_DIRS:=$(filter %/, $(wildcard wrappers/lua/*/ wrappers/python/*/ rpm/*/\
  archlinux/*/ obj-*/ $(CLKMGR_DIR)/*/$(LIB_D)/)) _site $(OBJ_DIR)\
  $(LIB_D) wrappers/perl/auto\
  wrappers/go/$(SWIG_LNAME) $(filter-out %.md,$(wildcard doc/*))
DISTCLEAN:=configure configure~ defs.mk aclocal.m4 libtool install-sh\
  ltmain.sh $(wildcard src/config.h* config.*)
DISTCLEAN_DIRS:=autom4te.cache m4

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
