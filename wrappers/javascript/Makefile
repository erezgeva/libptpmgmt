# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for JavaScript libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2026 Erez Geva
#
###############################################################################

define package_json
{
  "name": "$1",
  "version": "$(ver_maj).$(ver_min)",
  "description": "$3",
  "author": "Erez Geva <ErezGeva2@gmail.com>",
  "license": "$2",
  "main": "$1.js",
  "type": "commonjs",
  "keywords": [ $4 ],
  "repository": {
    "type": "git",
    "url": "https://github.com/erezgeva/libptpmgmt.git"
  },
  "homepage": "https://libptpmgmt.nwtime.org",
  "bugs": {
    "url": "https://github.com/erezgeva/libptpmgmt/issues"
  },
  "engines": {
    "node": ">=$(NODEJS_VER)"
  }
}

endef

ifdef JSCEN # Apple WebKit JavaScriptCore
JS_TGTS+=jsc
$(JS_D)/jsc/$(SWIG_NAME).o: $(JS_D)/jsc/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_JSC) $(JSCINC))
JSC_SO:=$(JS_D)/jsc/$(SWIG_LNAME).so
$(JSC_SO)_LDLIBS=$(JSCLIB)
$(JSC_SO): $(JS_D)/jsc/$(SWIG_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
$(JSC_RUN): $(JSC_RUN_SRC)
	$(Q_CC)$(CXX) -g $(JSCINC) $^ $(JSCLIB) -o $@
SWIG_ALL+=$(JSC_SO) $(JSC_RUN)
$(JS_D)/jsc/utest.o: $(JS_D)/jsc/utest.cpp
	$Q$(call LLC,$(CXXFLAGS_JSC) -Iutest $(JSCINC))
JSC_UTEST_SO:=$(JS_D)/jsc/utest_help.so
$(JSC_UTEST_SO)_LDLIBS=$(JSCLIB)
$(JSC_UTEST_SO): $(JS_D)/jsc/utest.o $(LIBUTEST) $(LIB_NAME_SO)
	$(SWIG_LD)
ifndef SKIP_CLKMGR
$(JS_D)/jsc/$(CLKMGR_NAME).o: $(JS_D)/jsc/$(CLKMGR_NAME).cpp $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_JSC) $(JSCINC) $(CLKMGR_CXXFLAGS))
JS_CLKMGR_SO:=$(JS_D)/jsc/$(CLKMGR_NAME).so
$(JS_CLKMGR_SO)_LDLIBS=$(JSCLIB)
$(JS_CLKMGR_SO): $(JS_D)/jsc/$(CLKMGR_NAME).o $(CLKMGR_LIB_SO)
	$(SWIG_LD)
SWIG_ALL+=$(JS_CLKMGR_SO)
endif # SKIP_CLKMGR
utest_js_jsc: $(LIB_NAME_SO) $(JSC_SO) $(JSC_RUN) $(JSC_UTEST_SO)
	$(call Q_UTEST,JSC)
	echo "module.exports = require('./jsc/$(SWIG_LNAME)');" >\
	  $(JS_D)/load$(SWIG_LNAME).js
	echo "module.exports = require('./jsc/utest_help');" >\
	  $(JS_D)/utest_help.js
	LD_PRELOAD=$(ASAN_PRE)./$< ADD_PATH=./ $(JSC_RUN) $(JS_D)/utest.js
	LD_PRELOAD=$(ASAN_PRE)./$<:$(LIBUTEST)\
	  ADD_PATH=./ $(JSC_RUN) $(JS_D)/utestSig.js

# LD_LIBRARY_PATH=$(JS_D)/jsc
endif # JSCEN - Apple WebKit JavaScriptCore

