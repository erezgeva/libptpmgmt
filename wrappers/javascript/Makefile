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
  "main": "./$1",
  "keywords": [
    "linuxptp",
    "ptp"
  ],
  "repository": {
    "type": "git",
    "url": "https://github.com/erezgeva/libptpmgmt.git"
  },
  "engines": {
    "node": ">=$(NODEJS_VER)"
  }
}

endef

ifdef JSCEN # JavaScriptCore
$(JS_D)/jsc/$(SWIG_NAME).o: $(JS_D)/jsc/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_JSC) $(JSCINC))
JSC_SO:=$(JS_D)/jsc/$(SWIG_LNAME).so
$(JSC_SO)_LDLIBS=$(JSCLIB)
$(JSC_SO): $(JS_D)/jsc/$(SWIG_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
$(JSC_RUN): $(JSC_RUN_SRC)
	$(Q_CC)$(CXX) -g $(JSCINC) $^ $(JSCLIB) -o $@
SWIG_ALL+=$(JSC_SO) $(JSC_RUN)

utest_js_jsc: $(LIB_NAME_SO) $(JSC_SO) $(JSC_RUN)
	$(call Q_UTEST,JSC)LD_PRELOAD=$(ASAN_PRE)./$<\
	  LD_LIBRARY_PATH=$(JS_D)/jsc $(JSC_RUN) $(JS_D)/utest.js
endif # JSCEN - JavaScriptCore

ifdef JSV8EN # Node.js V8
CXXFLAGS_V8+=-DBUILDING_NODE_EXTENSION=1
CXXFLAGS_V8+= -I$(JS_NODE_DIR) -include node.h
$(JS_D)/v8/$(SWIG_NAME).o: $(JS_D)/v8/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(call LLC17,$(CXXFLAGS_V8) $(JSV8INC))
V8_SO:=$(JS_D)/v8/$(SWIG_LNAME).node
$(V8_SO)_LDLIBS+=$(JSV8LIB)
$(V8_SO): $(JS_D)/v8/$(SWIG_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)
SWIG_ALL+=$(V8_SO)

utest_js_v8: $(LIB_NAME_SO) $(V8_SO)
	$(call Q_UTEST,V8)LD_PRELOAD=$(ASAN_PRE)./$<\
	  ENGINE=v8 $(NODEJS) $(JS_D)/utest.js
endif # JSV8EN - Node.js V8

ifdef JSNAPIEN # Node.js API
CXXFLAGS_NAPI+=-I$(JS_NODE_DIR)
# TODO 'Init(Napi::Env' fixed by SWIG ?.?.? in Lib/javascript/napi/javascriptinit.swg
$(JS_D)/napi/$(SWIG_NAME).o: $(JS_D)/napi/$(SWIG_NAME).cpp $(HEADERS)
	$Q$(SED) -i -e '/Napi::Object Init(Napi::Env/ s/Init/InitNapi/'\
	  -e '/NODE_API_MODULE/ s/Init)/InitNapi)/' $<
	$(call LLC17,$(CXXFLAGS_NAPI) $(JSNAPIINC))
NAPI_SO:=$(JS_D)/napi/$(SWIG_LNAME).node
$(NAPI_SO): $(JS_D)/napi/$(SWIG_NAME).o $(LIB_NAME_SO)
	$(SWIG_LD)

utest_js_napi: $(LIB_NAME_SO) $(NAPI_SO)
	$(call Q_UTEST,V8)LD_PRELOAD=$(ASAN_PRE)./$<\
	  ENGINE=napi $(NODEJS) $(JS_D)/utest.js

DESC1:="PTP management library, to communicate with ptp4l"
install_js_napi:
	$(Q)$(INSTALL_FOLDER) $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(SWIG_LNAME)
	echo "const $(SWIG_LNAME) = require('./$(SWIG_LNAME).node');" >\
	  $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(SWIG_LNAME)/$(SWIG_LNAME).js
	$(info $(call package_json,$(SWIG_LNAME),LGPL,$(DESC1))) >\
	  $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(SWIG_LNAME)/package.json
	$(INSTALL_LIB) $(NAPI_SO) $(DESTDIR)$(NODENAPI_SYS_MODULES)/$(SWIG_LNAME)
endif # JSNAPIEN - Node.js API

UTEST_JS_TGTS:=$(addprefix utest_js_,jsc v8 napi)
.PHONE: $(UTEST_JS_TGTS) install_js_napi
utest_javascript: $(UTEST_JS_TGTS)
install_javascript: install_js_napi
