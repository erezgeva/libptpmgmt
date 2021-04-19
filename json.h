/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief convert a management or a signaling message to JSON
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PMC_JSON_H
#define __PMC_JSON_H

#include <string>
#include <cstdint>
#include "msg.h"

/**
 * Convert Message to JSON string
 * @param[in] message received from PTP entity
 * @param[in] indent base indent for the JSON string
 * @return JSON string
 */
std::string msg2json(Message &message, int indent = 0);

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
    Json2msg(): m_managementId(NULL_PTP_MANAGEMENT), m_action(GET), m_have{0} {}
    /**
     * Convert JSON string to message
     * @param[in] json string
     * @return true if parsing success
     */
    bool fromJson(const std::string json);
    /**
     * Convert JSON object to message
     * @param[in] jobj pointer of JSON object
     * @return true if parsing success
     * @note jobj must be json_object pointer
     * @attention You must use the same JSON library used by this library!
     *  with PMC_USE_CJSON use the json-c library
     *  with PMC_USE_FCJSON use the fast json library
     */
    bool fromJsonObj(const void *jobj);
    /**
     * Get management ID
     * @return management ID
     */
    mng_vals_e managementId() { return m_managementId; }
    /**
     * Get dataField
     * @return dataField pointer or null
     */
    const BaseMngTlv *dataField() {return m_tlvData.get(); }
    /**
     * Get action field
     * @return action field
     */
    actionField_e actionField() { return m_action; }
    /**
     * Get Unicast flag
     * @return Unicast flag
     */
    bool isUnicast() {return m_unicastFlag; }
    /**
     * Is Unicast flag in JSON?
     * @return true if Unicast flag in JSON
     */
    bool haveIsUnicast() {return m_have[have_unicastFlag];}
    /**
     * Get PTP profile specific
     * @return PTP profile specific
     */
    uint8_t PTPProfileSpecific() {return m_PTPProfileSpecific; }
    /**
     * Is PTP profile specific in JSON?
     * @return true if PTP profile specific in JSON
     */
    bool havePTPProfileSpecific() {return m_have[have_PTPProfileSpecific];}
    /**
     * Get domain number
     * @return domain number
     */
    uint8_t domainNumber() {return m_domainNumber; }
    /**
     * Is domain number in JSON?
     * @return true if domain number in JSON
     */
    bool haveDomainNumber() {return m_have[have_domainNumber];}
    /**
     * Get PTP major version
     * @return PTP major version
     */
    uint8_t versionPTP() {return m_versionPTP; }
    /**
     * Is PTP major version in JSON?
     * @return true if PTP major version in JSON
     */
    bool haveVersionPTP() {return m_have[have_versionPTP];}
    /**
     * Get PTP minor version
     * @return PTP minor version
     */
    uint8_t minorVersionPTP() {return m_minorVersionPTP; }
    /**
     * Is PTP minor version in JSON?
     * @return true if PTP minor version in JSON
     */
    bool haveMinorVersionPTP() {return m_have[have_minorVersionPTP];}
    /**
     * Get sequence ID
     * @return sequence ID
     */
    uint16_t sequenceId() { return m_sequenceId; }
    /**
     * Is sequence ID in JSON?
     * @return true if sequence ID in JSON
     */
    bool haveSequenceId() { return m_have[have_sequenceId]; }
    /**
     * Get sdoId
     * @return sdoId
     */
    uint32_t sdoId() {return m_sdoId; }
    /**
     * Is sdoId in JSON?
     * @return true if sdoId in JSON
     */
    bool haveSdoId() {return m_have[have_sdoId];}
    /**
     * Get source port ID
     * @return source port ID
     */
    PortIdentity_t srcPort() {return m_srcPort; }
    /**
     * Is source port ID in JSON?
     * @return true if source port ID in JSON
     */
    bool haveSrcPort() {return m_have[have_srcPort];}
    /**
     * Get target port ID
     * @return target port ID
     */
    PortIdentity_t dstPort() {return m_dstPort; }
    /**
     * Is target port ID in JSON?
     * @return true if target port ID in JSON
     */
    bool haveDstPort() {return m_have[have_dstPort];}
};

#endif /*__PMC_JSON_H*/
