/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Convert a management or a signaling message to JSON
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_C_JSON_H
#define __PTPMGMT_C_JSON_H

#include "c/msg.h"

/** pointer to ptpmgmt json structure */
typedef struct ptpmgmt_json_t *ptpmgmt_json;

/** pointer to constant ptpmgmt json structure */
typedef const struct ptpmgmt_json_t *const_ptpmgmt_json;

/**
 * Convert Message to JSON string
 * @param[in] message received from PTP entity
 * @param[in] indent base indent for the JSON string
 * @return JSON string
 * @note The caller @b MUST free the string after use!
 */
char *ptpmgmt_json_msg2json(const_ptpmgmt_msg message, int indent);

/**
 * Convert PTP managment TLV to JSON string
 * @param[in] managementId PTP managment TLV id
 * @param[in] tlv PTP managment TLV
 * @param[in] indent base indent for the JSON string
 * @return JSON string
 * @note The caller @b MUST free the string after use!
 */
char *ptpmgmt_json_tlv2json(enum ptpmgmt_mng_vals_e managementId,
    const void *tlv, int indent);

/**
 * The ptpmgmt message structure hold the json object
 *  and call backs to call C++ methods
 * @note: C interface
 */
struct ptpmgmt_json_t {
    /**< @cond internal */
    void *_this; /**< pointer to actual C++ initialization object */
    struct ptpmgmt_PortIdentity_t *_srcPort; /**< Source port */
    struct ptpmgmt_PortIdentity_t *_dstPort; /**< Dest port */
    void *data; /**< Managment TLV C copy */
    void *dataTbl; /**< Managment TLV Table C copy */
    /**< @endcond */

    /**
     * Free json object
     * @param[in] j json object
     * @note: C interface
     */
    void (*free)(ptpmgmt_json j);
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
    bool (*selectLib)(const char *libName);
    /**
     * Return shared library name
     * @return shared library name or nullptr in not
     * @note When using static link, function return the shared library name
     *       of the same code!
     */
    const char *(*loadLibrary)();
    /**
     * Determine if library is shared or static
     * @return true if library is shared library
     */
    bool (*isLibShared)();
    /**
     * Convert JSON string to message
     * @param[in] json string
     * @return true if parsing success
     */
    bool (*fromJson)(ptpmgmt_json j, const char *json);
    /**
     * Convert JSON object to message
     * @param[in] jobj pointer of JSON object
     * @return true if parsing success
     * @note jobj must be json_object pointer
     * @attention You must use the same JSON library used by this library!
     *  build with USE_CJSON use the json-c library
     *  build with USE_FCJSON use the fast json library
     */
    bool (*fromJsonObj)(ptpmgmt_json j, const void *jobj);
    /**
     * Get management ID
     * @return management ID
     */
    enum ptpmgmt_mng_vals_e(*managementId)(const_ptpmgmt_json j);
    /**
     * Get dataField
     * @return dataField pointer or null
     * @note User @b should not try to free this TLV object
     */
    const void *(*dataField)(ptpmgmt_json j);
    /**
     * Get action field
     * @return action field
     */
    enum ptpmgmt_actionField_e(*actionField)(const_ptpmgmt_json j);
    /**
     * Get Unicast flag
     * @return Unicast flag
     */
    bool (*isUnicast)(const_ptpmgmt_json j);
    /**
     * Is Unicast flag in JSON?
     * @return true if Unicast flag in JSON
     */
    bool (*haveIsUnicast)(const_ptpmgmt_json j);
    /**
     * Get PTP profile specific
     * @return PTP profile specific
     */
    uint8_t (*PTPProfileSpecific)(const_ptpmgmt_json j);
    /**
     * Is PTP profile specific in JSON?
     * @return true if PTP profile specific in JSON
     */
    bool (*havePTPProfileSpecific)(const_ptpmgmt_json j);
    /**
     * Get domain number
     * @return domain number
     */
    uint8_t (*domainNumber)(const_ptpmgmt_json j);
    /**
     * Is domain number in JSON?
     * @return true if domain number in JSON
     */
    bool (*haveDomainNumber)(const_ptpmgmt_json j);
    /**
     * Get PTP major version
     * @return PTP major version
     */
    uint8_t (*versionPTP)(const_ptpmgmt_json j);
    /**
     * Is PTP major version in JSON?
     * @return true if PTP major version in JSON
     */
    bool (*haveVersionPTP)(const_ptpmgmt_json j);
    /**
     * Get PTP minor version
     * @return PTP minor version
     */
    uint8_t (*minorVersionPTP)(const_ptpmgmt_json j);
    /**
     * Is PTP minor version in JSON?
     * @return true if PTP minor version in JSON
     */
    bool (*haveMinorVersionPTP)(const_ptpmgmt_json j);
    /**
     * Get sequence ID
     * @return sequence ID
     */
    uint16_t (*sequenceId)(const_ptpmgmt_json j);
    /**
     * Is sequence ID in JSON?
     * @return true if sequence ID in JSON
     */
    bool (*haveSequenceId)(const_ptpmgmt_json j);
    /**
     * Get sdoId
     * @return sdoId
     */
    uint32_t (*sdoId)(const_ptpmgmt_json j);
    /**
     * Is sdoId in JSON?
     * @return true if sdoId in JSON
     */
    bool (*haveSdoId)(const_ptpmgmt_json j);
    /**
     * Get source port ID
     * @return source port ID
     */
    const struct ptpmgmt_PortIdentity_t *(*srcPort)(ptpmgmt_json j);
    /**
     * Is source port ID in JSON?
     * @return true if source port ID in JSON
     */
    bool (*haveSrcPort)(const_ptpmgmt_json j);
    /**
     * Get target port ID
     * @return target port ID
     */
    const struct ptpmgmt_PortIdentity_t *(*dstPort)(ptpmgmt_json j);
    /**
     * Is target port ID in JSON?
     * @return true if target port ID in JSON
     */
    bool (*haveDstPort)(const_ptpmgmt_json j);
    /**
     * Call message setAction with parsed JSON
     * @return message setAction result
     */
    bool (*setAction)(const_ptpmgmt_json j, ptpmgmt_msg message);
};

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
bool ptpmgmt_json_selectLib(const char *libName);
/**
 * Return shared library name
 * @return shared library name or nullptr in not
 * @note When using static link, function return the shared library name
 *       of the same code!
 */
const char *ptpmgmt_json_loadLibrary();
/**
 * Determine if library is shared or static
 * @return true if library is shared library
 */
bool ptpmgmt_json_isLibShared();
/**
 * Alocate new json structure
 * @return new json structure or null in case of error
 * @note: C interface
 */
ptpmgmt_json ptpmgmt_json_alloc();

#endif /* __PTPMGMT_C_JSON_H */