ifdef JSV8EN # Google V8 JavaScript engine
JS_TGTS+=v8
CXXFLAGS_V8+=-DBUILDING_NODE_EXTENSION=1
CXXFLAGS_V8+= -I$(JS_NODE_DIR) -include node.h
$(JS_D)/v8/$(SWIG_NAME).o: $(JS_D)/v8/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(call LLC20,$(CXXFLAGS_V8) $(JSV8INC))
V8_SO:=$(JS_D)/v8/$(SWIG_LNAME).node
$(V8_SO)_LDLIBS+=$(JSV8LIB)
$(V8_SO): $(JS_D)/v8/$(SWIG_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(V8_SO)
$(JS_D)/v8/utest.o: $(JS_D)/v8/utest.cpp
	$Q$(call LLC20,$(CXXFLAGS_V8) -Iutest $(JSV8INC))
V8_UTEST_SO:=$(JS_D)/v8/utest_help.node
$(V8_UTEST_SO)_LDLIBS=$(JSV8LIB)
$(V8_UTEST_SO): $(JS_D)/v8/utest.o $(LIBUTEST) $(LIB_NAME_SO)
	$(SWIG_LD)
ifndef SKIP_CLKMGR
$(JS_D)/v8/$(CLKMGR_NAME).o: $(JS_D)/v8/$(CLKMGR_NAME).cpp $(HEADERS)
	$Q$(call LLC20,$(CXXFLAGS_V8) $(JSV8INC) $(CLKMGR_CXXFLAGS))
V8_CLKMGR_SO:=$(JS_D)/v8/$(CLKMGR_NAME).node
$(V8_CLKMGR_SO)_LDLIBS+=$(JSV8LIB)
$(V8_CLKMGR_SO): $(JS_D)/v8/$(CLKMGR_NAME).o $(CLKMGR_LIB_SO)
	$(SWIG_LD)
SWIG_ALL+=$(V8_CLKMGR_SO)
endif # SKIP_CLKMGR
utest_js_v8: $(LIB_NAME_SO) $(V8_SO) $(V8_UTEST_SO)
	$(call Q_UTEST,V8)
	echo "module.exports = require('./v8/$(SWIG_LNAME).node');" >\
	  $(JS_D)/load$(SWIG_LNAME).js
	echo "module.exports = require('./v8/utest_help.node');" >\
	  $(JS_D)/utest_help.js
	LD_PRELOAD=$(ASAN_PRE)./$< ADD_PATH=./ $(NODEJS) $(JS_D)/utest.js
	LD_PRELOAD=$(ASAN_PRE)./$<:$(LIBUTEST) ADD_PATH=./ $(NODEJS)\
	  $(JS_D)/utestSig.js
endif # JSV8EN - Google V8 JavaScript engine

ifdef JSNAPIEN # Node.js N-API
JS_TGTS+=napi
CXXFLAGS_NAPI+=-I$(JS_NODE_DIR)
# TODO 'Init(Napi::Env' fixed by SWIG ?.?.? in Lib/javascript/napi/javascriptinit.swg
$(JS_D)/napi/$(SWIG_NAME).o: $(JS_D)/napi/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(SED) -i -e '/Napi::Object Init(Napi::Env/ s/Init/InitNapi/'\
	  -e '/NODE_API_MODULE/ s/Init)/InitNapi)/' $<
	$(call LLC20,$(CXXFLAGS_NAPI) $(JSNAPIINC))
NAPI_SO:=$(JS_D)/napi/$(SWIG_LNAME).node
$(NAPI_SO): $(JS_D)/napi/$(SWIG_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(NAPI_SO)
ifndef SKIP_CLKMGR
$(JS_D)/napi/$(CLKMGR_NAME).o: $(JS_D)/napi/$(CLKMGR_NAME).cpp $(HEADERS)
	$Q$(call LLC20,$(CXXFLAGS_NAPI) $(JSNAPIINC) $(CLKMGR_CXXFLAGS))
NAPI_CLKMGR_SO:=$(JS_D)/napi/$(CLKMGR_NAME).node
$(NAPI_CLKMGR_SO): $(JS_D)/napi/$(CLKMGR_NAME).o $(CLKMGR_LIB_SO)
	$(SWIG_LD)
SWIG_ALL+=$(NAPI_CLKMGR_SO)
endif # SKIP_CLKMGR

utest_js_napi: $(LIB_NAME_SO) $(NAPI_SO)
	$(call Q_UTEST,NAPI)
	echo "module.exports = require('./napi/$(SWIG_LNAME).node');" >\
	  $(JS_D)/load$(SWIG_LNAME).js
	LD_PRELOAD=$(ASAN_PRE)./$< ADD_PATH=./ $(NODEJS) $(JS_D)/utest.js

DESC1:=PTP management library Node.js wrapper
DESC2:=Clock Manager library Node.js wrapper
KEYS1:="linuxptp", "ptp"
KEYS2:="linuxptp", "ptp", "clock manager"
# filter to ensure package.json is pretty formated
JQ_FL:=$(if $(JQ),| $(JQ))
install_js_napi:
	$(Q)$(INSTALL_FOLDER) $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(SWIG_LNAME)/lib
	echo "module.exports = require('./lib/$(SWIG_LNAME).node');" >\
	  $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(SWIG_LNAME)/load$(SWIG_LNAME).js
	printf '$(call package_json,$(SWIG_LNAME),LGPL,$(DESC1),$(KEYS1))' \
	  $(JQ_FL) > $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(SWIG_LNAME)/package.json
	$(INSTALL_DATA) $(JS_D)/$(SWIG_LNAME).js\
	  $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(SWIG_LNAME)
	$(INSTALL_LIB) $(NAPI_SO) $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(SWIG_LNAME)/lib
ifndef SKIP_CLKMGR
	$(Q)$(INSTALL_FOLDER) $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(CLKMGR_NAME)
	echo "module.exports = require('./$(CLKMGR_NAME).node');" >\
	  $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(CLKMGR_NAME)/$(CLKMGR_NAME).js
	printf '$(call package_json,$(CLKMGR_NAME),BSD-3-Clause,$(DESC2),$(KEYS2))' \
	  $(JQ_FL) > $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(CLKMGR_NAME)/package.json
	$(INSTALL_LIB) $(NAPI_CLKMGR_SO)\
	  $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(CLKMGR_NAME)
endif # SKIP_CLKMGR
endif # JSNAPIEN - Node.js N-API

utest_javascript: $(addprefix utest_js_,jsc v8 napi)
install_javascript: install_js_napi
