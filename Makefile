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
#   clean            Clean intermediate files.                                 #
#                                                                              #
#   distclean        Perform full clean includes build application and         #
#                    generated documents.                                      #
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
#   deb_src          Build Debian source package.                              #
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
#   LIBDIR           Library folder for installation                           #
#                                                                              #
#   LD_SONAME        Link with soname                                          #
#                                                                              #
#   SONAME_USE_MAJ   Use soname with major version                             #
#                                                                              #
#   DEV_PKG          Development package name, default libptpmgmt-dev.         #
#                                                                              #
#   PY_LIBDIR        Python libraries folder, default /usr/lib/python          #
#                                                                              #
#   CPPFLAGS_OPT     Compilation optimization, default for debug               #
#                                                                              #
#   TARGET_ARCH      Taget architectue, used for cross compilation.            #
#                    On Amd and Intel 64 bits default is x86_64-linux-gnu      #
#                                                                              #
#   BUILD_ARCH       Build machine architectue, used for cross compilation.    #
#                    On Amd and Intel 64 bits default is x86_64-linux-gnu      #
#                                                                              #
#   NO_COL           Prevent colour output.                                    #
#                                                                              #
#   USE_COL          Force colour when using pipe for tools like 'aha'.        #
#                    For example: make -j USE_COL=1 | aha > out.html           #
#                                                                              #
#   NO_SWIG          Prevent compiling Swig plugins.                           #
#                                                                              #
#   NO_PERL          Prevent compiling Perl Swig plugin.                       #
#                                                                              #
#   NO_LUA           Prevent compiling Lua swig plugin.                        #
#                                                                              #
#   NO_RUBY          Prevent compiling Ruby Swig plugin.                       #
#                                                                              #
#   NO_PYTHON        Prevent compiling Python Swig plugin.                     #
#                                                                              #
#   NO_PHP           Prevent compiling PHP Swig plugin.                        #
#                                                                              #
#   NO_TCL           Prevent compiling TCL Swig plugin.                        #
#                                                                              #
#   NO_JSON          Preven compiling JSON parsing plugsins                    #
#                    and static link                                           #
#                                                                              #
#   NO_CJSON         Preven usinsg C JSON library                              #
#                                                                              #
#   NO_FCJSON        Preven usinsg fast JSON library                           #
#                                                                              #
#   USE_FCJSON       Use the fast JSON library in static link!                 #
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
TOP:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

# TOOLS
RL:=ranlib
LN:=ln -fs
SED:=sed
MD:=mkdir -p
TAR:=tar cfJ

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
ESC!=printf '\033['
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
COLOR_BUILD:=$(COLOR_MAGENTA)
Q_DOXY=$Q$(info $(COLOR_BUILD)Doxygen$(COLOR_NORM))
Q_FRMT=$Q$(info $(COLOR_BUILD)Format$(COLOR_NORM))
Q_TAGS=$Q$(info $(COLOR_BUILD)[TAGS]$(COLOR_NORM))
Q_GEN=$Q$(info $(COLOR_BUILD)[GEN] $@$(COLOR_NORM))
Q_SWIG=$Q$(info $(COLOR_BUILD)[SWIG] $@$(COLOR_NORM))
Q_CLEAN=$Q$(info $(COLOR_BUILD)Cleaning$(COLOR_NORM))
Q_DISTCLEAN=$Q$(info $(COLOR_BUILD)Cleaning all$(COLOR_NORM))
Q_LD=$Q$(info $(COLOR_BUILD)[LD] $@$(COLOR_NORM))
Q_AR=$Q$(info $(COLOR_BUILD)[AR] $@$(COLOR_NORM))
Q_LCC=$(info $(COLOR_BUILD)[LCC] $*.cpp$(COLOR_NORM))
Q_CC=$Q$(info $(COLOR_BUILD)[CC] $<$(COLOR_NORM))
LIBTOOL_QUIET:=--quiet
MAKE_NO_DIRS:=--no-print-directory
# Filter normal output, send error output to stdout
QE:=2>&1 >/dev/null | $(SED) 's@^$(TOP)@@'
endif

