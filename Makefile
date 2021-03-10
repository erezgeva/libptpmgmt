# SPDX-License-Identifier: LGPL-3.0-or-later

# Makefile Create libpmc and pmc for testing
#
# Authors: Erez Geva <ErezGeva2@gmail.com>

PMC_USE_LIB?=a # 'a' for static and 'so' for dynamic

define depend
$(1): $(2)

endef

include version
# Ensure linker link using C++
CC:=g++
RL:=ranlib
CPPFLAGS+=-Wall -std=c++11 -g
CPPFLAGS+= -MT $@ -MMD -MP -MF $(basename $@).d
CPPFLAGS_msg:=-DVER_MAJ=$(ver_maj) -DVER_MIN=$(ver_min)
LIBTOOL_CC:=libtool --mode=compile --tag=CXX
LIB_MODULES:=cfg msg sock ptp ids
LIB_OBJS:=$(foreach mod,$(LIB_MODULES),$(mod).o)

all: pmc libpmc.so libpmc.a

# clang-format indent

# Compile library source code
$(LIB_OBJS):
	$(LIBTOOL_CC) $(CXX) -c $(CPPFLAGS) $(CPPFLAGS_$(basename $@))\
		$(basename $@).cpp -o $@
# Depened shared library objects on the static library to ensure build
$(eval $(foreach obj,$(LIB_OBJS), $(call depend,.libs/$(obj),$(obj))))

libpmc.a: $(LIB_OBJS)
	$(AR) rcs $@ $^
	$(RL) $@
libpmc.so: $(foreach mod,$(LIB_MODULES),.libs/$(mod).o)
	$(CXX) $(LDFLAGS) -shared $^ $(LOADLIBES) $(LDLIBS) -o $@

pmc: pmc.o libpmc.$(PMC_USE_LIB)

include $(wildcard *.d)

clean:
	$(RM) -f *.o *.lo *.d .libs/*
distclean: clean
	$(RM) -f libpmc.a libpmc.so pmc
	$(RM) -Rf .libs
