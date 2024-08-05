/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief HMAC wrapper Library using nettle
 *
 * The library depends on nettle library
 * And uses the libptpmgmt library!
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "comp.h"
#include <nettle/hmac.h>
#include <nettle/memops.h>
#include <nettle/nettle-meta.h>

__PTPMGMT_NAMESPACE_USE;

static const char *vals[] = {
    [HMAC_SHA256] = "hmac_sha256",
    [HMAC_AES128] = "cmac_aes128",
    [HMAC_AES256] = "cmac_aes256",
};

struct Nettle : public HMAC_Key {
    void *m_ctx = nullptr;
    const struct nettle_mac *m_mac = nullptr;
    ~Nettle() override {free(m_ctx);};
    bool init(HMAC_t type) override;
    bool digest(const void *data, size_t len, Binary &mac) override;
    bool verify(const void *data, size_t len, Binary &mac) override;
};
bool Nettle::init(HMAC_t type)
{
    const char *name = vals[type];
    for(int i = 0; nettle_macs[i] != nullptr; i++) {
        m_mac = nettle_macs[i];
        if(!strcmp(name, m_mac->name)) {
            if(m_mac->context_size == 0 || m_mac->set_key == nullptr)
                return false;
            if(type == HMAC_SHA256) {
                m_ctx = malloc(sizeof(hmac_sha256_ctx));
                if(m_ctx == nullptr)
                    return false;
                hmac_sha256_set_key((hmac_sha256_ctx *)m_ctx, m_key.size(),
                    m_key.get());
            } else {
                m_ctx = malloc(m_mac->context_size);
                if(m_ctx == nullptr)
                    return false;
                m_mac->set_key(m_ctx, m_key.get());
            }
            return true;
        }
    }
    return false;
}
bool Nettle::digest(const void *data, size_t len, Binary &mac)
{
    size_t size = mac.size();
    if(m_mac == nullptr || m_mac->update == nullptr || m_mac->digest == nullptr) {
        PTPMGMT_ERROR("Nettle is not initialised");
        return false;
    }
    if(size > HMAC_MAX_MAC_SIZE) {
        PTPMGMT_ERROR("MAC size too big");
        return false;
    }
    uint8_t buf[HMAC_MAX_MAC_SIZE];
    m_mac->update(m_ctx, len, (const uint8_t *)data);
    m_mac->digest(m_ctx, size, buf);
    mac.setBin(buf, size);
    PTPMGMT_ERROR_CLR;
    return true;
}
bool Nettle::verify(const void *data, size_t len, Binary &mac)
{
    size_t size = mac.size();
    Binary o(size);
    digest(data, len, o);
    return memeql_sec(mac.get(), o.get(), size);
}

HMAC_DECL(Nettle)