include version
CPPFLAGS_OPT?=-Og
CPPFLAGS+=-Wdate-time -Wall -std=c++11 -g $(CPPFLAGS_OPT)
CPPFLAGS+=-MT $@ -MMD -MP -MF $*.d
LIBTOOL_CC=$Q$(Q_LCC)libtool --mode=compile --tag=CXX $(LIBTOOL_QUIET)
LIB_VER:=$(ver_maj).$(ver_min)
VER_VAL!=printf '0x%.2x%.2x' $(ver_maj) $(ver_min)
ifdef SONAME_USE_MAJ
SONAME:=.$(ver_maj)
endif
LIB_NAME:=libptpmgmt
LIB_NAME_SO:=$(LIB_NAME).so
SRCS:=$(wildcard *.cpp)
ifdef LD_SONAME
LDFLAGS_NM=-Wl,--version-script,scripts/lib.ver -Wl,-soname,$@$(SONAME)
endif
$(LIB_NAME_SO)_LDLIBS:=-lm -ldl
PMC_OBJS:=$(patsubst %.cpp,%.o,$(wildcard pmc*.cpp))
JSON_FROM_OBJS:=$(patsubst %.cpp,%.o,$(wildcard jsonF*.cpp))
LIB_OBJS:=$(filter-out $(JSON_FROM_OBJS) $(PMC_OBJS),$(patsubst %.cpp,%.o,$(SRCS)))
# Static only objects
LIB_A_OBJS:=
PMC_NAME:=pmc
ver.o: CPPFLAGS+=-DVER_MAJ=$(ver_maj) -DVER_MIN=$(ver_min) -DVER_VAL=$(VER_VAL)
D_INC=$(SED) -i 's@$($1)@\$$($1)@g' $*.d
LLC=$(Q_LCC)$(CXX) $(CPPFLAGS) $(CPPFLAGS_SWIG) -fPIC -DPIC -I. $1 -c $< -o $@

ifeq ($(call verCheck,$(shell $(CXX) -dumpversion),4.9),)
# GCC output colors
ifndef NO_COL
CPPFLAGS_COLOR:=-fdiagnostics-color=always
else
CPPFLAGS_COLOR:=-fdiagnostics-color=never
endif
endif # verCheck CXX 4.9
CPPFLAGS+=$(CPPFLAGS_COLOR)

ALL:=$(PMC_NAME) $(LIB_NAME_SO) $(LIB_NAME).a

ifneq ($(call which,dpkg-architecture),)
DEB_TARGET_MULTIARCH?=$(shell dpkg-architecture -qDEB_TARGET_MULTIARCH)
DEB_BUILD_MULTIARCH?=$(shell dpkg-architecture -qDEB_BUILD_MULTIARCH)
endif # which dpkg-architecture
TARGET_ARCH?=$(DEB_TARGET_MULTIARCH)
BUILD_ARCH?=$(DEB_BUILD_MULTIARCH)
ifneq ($(BUILD_ARCH),$(TARGET_ARCH)) # Cross compilation
CROSS_COMP:=1
rep_arch_f=$(subst /$(BUILD_ARCH)/,/$(TARGET_ARCH)/,$1)
rep_arch_p=$(subst /$(BUILD_ARCH),/$(TARGET_ARCH),$1)
rep_arch_o=$(subst $(BUILD_ARCH),$(TARGET_ARCH),$1)
endif
ifneq ($(TARGET_ARCH),)
LIBDIR?=/usr/lib/$(TARGET_ARCH)
else
LIBDIR?=/usr/lib
endif
%.so:
	$(Q_LD)$(CXX) $(LDFLAGS) $(LDFLAGS_NM) -shared $^ $(LOADLIBES) $($@_LDLIBS)\
	  $(LDLIBS) -o $@

# JSON libraries
JSON_C:=
ifneq ($(NO_JSON),)
# Disabe all JSON libraries
NO_CJSON:=1
NO_FCJSON:=1
endif

# Using json-c
JSONC_INC:=/usr/include/json-c
JSONC_LD:=-ljson-c
JSONC_LIB:=$(LIB_NAME)_jsonc.so
JSONC_FLIB:=$(JSONC_LIB)$(SONAME)
JSON_C+=\"$(JSONC_FLIB)\",
ifeq ($(NO_CJSON),) # No point probing if user defer
ifeq ($(wildcard $(JSONC_INC)/json.h),)
NO_CJSON:=1 # No header
else # wildcard json-c
# As json-c development do not support multiple architectures
# we need to verify library available on current architecture
JSONC_LIB_PROB!=$(CXX) -shared $(JSONC_LD) 2>&1 && rm a.out
ifneq ($(JSONC_LIB_PROB),) # Error indicat library not found
NO_CJSON:=1
endif
endif # wildcard json-c
endif # NO_CJSON
ifeq ($(NO_CJSON),)
JSONC_CFLAGS:=-include $(JSONC_INC)/json.h
jsonFromJc.o: jsonFrom.cpp
	$Q$(call LLC,$(JSONC_CFLAGS))
	$(call D_INC,JSONC_INC)
$(JSONC_LIB)_LDLIBS:=$(JSONC_LD)
$(JSONC_LIB): jsonFromJc.o $(LIB_NAME).so
ALL+=$(JSONC_LIB)
ifeq ($(USE_FCJSON),) # User prefer static link with fastjson
json.o: CPPFLAGS+=-DJSON_C_SLINK=$(JSONC_FLIB)
LIB_A_OBJS+=jsonFrom.o
jsonFrom.o_INC:=JSONC_INC
CPPFLAGS_jsonFrom.o+=$(JSONC_CFLAGS)
JSON_C_ST_DONE:=1
endif # USE_FCJSON
endif # NO_CJSON

