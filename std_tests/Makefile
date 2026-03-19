# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com>

which=$(shell which $1 2>/dev/null)

define c_cmp
cmp_c_$1: cmp_c.c
	$(CC) $(CPPFLAGS) -std=c$1 $$< $(LDLIBS) -o $$@

cmp_gnuc_$1: cmp_c.c
	$(CC) $(CPPFLAGS) -std=gnu$1 $$< $(LDLIBS) -o $$@

cmp_c_lang_$1: cmp_c.c
	$(CCLANG) $(CPPFLAGS) -std=c$1 $$< $(LDLIBS) -o $$@

cmp_gnuc_lang_$1: cmp_c.c
	$(CCLANG) $(CPPFLAGS) -std=gnu$1 $$< $(LDLIBS) -o $$@

endef
define cpp_cmp
cmp_cpp_$1: cmp_cpp.cpp
	$(CXX) $(CPPFLAGS) -std=c++$1 $$< $(LDLIBS) -o $$@

cmp_gnucpp_$1: cmp_cpp.cpp
	$(CXX) $(CPPFLAGS) -std=gnu++$1 $$< $(LDLIBS) -o $$@

cmp_cpp_lang_$1: cmp_cpp.cpp
	$(CXXLANG) $(CPPFLAGS) $(CLANG_FLAGS) -std=c++$1 $$< $(LDLIBS) -o $$@

cmp_gnucpp_lang_$1: cmp_cpp.cpp
	$(CXXLANG) $(CPPFLAGS) $(CLANG_FLAGS) -std=gnu++$1 $$< $(LDLIBS) -o $$@

endef

CCLANG:=clang
CXXLANG:=clang++
CPPFLAGS:=-Wall -Wextra -Winline -Werror -g -Wpedantic
LDLIBS:= -lptpmgmt -lclkmgr
C_STD:=11 17 2x
CPP_STD:=11 14 17 20
ALL:=$(addprefix cmp_c_,$(C_STD)) $(foreach n,$(C_STD),cmp_gnuc_$(n))\
     $(addprefix cmp_cpp_,$(CPP_STD) 23) $(addprefix cmp_gnucpp_,$(CPP_STD) 23)

ifneq ($(call which,$(CCLANG)),)
ALL+=$(foreach n,$(C_STD),cmp_c_lang_$(n) cmp_gnuc_lang_$(n))\
     $(foreach n,$(CPP_STD),cmp_cpp_lang_$(n) cmp_gnucpp_lang_$(n))
endif

all: $(ALL)
clean:
	$(RM) $(ALL) *.o

$(eval $(foreach n,$(C_STD),$(call c_cmp,$n)))
$(eval $(foreach n,$(CPP_STD) 23,$(call cpp_cmp,$n)))
