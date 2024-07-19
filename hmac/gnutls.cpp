/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief HMAC wrapper Library using gnutls library
 *
 * The library depends on gnutls library
 * And uses the libptpm library!
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "comp.h"
#include <gnutls/crypto.h>

static int count = 0;

__PTPMGMT_NAMESPACE_USE;

static gnutls_mac_algorithm_t vals[] = {
    [HMAC_SHA256] = GNUTLS_MAC_SHA256,
    [HMAC_AES128] = GNUTLS_MAC_AES_CMAC_128,
    [HMAC_AES256] = GNUTLS_MAC_AES_CMAC_256,
};

struct Gnutls : public HMAC_Key {
    gnutls_mac_algorithm_t m_algorithm;
    gnutls_hmac_hd_t m_dig;
    bool m_keyInit;
    Gnutls(): m_keyInit(false) {}
    ~Gnutls() override;
    bool init(HMAC_t type) override;
    bool digest(const void *data, size_t len, Binary &mac) override;
    bool verify(const void *data, size_t len, Binary &mac) override;
};
Gnutls::~Gnutls()
{
    if(m_keyInit)
        gnutls_hmac_deinit(m_dig, nullptr);
    count--;
    if(count == 0)
        gnutls_global_deinit();
}
bool Gnutls::init(HMAC_t type)
{
    m_algorithm = vals[type];
    int err;
    count++;
    if(count == 1) {
        err = gnutls_global_init();
        if(err < 0) {
            PTPMGMT_ERROR("gnutls global initializing fail %s",
                gnutls_strerror(err));
            return false;
        }
    }
    err = gnutls_hmac_init(&m_dig, m_algorithm, m_key.get(), m_key.size());
    if(err < 0) {
        PTPMGMT_ERROR("gnutls global initializing fail %s", gnutls_strerror(err));
        return false;
    }
    m_keyInit = true;
    return true;
}
bool Gnutls::digest(const void *data, size_t len, Binary &mac)
{
    size_t size = max(mac.size(), (size_t)gnutls_hmac_get_len(m_algorithm));
    if(size > HMAC_MAX_MAC_SIZE) {
        PTPMGMT_ERROR("MAC size too big");
        return false;
    }
    uint8_t buf[HMAC_MAX_MAC_SIZE];
    PTPMGMT_ERROR_CLR;
    if(gnutls_hmac(m_dig, data, len) < 0) {
        // Clean for next digest
        gnutls_hmac_output(m_dig, buf);
        return false;
    }
    gnutls_hmac_output(m_dig, buf);
    mac.setBin(buf, mac.size());
    return true;
}
bool Gnutls::verify(const void *data, size_t len, Binary &mac)
{
    size_t size = mac.size();
    Binary o(size);
    if(digest(data, len, o))
        return gnutls_memcmp(mac.get(), o.get(), size) == 0;
    return false;
}

HMAC_DECL(Gnutls)
