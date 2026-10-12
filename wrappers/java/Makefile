# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for Java libptpmgmt wrapper
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2026 Erez Geva
#
###############################################################################

java_SFLAGS+=-java
JAVA_D:=wrappers/java
JAVA_BASE:=$(JAVA_D)/$(SWIG_NAME)
JAVA_SWIG_DIR:=$(JAVA_D)/$(SWIG_LNAME)
JAVA_JAR:=$(JAVA_D)/$(SWIG_LNAME).jar
JAVA_SO:=$(JAVA_D)/$(LIB_NAME).so
$(JAVA_SWIG_DIR):
	$Q$(MKDIR_P) "$@"
$(JAVA_BASE).cpp: $(JAVA_SWIG_DIR)
$(JAVA_BASE).cpp: java_SFLAGS+=-outdir $(JAVA_SWIG_DIR) -package $(SWIG_LNAME)
$(JAVA_BASE).o: $(JAVA_BASE).cpp $(HEADERS)
	$Q$(call LLC,$(CXXFLAGS_JAVA) -I$(JAVA_D) $(JAVAINC))
$(JAVA_SO): $(JAVA_BASE).o $(LIB_NAME_SO)
	$(SWIG_LD)
$(JAVA_JAR): $(JAVA_SO) $(JAVA_BASE).cpp $(LIB_NAME_SO)
	$(Q_JAR)$(JAVAC) $(JAVAC_OPTIONS) $(JAVA_SWIG_DIR)/*.java
	cd $(JAVA_D) && $(JAR) cvf $(notdir $@) $(SWIG_LNAME)/*.class $(Q_OUT)
SWIG_ALL+=$(JAVA_JAR)
ifndef SKIP_CLKMGR
JAVA_CLKMGR:=$(JAVA_D)/$(CLKMGR_NAME)
$(JAVA_CLKMGR):
	$Q$(MKDIR_P) "$@"
$(JAVA_CLKMGR).cpp: $(JAVA_CLKMGR)
$(JAVA_CLKMGR).cpp: java_SFLAGS+=-outdir $(JAVA_CLKMGR) -package $(CLKMGR_NAME)
$(JAVA_CLKMGR).o: $(JAVA_CLKMGR).cpp
	$Q$(call LLC,$(CXXFLAGS_JAVA) -I$(JAVA_D) $(JAVAINC) $(CLKMGR_CXXFLAGS))
$(JAVA_D)/$(CLKMGR_LIB).so: $(JAVA_CLKMGR).o $(CLKMGR_LIB_SO)
	$(SWIG_LD)
$(JAVA_D)/$(CLKMGR_NAME).jar: $(JAVA_D)/$(CLKMGR_LIB).so $(JAVA_CLKMGR).cpp\
                $(CLKMGR_LIB_SO)
	$(Q_JAR)$(JAVAC) $(JAVAC_OPTIONS) $(JAVA_CLKMGR)/*.java
	cd $(JAVA_D) && $(JAR) cvf $(notdir $@) $(CLKMGR_NAME)/*.class $(Q_OUT)
SWIG_ALL+=$(JAVA_D)/$(CLKMGR_NAME).jar
endif # SKIP_CLKMGR

# Possible:
# none - for no output
# tree - full output
# summary - summery only
JAVA_UTEST_MODE:=none
JAVA_UTEST_OPTS:=--disable-banner --details=$(JAVA_UTEST_MODE)

ifdef JAVA_JUNIT_API
JAVA_UTEST_DIR:=$(JAVA_D)/utest
JAVA_UTEST_SRC:=$(JAVA_D)/utest.java
$(JAVA_UTEST_DIR):
	$Q$(MKDIR_P) "$@"
$(JAVA_UTEST_DIR)/utest.class: $(JAVA_UTEST_DIR) $(JAVA_JAR) $(JAVA_UTEST_SRC)
	$(Q_JAR)$(JAVAC) -d $(JAVA_UTEST_DIR) \
	 -cp $(JAVA_JAR):$(JAVA_JUNIT_API) $(JAVA_UTEST_SRC)
JAVA_UTEST:=$(JAVA_D)/utest
JAVA_UTEST_HELP_DIR:=$(JAVA_D)/utest_help
JAVA_UTEST_SO:=$(JAVA_D)/libutest_help.so
JAVA_UTEST_JAR:=$(JAVA_UTEST)_help.jar
$(JAVA_UTEST_SO)_LDLIBS:=$(JAVA_LIB_FLAGS)
$(JAVA_UTEST_HELP_DIR):
	$Q$(MKDIR_P) "$@"
$(JAVA_UTEST).cpp: $(JAVA_UTEST_HELP_DIR)
$(JAVA_UTEST).cpp: java_SFLAGS+=-outdir $(JAVA_UTEST_HELP_DIR) -package utest_help
$(JAVA_UTEST).o: $(JAVA_UTEST).cpp
	$Q$(call LLC,$(CXXFLAGS_JAVA) -Iutest $(JAVAINC))
$(JAVA_UTEST_SO): $(JAVA_UTEST).o $(LIBUTEST)
	$(SWIG_LD)
$(JAVA_UTEST_JAR): $(JAVA_UTEST_SO) $(JAVA_UTEST).cpp
	$(Q_JAR)$(JAVAC) $(JAVAC_OPTIONS) $(JAVA_UTEST_HELP_DIR)/*.java
	cd $(JAVA_D) && $(JAR) cvf $(notdir $@) utest_help/*.class $(Q_OUT)
JAVA_UTEST_SIG_DIR:=$(JAVA_D)/utestSig
JAVA_UTEST_SIG_SRC:=$(JAVA_D)/utestSig.java
$(JAVA_UTEST_SIG_DIR):
	$Q$(MKDIR_P) "$@"
$(JAVA_UTEST_SIG_DIR)/utestSig.class: $(JAVA_UTEST_SIG_DIR) $(JAVA_JAR)\
                $(JAVA_UTEST_JAR) $(JAVA_UTEST_SIG_SRC)
	$(Q_JAR)$(JAVAC) -d $(JAVA_UTEST_SIG_DIR) \
	 -cp $(JAVA_JAR):$(JAVA_UTEST_JAR):$(JAVA_JUNIT_API) $(JAVA_UTEST_SIG_SRC)
endif # JAVA_JUNIT_API

UTEST_JAVA_DEPS:=$(LIB_NAME_SO) $(JAVA_JAR) $(JAVA_UTEST_DIR)/utest.class\
                $(JAVA_UTEST_SIG_DIR)/utestSig.class
ifdef JAVA_JUNIT
utest_java: $(UTEST_JAVA_DEPS)
	$(call Q_UTEST,Java)LD_PRELOAD=$(ASAN_PRE)./$< \
	  LD_LIBRARY_PATH=$(JAVA_D) $(JAVA_JUNIT_OPT) $(JAVA_JUNIT) \
	  execute $(JAVA_UTEST_OPTS) -cp $(JAVA_JAR):$(JAVA_UTEST_DIR) -c utest \
	  2>/dev/null
	if test $$? -ne 0;then exit 1;fi
	LD_PRELOAD=$(ASAN_PRE)./$< LD_LIBRARY_PATH=$(JAVA_D) \
	  $(JAVA_JUNIT_OPT) $(JAVA_JUNIT) execute $(JAVA_UTEST_OPTS) \
	  -cp $(JAVA_JAR):$(JAVA_UTEST_JAR):$(JAVA_UTEST_SIG_DIR) -c utestSig \
	  2>/dev/null
	if test $$? -ne 0;then exit 1;fi
else ifdef JAVA_JUNIT_CONSOLE
utest_java: $(UTEST_JAVA_DEPS)
	$(call Q_UTEST,Java)LD_PRELOAD=$(ASAN_PRE)./$< $(JAVA) \
	  $(JAVAFLAGS) -Djava.library.path=$(JAVA_D) \
	  -jar $(JAVA_JUNIT_CONSOLE) execute $(JAVA_UTEST_OPTS) \
	  -cp $(JAVA_JAR):$(JAVA_UTEST_DIR) -c utest
	if test $$? -ne 0;then exit 1;fi
	LD_PRELOAD=$(ASAN_PRE)./$< $(JAVA) -Djava.library.path=$(JAVA_D) \
	  $(JAVAFLAGS) -jar $(JAVA_JUNIT_CONSOLE) execute $(JAVA_UTEST_OPTS) \
	  -cp $(JAVA_JAR):$(JAVA_UTEST_JAR):$(JAVA_UTEST_SIG_DIR) -c utestSig
	if test $$? -ne 0;then exit 1;fi
endif # JAVA_JUNIT
ifdef JAVA_JNI_DIR
install_java:
	$(Q)$(INSTALL_FOLDER) $(DESTDIR)$(JAVA_JNI_DIR) $(DESTDIR)$(JAVA_JAR_DIR)
	cp -a $(JAVA_D)/*.so $(DESTDIR)$(JAVA_JNI_DIR)
	cp -a $(JAVA_D)/*.jar $(DESTDIR)$(JAVA_JAR_DIR)
endif
