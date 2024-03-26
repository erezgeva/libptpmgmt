# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com>
#
# Makefile for libptpmgmt HMAC wrapper libraries
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2024 Erez Geva
#
###############################################################################

# List of all HMAC shared libraries
HMAC_LIBS:=

UTEST_SSL:=$(OBJ_DIR)/utest_ssl
UTEST_GCRYPT:=$(OBJ_DIR)/utest_gcrypt
UTEST_GNUTLS:=$(OBJ_DIR)/utest_gnutls
UTEST_NETTLE:=$(OBJ_DIR)/utest_nettle
UTEST_NO_HMAC:=$(OBJ_DIR)/utest_no_hmac
UTEST_DYN_HMAC:=$(OBJ_DIR)/utest_dyn_hmac

# Add hmac library functions to static link
HMAC_LIBA_FLAGS:=-uptpm_hmac

# Using ssl
HMAC_LIBS+=\"$(SSL_FLIB)\",
ifneq ($(HAVE_SSL_HEADER),)
SSL_CFLAGS:=-DHLIB_NAME=\"$(SSL_LIBA)\"
HMAC_ALIB:=$(SSL_LIBA)
HMAC_ALIB_FLAGS:=$(SSL_LIB_FLAGS)
$(OBJ_DIR)/ssl.o: $(HMAC_SRC)/ssl.cpp | $(COMP_DEPS)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(SSL_CFLAGS) $< -o $@
$(OBJ_DIR)/.libs/ssl.o: $(OBJ_DIR)/ssl.o
$(SSL_LIB)_LDLIBS:=$(SSL_LIB_FLAGS)
$(SSL_FLIB): $(SSL_LIB)
	$Q$(LN) $^ $@
$(SSL_LIB): $(OBJ_DIR)/.libs/ssl.o $(LIB_NAME).so
$(SSL_LIBA): $(OBJ_DIR)/ssl.o
	$(LLA)
ALL+=$(SSL_FLIB) $(SSL_LIBA)
ifneq ($(GTEST_LIB_FLAGS),)
$(UTEST_SSL): $(OBJ_DIR)/utest_m.o utest/hmac.o $(LIB_NAME_A) $(SSL_LIBA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LIB_NAME_A) $(LOADLIBES) $(LDLIBS)\
	  $(SSL_LIB_FLAGS) $(HMAC_LIBA_FLAGS) $(GTEST_LIB_FLAGS) -o $@
utest_ssl: $(HEADERS_GEN_COMP) $(UTEST_SSL)
	$(call Q_UTEST,SSL)$(UVGD)$(UTEST_SSL) $(GTEST_NO_COL) $(GTEST_FILTERS)
endif # GTEST_LIB_FLAGS
endif # HAVE_SSL_HEADER

# Using LGPL Crypto library
HMAC_LIBS+=\"$(GCRYPT_FLIB)\",
ifneq ($(HAVE_GCRYPT_HEADER),)
GCRYPT_CFLAGS:=-DHLIB_NAME=\"$(GCRYPT_LIBA)\"
ifeq ($(HMAC_ALIB),)
HMAC_ALIB:=$(GCRYPT_LIBA)
HMAC_ALIB_FLAGS:=$(GCRYPT_LIB_FLAGS)
endif
$(OBJ_DIR)/gcrypt.o: $(HMAC_SRC)/gcrypt.cpp | $(COMP_DEPS)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(GCRYPT_CFLAGS) $< -o $@
$(OBJ_DIR)/.libs/gcrypt.o: $(OBJ_DIR)/gcrypt.o
$(GCRYPT_LIB)_LDLIBS:=$(GCRYPT_LIB_FLAGS)
$(GCRYPT_FLIB): $(GCRYPT_LIB)
	$Q$(LN) $^ $@
$(GCRYPT_LIB): $(OBJ_DIR)/.libs/gcrypt.o $(LIB_NAME).so
$(GCRYPT_LIBA): $(OBJ_DIR)/gcrypt.o
	$(LLA)
ALL+=$(GCRYPT_FLIB) $(GCRYPT_LIBA)
ifneq ($(GTEST_LIB_FLAGS),)
$(UTEST_GCRYPT): $(OBJ_DIR)/utest_m.o utest/hmac.o $(LIB_NAME_A) $(GCRYPT_LIBA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LIB_NAME_A) $(LOADLIBES) $(LDLIBS)\
	  $(GCRYPT_LIB_FLAGS) $(HMAC_LIBA_FLAGS) $(GTEST_LIB_FLAGS) -o $@
utest_gcrypt: $(HEADERS_GEN_COMP) $(UTEST_GCRYPT)
	$(call Q_UTEST,GCRYPT)$(UVGD)$(UTEST_GCRYPT) $(GTEST_NO_COL) $(GTEST_FILTERS)
endif # GTEST_LIB_FLAGS
endif # HAVE_GCRYPT_HEADER

# Using gnutls
HMAC_LIBS+=\"$(GNUTLS_FLIB)\",
ifneq ($(HAVE_GNUTLS_HEADER),)
GNUTLS_CFLAGS:=-DHLIB_NAME=\"$(GNUTLS_LIBA)\"
ifeq ($(HMAC_ALIB),)
HMAC_ALIB:=$(GNUTLS_LIBA)
HMAC_ALIB_FLAGS:=$(GNUTLS_LIB_FLAGS)
endif
HMAC_ALIBS+=$(GNUTLS_LIBA)
$(OBJ_DIR)/gnutls.o: $(HMAC_SRC)/gnutls.cpp | $(COMP_DEPS)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(GNUTLS_CFLAGS) $< -o $@
$(OBJ_DIR)/.libs/gnutls.o: $(OBJ_DIR)/gnutls.o
$(GNUTLS_LIB)_LDLIBS:=$(GNUTLS_LIB_FLAGS)
$(GNUTLS_FLIB): $(GNUTLS_LIB)
	$Q$(LN) $^ $@
