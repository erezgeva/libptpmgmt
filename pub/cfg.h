/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Read ptp4l Configuration file
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 * @details
 *  Classes to read ptp4l configuration file.
 *  We @b only read the values needed by Management
 *
 */

#ifndef __PTPMGMT_CFG_H
#define __PTPMGMT_CFG_H

#ifdef __cplusplus
#include <map>
#include "bin.h"

__PTPMGMT_NAMESPACE_BEGIN

class SaFile;

/**
 * @brief Hold configuration parameters
 * @details
 *  Stores provides and parses parameters
 *  from a configuration file for all sections
 */
class ConfigFile
{
  private:
    #ifndef SWIG
    /** @cond internal */
    class ConfigSection
    {
      public:
        enum {
            val_base_val,
            transportSpecific_val = val_base_val,
            domainNumber_val,
            udp6_scope_val,
            udp_ttl_val,
            socket_priority_val,
            network_transport_val,
            active_key_id_val,
            spp_val,
            allow_unauth_val,
            str_base_val,
            uds_address_val = str_base_val,
            sa_file_val,
            bin_base_val,
            ptp_dst_mac_val = bin_base_val,
            p2p_dst_mac_val,
            last_val,
        };
        /* new values must be add to ranges[] */
        struct range_t {
            const char *name;
            const char *defStr;
            uint32_t defVal;
            uint32_t min;
            uint32_t max;
        };
        /* ranges and default value */
        static const range_t ranges[];
        /* String values */
        std::string m_str_vals[bin_base_val - str_base_val];
        /* Binaries values */
        Binary m_bin_vals[last_val - bin_base_val];
        /* integer values */
        uint32_t m_vals[str_base_val - val_base_val];
        /* Determine if a value is set in the configuration file.
         * Relevant for non global sections. */
        bool m_set[last_val] = { false };

        void setGlobal();
        bool set_val(char *line);
    };
    std::map<std::string, ConfigSection> cfgSec;
    ConfigSection *cfgGlobal; /* Not the owner, just a shortcut */
    void clear_sections();

    uint32_t get_num(int idx, const std::string &section) const;
    const std::string &get_str(int idx, const std::string &section) const;
    const Binary &get_bin(int idx, const std::string &section) const;
    bool is_global(int idx, const std::string &section) const;
    /**< @endcond */
    #endif
  public:
    ConfigFile();
    /**
     * Read a configuration file and parse it
     * @param[in] file name with path
     * @return true if parse success
     */
    bool read_cfg(const std::string &file);
    /**
     * Get the transportSpecific value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t transportSpecific(const std::string &section = "") const;
    /**
     * Get the domainNumber value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t domainNumber(const std::string &section = "") const;
    /**
     * Get the udp6_scope value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t udp6_scope(const std::string &section = "") const;
    /**
     * Get the udp_ttl value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t udp_ttl(const std::string &section = "") const;
    /**
     * Get the socket_priority value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t socket_priority(const std::string &section = "") const;
    /**
     * Get the network_transport value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t network_transport(const std::string &section = "") const;
    /**
     * Get the authentication key ID value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint32_t active_key_id(const std::string &section = "") const;
    /**
     * Get the authentication security parameter pointer value
     * @param[in] section (optional)
     * @return SPP ID
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t spp(const std::string &section = "") const;
    /**
     * Get accept unauthenticated response messages value
     * @param[in] section (optional)
     * @return the allow unauthenticated messages value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t allow_unauth(const std::string &section = "") const;
    /**
     * Query whether the authentication security parameter pointer is set
     * @param[in] section (optional)
     * @return true if value is set
     * @note calling without section will Query the @"global@" section
     */
    bool haveSpp(const std::string &section = "") const;
    /**
     * Get the uds_address value
     * @param[in] section (optional)
     * @return string object with value
     * @note calling without section will fetch value from @"global@" section
     */
    const std::string &uds_address(const std::string &section = "") const;
    /**
     * Get the authentication security association file value
     * @param[in] section (optional)
     * @return string object with value
     * @note calling without section will fetch value from @"global@" section
     */
    const std::string &sa_file(const std::string &section = "") const;
    /**
     * Get the ptp_dst_mac value
     * @param[in] section (optional)
     * @return Binary object with value
     * @note calling without section will fetch value from @"global@" section
     */
    const Binary &ptp_dst_mac(const std::string &section = "") const;
    /**
     * Get the p2p_dst_mac value
     * @param[in] section (optional)
     * @return Binary object with value
     * @note calling without section will fetch value from @"global@" section
     */
    const Binary &p2p_dst_mac(const std::string &section = "") const;
};

