/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief HMAC wrapper Library using openssl
 *
 * The library depends on ssl library
 * And uses the libptpmgmt library!
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "comp.h"
#include <openssl/crypto.h>
#include <openssl/core_names.h>
#include <openssl/obj_mac.h>
#include <openssl/evp.h>
#include <openssl/err.h>

__PTPMGMT_NAMESPACE_USE;

struct triple {
    const char *algorithm;
    const char *key;
    const char *algo;
};
static triple vals[] = {
    [HMAC_SHA256] =  { OSSL_MAC_NAME_HMAC, OSSL_ALG_PARAM_DIGEST, SN_sha256},
    [HMAC_AES128] =  { OSSL_MAC_NAME_CMAC, OSSL_ALG_PARAM_CIPHER, SN_aes_128_cbc},
    [HMAC_AES256] =  { OSSL_MAC_NAME_CMAC, OSSL_ALG_PARAM_CIPHER, SN_aes_256_cbc},
};

struct HMAC_SSL : public HMAC_Key {
    char m_err_str[120]; // For error
    EVP_MAC *m_mac;
    EVP_MAC_CTX *m_ctx;
    size_t m_min_digest;
    HMAC_SSL();
    ~HMAC_SSL() override;
    bool init(HMAC_t type) override;
    bool digest(const void *data, size_t len, Binary &mac) override;
    bool verify(const void *data, size_t len, Binary &mac) override;
    const char *ssl_err();
};
HMAC_SSL::HMAC_SSL() : m_mac(nullptr), m_ctx(nullptr)
{
    m_err_str[sizeof m_err_str - 1] = 0;
}
HMAC_SSL::~HMAC_SSL()
{
    if(m_mac != nullptr)
        EVP_MAC_free(m_mac);
    if(m_ctx != nullptr)
        EVP_MAC_CTX_free(m_ctx);
}
bool HMAC_SSL::init(HMAC_t type)
{
    m_mac = EVP_MAC_fetch(nullptr, vals[type].algorithm, nullptr);
    if(m_mac == nullptr)
        return false;
    m_ctx = EVP_MAC_CTX_new(m_mac);
    if(m_ctx == nullptr)
        return false;
    const OSSL_PARAM params[] = {
        OSSL_PARAM_construct_utf8_string(vals[type].key,
            (char *)vals[type].algo, 0),
        OSSL_PARAM_END
    };
    if(!EVP_MAC_CTX_set_params(m_ctx, params)) {
        PTPMGMT_ERROR("EVP_MAC_CTX_set_params fail %s", ssl_err());
        return false;
    }
    m_min_digest = (type == HMAC_SHA256) ? 32 : 16;
    if(!EVP_MAC_init(m_ctx, m_key.get(), m_key.size(), nullptr)) {
        PTPMGMT_ERROR("EVP_MAC_init fail %s", ssl_err());
        return false;
    }
    return true;
}
bool HMAC_SSL::digest(const void *data, size_t len, Binary &mac)
{
    if(!EVP_MAC_init(m_ctx, nullptr, 0, nullptr) ||
        !EVP_MAC_update(m_ctx, (const uint8_t *)data, len))
        return false;
    size_t size = max(mac.size(), m_min_digest);
    if(size > HMAC_MAX_MAC_SIZE) {
        PTPMGMT_ERROR("MAC size too big");
        return false;
    }
    uint8_t buf[HMAC_MAX_MAC_SIZE];
    if(!EVP_MAC_final(m_ctx, buf, &size, size)) {
        PTPMGMT_ERROR("EVP_MAC_final %s", ssl_err());
        return false;
    }
    mac.setBin(buf, mac.size());
    PTPMGMT_ERROR_CLR;
    return true;
}
bool HMAC_SSL::verify(const void *data, size_t len, Binary &mac)
{
    size_t size = mac.size();
    Binary o(size);
    if(digest(data, len, o))
        return CRYPTO_memcmp(mac.get(), o.get(), size) == 0;
    return false;
}
const char *HMAC_SSL::ssl_err()
{
    ERR_error_string_n(ERR_get_error(), m_err_str, sizeof m_err_str - 1);
    return m_err_str;
}

HMAC_DECL(HMAC_SSL)
