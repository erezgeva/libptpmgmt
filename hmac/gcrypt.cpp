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
#include <gcrypt.h>

__PTPMGMT_NAMESPACE_USE;

static int algo_vals[] = {
    [HMAC_SHA256] = GCRY_MAC_HMAC_SHA256,
    [HMAC_AES128] = GCRY_MAC_CMAC_AES,
    [HMAC_AES256] = GCRY_MAC_CMAC_AES
};
struct HMAC_gcrypt : public HMAC_Key {
    gcry_mac_hd_t hd;
    ~HMAC_gcrypt() override;
    bool init(HMAC_t type) override final;
    bool digest(const void *data, size_t len, Binary &mac) override final;
    bool verify(const void *data, size_t len, Binary &mac) override final;
    bool update(const void *data, size_t len);
};
HMAC_gcrypt::~HMAC_gcrypt()
{
    gcry_mac_close(hd);
}
bool HMAC_gcrypt::init(HMAC_t type)
{
    gcry_error_t err = gcry_mac_open(&hd, algo_vals[type], 0, nullptr);
    if(err != GPG_ERR_NO_ERROR) {
        PTPMGMT_ERROR("gcry_mac_open fail %d", err);
        return false;
    }
    err = gcry_mac_setkey(hd, m_key.get(), m_key.size());
    if(err != GPG_ERR_NO_ERROR) {
        PTPMGMT_ERROR("gcry_mac_setkey fail %d", err);
        return false;
    }
    return true;
}
bool HMAC_gcrypt::update(const void *data, size_t len)
{
    gcry_error_t err = gcry_mac_reset(hd);
    if(err != GPG_ERR_NO_ERROR) {
        PTPMGMT_ERROR("gcry_mac_reset fail %d", err);
        return false;
    }
    err = gcry_mac_write(hd, data, len);
    if(err != GPG_ERR_NO_ERROR) {
        PTPMGMT_ERROR("gcry_mac_write fail %d", err);
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
bool HMAC_gcrypt::digest(const void *data, size_t len, Binary &mac)
{
    if(!update(data, len))
        return false;
    size_t size = mac.size();
    if(size > HMAC_MAX_MAC_SIZE) {
        PTPMGMT_ERROR("MAC size too big");
        return false;
    }
    uint8_t buf[HMAC_MAX_MAC_SIZE];
    gcry_error_t err = gcry_mac_read(hd, buf, &size);
    if(err != GPG_ERR_NO_ERROR) {
        PTPMGMT_ERROR("gcry_mac_read fail %d", err);
        return false;
    }
    mac.setBin(buf, size);
    return true;
}
bool HMAC_gcrypt::verify(const void *data, size_t len, Binary &mac)
{
    if(!update(data, len))
        return false;
    gcry_error_t err = gcry_mac_verify(hd, mac.get(), mac.size());
    switch(err) {
        case GPG_ERR_NO_ERROR:
            return true;
        case GPG_ERR_CHECKSUM: // Do not report on wrong mac
            break;
        default:
            PTPMGMT_ERROR("gcry_mac_verify fail %d", err);
            break;
    }
    return false;
}

HMAC_DECL(HMAC_gcrypt)