/**
 * @brief Authentication key type
 * @details
 *  For Hashed Message Authentication Mode (HMAC) digests.
 */
enum HMAC_t {
    HMAC_SHA256, /**< Using Secure Hash Algorithm 2 with 256 bits */
    /**
     * Using NIST Advanced Encryption Standard with 128 bits key
     */
    HMAC_AES128,
    /**
     * Using NIST Advanced Encryption Standard with 256 bits key
     */
    HMAC_AES256
};

/**
 * @brief authentication security parameters
 * @details
 *  Pool of authentication keys, used by the Massage class to calculate
 *   the Integrity Check Value (ICV) in the authentication TLV.
 */
class Spp
{
  private:
    #ifndef SWIG
    struct key_t {
        HMAC_t type = HMAC_SHA256;
        Binary key;
        size_t mac_size = 0;
        void operator()(HMAC_t _t, const Binary &_k, size_t _m) {
            type = _t;
            key = _k;
            mac_size = _m;
        }
    };
    std::map<uint32_t, key_t> m_keys;
    int8_t m_own_id;
  protected:
    /** @cond internal */
    bool set_val(char *line);
    void set(long id) { m_own_id = (uint8_t)id; m_keys.clear(); }
    friend class SaFile;
    /**< @endcond */
    #endif
  public:
    /**
     * Set a new key
     * @param[in] id id the Key
     * @param[in] type of the key
     * @param[in] value of the key
     * @param[in] digest size to use with the key
     * @param[in] replace an existing key if exist
     * @return true, if new key is valid
     */
    bool addKey(uint32_t id, HMAC_t type, const Binary &value, size_t digest,
        bool replace = true);
    /**
     * Verify key exist
     * @param[in] key ID
     * @return true if key exist
     */
    bool have(uint32_t key) const;
    /**
     * Get MAC digest size to use with the key
     * @param[in] key ID
     * @return MAC digest size or 0 if the key does not exist
     */
    size_t mac_size(uint32_t key) const;
    /**
     * Get key
     * @param[in] key ID
     * @return Key or empty binary if key does not exist
     */
    const Binary &key(uint32_t key) const;
    /**
     * Get number of keys
     * @return Key or empty binary if key does not exist
     */
    size_t keys() const;
    /**
     * Get key type
     * @param[in] key ID
     * @return Key type
     */
    HMAC_t htype(uint32_t key) const;
    /**
     * Return own SPP ID
     * @return own SPP ID
     */
    uint8_t ownID() const;
    /**
     * Construct empty SPP
     * @param[in] id of SPP
     */
    Spp(uint8_t id);
    Spp();
};

/**
 * @brief authentication security association file
 * @details
 *  Read the security association file with keys
 *  And fill the SPPs
 */
class SaFile
{
  private:
    std::map<uint8_t, Spp> m_spps;
  public:
    /**
     * Read a SA file and parse it
     * @param[in] file name with path
     * @return true if parse success
     */
    bool read_sa(const std::string &file);
    /**
     * Read a SA file and parse it.
     * Get file name from the configuration file.
     * @param[in] cfg reference to configuration file object
     * @param[in] section (optional)
     * @return true if parse success, false if fail
     *         or file does not exist
     *         or configuration file have an empty sa_file.
     */
    bool read_sa(const ConfigFile &cfg, const std::string &section = "");
    /**
     * Verify SPP is in the SA_file and have at least one key
     * @param[in] spp ID
     * @return true if SPP exist with at least one key
     */
    bool have(uint8_t spp) const;
    /**
     * Verify key exist in an SPP
     * @param[in] spp ID
     * @param[in] key ID
     * @return true if key exist
     */
    bool have(uint8_t spp, uint32_t key) const;
    /**
     * Get SPP by spp ID
     * @param[in] spp ID
     * @return SPP if exist or empty SPP
     */
    const Spp &spp(uint8_t spp) const;
};

__PTPMGMT_NAMESPACE_END
#else /* __cplusplus */
#include "c/cfg.h"
#endif /* __cplusplus */

#endif /* __PTPMGMT_CFG_H */
