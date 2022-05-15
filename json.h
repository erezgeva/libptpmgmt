/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief convert a management or a signaling message to JSON
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_JSON_H
#define __PTPMGMT_JSON_H

#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include "msg.h"

#ifndef SWIG
namespace ptpmgmt
{
#endif

/**
 * Convert Message to JSON string
 * @param[in] message received from PTP entity
 * @param[in] indent base indent for the JSON string
 * @return JSON string
 */
std::string msg2json(const Message &message, int indent = 0);

/**
 * Parse JSON to PTP management message
 * Class provide converting function and
 *  store the result of the parsing.
 */
class Json2msg
{
  private:
    std::unique_ptr<BaseMngTlv> m_tlvData;
    /* Mandatory */
    mng_vals_e m_managementId;
    actionField_e m_action;
    /* optional */
    enum {
        have_unicastFlag,
        have_PTPProfileSpecific,
        have_domainNumber,
        have_versionPTP,
        have_minorVersionPTP,
        have_sequenceId,
        have_sdoId,
        have_srcPort,
        have_dstPort,
        have_limit,
    };
    bool m_have[have_limit];
    bool m_unicastFlag;
    uint8_t m_PTPProfileSpecific;
    uint8_t m_domainNumber;
    uint8_t m_versionPTP;
    uint8_t m_minorVersionPTP;
    uint16_t m_sequenceId;
    uint32_t m_sdoId;
    PortIdentity_t m_srcPort;
    PortIdentity_t m_dstPort;
  public:
    Json2msg();
    ~Json2msg();
    /**
     * Try to load the a specific library
     * @param[in] libName partial library name
     * @return true if library found and load
     * @note This function always return false when called from static library
     * @note The libName can be partial and is case insensitive.
     *       Library will load only if found exactly one match.
     * @note If this function fails, fromJson() and fromJsonObj(),
     *       will try to load any available library.
     * @note if Library is already load, return true if it matchs
     */
    static bool selectLib(const std::string &libName);
    /**
     * Return shared library name
     * @return shared library name or nullptr in not
     * @note When using static link, function return the shared library name
     *       of the same code!
     */
    static const char *loadLibrary();
    /**
     * Determine if library is shared or static
     * @return true if library is shared library
     */
    static bool isLibShared();
    /**
     * Convert JSON string to message
     * @param[in] json string
     * @return true if parsing success
     */
    bool fromJson(const std::string &json);
    /**
     * Convert JSON object to message
     * @param[in] jobj pointer of JSON object
     * @return true if parsing success
     * @note jobj must be json_object pointer
     * @attention You must use the same JSON library used by this library!
     *  build with USE_CJSON use the json-c library
     *  build with USE_FCJSON use the fast json library
     */
    bool fromJsonObj(const void *jobj);
    /**
     * Get management ID
     * @return management ID
     */
    mng_vals_e managementId() const { return m_managementId; }
    /**
     * Get dataField
     * @return dataField pointer or null
     * @note User @b should not try to free this TLV object
     */
    const BaseMngTlv *dataField() const { return m_tlvData.get(); }
    /**
     * Get action field
     * @return action field
     */
    actionField_e actionField() const { return m_action; }
    /**
     * Get Unicast flag
     * @return Unicast flag
     */
    bool isUnicast() const { return m_unicastFlag; }
    /**
     * Is Unicast flag in JSON?
     * @return true if Unicast flag in JSON
     */
    bool haveIsUnicast() const { return m_have[have_unicastFlag]; }
    /**
     * Get PTP profile specific
     * @return PTP profile specific
     */
    uint8_t PTPProfileSpecific() const { return m_PTPProfileSpecific; }
    /**
     * Is PTP profile specific in JSON?
     * @return true if PTP profile specific in JSON
     */
    bool havePTPProfileSpecific() const { return m_have[have_PTPProfileSpecific]; }
    /**
     * Get domain number
     * @return domain number
     */
    uint8_t domainNumber() const { return m_domainNumber; }
    /**
     * Is domain number in JSON?
     * @return true if domain number in JSON
     */
    bool haveDomainNumber() const { return m_have[have_domainNumber]; }
    /**
     * Get PTP major version
     * @return PTP major version
     */
    uint8_t versionPTP() const { return m_versionPTP; }
    /**
     * Is PTP major version in JSON?
     * @return true if PTP major version in JSON
     */
    bool haveVersionPTP() const { return m_have[have_versionPTP]; }
    /**
     * Get PTP minor version
     * @return PTP minor version
     */
    uint8_t minorVersionPTP() const { return m_minorVersionPTP; }
    /**
     * Is PTP minor version in JSON?
     * @return true if PTP minor version in JSON
     */
    bool haveMinorVersionPTP() const { return m_have[have_minorVersionPTP]; }
    /**
     * Get sequence ID
     * @return sequence ID
     */
    uint16_t sequenceId() const { return m_sequenceId; }
    /**
     * Is sequence ID in JSON?
     * @return true if sequence ID in JSON
     */
    bool haveSequenceId() const { return m_have[have_sequenceId]; }
    /**
     * Get sdoId
     * @return sdoId
     */
    uint32_t sdoId() const { return m_sdoId; }
    /**
     * Is sdoId in JSON?
     * @return true if sdoId in JSON
     */
    bool haveSdoId() const { return m_have[have_sdoId]; }
    /**
     * Get source port ID
     * @return source port ID
     */
    const PortIdentity_t &srcPort() const { return m_srcPort; }
    /**
     * Is source port ID in JSON?
     * @return true if source port ID in JSON
     */
    bool haveSrcPort() const { return m_have[have_srcPort]; }
    /**
     * Get target port ID
     * @return target port ID
     */
    const PortIdentity_t &dstPort() const { return m_dstPort; }
    /**
     * Is target port ID in JSON?
     * @return true if target port ID in JSON
     */
    bool haveDstPort() const { return m_have[have_dstPort]; }
};

#ifndef SWIG
}; /* namespace ptpmgmt */
#endif

#endif /* __PTPMGMT_JSON_H */