# Using fastjson
FJSON_INC:=/usr/include/libfastjson
FJSON_LIB:=$(LIB_NAME)_fastjson.so
FJSON_FLIB:=$(FJSON_LIB)$(SONAME)
JSON_C+=\"$(FJSON_FLIB)\",
ifeq ($(wildcard $(FJSON_INC)/json.h),)
NO_FCJSON:=1
endif
ifeq ($(NO_FCJSON),)
FJSON_CFLAGS:=-include $(FJSON_INC)/json.h
jsonFromFj.o: jsonFrom.cpp
	$Q$(call LLC,$(FJSON_CFLAGS))
	$(call D_INC,FJSON_INC)
$(FJSON_LIB)_LDLIBS:=-lfastjson
$(FJSON_LIB): jsonFromFj.o $(LIB_NAME).so
ALL+=$(FJSON_LIB)
ifeq ($(JSON_C_ST_DONE),)
json.o: CPPFLAGS+=-DJSON_C_SLINK=$(FJSON_FLIB)
LIB_A_OBJS+=jsonFrom.o
jsonFrom.o_INC:=FJSON_INC
CPPFLAGS_jsonFrom.o+=$(FJSON_CFLAGS)
JSON_C_ST_DONE:=1
endif # JSON_C_ST_DONE
else # NO_FCJSON
ifneq ($(USE_FCJSON),)
$(error libfastjson is unavailable for static link)
endif
endif # NO_FCJSON

# Add jsonFrom libraries to search
json.o: CPPFLAGS+=-DJSON_C="$(JSON_C)"

# Compile library source code
$(LIB_A_OBJS): %.o: %.cpp
	$Q$(Q_LCC)$(CXX) -c $(CPPFLAGS) $(CPPFLAGS_$@) $< -o $@
	$(call D_INC,$($@_INC))
$(LIB_OBJS): %.o: %.cpp
	$(LIBTOOL_CC) $(CXX) -c $(CPPFLAGS) $< -o $@

# Depened shared library objects on the static library to ensure build
$(eval $(foreach obj,$(LIB_OBJS), $(call depend,.libs/$(obj),$(obj))))

$(LIB_NAME).a: $(LIB_OBJS) $(LIB_A_OBJS)
	$(Q_AR)$(AR) rcs $@ $^
	$(RL) $@
$(LIB_NAME_SO): $(foreach obj,$(LIB_OBJS),.libs/$(obj))

# pmc tool
$(PMC_OBJS): %.o: %.cpp
	$(Q_CC)$(CXX) $(CPPFLAGS) -c -o $@ $<
$(PMC_NAME): $(PMC_OBJS) $(LIB_NAME).$(PMC_USE_LIB)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

