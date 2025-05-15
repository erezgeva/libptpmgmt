/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief HMAC wrapper Library using gnutls library
 *
 * The library depends on gnutls library
 * And uses the libptpmgmt library!
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "comp.h"
#include <gnutls/crypto.h>

__PTPMGMT_NAMESPACE_USE;

static gnutls_mac_algorithm_t vals[] = {
    [HMAC_SHA256] = GNUTLS_MAC_SHA256,
    [HMAC_AES128] = GNUTLS_MAC_AES_CMAC_128,
    [HMAC_AES256] = GNUTLS_MAC_AES_CMAC_256,
};

struct Gnutls : public HMAC_Key {
    gnutls_mac_algorithm_t m_algorithm = GNUTLS_MAC_SHA256;
    gnutls_hmac_hd_t m_dig = nullptr;
    bool m_keyInit = false;
    ~Gnutls() override;
    bool init() override final;
    bool digest(const void *hData, size_t len, Binary &mac) override final;
    bool verify(const void *hData, size_t len, Binary &mac) override final;
};
Gnutls::~Gnutls()
{
    if(m_keyInit)
        gnutls_hmac_deinit(m_dig, nullptr);
}
bool Gnutls::init()
{
    m_algorithm = vals[m_type];
    int err = gnutls_hmac_init(&m_dig, m_algorithm, m_key.get(), m_key.size());
    if(err < 0) {
        PTPMGMT_ERROR("gnutls global initializing fail %s", gnutls_strerror(err));
        return false;
    }
    m_keyInit = true;
    return true;
}
bool Gnutls::digest(const void *hData, size_t len, Binary &mac)
{
    size_t size = max(mac.size(), (size_t)gnutls_hmac_get_len(m_algorithm));
    if(size > HMAC_MAX_MAC_SIZE) {
        PTPMGMT_ERROR("MAC size too big");
        return false;
    }
    uint8_t buf[HMAC_MAX_MAC_SIZE];
    PTPMGMT_ERROR_CLR;
    if(gnutls_hmac(m_dig, hData, len) < 0) {
        // Clean for next digest
        gnutls_hmac_output(m_dig, buf);
        return false;
    }
    gnutls_hmac_output(m_dig, buf);
    mac.setBin(buf, mac.size());
    return true;
}
bool Gnutls::verify(const void *hData, size_t len, Binary &mac)
{
    size_t size = mac.size();
    Binary o(size);
    if(digest(hData, len, o))
        return gnutls_memcmp(mac.get(), o.get(), size) == 0;
    return false;
}

static bool Load()
{
    int err = gnutls_global_init();
    if(err < 0) {
        PTPMGMT_ERROR("gnutls global initializing fail %s",
            gnutls_strerror(err));
        return false;
    }
    return true;
}
static void Unload()
{
    gnutls_global_deinit();
}
HMAC_DECL(Gnutls)
