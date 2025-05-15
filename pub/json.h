/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Convert a management or a signalling message to JSON
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_JSON_H
#define __PTPMGMT_JSON_H

#ifdef __cplusplus
#include "msg.h"

__PTPMGMT_NAMESPACE_BEGIN

/**
 * Convert Message to JSON string
 * @param[in] message received from PTP entity
 * @param[in] indent base indent for the JSON string
 * @return JSON string
 */
std::string msg2json(const Message &message, int indent = 0);

/**
 * Convert PTP management TLV to JSON string
 * @param[in] managementId PTP management TLV id
 * @param[in] tlv PTP management TLV
 * @param[in] indent base indent for the JSON string
 * @return JSON string
 */
std::string tlv2json(mng_vals_e managementId, const BaseMngTlv *tlv,
    int indent = 0);

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
    mng_vals_e m_managementId = NULL_PTP_MANAGEMENT;
    actionField_e m_action = GET;
    /* optional */
    enum {
        have_isUnicast,
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
    bool m_have[have_limit] = { false };
    bool m_isUnicast = false;
    uint8_t m_PTPProfileSpecific = 0, m_domainNumber = 0,
            m_versionPTP = 2, m_minorVersionPTP = 1;
    uint16_t m_sequenceId = 0;
    uint32_t m_sdoId = 0;
    PortIdentity_t m_srcPort = {{0}, 0}, m_dstPort = {{0}, 0};
  public:
    /**
     * Convert JSON string to message
     * @param[in] json string
     * @return true if parsing success
     */
    bool fromJson(const std::string &json);
    /**
     * Get management ID
     * @return management ID
     */
    mng_vals_e managementId() const;
    /**
     * Get dataField
     * @return dataField pointer or null
     * @note User @b should not try to free this TLV object
     */
    const BaseMngTlv *dataField() const;
    /**
     * Get action field
     * @return action field
     */
    actionField_e actionField() const;
    /**
     * Get Unicast flag
     * @return Unicast flag
     */
    bool isUnicast() const;
    /**
     * Is Unicast flag in JSON?
     * @return true if Unicast flag in JSON
     */
    bool haveIsUnicast() const;
    /**
     * Get PTP profile specific
     * @return PTP profile specific
     */
    uint8_t PTPProfileSpecific() const;
    /**
     * Is PTP profile specific in JSON?
     * @return true if PTP profile specific in JSON
     */
    bool havePTPProfileSpecific() const;
    /**
     * Get domain number
     * @return domain number
     */
    uint8_t domainNumber() const;
    /**
     * Is domain number in JSON?
     * @return true if domain number in JSON
     */
    bool haveDomainNumber() const;
    /**
     * Get PTP major version
     * @return PTP major version
     */
    uint8_t versionPTP() const;
    /**
     * Is PTP major version in JSON?
     * @return true if PTP major version in JSON
     */
    bool haveVersionPTP() const;
    /**
     * Get PTP minor version
     * @return PTP minor version
     */
    uint8_t minorVersionPTP() const;
    /**
     * Is PTP minor version in JSON?
     * @return true if PTP minor version in JSON
     */
    bool haveMinorVersionPTP() const;
    /**
     * Get sequence ID
     * @return sequence ID
     */
    uint16_t sequenceId() const;
    /**
     * Is sequence ID in JSON?
     * @return true if sequence ID in JSON
     */
    bool haveSequenceId() const;
    /**
     * Get sdoId
     * @return sdoId
     */
    uint32_t sdoId() const;
    /**
     * Is sdoId in JSON?
     * @return true if sdoId in JSON
     */
    bool haveSdoId() const;
    /**
     * Get source port ID
     * @return source port ID
     */
    const PortIdentity_t &srcPort() const;
    /**
     * Is source port ID in JSON?
     * @return true if source port ID in JSON
     */
    bool haveSrcPort() const;
    /**
     * Get target port ID
     * @return target port ID
     */
    const PortIdentity_t &dstPort() const;
    /**
     * Is target port ID in JSON?
     * @return true if target port ID in JSON
     */
    bool haveDstPort() const;
    /**
     * Call message setAction with parsed JSON
     * @param[in, out] message object
     * @return message setAction result
     */
    bool setAction(Message &message) const;
};

__PTPMGMT_NAMESPACE_END
#else /* __cplusplus */
#include "c/json.h"
#endif /* __cplusplus */

#endif /* __PTPMGMT_JSON_H */