D_FILES:=$(wildcard *.d */*.d */*/*.d)
CLEAN:=$(wildcard *.o */*.o */*/*.o) *.lo .libs/* $(D_FILES)
DISTCLEAN:=$(ALL) $(wildcard *.so */*.so */*/*.so)
DISTCLEAN_DIRS:=.libs

clean:
	$(Q_CLEAN)$(RM) $(CLEAN)
distclean: deb_clean clean
	$(Q_DISTCLEAN)$(RM) $(DISTCLEAN)
	$(RM) -R $(DISTCLEAN_DIRS)

HEADERS_GEN:=mngIds.h callDef.h vecDef.h verDef.h
HEADERS_GEN_COMP:=mngIds.h callDef.h
HEADERS_SRCS:=$(filter-out $(HEADERS_GEN),$(wildcard *.h))
HEADERS:=$(filter-out pmc.h,$(HEADERS_SRCS)) $(HEADERS_GEN_COMP)
HEADERS_INST:=$(filter-out end.h err.h jsonDef.h comp.h msgProc.h ids.h,$(HEADERS))\
  verDef.h
verDef.h: GEN_FLAGS+=-Dver_maj=$(ver_maj) -Dver_min=$(ver_min) -DVER=$(LIB_VER)\
  -DVER_VAL=$(VER_VAL)
# MAP for  %.cc to %.h:
#  %@ => '/'    - Use when a slash is next to a star character
#  %! => '%'    - Self escape, escape precent sign character
#  %# => '#'    - Use on line start when starting a preproccesor in result file
#  %& => '"'    - Escape for double quote character
#  %_ =>        - Place marker, retain empty lines
#  %- => ' '    - When need 2 spaces or more. Use with a space between
#  %^ => '\n'   - Add new line in a preprocessor definition only
%.h: %.cc
	$(Q_GEN)$(CXX) -E $< $(GEN_FLAGS) |\
	  $(SED) -e 's/^#.*//;/^\s*$$/d;s#%@#/#g;s#%&#"#g'\
	  -e 's/^%#/#/;s/%-/ /g;s/%^/\n/g;s/%_//;s/%!/%/g' > $@
version:
DISTCLEAN+=$(HEADERS_GEN)

ifneq ($(call which,astyle),)
astyle_ver:=$(lastword $(shell astyle -V))
ifeq ($(call verCheck,$(astyle_ver),3.1),)
format: $(HEADERS_GEN) $(HEADERS_SRCS) $(SRCS) $(wildcard sample/*.cpp)
	$(Q_FRMT)astyle --project=none --options=astyle.opt $^
	./format.pl
ifneq ($(call which,cppcheck),)
	cppcheck --quiet --language=c++ --error-exitcode=-1 $^
endif
endif
endif # which astyle

ifndef NO_SWIG
ifneq ($(call which,swig),)
swig_ver=$(lastword $(shell swig -version | grep Version))
# We need swig 3.0, or above
ifeq ($(call verCheck,$(swig_ver),3.0),)
SWIG:=swig
SWIG_ALL:=
SWIG_NAME:=PtpMgmtLib
ifneq ($(call verCheck,$(swig_ver),4.1),)
# Only python and ruby have argcargv.i
perl_SFLAGS+=-Iswig/perl5
$(foreach n,lua php tcl,$(eval $(n)_SFLAGS+=-Iswig/$n))
# SWIG warnings
# comparison integer of different signedness
CPPFLAGS_RUBY+=-Wno-sign-compare
# ANYARGS is deprecated (seems related to ruby headers)
CPPFLAGS_RUBY+=-Wno-deprecated-declarations
# label 'thrown' is not used
CPPFLAGS_PHP+=-Wno-unused-label
# 'result' may be used uninitialized
CPPFLAGS_LUA+=-Wno-maybe-uninitialized
ifeq ($(PY_USE_S_THRD),)
# PyEval_InitThreads is deprecated
CPPFLAGS_PY+=-Wno-deprecated-declarations
endif
ifneq ($(call verCheck,$(swig_ver),4.0),)
# catching polymorphic type 'class std::out_of_range' by value
CPPFLAGS_RUBY+=-Wno-catch-value
ifneq ($(call verCheck,$(swig_ver),3.0.12),)
# Old SWIG does not support PHP 7
NO_PHP=1
endif ## ! swig 3.0.12
endif # ! swig 4.0
endif # ! swig 4.1
%/$(SWIG_NAME).cpp: $(LIB_NAME).i $(HEADERS) $(HEADERS_GEN_COMP)
	$(Q_SWIG)$(SWIG) -c++ -I. -I$(@D) -outdir $(@D) -Wextra $($(@D)_SFLAGS) -o $@ $<
# As SWIG does not create a dependencies file
# We create it during compilation from the compilation dependencies file
SWIG_DEP=$(SED) -e '1 a\ libptpmgmt.i mngIds.h \\'\
  $(foreach n,$(wildcard $(<D)/*.i),-e '1 a\ $n \\')\
  -e 's@.*\.o:\s*@@;s@\.cpp\s*@.cpp: @' $*.d > $*_i.d
SWIG_LD=$(Q_LD)$(CXX) $(LDFLAGS) -shared $^ $(LOADLIBES) $(LDLIBS)\
  $($@_LDLIBS) -o $@
ifndef NO_PERL
ifneq ($(call which,perl),)
PERL_INC!=perl -e 'for(@INC){print "$$_/CORE" if-f "$$_/CORE/EXTERN.h"}'
PERLDIR:=$(DESTDIR)$(lastword $(shell perl -e '$$_=$$^V;s/^v//;\
  s/^(\d+\.\d+).*/\1/;$$v=$$_;for(@INC){print "$$_\n" if /$$v/ and /lib/}'))
# Perl does not "know" how to cross properly
ifdef CROSS_COMP
PERL_INC:=$(call rep_arch_f,$(PERL_INC))
PERLDIR:=$(call rep_arch_f,$(PERLDIR))
endif
PERL_NAME:=perl/$(SWIG_NAME)
perl_SFLAGS+=-perl5
$(PERL_NAME).o: $(PERL_NAME).cpp $(HEADERS)
	$Q$(call LLC,-I$(PERL_INC))
	$(call D_INC,PERL_INC)
	$(SWIG_DEP)
$(PERL_NAME).so: $(PERL_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(PERL_NAME).so
CLEAN+=$(PERL_NAME).cpp
DISTCLEAN+=$(foreach e,pm,$(PERL_NAME).$e)
else # which perl
NO_PERL=1
endif
endif # NO_PERL

ifndef NO_LUA
ifneq ($(call which,lua),)
LUA_LIB_NAME:=ptpmgmt.so
lua_SFLAGS+=-lua
CLEAN+=lua/$(SWIG_NAME).cpp
define lua
LUA_FLIB_$1:=liblua$1-$(LUA_LIB_NAME)
LUA_LIB_$1:=lua/$1/$(LUA_LIB_NAME)
LUA_INC_$1:=/usr/include/lua$1
ifdef LD_SONAME
$$(LUA_LIB_$1)_LDLIBS:=-Wl,-soname,$$(LUA_FLIB_$1)$(SONAME)
endif
lua/$1/$(SWIG_NAME).o: lua/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(MD) lua/$1
	$$(call LLC,$$(CPPFLAGS_LUA) -I$$(LUA_INC_$1))
	$$(call D_INC,LUA_INC_$1)
	$$(SWIG_DEP)
$$(LUA_LIB_$1): lua/$1/$(SWIG_NAME).o $(LIB_NAME_SO)
	$$(SWIG_LD)
SWIG_ALL+=$$(LUA_LIB_$1)
DISTCLEAN_DIRS+=lua/$1

endef
# Build multiple Lua versions
LUA_VERSIONS:=$(subst /,,$(subst /usr/include/lua,,$(dir\
  $(wildcard /usr/include/lua*/lua.h))))
$(eval $(foreach n,$(LUA_VERSIONS),$(call lua,$n)))
# Build single Lua version
ifneq ($(wildcard /usr/include/lua.h),)
LUA_INC:=/usr/include
# Get Lua version and library base
LUA_VER:=$(lastword $(shell lua -e 'print(_VERSION)'))
# Do we have default lua lib, or is it versioned?
ifeq ($(shell /sbin/ldconfig -p | grep liblua.so),)
LUA_BASE:=$(firstword $(subst .so, ,$(notdir $(lastword $(shell\
   /sbin/ldconfig -p | grep "lua.*$(LUA_VER)\.so$$")))))
LUA_FLIB:=$(LUA_BASE)-$(LUA_LIB_NAME)
else
LUA_FLIB:=liblua-$(LUA_LIB_NAME)
endif
ifdef LD_SONAME
$(LUA_LIB)_LDLIBS:=-Wl,-soname,$(LUA_FLIB)$(SONAME)
endif
LUA_LIB:=lua/$(LUA_LIB_NAME)
lua/$(SWIG_NAME).o: lua/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(call LLC,$(CPPFLAGS_LUA))
	$(call D_INC,LUA_VER)
	$(SWIG_DEP)
$(LUA_LIB): lua/$(SWIG_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(LUA_LIB)
endif # /usr/include/lua.h
else # which lua
NO_LUA=1
endif
endif # NO_LUA

ifndef NO_PYTHON
define python
PY_BASE_$1:=python/$1/$(SWIG_NAME)
PY_SO_$1:=python/$1/$(PY_LIB_NAME).so
PY_INC_$1!=python$1-config --includes
$$(PY_SO_$1)_LDLIBS!=python$1-config --libs
PY_INC_BASE_$1:=$$(subst -I,,$$(firstword $$(PY_INC_$1)))
PY$1_DIR:=$(DESTDIR)$$(lastword $$(shell python$1 -c 'import site;\
  print("\n".join(site.getsitepackages()))' | grep $(PY_LIBDIR)))
$$(PY_BASE_$1).o: $(PY_BASE).cpp $(HEADERS)
	$Q$(MD) python/$1
	$$(call LLC,$(CPPFLAGS_PY) $$(PY_INC_$1))
	$$(call D_INC,PY_INC_BASE_$1)
	$$(SWIG_DEP)
$$(PY_SO_$1): $$(PY_BASE_$1).o $(LIB_NAME_SO)
	$$(SWIG_LD)
SWIG_ALL+=$$(PY_SO_$1)
DISTCLEAN_DIRS+=python/$1

endef

ifneq ($(call which,python3-config),)
USE_PY3:=1
USE_PY:=1
endif

ifdef USE_PY
PY_BASE:=python/$(SWIG_NAME)
PY_LIB_NAME:=_ptpmgmt
PY_LIBDIR?=/usr/lib/python
python_SFLAGS+=-python
ifeq ($(PY_USE_S_THRD),)
python_SFLAGS+=-threads -DSWIG_USE_MULTITHREADS
endif
CLEAN+=$(PY_BASE).cpp
DISTCLEAN+=python/ptpmgmt.py python/ptpmgmt.pyc
DISTCLEAN_DIRS+=python/__pycache__
ifdef USE_PY3
$(eval $(call python,3))
PY3_EXT!=python3-config --extension-suffix
ifdef CROSS_COMP
PY3_EXT:=$(call rep_arch_o,$(PY3_EXT))
endif
endif # USE_PY3

endif # USE_PY
endif # NO_PYTHON

ifndef NO_RUBY
ifneq ($(call which,ruby),)
# configuration comes from /usr/lib/*/ruby/*/rbconfig.rb
RUBY_SCRIPT_LIB:='puts "-l" + RbConfig::CONFIG["RUBY_SO_NAME"]'
RUBY_SCRIPT_VDIR:='puts RbConfig::CONFIG["vendorarchdir"]'
RUBY_INC_BASE!=ruby -rrbconfig -e 'puts RbConfig::CONFIG["rubyhdrdir"]'
RUBY_INC_ARC!=ruby -rrbconfig -e 'puts RbConfig::CONFIG["rubyarchhdrdir"]'
RUBY_INC:=-I$(RUBY_INC_BASE) -I$(RUBY_INC_ARC)
RUBY_LIB!=ruby -rrbconfig -e $(RUBY_SCRIPT_LIB)
RUBYDIR:=$(DESTDIR)$(shell ruby -rrbconfig -e $(RUBY_SCRIPT_VDIR))
# Ruby does not "know" how to cross properly
ifdef CROSS_COMP
RUBY_INC:=$(call rep_arch_f,$(RUBY_INC))
RUBY_LIB:=$(call rep_arch_f,$(RUBY_LIB))
RUBYDIR:=$(call rep_arch_f,$(RUBYDIR))
endif
RUBY_NAME:=ruby/$(SWIG_NAME).cpp
RUBY_LNAME:=ruby/ptpmgmt
ruby_SFLAGS:=-ruby
$(RUBY_LNAME).so_LDLIBS:=$(RUBY_LIB)
$(RUBY_LNAME).o: $(RUBY_NAME) $(HEADERS)
	$Q$(call LLC,$(CPPFLAGS_RUBY) $(RUBY_INC))
	$(SED) -i -e 's#$(RUBY_INC_BASE)#\$$(RUBY_INC_BASE)#g;'\
	  -e 's#$(RUBY_INC_ARC)#\$$(RUBY_INC_ARC)#g' $*.d
	$(SWIG_DEP)
$(RUBY_LNAME).so: $(RUBY_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(RUBY_LNAME).so
CLEAN+=$(RUBY_NAME)
else # which ruby
NO_RUBY=1
endif
endif # NO_RUBY

ifneq ($(call which,php-config),)
PHPCFG:=php-config
endif
ifneq ($(call which,php-config7),)
PHPCFG:=php-config7
endif
ifeq ($(PHPCFG),)
NO_PHP=1
else # PHPCFG
php_ver=$(subst $(SP),.,$(wordlist 1,2,$(subst ., ,$(shell $(PHPCFG) --version))))
ifneq ($(call verCheck,$(php_ver),7.0),)
NO_PHP=1
endif # PHP 7
endif # PHPCFG
ifndef NO_PHP
PHPEDIR:=$(DESTDIR)$(shell $(PHPCFG) --extension-dir)
PHPIDIR:=$(DESTDIR)$(lastword $(subst :, ,$(shell\
        php -r 'echo get_include_path();')))
PHP_INC:=-Iphp $(shell $(PHPCFG) --includes)
PHP_INC_BASE!=$(PHPCFG) --include-dir
PHP_NAME:=php/$(SWIG_NAME).cpp
PHP_LNAME:=php/ptpmgmt
php_SFLAGS+=-php7
$(PHP_LNAME).o: $(PHP_NAME) $(HEADERS)
	$Q$(call LLC,$(CPPFLAGS_PHP) $(PHP_INC))
	$(call D_INC,PHP_INC_BASE)
	$(SWIG_DEP)
$(PHP_LNAME).so: $(PHP_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(PHP_LNAME).so
CLEAN+=$(PHP_NAME) php/php_ptpmgmt.h
DISTCLEAN+=$(PHP_LNAME).php php/php.ini
endif # NO_PHP

ifneq ($(wildcard /usr/include/tcl/tcl.h),)
TCL_INC:=/usr/include/tcl
else # tcl/tcl.h
ifneq ($(wildcard /usr/include/tcl.h),)
TCL_INC:=/usr/include
else # tcl.h
NO_TCL:=1
endif # tcl.h
endif # tcl/tcl.h
ifndef NO_TCL
ifneq ($(call which,tclsh)),)
tcl_ver!=echo 'puts $$tcl_version;exit 0' | tclsh
ifeq ($(call verCheck,$(tcl_ver),8.0),)
TCL_NAME:=tcl/$(SWIG_NAME).cpp
TCL_LNAME:=tcl/ptpmgmt
CPPFLAGS_TCL+=-I$(TCL_INC)
tcl_SFLAGS+=-tcl8 -namespace
$(TCL_LNAME).o: $(TCL_NAME) $(HEADERS)
	$Q$(call LLC,$(CPPFLAGS_TCL))
	$(call D_INC,TCL_INC)
	$(SWIG_DEP)
$(TCL_LNAME).so: $(TCL_LNAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(TCL_LNAME).so
CLEAN+=$(TCL_NAME)
tcl_paths!=echo 'puts $$auto_path;exit 0' | tclsh
ifneq ($(TARGET_ARCH),)
TCL_LIB:=$(firstword $(shell echo $(tcl_paths) |\
  $(SED) 's/ /\n/g' | grep '$(BUILD_ARCH)'))
ifdef CROSS_COMP
TCL_LIB:=$(call rep_arch_p,$(TCL_LIB))
endif
else
TCL_LIB:=$(firstword $(shell echo $(tcl_paths) |\
  $(SED) 's/ /\n/g' | grep '/usr/lib.*/tcl'))
endif
TCLDIR:=$(DESTDIR)$(TCL_LIB)/ptpmgmt
# TODO how the hell tcl "know" the library version? Why does it think it's 0.0?
define pkgIndex
if {![package vsatisfies [package provide Tcl] $(tcl_ver)]} {return}
package ifneeded ptpmgmt 0.0 [list load [file join $$dir ptpmgmt.so]]
endef
else # tcl_ver 8.0
NO_TCL=1
endif
else # which tclsh
NO_TCL=1
endif
endif # NO_TCL

ALL+=$(SWIG_ALL)
else # swig 3.0
NO_SWIG=1
endif
else # which swig
NO_SWIG=1
endif
endif # NO_SWIG

ifneq ($(call which,doxygen),)
ifeq ($(call verCheck,$(shell doxygen -v),1.8),)
doxygen: $(HEADERS_GEN) $(HEADERS)
	$(Q_DOXY)doxygen doxygen.cfg $(QE)
DISTCLEAN_DIRS+=doc
endif
endif # which doxygen

ifneq ($(call which,ctags),)
tags: $(HEADERS_GEN) $(filter-out ids.h,$(HEADERS_SRCS)) $(SRCS)
	$(Q_TAGS)ctags -R $^
ALL+=tags
DISTCLEAN+=tags
endif # which ctags

all: $(HEADERS_GEN_COMP) $(ALL)
	@:
.DEFAULT_GOAL=all
.ONESHELL: # Run rules in a single shell

####### Debain build #######
ifneq ($(and $(wildcard debian/rules),$(call which,dpkg-buildpackage)),)
# Remove all link result as the library have soname
#  and json.o, as it compile with list of libraries name with soname
DEB_ALL_CLEAN=$(PMC_NAME) $(wildcard *.so */*.so */*/*.so) json.o
deb_src: distclean
	$(Q)dpkg-source -b .
deb:
	$(Q)MAKEFLAGS=$(MAKE_NO_DIRS) Q=$Q dpkg-buildpackage -b -uc
	$(RM) $(DEB_ALL_CLEAN)
ifneq ($(DEB_ARC),)
deb_arc:
	$(Q)MAKEFLAGS=$(MAKE_NO_DIRS) Q=$Q dpkg-buildpackage -b -uc -a$(DEB_ARC)
	$(RM) $(DEB_ALL_CLEAN)
endif
deb_clean:
	$Q$(MAKE) $(MAKE_NO_DIRS) -f debian/rules deb_clean Q=$Q
endif # and wildcard debian/rules, which dpkg-buildpackage

SRC_FILES:=$(wildcard *.cc *.i */test.* scripts/* *.sh *.pl *.md *.cfg *.opt\
  php/*.sh swig/*/* */*.i phc_ctl*) LICENSE $(HEADERS_SRCS) $(SRCS)\
  $(wordlist 1,2,$(MAKEFILE_LIST))
SRC_NAME:=libptpmgmt-$(LIB_VER)

####### RPM build #######
RPM_SRC:=rpm/SOURCES/$(SRC_NAME).txz
$(RPM_SRC): $(SRC_FILES)
	$Q$(MD) rpm/SOURCES
	$(TAR) $@ $^ --transform "s#^#$(SRC_NAME)/#S"
ifneq ($(call which,rpmbuild),)
rpm: $(RPM_SRC)
	$(Q)rpmbuild --define "_topdir $(PWD)/rpm" -bb rpm/libptpmgmt.spec
endif # which rpmbuild
rpmsrc: $(RPM_SRC)
DISTCLEAN_DIRS+=$(wildcard rpm/[BRS]*)

####### Arch Linux build #######
ARCHL_SRC:=archlinux/$(SRC_NAME).txz
ARCHL_BLD:=archlinux/PKGBUILD
$(ARCHL_SRC): $(SRC_FILES)
	$Q$(TAR) $@ $^
$(ARCHL_BLD): $(ARCHL_BLD).org | $(ARCHL_SRC)
	$(Q)cp $^ $@
	printf "sha256sums=('%s')\n" $(firstword $(shell sha256sum $(ARCHL_SRC))) >> $@
ifneq ($(call which,makepkg),)
pkg: $(ARCHL_BLD)
	$(Q)cd archlinux && makepkg
endif # which makepkg
pkgsrc: $(ARCHL_BLD)
DISTCLEAN+=$(ARCHL_SRC) $(ARCHL_BLD) $(wildcard archlinux/*.pkg.tar.zst)
DISTCLEAN_DIRS+=archlinux/src archlinux/pkg

####### installation #######
URL:=html/index.html
REDIR:="<meta http-equiv=\"refresh\" charset=\"utf-8\" content=\"0; url=$(URL)\"/>"
INSTALL?=install -p
NINST:=$(INSTALL) -m 644
DINST:=$(INSTALL) -d
BINST:=$(INSTALL)
DEV_PKG?=libptpmgmt-dev
SBINDIR?=/usr/sbin
LUADIR:=$(DESTDIR)$(LIBDIR)
DOCDIR:=$(DESTDIR)/usr/share/doc/libptpmgmt-doc
MANDIR:=$(DESTDIR)/usr/share/man/man8

install:
ifdef SONAME_USE_MAJ
	$(Q)for lib in $(LIB_NAME)*.so; do\
	  $(NINST) -D $$lib $(DESTDIR)$(LIBDIR)/$$lib.$(LIB_VER);\
	  $(LN) $$lib.$(LIB_VER) $(DESTDIR)$(LIBDIR)/$$lib$(SONAME);done
	$(LN) $(LIB_NAME_SO)$(SONAME) $(DESTDIR)$(LIBDIR)/$(LIB_NAME_SO)
else
	$Q$(NINST) -D $(LIB_NAME)*.so -t $(DESTDIR)$(LIBDIR)
endif
	$(NINST) $(LIB_NAME).a $(DESTDIR)$(LIBDIR)
	$(NINST) -D $(HEADERS_INST) -t $(DESTDIR)/usr/include/ptpmgmt
	$(foreach f,$(HEADERS_INST),$(SED) -i\
	  's!#include\s*\"\([^"]\+\)\"!#include <ptpmgmt/\1>!'\
	  $(DESTDIR)/usr/include/ptpmgmt/$f;)
	$(NINST) -D scripts/*.mk -t $(DESTDIR)/usr/share/$(DEV_PKG)
	$(BINST) -D pmc $(DESTDIR)$(SBINDIR)/pmc-ptpmgmt
	$(BINST) -D phc_ctl $(DESTDIR)$(SBINDIR)/phc_ctl-ptpmgmt
	$(BINST) -D phc_ctl.8 $(MANDIR)/phc_ctl-ptpmgmt.8
	gzip $(MANDIR)/phc_ctl-ptpmgmt.8
	$(LN) pmc.8.gz $(MANDIR)/pmc-ptpmgmt.8.gz
	$(RM) doc/html/*.md5
	$(DINST) $(DOCDIR)
	cp -a doc/html $(DOCDIR)
	printf $(REDIR) > $(DOCDIR)/index.html
ifndef NO_SWIG
ifndef NO_PERL
	$(NINST) -D perl/$(SWIG_NAME).so -t $(PERLDIR)/auto/$(SWIG_NAME)
	$(NINST) perl/$(SWIG_NAME).pm $(PERLDIR)
endif # NO_PERL
ifndef NO_LUA
	$(foreach n,$(LUA_VERSIONS),\
	  $(NINST) -D $(LUA_LIB_$n) $(LUADIR)/$(LUA_FLIB_$n).$(LIB_VER);\
	  $(LN) $(LUA_FLIB_$n).$(LIB_VER) $(LUADIR)/$(LUA_FLIB_$n)$(SONAME);\
	  $(DINST) $(LUADIR)/lua/$n;\
	  $(LN) ../../$(LUA_FLIB_$n).$(LIB_VER) $(LUADIR)/lua/$n/$(LUA_LIB_NAME);)
ifdef LUA_VER
	$(NINST) -D $(LUA_LIB) $(LUADIR)/$(LUA_FLIB)
	$(DINST) $(LUADIR)/lua/$(LUA_VER)
	$(LN) ../../$(LUA_FLIB) $(LUADIR)/lua/$(LUA_VER)/$(LUA_LIB_NAME)
endif # LUA_VER
endif # NO_LUA
ifdef USE_PY3
	$(NINST) -D python/3/$(PY_LIB_NAME).so\
	  $(PY3_DIR)/$(PY_LIB_NAME)$(PY3_EXT)
	$(NINST) python/ptpmgmt.py $(PY3_DIR)
endif
ifndef NO_RUBY
	$(NINST) -D $(RUBY_LNAME).so -t $(RUBYDIR)
endif # NO_RUBY
ifndef NO_PHP
	$(NINST) -D $(PHP_LNAME).so -t $(PHPEDIR)
	$(NINST) -D $(PHP_LNAME).php -t $(PHPIDIR)
endif # NO_PHP
ifndef NO_TCL
	$(NINST) -D $(TCL_LNAME).so -t $(TCLDIR)
	printf '$(subst $(line),\n,$(pkgIndex))\n' > $(TCLDIR)/pkgIndex.tcl
endif # NO_TCL
endif # NO_SWIG

include $(D_FILES)

checkall: format doxygen

help:
	$(info $(help))
	@:

.PHONY: all clean distclean format install deb_src deb deb_arc deb_clean\
        doxygen checkall help rpm rpmsrc pkg pkgsrc