$(GNUTLS_LIB): $(OBJ_DIR)/.libs/gnutls.o $(LIB_NAME).so
$(GNUTLS_LIBA): $(OBJ_DIR)/gnutls.o
	$(LLA)
ALL+=$(GNUTLS_FLIB) $(GNUTLS_LIBA)
ifneq ($(GTEST_LIB_FLAGS),)
$(UTEST_GNUTLS): $(OBJ_DIR)/utest_m.o utest/hmac.o $(LIB_NAME_A) $(GNUTLS_LIBA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LIB_NAME_A) $(LOADLIBES) $(LDLIBS)\
	  $(GNUTLS_LIB_FLAGS) $(HMAC_LIBA_FLAGS) $(GTEST_LIB_FLAGS) -o $@
utest_gnutls: $(HEADERS_GEN_COMP) $(UTEST_GNUTLS)
	$(call Q_UTEST,GNUTLS)$(UVGD)$(UTEST_GNUTLS) $(GTEST_NO_COL) $(GTEST_FILTERS)
endif # GTEST_LIB_FLAGS
endif # HAVE_GNUTLS_HEADER

# Using nettle
HMAC_LIBS+=\"$(NETTLE_FLIB)\",
ifneq ($(HAVE_NETTLE_HEADER),)
NETTLE_CFLAGS:=-DHLIB_NAME=\"$(NETTLE_LIBA)\"
ifeq ($(HMAC_ALIB),)
HMAC_ALIB:=$(NETTLE_LIBA)
HMAC_ALIB_FLAGS:=$(NETTLE_LIB_FLAGS)
endif
HMAC_ALIBS+=$(NETTLE_LIBA)
$(OBJ_DIR)/nettle.o: $(HMAC_SRC)/nettle.cpp | $(COMP_DEPS)
	$(LIBTOOL_CC) $(CXX) -c $(CXXFLAGS) $(NETTLE_CFLAGS) $< -o $@
$(OBJ_DIR)/.libs/nettle.o: $(OBJ_DIR)/nettle.o
$(NETTLE_LIB)_LDLIBS:=$(NETTLE_LIB_FLAGS)
$(NETTLE_FLIB): $(NETTLE_LIB)
	$Q$(LN) $^ $@
$(NETTLE_LIB): $(OBJ_DIR)/.libs/nettle.o $(LIB_NAME).so
$(NETTLE_LIBA): $(OBJ_DIR)/nettle.o
	$(LLA)
ALL+=$(NETTLE_FLIB) $(NETTLE_LIBA)
ifneq ($(GTEST_LIB_FLAGS),)
$(UTEST_NETTLE): $(OBJ_DIR)/utest_m.o utest/hmac.o $(LIB_NAME_A) $(NETTLE_LIBA)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LIB_NAME_A) $(LOADLIBES) $(LDLIBS)\
	  $(NETTLE_LIB_FLAGS) $(HMAC_LIBA_FLAGS) $(GTEST_LIB_FLAGS) -o $@
utest_nettle: $(HEADERS_GEN_COMP) $(UTEST_NETTLE)
	$(call Q_UTEST,NETTLE)$(UVGD)$(UTEST_NETTLE) $(GTEST_NO_COL) $(GTEST_FILTERS)
endif # GTEST_LIB_FLAGS
endif # HAVE_NETTLE_HEADER

# Check calling hmac without using HMAC library
ifneq ($(GTEST_LIB_FLAGS),)
$(UTEST_NO_HMAC): $(OBJ_DIR)/utest_m.o utest/hmac_no.o $(LIB_NAME_A)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(GTEST_LIB_FLAGS) -o $@
utest_no_hmac: $(HEADERS_GEN_COMP) $(UTEST_NO_HMAC)
	$(call Q_UTEST,NO HMAC)$(UVGD)$(UTEST_NO_HMAC) $(GTEST_NO_COL) $(GTEST_FILTERS)
$(UTEST_DYN_HMAC): $(OBJ_DIR)/utest_m.o utest/hmac_dyn.o $(LIB_NAME_SO)
	$(Q_LD)$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(GTEST_LIB_FLAGS) -o $@
utest_dyn_hmac: $(HEADERS_GEN_COMP) $(UTEST_DYN_HMAC) $(HMAC_FLIBS)
	$(call Q_UTEST,DYN HMAC)LD_PRELOAD=./$(LIB_NAME_SO) LD_LIBRARY_PATH=.\
	  $(UVGD)$(UTEST_DYN_HMAC) $(GTEST_NO_COL) $(GTEST_FILTERS)
endif # GTEST_LIB_FLAGS

UTEST_HMAC:=$(addprefix utest_,ssl gcrypt gnutls nettle no_hmac dyn_hmac)
.PHONY: $(UTEST_HMAC)
utest_hmac: $(UTEST_HMAC)

# Add HMAC libraries to search
$(OBJ_DIR)/hmac.o: override CXXFLAGS+=-DHMAC_LIBS="$(HMAC_LIBS)"
