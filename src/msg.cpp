/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Create and parse PTP management messages
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 * Created following "IEEE Std 1588-2008", PTP version 2
 */

#include <cmath>
#include "msg.h"
#include "c/msg.h"
#include "timeCvrt.h"
#include "comp.h"

__PTPMGMT_NAMESPACE_BEGIN

const uint8_t ptp_major_ver = 0x2; // low Nibble, portDS.versionNumber
// IEEE 1588-2019 uses PTP minor version 1
const uint8_t controlFieldMng = 0x04; // For Management
// For Delay_Req, Signalling, Management, Pdelay_Resp, Pdelay_Resp_Follow_Up
const uint8_t logMessageIntervalDef = 0x7f;
const uint16_t allPorts = UINT16_MAX;
const ClockIdentity_t allClocks = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

enum flagField_e {
    unicastFlag = 1 << 2,
    PTPProfileSpecific1 = 1 << 5,
    PTPProfileSpecific2 = 1 << 6,
};
enum allowAction_e { // bits of actionField_e
    A_GET = 1 << GET,
    A_SET = 1 << SET,
    A_COMMAND = 1 << COMMAND,
    A_USE_LINUXPTP = 1 << 5, // Out side of actionField_e
};
enum scope_e : uint8_t {
    s_port,  // 'PTP port' in IEEE Std 1588-2019
    s_clock, // 'PTP Instance' in IEEE Std 1588-2019
};

PACK(struct ClockIdentity_p {
    Octet_t v[8];
});
PACK(struct PortIdentity_p {
    ClockIdentity_p clockIdentity;
    UInteger16_t portNumber;
});
PACK(struct managementMessage_p {
    // Header 34 Octets
    Nibble_t       messageType_majorSdoId; // majorSdoId == transportSpecific;
    Nibble_t       versionPTP; // minorVersionPTP | versionPTP
    UInteger16_t   messageLength;
    UInteger8_t    domainNumber;
    UInteger8_t    minorSdoId;
    Octet_t        flagField[2]; // [1] is always 0 for management
    Integer64_t    correctionField; // always 0 for management and Signaling
    Octet_t        messageTypeSpecific[4];
    PortIdentity_p sourcePortIdentity;
    UInteger16_t   sequenceId;
    UInteger8_t    controlField;
    Integer8_t     logMessageInterval;
    // Management message
    PortIdentity_p targetPortIdentity;
    uint8_t        startingBoundaryHops;
    uint8_t        boundaryHops;
    uint8_t        actionField; // low Nibble
    uint8_t        res5;
});
// The AUTHENTICATION TLV may be appended to all
//  PTP messages according to the security policy.
// Definition of the AUTHENTICATION TLV
PACK(struct authenticationTlv_p {
    uint16_t       tlvType;     // tlvType_e.AUTHENTICATION
    uint16_t       lengthField; //
    UInteger8_t    spp;         // Security Parameter Pointe
    Octet_t        secParamIndicator; // Security Parameter Indicator
    UInteger32_t   keyID;       // Key Identifier or Current Key
    // The follow are uint16_t based
    // disclosedKey (optional) Disclosed key from previous interval
    // sequenceNo   (optional) Sequence number
    // Reserved     (optional)
    // ICV                      Integrity Check Value
});
const size_t authTlvSize = sizeof(authenticationTlv_p);
const ssize_t sigBaseSize = 34 + sizeof(PortIdentity_p);
const uint16_t tlvSizeHdr = 4;
PACK(struct managementTLV_t {
    // TLV header 4 Octets
    uint16_t tlvType;      // tlvType_e.MANAGEMENT
    uint16_t lengthField;  // lengthFieldMngBase + dataField length
    // Management part
    uint16_t managementId; // mng_all_vals.value
    // dataField is even length
});
const uint16_t lengthFieldMngBase = sizeof(managementTLV_t) - tlvSizeHdr;
PACK(struct managementErrorTLV_p {
    uint16_t managementErrorId; // managementErrorId_e
    uint16_t managementId;      // mng_all_vals.value
    uint32_t reserved;
    // displayData              // PTPText
});
const size_t mngMsgBaseSize = sizeof(managementMessage_p) +
    sizeof(managementTLV_t);
const ssize_t mngMsgMinSize = sizeof(managementMessage_p) + tlvSizeHdr;

/** Properties of a PTP management TLV */
struct ManagementId_t {
    uint16_t value;   /**< managementId value */
    uint8_t scope;   /**< Applies port or clock using scope_e */
    uint8_t allowed; /**< Allowed actions, bits from allowAction_e */
    /**
     * TLV dataField size
     * @li positive size of TLV dataField when it is fixed length
     * @li 0 for TLV without dataField
     * @li -1 TLV is not supported
     * @li -2 The dataField length need calculation based on values
     * @note the size might be even. Just pad another byte at the end.
     */
    ssize_t size;
};

// Shortcuts for the ids table
#define use_GSC A_GET | A_SET | A_COMMAND
#define use_GS  A_GET | A_SET
#define use_GL  A_GET | A_USE_LINUXPTP
#define use_GSL A_GET | A_SET | A_USE_LINUXPTP
#define use_CL A_COMMAND | A_USE_LINUXPTP
const static ManagementId_t mng_all_vals[] = {
#define A(n, v, sc, a, sz, f)\
    [n] = {.value = 0x##v, .scope = s_##sc, .allowed = a, .size = sz},
#include "ids.h"
};
bool Message::findTlvId(uint16_t val, mng_vals_e &rid, implementSpecific_e spec)
{
    mng_vals_e id;
    uint16_t value = net_to_cpu16(val);
    switch(value) {
#define A(n, v, sc, a, sz, f) case 0x##v: id = n; break;
#include "ids.h"
        default:
            return false;
    }
    /* block linuxptp if it is not used */
    if(spec != linuxptp && mng_all_vals[id].allowed & A_USE_LINUXPTP)
        return false;
    rid = id;
    return true;
}
bool Message::checkReplyAction(uint8_t actionField)
{
    uint8_t allowed = mng_all_vals[m_replayTlv_id].allowed;
    if(actionField == ACKNOWLEDGE)
        return allowed & A_COMMAND;
    else if(actionField == RESPONSE)
        return allowed & (A_SET | A_GET);
    return false;
}
bool Message::allowedAction(mng_vals_e id, actionField_e action)
{
    switch(action) {
        case GET:
            break;
        case SET:
            break;
        case COMMAND:
            break;
        default:
            return false;
    }
    if(id < FIRST_MNG_ID || id >= LAST_MNG_ID)
        return false;
    if(m_prms.implementSpecific != linuxptp &&
        mng_all_vals[id].allowed & A_USE_LINUXPTP)
        return false;
    return mng_all_vals[id].allowed & (1 << action);
}
Message::Message() :
    m_sigTlvs(*this),
    m_init(m_peer),
    m_init(m_target)
{
}
Message::Message(const MsgParams &prms) :
    m_sigTlvs(*this),
    m_prms(prms),
    m_init(m_peer),
    m_init(m_target)
{
    if(m_prms.transportSpecific > 0xf)
        m_prms.transportSpecific = 0;
}
Message::~Message()
{
    m_hmac.reset(nullptr); // delete m_hmac
    hmac_freeLib();
}
ssize_t Message::getMsgPlanedLen() const
{
    // That should not happen, precaution
    if(m_tlv_id < FIRST_MNG_ID || m_tlv_id >= LAST_MNG_ID)
        return -1; // Not supported
    const BaseMngTlv *tlv = nullptr;
    size_t addAuth = (m_haveAuth && m_prms.sendAuth) ? authTlvSize +
        m_sa.spp(m_sppID).mac_size(m_keyID) : 0;
    if(m_sendAction != GET)
        tlv = m_dataSend;
    else if(m_prms.useZeroGet)
        return mngMsgBaseSize + addAuth; // GET with zero dataField!
    ssize_t ret = mng_all_vals[m_tlv_id].size;
    if(ret == -2) { // variable length TLV
        if(tlv == nullptr && m_sendAction != GET)
            return -2; // SET and COMMAND must have data
        ret = dataFieldSize(tlv); // Calculate variable length
    }
    if(ret < 0)
        return ret;
    // Function return:
    //  -1  tlv (m_tlv_id) is not supported
    //  -2  tlv (m_tlv_id) can not be calculate
    if(ret & 1) // Ensure even size for calculated size
        ret++;
    return ret + mngMsgBaseSize + addAuth;
    // return total length of to the message to be send
}
const MsgParams &Message::getParams() const
{
    return m_prms;
}
bool Message::updateParams(const MsgParams &prms)
{
    if(prms.transportSpecific > 0xf)
        return false;
    m_prms = prms;
    return true;
}
bool Message::useAuth(const ConfigFile &cfg, const string &section)
{
    uint32_t keyID = cfg.active_key_id(section);
    if(cfg.haveSpp(section) && keyID > 0) {
        SaFile sa;
        if(sa.read_sa(cfg, section))
            return useAuth(sa, cfg.spp(section), keyID);
    }
    return false;
}
bool Message::useAuth(const SaFile &sa, uint8_t sppID, uint32_t keyID)
{
    if(sa.have(sppID, keyID)) {
        const Spp &s = sa.spp(sppID);
        HMAC_Key *hmac = hmac_allocHMAC(s.htype(keyID), s.key(keyID));
        if(hmac == nullptr)
            return false;
        m_hmac.reset(hmac);
        m_sa = sa;
        m_sppID = sppID;
        m_keyID = keyID;
        m_haveAuth = true;
        return true;
    }
    return false;
}
bool Message::changeAuth(uint8_t sppID, uint32_t keyID)
{
    if(m_haveAuth && m_sa.have(sppID, keyID)) {
        const Spp &s = m_sa.spp(sppID);
        HMAC_Key *hmac = hmac_allocHMAC(s.htype(keyID), s.key(keyID));
        if(hmac == nullptr)
            return false;
        m_hmac.reset(hmac);
        m_sppID = sppID;
        m_keyID = keyID;
        return true;
    }
    return false;
}
bool Message::changeAuth(uint32_t keyID)
{
    if(m_haveAuth && m_sa.have(m_sppID, keyID)) {
        const Spp &s = m_sa.spp(m_sppID);
        HMAC_Key *hmac = hmac_allocHMAC(s.htype(keyID), s.key(keyID));
        if(hmac == nullptr)
            return false;
        m_hmac.reset(hmac);
        m_keyID = keyID;
        return true;
    }
    return false;
}
bool Message::is_LI_61(uint8_t flags)
{
    return (flags & F_LI_61) > 0;
}
bool Message::is_LI_59(uint8_t flags)
{
    return (flags & F_LI_59) > 0;
}
bool Message::is_UTCV(uint8_t flags)
{
    return (flags & F_UTCV) > 0;
}
bool Message::is_PTP(uint8_t flags)
{
    return (flags & F_PTP) > 0;
}
bool Message::is_TTRA(uint8_t flags)
{
    return (flags & F_TTRA) > 0;
}
bool Message::is_FTRA(uint8_t flags)
{
    return (flags & F_FTRA) > 0;
}
bool Message::isEmpty(mng_vals_e id)
{
    return id >= FIRST_MNG_ID && id < LAST_MNG_ID && mng_all_vals[id].size == 0;
}
bool Message::isValidId(mng_vals_e id) const
{
    if(id < FIRST_MNG_ID || id >= LAST_MNG_ID)
        return false;
    if(m_prms.implementSpecific != linuxptp &&
        mng_all_vals[id].allowed & A_USE_LINUXPTP)
        return false;
    return true;
}
bool Message::verifyTlv(mng_vals_e tlv_id, const BaseMngTlv *tlv)
{
    if(tlv == nullptr)
        return false;
#define _ptpmCaseNA(n) case n: break;
#define _ptpmCaseUF(n) case n: {\
            const n##_t *t = dynamic_cast<const n##_t *>(tlv);\
            return t != nullptr; }
    switch(tlv_id) {
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n);
#include "ids.h"
        default:
            break;
    }
    return false;
}
bool Message::setAction(actionField_e actionField, mng_vals_e tlv_id,
    const BaseMngTlv *dataSend)
{
    if(!allowedAction(tlv_id, actionField))
        return false;
    if(tlv_id != NULL_PTP_MANAGEMENT && actionField != GET &&
        mng_all_vals[tlv_id].size != 0) {
        if(!verifyTlv(tlv_id, dataSend))
            return false;
        m_dataSend = dataSend;
    } else
        m_dataSend = nullptr;
    m_sendAction = actionField;
    m_tlv_id = tlv_id;
    return true;
}
void Message::clearData()
{
    if(m_dataSend != nullptr) {
        // Prevent use of SET or COMMAND, which need m_dataSend
        // If user want SET or COMMAND, he/she need to set the data to send!
        m_sendAction = GET;
        // Message do not allocate the object, so do not delete it!
        m_dataSend = nullptr;
    }
}
MNG_PARSE_ERROR_e Message::build(void *buf, size_t bufSize, uint16_t sequence)
{
    if(buf == nullptr || bufSize < mngMsgBaseSize)
        return MNG_PARSE_ERROR_TOO_SMALL;
    managementMessage_p *msg = (managementMessage_p *)buf;
    *msg = {0};
    msg->messageType_majorSdoId = (Management |
            (m_prms.transportSpecific << 4)) & UINT8_MAX;
    msg->versionPTP = (m_prms.minorVersion << 4) | ptp_major_ver;
    msg->domainNumber = m_prms.domainNumber;
    if(m_prms.isUnicast)
        msg->flagField[0] |= unicastFlag;
    msg->sequenceId = cpu_to_net16(sequence);
    // Only needed for PTP V1 hardware with IPv4.
    // Keep as backward with old hardware.
    msg->controlField = controlFieldMng;
    msg->logMessageInterval = logMessageIntervalDef;
    msg->startingBoundaryHops = m_prms.boundaryHops;
    msg->boundaryHops = m_prms.boundaryHops;
    msg->actionField = m_sendAction;
    memcpy(msg->targetPortIdentity.clockIdentity.v, m_prms.target.clockIdentity.v,
        m_prms.target.clockIdentity.size());
    msg->targetPortIdentity.portNumber = cpu_to_net16(m_prms.target.portNumber);
    memcpy(msg->sourcePortIdentity.clockIdentity.v, m_prms.self_id.clockIdentity.v,
        m_prms.self_id.clockIdentity.size());
    msg->sourcePortIdentity.portNumber = cpu_to_net16(m_prms.self_id.portNumber);
    managementTLV_t *tlv = (managementTLV_t *)(msg + 1);
    tlv->tlvType = cpu_to_net16(MANAGEMENT);
    tlv->managementId = cpu_to_net16(mng_all_vals[m_tlv_id].value);
    MsgProc mp;
    mp.m_cur = (uint8_t *)(tlv + 1); // point on dataField
    size_t size = mngMsgBaseSize;
    mp.m_left = bufSize - size;
    ssize_t tlvSize = mng_all_vals[m_tlv_id].size;
    if(m_sendAction != GET && m_dataSend != nullptr) {
        mp.m_build = true;
        // call_tlv_data() do not change data on build,
        // but does on parsing!
        BaseMngTlv *tlv = const_cast<BaseMngTlv *>(m_dataSend);
        MNG_PARSE_ERROR_e err = mp.call_tlv_data(m_tlv_id, tlv);
        if(err != MNG_PARSE_ERROR_OK)
            return err;
        // Add 'reserve' at end of message
        if((mp.m_size & 1) && mp.procRes()) // length need to be even
            return MNG_PARSE_ERROR_TOO_SMALL;
    } else if(m_sendAction == GET && !m_prms.useZeroGet && tlvSize != 0) {
        if(tlvSize == -2)
            tlvSize = dataFieldSize(nullptr); // Calculate empty variable length
        if(tlvSize < 0)
            return MNG_PARSE_ERROR_INVALID_ID;
        if(tlvSize & 1)
            tlvSize++;
        if(tlvSize > mp.m_left)
            return MNG_PARSE_ERROR_TOO_SMALL;
        memset(mp.m_cur, 0, tlvSize);
        mp.move(tlvSize);
    }
    size += mp.m_size;
    if(size & 1) // length need to be even
        return MNG_PARSE_ERROR_SIZE;
    tlv->lengthField = cpu_to_net16(lengthFieldMngBase + mp.m_size);
    if(m_haveAuth && m_prms.sendAuth) {
        const Spp &s = m_sa.spp(m_sppID);
        size_t mac_size = s.mac_size(m_keyID);
        size_t sz_wo_icv = authTlvSize;
        // Add authentication optional length here!
        if((ssize_t)(sz_wo_icv + mac_size) > mp.m_left)
            return MNG_PARSE_ERROR_TOO_SMALL;
        authenticationTlv_p *atlv = (authenticationTlv_p *)mp.m_cur;
        atlv->tlvType = cpu_to_net16(AUTHENTICATION);
        atlv->lengthField = cpu_to_net16(sz_wo_icv + mac_size - tlvSizeHdr);
        atlv->spp = m_sppID;
        atlv->secParamIndicator = 0;
        atlv->keyID = cpu_to_net32(m_keyID);
        // Assign authentication optional fields here
        mp.move(sz_wo_icv);
        size += sz_wo_icv + mac_size;
        // We must update the length before calculate the ICV
        msg->messageLength = cpu_to_net16(size);
        Binary mac(mac_size);
        if(!m_hmac->digest(buf, size - mac_size, mac))
            return MNG_PARSE_ERROR_AUTH;
        mac.copy(mp.m_cur); // Copy the ICV to location
    } else
        msg->messageLength = cpu_to_net16(size);
    m_msgLen = size;
    return MNG_PARSE_ERROR_OK;
}
MNG_PARSE_ERROR_e Message::build(Buf &buf, uint16_t sequence)
{
    return build(buf.get(), buf.size(), sequence);
}
actionField_e Message::getSendAction() const
{
    return m_sendAction;
}
size_t Message::getMsgLen() const
{
    return m_msgLen;
}
MNG_PARSE_ERROR_e Message::parse(const void *buf, ssize_t bufSize)
{
    if(bufSize < sigBaseSize)
        return MNG_PARSE_ERROR_TOO_SMALL;
    managementMessage_p *msg = (managementMessage_p *)buf;
    ssize_t msgSize = (ssize_t)net_to_cpu16(msg->messageLength);
    if(msgSize > bufSize)
        return MNG_PARSE_ERROR_HEADER;
    m_type = (msgType_e)(msg->messageType_majorSdoId & 0xf);
    switch(m_type) {
        case Signaling:
            if(!m_prms.rcvSignaling)
                return MNG_PARSE_ERROR_HEADER;
            break;
        case Management:
            if(msgSize < mngMsgMinSize)
                return MNG_PARSE_ERROR_TOO_SMALL;
            break;
        default:
            return MNG_PARSE_ERROR_HEADER;
    }
    m_versionPTP = msg->versionPTP & 0xf;
    m_minorVersionPTP = msg->versionPTP >> 4;
    if(m_versionPTP != ptp_major_ver ||
        msg->logMessageInterval != logMessageIntervalDef)
        return MNG_PARSE_ERROR_HEADER;
    m_sdoId = msg->minorSdoId | ((msg->messageType_majorSdoId & 0xf0) << 4);
    m_domainNumber = msg->domainNumber;
    m_isUnicast = msg->flagField[0] & unicastFlag;
    m_PTPProfileSpecific = msg->flagField[0] &
        (PTPProfileSpecific1 | PTPProfileSpecific2);
    m_sequence = net_to_cpu16(msg->sequenceId);
    m_peer.portNumber = net_to_cpu16(msg->sourcePortIdentity.portNumber);
    memcpy(m_peer.clockIdentity.v, msg->sourcePortIdentity.clockIdentity.v,
        m_peer.clockIdentity.size());
    // Exist in both Management and signalling
    m_target.portNumber = net_to_cpu16(msg->targetPortIdentity.portNumber);
    memcpy(m_target.clockIdentity.v, msg->targetPortIdentity.clockIdentity.v,
        m_target.clockIdentity.size());
    MsgProc mp;
    if(m_type == Signaling) {
        // Real initializing
        mp.m_cur = (uint8_t *)buf + sigBaseSize;
        mp.m_size = msgSize - sigBaseSize; // pass left to parseSig()
        return parseSig(buf, &mp);
    }
    m_sigTlvs.m_lastSig = false;
    // Management message part
    uint8_t actionField = 0xf & msg->actionField;
    if(actionField != RESPONSE && actionField != ACKNOWLEDGE &&
        actionField != COMMAND)
        return MNG_PARSE_ERROR_ACTION;
    uint16_t *cur = (uint16_t *)(msg + 1);
    uint16_t tlvType = net_to_cpu16(*cur++);
    m_mngType = (tlvType_e)tlvType;
    uint16_t tlvSize = net_to_cpu16(*cur++); // lengthField
    msgSize -= mngMsgMinSize;
    if(msgSize < (ssize_t)tlvSize || (tlvSize & 1) > 0)
        return MNG_PARSE_ERROR_TOO_SMALL;
    msgSize -= tlvSize;
    MNG_PARSE_ERROR_e errAuth = MNG_PARSE_ERROR_OK;
    if(m_haveAuth && (m_prms.rcvAuth & RCV_AUTH_MNG) > 0) {
        errAuth = parseAuth(buf, (uint8_t *)cur + tlvSize, msgSize, true);
        if(errAuth != MNG_PARSE_ERROR_OK && (m_prms.rcvAuth & RCV_AUTH_IGNORE) == 0)
            return errAuth;
    }
    BaseMngTlv *tlv;
    mp.m_left = tlvSize;
    MNG_PARSE_ERROR_e err;
    switch(tlvType) {
        case MANAGEMENT_ERROR_STATUS:
            if(actionField != RESPONSE && actionField != ACKNOWLEDGE)
                return MNG_PARSE_ERROR_ACTION;
            m_replyAction = (actionField_e)actionField;
            managementErrorTLV_p *errTlv;
            // check minimum size and even
            if(mp.m_left < (ssize_t)sizeof(*errTlv))
                return MNG_PARSE_ERROR_TOO_SMALL;
            errTlv = (managementErrorTLV_p *)cur;
            if(!findTlvId(errTlv->managementId, m_replayTlv_id,
                    m_prms.implementSpecific))
                return MNG_PARSE_ERROR_INVALID_ID;
            if(!checkReplyAction(actionField))
                return MNG_PARSE_ERROR_ACTION;
            m_errorId =
                (managementErrorId_e)net_to_cpu16(errTlv->managementErrorId);
            mp.m_left -= sizeof(*errTlv);
            mp.m_cur = (uint8_t *)(errTlv + 1);
            // Check displayData size
            if(mp.m_left > 1 && mp.proc(m_errorDisplay))
                return MNG_PARSE_ERROR_TOO_SMALL;
            if(errAuth != MNG_PARSE_ERROR_OK)
                return errAuth;
            return MNG_PARSE_ERROR_MSG;
        case MANAGEMENT:
            if(actionField != RESPONSE && actionField != ACKNOWLEDGE)
                return MNG_PARSE_ERROR_ACTION;
            m_replyAction = (actionField_e)actionField;
            // Check minimum size and even
            if(mp.m_left < lengthFieldMngBase)
                return MNG_PARSE_ERROR_TOO_SMALL;
            // managementId
            if(!findTlvId(*cur++, m_replayTlv_id, m_prms.implementSpecific))
                return MNG_PARSE_ERROR_INVALID_ID;
            if(!checkReplyAction(actionField))
                return MNG_PARSE_ERROR_ACTION;
            mp.m_left -= lengthFieldMngBase;
            if(mp.m_left == 0)
                return errAuth;
            mp.m_cur = (uint8_t *)cur;
            err = mp.call_tlv_data(m_replayTlv_id, tlv);
            if(err != MNG_PARSE_ERROR_OK)
                return err;
            m_dataGet.reset(tlv);
            return errAuth;
        case ORGANIZATION_EXTENSION:
            if(m_prms.rcvSMPTEOrg) {
                // rcvSMPTEOrg uses the COMMAND message
                if(actionField != COMMAND)
                    return MNG_PARSE_ERROR_ACTION;
                m_replyAction = COMMAND;
                mp.m_cur = (uint8_t *)cur;
                SMPTE_ORGANIZATION_EXTENSION_t *tlvOrg;
                tlvOrg = new SMPTE_ORGANIZATION_EXTENSION_t;
                if(tlvOrg == nullptr)
                    return MNG_PARSE_ERROR_MEM;
                if(mp.SMPTE_ORGANIZATION_EXTENSION_f(*tlvOrg)) {
                    delete tlvOrg;
                    return mp.m_err;
                }
                m_dataGet.reset(tlvOrg);
                m_replayTlv_id = SMPTE_MNG_ID;
                if(errAuth != MNG_PARSE_ERROR_OK)
                    return errAuth;
                return MNG_PARSE_ERROR_SMPTE;
            }
            FALLTHROUGH;
        default:
            break;
    }
    return MNG_PARSE_ERROR_INVALID_TLV;
}
MNG_PARSE_ERROR_e Message::parse(const Buf &buf, ssize_t bufSize)
{
    // That should not happens!
    // As user used too big size in the recieve function
    // But if it does, we need protection!
    return (ssize_t)buf.size() < bufSize ? MNG_PARSE_ERROR_TOO_SMALL :
        parse(buf.get(), bufSize);
}
MNG_PARSE_ERROR_e Message::parseAuth(const void *buf, const void *auth,
    ssize_t left, bool check)
{
    if(left < (ssize_t)authTlvSize)
        return MNG_PARSE_ERROR_AUTH_NONE;
    authenticationTlv_p *atlv = (authenticationTlv_p *)auth;
    if(check && net_to_cpu16(atlv->tlvType) != AUTHENTICATION)
        return MNG_PARSE_ERROR_AUTH_NONE;
    uint8_t spp = atlv->spp;
    uint32_t keyID = net_to_cpu32(atlv->keyID);
    if(!m_sa.have(spp, keyID))
        return MNG_PARSE_ERROR_AUTH_NOKEY;
    size_t len;
    if(check) {
        len = (ssize_t)net_to_cpu16(atlv->lengthField) + tlvSizeHdr;
        if((ssize_t)len > left)
            return MNG_PARSE_ERROR_AUTH;
        if(len < authTlvSize)
            return MNG_PARSE_ERROR_AUTH;
    } else
        len = left;
    len -= authTlvSize;
    // Reduce authentication optional length here!
    if(atlv->secParamIndicator != 0)
        return MNG_PARSE_ERROR_AUTH; // We do not support optional
    if((len & 1) > 0 || len < 2)
        return MNG_PARSE_ERROR_AUTH;
    // Delete hmac once the function exit :-)
    unique_ptr<HMAC_Key> hmac_alloc;
    HMAC_Key *hmac;
    const Spp &s = m_sa.spp(spp);
    if(len > s.mac_size(m_keyID))
        return MNG_PARSE_ERROR_AUTH_WRONG;
    if(m_sppID == spp && m_keyID == keyID)
        // Using the same key for sending
        hmac = m_hmac.get();
    else {
        hmac = hmac_allocHMAC(s.htype(keyID), s.key(keyID));
        if(hmac == nullptr)
            return MNG_PARSE_ERROR_AUTH;
        // We allocate, we will delete on exit
        hmac_alloc.reset(hmac);
    }
    // Add authentication optional length here
    uint8_t *icv = (uint8_t *)(atlv + 1);
    size_t size = icv - (uint8_t *)buf;
    Binary mac(icv, len);
    // Verify the ICV
    if(!hmac->verify(buf, size, mac)) {
        // Calculate for unit test
        //if(hmac->digest(buf, size, mac))
        //    printf("Should be ICV: 0x%s\n", mac.toId(", 0x").c_str());
        return MNG_PARSE_ERROR_AUTH_WRONG;
    }
    return MNG_PARSE_ERROR_OK;
}
#define caseBuildAct(n) {\
        n##_t *t = new n##_t;\
        if(t == nullptr)\
            return MNG_PARSE_ERROR_MEM;\
        if(mp.n##_f(*t)) {\
            delete t;\
            return mp.m_err;\
        }\
        tlv = t;\
        break;\
    }
#define caseBuild(n) n: caseBuildAct(n)
MNG_PARSE_ERROR_e Message::parseSig(const void *buf, MsgProc *pMp)
{
    MsgProc &mp = *pMp;
    ssize_t leftAll = mp.m_size;
    m_sigTlvs.clearToUse();
    tlvType_e lastTlv = (tlvType_e)0;
    void *lastAuth = nullptr;
    uint16_t lastAuthLen = 0;
    MNG_PARSE_ERROR_e errAuth = MNG_PARSE_ERROR_OK;
    while(leftAll >= tlvSizeHdr) {
        uint16_t *cur = (uint16_t *)mp.m_cur;
        tlvType_e tlvType = (tlvType_e)net_to_cpu16(*cur++);
        uint16_t lengthField = net_to_cpu16(*cur);
        lastTlv = tlvType;
        mp.m_cur += tlvSizeHdr;
        leftAll -= tlvSizeHdr;
        if(lengthField > leftAll)
            return MNG_PARSE_ERROR_TOO_SMALL;
        leftAll -= lengthField;
        if(tlvType == AUTHENTICATION) {
            if(m_haveAuth) {
                lastAuthLen = lengthField + tlvSizeHdr;
                lastAuth = cur - 1;
                if((m_prms.rcvAuth & RCV_AUTH_SIG_ALL) > 0) {
                    errAuth = parseAuth(buf, lastAuth, lastAuthLen);
                    if(errAuth != MNG_PARSE_ERROR_OK &&
                        (m_prms.rcvAuth & RCV_AUTH_IGNORE) == 0)
                        return errAuth;
                }
            }
            continue;
        }
        // Check signalling filter
        if(m_prms.filterSignaling && !m_prms.isSigTlv(tlvType)) {
            // TLV not in filter is skiped
            mp.m_cur += lengthField;
            continue;
        }
        mp.m_left = lengthField; // for build functions
        BaseSigTlv *tlv = nullptr;
        mng_vals_e managementId;
        managementErrorTLV_p *errTlv;
        switch(tlvType) {
            case ORGANIZATION_EXTENSION_PROPAGATE:
                FALLTHROUGH;
            case ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE:
                FALLTHROUGH;
            case caseBuild(ORGANIZATION_EXTENSION);
            case caseBuild(PATH_TRACE);
            case caseBuild(ALTERNATE_TIME_OFFSET_INDICATOR);
            case caseBuild(ENHANCED_ACCURACY_METRICS);
            case caseBuild(L1_SYNC);
            case caseBuild(PORT_COMMUNICATION_AVAILABILITY);
            case caseBuild(PROTOCOL_ADDRESS);
            case caseBuild(SLAVE_RX_SYNC_TIMING_DATA);
            case caseBuild(SLAVE_RX_SYNC_COMPUTED_DATA);
            case caseBuild(SLAVE_TX_EVENT_TIMESTAMPS);
            case caseBuild(CUMULATIVE_RATE_RATIO);
            case MANAGEMENT_ERROR_STATUS:
                if(mp.m_left < (ssize_t)sizeof(*errTlv))
                    return MNG_PARSE_ERROR_TOO_SMALL;
                errTlv = (managementErrorTLV_p *)mp.m_cur;
                if(findTlvId(errTlv->managementId, managementId,
                        m_prms.implementSpecific)) {
                    MANAGEMENT_ERROR_STATUS_t *d = new MANAGEMENT_ERROR_STATUS_t;
                    if(d == nullptr)
                        return MNG_PARSE_ERROR_MEM;
                    mp.m_cur += sizeof(*errTlv);
                    mp.m_left -= sizeof(*errTlv);
                    if(mp.m_left > 1 && mp.proc(d->displayData)) {
                        delete d;
                        return MNG_PARSE_ERROR_TOO_SMALL;
                    }
                    d->managementId = managementId;
                    d->managementErrorId = (managementErrorId_e)
                        net_to_cpu16(errTlv->managementErrorId);
                    tlv = d;
                }
                break;
            case MANAGEMENT:
                if(mp.m_left < 2)
                    return MNG_PARSE_ERROR_TOO_SMALL;
                // Ignore empty and unknown management TLVs
                if(findTlvId(*(uint16_t *)mp.m_cur, managementId,
                        m_prms.implementSpecific) && mp.m_left > 2) {
                    mp.m_cur += 2; // 2 bytes of managementId
                    mp.m_left -= 2;
                    BaseMngTlv *mtlv;
                    MNG_PARSE_ERROR_e err = mp.call_tlv_data(managementId, mtlv);
                    if(err != MNG_PARSE_ERROR_OK)
                        return err;
                    MANAGEMENT_t *d = new MANAGEMENT_t;
                    if(d == nullptr) {
                        delete mtlv;
                        return MNG_PARSE_ERROR_MEM;
                    }
                    d->managementId = managementId;
                    d->tlvData.reset(mtlv);
                    tlv = d;
                }
                break;
            case SLAVE_DELAY_TIMING_DATA_NP:
                if(m_prms.implementSpecific == linuxptp)
                    caseBuildAct(SLAVE_DELAY_TIMING_DATA_NP);
                break;
            default: // Ignore TLV
                break;
        }
        if(mp.m_left > 0)
            mp.m_cur += mp.m_left;
        m_sigTlvs.push(tlvType, tlv);
    }
    if(m_haveAuth &&
        (m_prms.rcvAuth & (RCV_AUTH_SIG_ALL | RCV_AUTH_SIG_LAST)) > 0) {
        if(lastTlv != AUTHENTICATION)
            return MNG_PARSE_ERROR_AUTH_NONE;
        if((m_prms.rcvAuth & RCV_AUTH_SIG_ALL) == 0)
            errAuth = parseAuth(buf, lastAuth, lastAuthLen);
        if(errAuth != MNG_PARSE_ERROR_OK)
            return errAuth;
    }
    return MNG_PARSE_ERROR_SIG; // We have signalling message
}
actionField_e Message::getReplyAction() const
{
    return m_replyAction;
}
bool Message::isUnicast() const
{
    return m_isUnicast;
}
uint8_t Message::getPTPProfileSpecific() const
{
    return m_PTPProfileSpecific;
}
uint16_t Message::getSequence() const
{
    return m_sequence;
}
const PortIdentity_t &Message::getPeer() const
{
    return m_peer;
}
const PortIdentity_t &Message::getTarget() const
{
    return m_target;
}
uint32_t Message::getSdoId() const
{
    return m_sdoId;
}
uint8_t Message::getDomainNumber() const
{
    return m_domainNumber;
}
uint8_t Message::getVersionPTP() const
{
    return m_versionPTP;
}
uint8_t Message::getMinorVersionPTP() const
{
    return m_minorVersionPTP;
}
const BaseMngTlv *Message::getData() const
{
    return m_dataGet.get();
}
const BaseMngTlv *Message::getSendData() const
{
    return m_dataSend;
}
managementErrorId_e Message::getErrId() const
{
    return m_errorId;
}
const string &Message::getErrDisplay() const
{
    return m_errorDisplay.textField;
}
const char *Message::getErrDisplay_c() const
{
    return m_errorDisplay.string();
}
bool Message::isLastMsgSig() const
{
    return m_type == Signaling;
}
bool Message::isLastMsgSMPTE() const
{
    return m_type == Management && m_mngType == ORGANIZATION_EXTENSION &&
        m_replayTlv_id == SMPTE_MNG_ID;
}
msgType_e Message::getType() const
{
    return m_type;
}
tlvType_e Message::getMngType() const
{
    return m_mngType;
}
bool Message::traversSigTlvs(function<bool (const Message &msg,
        tlvType_e tlvType, const BaseSigTlv *tlv)> callback) const
{
    return m_sigTlvs.traverse(callback);
}
bool Message::traversSigTlvsCl(MessageSigTlvCallback &callback) const
{
    return m_sigTlvs.traverse(callback);
}
size_t Message::getSigTlvsCount() const
{
    return m_sigTlvs.size();
}
const BaseSigTlv *Message::getSigTlv(size_t pos) const
{
    return m_sigTlvs.get(pos);
}
tlvType_e Message::getSigTlvType(size_t pos) const
{
    return m_sigTlvs.getType(pos);
}
mng_vals_e Message::getSigMngTlvType(size_t pos) const
{
    return m_sigTlvs.getTlvId(pos);
}
const BaseMngTlv *Message::getSigMngTlv(size_t pos) const
{
    return m_sigTlvs.getMngTlv(pos);
}
const MessageSigTlvs &Message::getSigTlvs() const
{
    return m_sigTlvs;
}
bool Message::disableAuth()
{
    m_haveAuth = false;
    return true;
}
int Message::usedAuthSppID() const
{
    return m_haveAuth ? m_sppID : -1;
}
uint32_t Message::usedAuthKeyID() const
{
    return m_haveAuth ? m_keyID : 0;
}
const SaFile &Message::getSa() const
{
    return m_sa;
}
bool Message::haveAuth() const
{
    return m_haveAuth;
}
mng_vals_e Message::getTlvId() const
{
    return m_replayTlv_id;
}
mng_vals_e Message::getBuildTlvId() const
{
    return m_tlv_id;
}
void Message::setAllClocks()
{
    m_prms.target.portNumber = allPorts;
    m_prms.target.clockIdentity = allClocks;
}
bool Message::isAllClocks() const
{
    return m_prms.target.portNumber == allPorts &&
        memcmp(&m_prms.target.clockIdentity, &allClocks, sizeof allClocks) == 0;
}
bool Message::useConfig(const ConfigFile &cfg, const string &section)
{
    uint8_t transportSpecific = cfg.transportSpecific(section);
    if(transportSpecific > 0xf)
        return false;
    m_prms.transportSpecific = transportSpecific;
    m_prms.domainNumber = cfg.domainNumber(section);
    return true;
}
const char *Message::err2str_c(MNG_PARSE_ERROR_e err)
{
    switch(err) {
        case caseItem(MNG_PARSE_ERROR_OK);
        case caseItem(MNG_PARSE_ERROR_MSG);
        case caseItem(MNG_PARSE_ERROR_SIG);
        case caseItem(MNG_PARSE_ERROR_SMPTE);
        case caseItem(MNG_PARSE_ERROR_INVALID_ID);
        case caseItem(MNG_PARSE_ERROR_INVALID_TLV);
        case caseItem(MNG_PARSE_ERROR_MISMATCH_TLV);
        case caseItem(MNG_PARSE_ERROR_SIZE_MISS);
        case caseItem(MNG_PARSE_ERROR_TOO_SMALL);
        case caseItem(MNG_PARSE_ERROR_SIZE);
        case caseItem(MNG_PARSE_ERROR_VAL);
        case caseItem(MNG_PARSE_ERROR_HEADER);
        case caseItem(MNG_PARSE_ERROR_ACTION);
        case caseItem(MNG_PARSE_ERROR_UNSUPPORT);
        case caseItem(MNG_PARSE_ERROR_MEM);
        case caseItem(MNG_PARSE_ERROR_AUTH);
        case caseItem(MNG_PARSE_ERROR_AUTH_NONE);
        case caseItem(MNG_PARSE_ERROR_AUTH_WRONG);
        case caseItem(MNG_PARSE_ERROR_AUTH_NOKEY);
    }
    return "unknown";
}
const char *Message::type2str_c(msgType_e type)
{
    switch(type) {
        case caseItem(Sync);
        case caseItem(Delay_Req);
        case caseItem(Pdelay_Req);
        case caseItem(Pdelay_Resp);
        case caseItem(Follow_Up);
        case caseItem(Delay_Resp);
        case caseItem(Pdelay_Resp_Follow_Up);
        case caseItem(Announce);
        case caseItem(Signaling);
        case caseItem(Management);
    }
    return "unknown message type";
}
const char *Message::tlv2str_c(tlvType_e type)
{
    switch(type) {
        case caseItem(MANAGEMENT);
        case caseItem(MANAGEMENT_ERROR_STATUS);
        case caseItem(ORGANIZATION_EXTENSION);
        case caseItem(REQUEST_UNICAST_TRANSMISSION);
        case caseItem(GRANT_UNICAST_TRANSMISSION);
        case caseItem(CANCEL_UNICAST_TRANSMISSION);
        case caseItem(ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION);
        case caseItem(PATH_TRACE);
        case caseItem(ALTERNATE_TIME_OFFSET_INDICATOR);
        case caseItem(ORGANIZATION_EXTENSION_PROPAGATE);
        case caseItem(ENHANCED_ACCURACY_METRICS);
        case caseItem(ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE);
        case caseItem(L1_SYNC);
        case caseItem(PORT_COMMUNICATION_AVAILABILITY);
        case caseItem(PROTOCOL_ADDRESS);
        case caseItem(SLAVE_RX_SYNC_TIMING_DATA);
        case caseItem(SLAVE_RX_SYNC_COMPUTED_DATA);
        case caseItem(SLAVE_TX_EVENT_TIMESTAMPS);
        case caseItem(CUMULATIVE_RATE_RATIO);
        case caseItem(AUTHENTICATION);
        case caseItem(SLAVE_DELAY_TIMING_DATA_NP);
        case TLV_PAD:
            return "PAD";
    }
    return "unknown TLV";
}
const char *Message::act2str_c(actionField_e action)
{
    switch(action) {
        case caseItem(GET);
        case caseItem(SET);
        case caseItem(RESPONSE);
        case caseItem(COMMAND);
        case caseItem(ACKNOWLEDGE);
    }
    return "unknown";
}
const char *Message::mng2str_c(mng_vals_e id)
{
    switch(id) {
#define A(n, v, sc, a, sz, f) case n: return #n;
#include "ids.h"
        default:
            break;
    }
    return "unknown";
}
const bool Message::findMngID(const string &str, mng_vals_e &id, bool exact)
{
    if(str.empty())
        return false;
    int (*_strcmp)(const char *, const char *);
    if(!exact) {
        _strcmp = strcasecmp;
        if(strcasestr(str.c_str(), "NULL") != nullptr) {
            id = NULL_PTP_MANAGEMENT;
            return true;
        }
    } else
        _strcmp = strcmp; // Excect match
    int find = 0;
    for(int i = FIRST_MNG_ID; i < LAST_MNG_ID; i++) {
        mng_vals_e cid = (mng_vals_e)i;
        const char *sid = mng2str_c(cid);
        // A whole word match!
        if(_strcmp(str.c_str(), sid) == 0) {
            id = cid;
            return true;
        }
        // Partial match
        if(!exact && strcasestr(sid, str.c_str()) != nullptr) {
            id = cid;
            find++;
            // Once we have 2 partial match
            // We stick to a whole word match
            if(find > 1)
                exact = true;
        }
    }
    // We found 1 partial match :-)
    return find == 1;
}
const char *Message::errId2str_c(managementErrorId_e err)
{
    switch(err) {
        case caseItem(RESPONSE_TOO_BIG);
        case caseItem(NO_SUCH_ID);
        case caseItem(WRONG_LENGTH);
        case caseItem(WRONG_VALUE);
        case caseItem(NOT_SETABLE);
        case caseItem(NOT_SUPPORTED);
        case caseItem(GENERAL_ERROR);
    }
    return "unknown";
}
const char *Message::clkType2str_c(clockType_e val)
{
    switch(val) {
        case caseItem(ordinaryClock);
        case caseItem(boundaryClock);
        case caseItem(p2pTransparentClock);
        case caseItem(e2eTransparentClock);
        case caseItem(managementClock);
    }
    return "unknown";
}
const char *Message::netProt2str_c(networkProtocol_e val)
{
    switch(val) {
        case caseItem(UDP_IPv4);
        case caseItem(UDP_IPv6);
        case caseItem(IEEE_802_3);
        case caseItem(DeviceNet);
        case caseItem(ControlNet);
        case caseItem(PROFINET);
    }
    return "unknown";
}
const char *Message::clockAcc2str_c(clockAccuracy_e val)
{
    const size_t off = 9; // Remove prefix 'Accurate_'
    switch(val) {
        case caseItemOff(Accurate_within_1ps);
        case caseItemOff(Accurate_within_2_5ps);
        case caseItemOff(Accurate_within_10ps);
        case caseItemOff(Accurate_within_25ps);
        case caseItemOff(Accurate_within_100ps);
        case caseItemOff(Accurate_within_250ps);
        case caseItemOff(Accurate_within_1ns);
        case caseItemOff(Accurate_within_2_5ns);
        case caseItemOff(Accurate_within_10ns);
        case caseItemOff(Accurate_within_25ns);
        case caseItemOff(Accurate_within_100ns);
        case caseItemOff(Accurate_within_250ns);
        case caseItemOff(Accurate_within_1us);
        case caseItemOff(Accurate_within_2_5us);
        case caseItemOff(Accurate_within_10us);
        case caseItemOff(Accurate_within_25us);
        case caseItemOff(Accurate_within_100us);
        case caseItemOff(Accurate_within_250us);
        case caseItemOff(Accurate_within_1ms);
        case caseItemOff(Accurate_within_2_5ms);
        case caseItemOff(Accurate_within_10ms);
        case caseItemOff(Accurate_within_25ms);
        case caseItemOff(Accurate_within_100ms);
        case caseItemOff(Accurate_within_250ms);
        case caseItemOff(Accurate_within_1s);
        case caseItemOff(Accurate_within_10s);
        case caseItemOff(Accurate_more_10s);
        case caseItemOff(Accurate_Unknown);
        default:
            if(val < Accurate_within_1ps)
                return "Accurate_small_1ps";
            return "Accurate_unknown_val";
    }
}
const char *Message::faultRec2str_c(faultRecord_e val)
{
    const size_t off = 2; // Remove prefix 'F_'
    switch(val) {
        case caseItemOff(F_Emergency);
        case caseItemOff(F_Alert);
        case caseItemOff(F_Critical);
        case caseItemOff(F_Error);
        case caseItemOff(F_Warning);
        case caseItemOff(F_Notice);
        case caseItemOff(F_Informational);
        case caseItemOff(F_Debug);
    }
    return "unknown fault record";
}
const char *Message::timeSrc2str_c(timeSource_e val)
{
    switch(val) {
        case caseItem(ATOMIC_CLOCK);
        case caseItem(GNSS);
        case caseItem(TERRESTRIAL_RADIO);
        case caseItem(SERIAL_TIME_CODE);
        case caseItem(PTP);
        case caseItem(NTP);
        case caseItem(HAND_SET);
        case caseItem(OTHER);
        case caseItem(INTERNAL_OSCILLATOR);
    }
    return "unknown clock";
}
const bool Message::findTimeSrc(const string &str, timeSource_e &type,
    bool exact)
{
    if(str.empty())
        return false;
    int (*_strcmp)(const char *, const char *);
    if(!exact)
        _strcmp = strcasecmp;
    else
        _strcmp = strcmp; // Excect match
    if(_strcmp(str.c_str(), "GPS") == 0) {
        type = GNSS;
        return true;
    }
    int find = 0;
    for(int i = ATOMIC_CLOCK; i <= INTERNAL_OSCILLATOR; i++) {
        timeSource_e ty = (timeSource_e)i;
        const char *cmp = timeSrc2str_c(ty);
        // A whole word match!
        if(_strcmp(str.c_str(), cmp) == 0) {
            type = ty;
            return true;
        }
        // Partial match
        if(!exact && strcasestr(cmp, str.c_str()) != nullptr) {
            type = ty;
            find++;
            // Once we have 2 partial match
            // We stick to a whole word
            if(find > 1)
                exact = true;
        }
    }
    // We found 1 partial match :-)
    return find == 1;
}
const char *Message::portState2str_c(portState_e val)
{
    switch(val) {
        case caseItem(INITIALIZING);
        case caseItem(FAULTY);
        case caseItem(DISABLED);
        case caseItem(LISTENING);
        case caseItem(PRE_TIME_TRANSMITTER);
        case caseItem(TIME_TRANSMITTER);
        case caseItem(PASSIVE);
        case caseItem(UNCALIBRATED);
        case caseItem(TIME_RECEIVER);
    }
    return "unknown state";
}
const bool Message::findPortState(const string &str, portState_e &state,
    bool caseSens)
{
    if(str.empty())
        return false;
    int (*_strcmp)(const char *, const char *);
    if(caseSens)
        _strcmp = strcmp; // Excect match
    else
        _strcmp = strcasecmp;
    for(int i = INITIALIZING; i <= TIME_RECEIVER; i++) {
        portState_e v = (portState_e)i;
        if(_strcmp(str.c_str(), portState2str_c(v)) == 0) {
            state = v;
            return true;
        }
    }
#define PROC_STR(val)\
    if(_strcmp(str.c_str(), #val) == 0) {\
        state = val;\
        return true;\
    }
    PROC_STR(PRE_MASTER) // PRE_TIME_TRANSMITTER
    PROC_STR(MASTER)     // TIME_TRANSMITTER
    PROC_STR(SLAVE)      // TIME_RECEIVER
    return false;
}
const char *Message::delayMech2str_c(delayMechanism_e type)
{
    switch(type) {
        case caseItem(AUTO);
        case caseItem(E2E);
        case caseItem(P2P);
        case caseItem(NO_MECHANISM);
        case caseItem(COMMON_P2P);
        case caseItem(SPECIAL);
    }
    return "unknown";
}
const bool Message::findDelayMech(const string &str, delayMechanism_e &type,
    bool exact)
{
    if(str.empty())
        return false;
    int (*_strcmp)(const char *, const char *);
    if(!exact)
        _strcmp = strcasecmp;
    else
        _strcmp = strcmp; // Excect match
    for(int i = AUTO; i <= SPECIAL; i++) {
        delayMechanism_e v = (delayMechanism_e)i;
        if(_strcmp(str.c_str(), delayMech2str_c(v)) == 0) {
            type = v;
            return true;
        }
    }
    // As range has a huge gap
    if(_strcmp(str.c_str(), delayMech2str_c(NO_MECHANISM)) == 0) {
        type = NO_MECHANISM;
        return true;
    }
    return false;
}
const char *Message::smpteLck2str_c(SMPTEmasterLockingStatus_e val)
{
    const size_t off = 6; // Remove prefix 'SMPTE_'
    switch(val) {
        case caseItemOff(SMPTE_NOT_IN_USE);
        case caseItemOff(SMPTE_FREE_RUN);
        case caseItemOff(SMPTE_COLD_LOCKING);
        case caseItemOff(SMPTE_WARM_LOCKING);
        case caseItemOff(SMPTE_LOCKED);
    }
    return "unknown";
}
const char *Message::ts2str_c(linuxptpTimeStamp_e val)
{
    const size_t off = 3; // Remove prefix 'TS_'
    switch(val) {
        case caseItemOff(TS_SOFTWARE);
        case caseItemOff(TS_HARDWARE);
        case caseItemOff(TS_LEGACY_HW);
        case caseItemOff(TS_ONESTEP);
        case caseItemOff(TS_P2P1STEP);
    }
    return "unknown";
}
const char *Message::pwr2str_c(linuxptpPowerProfileVersion_e ver)
{
    const size_t off = 21; // Remove prefix 'IEEE_C37_238_VERSION_'
    switch(ver) {
        case caseItemOff(IEEE_C37_238_VERSION_NONE);
        case caseItemOff(IEEE_C37_238_VERSION_2011);
        case caseItemOff(IEEE_C37_238_VERSION_2017);
    }
    return "unknown state";
}
const char *Message::us2str_c(linuxptpUnicastState_e state)
{
    const size_t off = 3; // Remove prefix 'UC_'
    switch(state) {
        case caseItemOff(UC_WAIT);
        case caseItemOff(UC_HAVE_ANN);
        case caseItemOff(UC_NEED_SYDY);
        case caseItemOff(UC_HAVE_SYDY);
    }
    return "???";
}
MessageSigTlv::MessageSigTlv(BaseSigTlv *tlv,
    tlvType_e tlvType) : m_tlvType(tlvType)
{
    m_tlv.reset(tlv);
}
MessageSigTlv::MessageSigTlv(const MessageSigTlv &o)
{
#define copySigTlv(n) n: { n##_t *t = new n##_t;\
            if(t != nullptr) {\
                n##_t *t2 = dynamic_cast<n##_t *>(ot);\
                if(t2 != nullptr) {*t = *t2; tlv = t;}\
            } break; }
    BaseSigTlv *tlv = nullptr;
    BaseSigTlv *ot = o.m_tlv.get();
    if(ot != nullptr)
        switch(o.m_tlvType) {
            case ORGANIZATION_EXTENSION_PROPAGATE:
                FALLTHROUGH;
            case ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE:
                FALLTHROUGH;
            case copySigTlv(ORGANIZATION_EXTENSION);
            case copySigTlv(PATH_TRACE);
            case copySigTlv(ALTERNATE_TIME_OFFSET_INDICATOR);
            case copySigTlv(ENHANCED_ACCURACY_METRICS);
            case copySigTlv(L1_SYNC);
            case copySigTlv(PORT_COMMUNICATION_AVAILABILITY);
            case copySigTlv(PROTOCOL_ADDRESS);
            case copySigTlv(SLAVE_RX_SYNC_TIMING_DATA);
            case copySigTlv(SLAVE_RX_SYNC_COMPUTED_DATA);
            case copySigTlv(SLAVE_TX_EVENT_TIMESTAMPS);
            case copySigTlv(CUMULATIVE_RATE_RATIO);
            case copySigTlv(MANAGEMENT_ERROR_STATUS);
            case copySigTlv(MANAGEMENT);
            case copySigTlv(SLAVE_DELAY_TIMING_DATA_NP);
            default: // Ignore TLV
                break;
        }
    if(tlv != nullptr) {
        m_tlv.reset(tlv);
        m_tlvType = o.m_tlvType;
    } else
        m_tlvType = (tlvType_e)0;
}
MessageSigTlv::MessageSigTlv(MessageSigTlv &&o) : m_tlv(std::move(o.m_tlv)),
    m_tlvType(o.m_tlvType)
{
    o.m_tlvType = (tlvType_e)0;
}
tlvType_e MessageSigTlv::tlvType() const
{
    return m_tlvType;
}
const BaseSigTlv *MessageSigTlv::tlv() const
{
    return m_tlv.get();
}
MessageSigTlvs::MessageSigTlvs(const MessageSigTlvs &o) : m_msg(o.m_msg),
    m_lastSig(o.m_lastSig && !o.m_tlvs.empty())
{
    if(m_lastSig)
        for(const auto &m : o.m_tlvs)
            m_tlvs.push_back(m);
}
MessageSigTlvs::MessageSigTlvs(MessageSigTlvs &&o) : m_msg(o.m_msg),
    m_lastSig(false)
{
    if(!o.m_tlvs.empty()) {
        if(o.m_lastSig)  {
            m_tlvs = std::move(o.m_tlvs);
            m_lastSig = true;
        } else
            o.m_tlvs.clear();
    }
    o.m_lastSig = false;
}
void MessageSigTlvs::push(tlvType_e tlvType, BaseSigTlv *tlv)
{
    if(tlv != nullptr)
        m_tlvs.push_back(MessageSigTlv(tlv, tlvType));
}
bool MessageSigTlvs::traverse(const function<bool (const Message &msg,
        tlvType_e tlvType, const BaseSigTlv *tlv)> &callback) const
{
    if(m_lastSig)
        for(const auto &m : m_tlvs) {
            if(callback(m_msg, m.tlvType(), m.tlv()))
                return true;
        }
    return false;
}
bool MessageSigTlvs::traverse(MessageSigTlvCallback &callback) const
{
    if(m_lastSig)
        for(const auto &m : m_tlvs) {
            if(callback.callback(m_msg, m.tlvType(), m.tlv()))
                return true;
        }
    return false;
}
size_t MessageSigTlvs::size() const
{
    return m_lastSig ? m_tlvs.size() : 0;
}
const BaseSigTlv *MessageSigTlvs::get(size_t pos) const
{
    return m_lastSig && pos < m_tlvs.size() ? m_tlvs[pos].tlv() : nullptr;
}
tlvType_e MessageSigTlvs::getType(size_t pos) const
{
    return m_lastSig && pos < m_tlvs.size() ? m_tlvs[pos].tlvType() : (tlvType_e)0;
}
const MANAGEMENT_t *MessageSigTlvs::getMng(size_t pos) const
{
    if(m_lastSig && pos < m_tlvs.size() && m_tlvs[pos].tlvType() == MANAGEMENT)
        return dynamic_cast<const MANAGEMENT_t *>(m_tlvs[pos].tlv());
    return nullptr;
}
mng_vals_e MessageSigTlvs::getTlvId(size_t pos) const
{
    const MANAGEMENT_t *mng = getMng(pos);
    return mng != nullptr ? mng->managementId : NULL_PTP_MANAGEMENT;
}
const BaseMngTlv *MessageSigTlvs::getMngTlv(size_t pos) const
{
    const MANAGEMENT_t *mng = getMng(pos);
    return mng != nullptr ? mng->tlvData.get() : nullptr;
}
const MessageSigTlv &MessageSigTlvs::getTlv(size_t pos) const
{
    static MessageSigTlv dummy(nullptr, (tlvType_e)0);
    return m_lastSig && pos < m_tlvs.size() ? m_tlvs[pos] : dummy;
}
MessageSigTlvs::iterator::iterator(const vector<MessageSigTlv>::const_iterator
    &_it) : it(_it)
{
}
MessageSigTlvs::iterator &MessageSigTlvs::iterator::operator++()
{
    ++it;
    return *this;
}
MessageSigTlvs::iterator &MessageSigTlvs::iterator::operator++(int)
{
    it++;
    return *this;
}
const MessageSigTlv &MessageSigTlvs::iterator::operator*()
{
    return *it;
}
bool MessageSigTlvs::iterator::operator!=(MessageSigTlvs::iterator &o)
{
    return it != o.it;
}
MessageSigTlvs::iterator MessageSigTlvs::begin() const
{
    return MessageSigTlvs::iterator(m_tlvs.begin());
}
MessageSigTlvs::iterator MessageSigTlvs::end() const
{
    return MessageSigTlvs::iterator(m_tlvs.end());
}

__PTPMGMT_NAMESPACE_END

__PTPMGMT_NAMESPACE_USE;

class unq_cptr // Used with vector to store allocations in _memHndl
{
  public:
    void *p = nullptr; // pointer to release
    unq_cptr() = delete; // prevent
    unq_cptr(void *_p) : p(_p) {}
    unq_cptr(const unq_cptr &o) = delete; // prevent copy
    // Handle move properly
    unq_cptr(unq_cptr &&o) : p(o.p) { o.p = nullptr; }
    // assignment is needed by vector header, but we do not use it in practice!
    unq_cptr &operator=(const unq_cptr &) { return *this; }
    ~unq_cptr() { free(p); }
};

extern "C" {
    extern ptpmgmt_safile ptpmgmt_safile_alloc_wrap(const SaFile &sa);
    extern void cpyMsgParams(ptpmgmt_pMsgParams p);
    extern void ptpmgmt_MsgParams_alloc_wrap(ptpmgmt_pMsgParams p);

    static inline MsgParams &getMsgParams(ptpmgmt_cpMsgParams p)
    {
        MsgParams &r = *(MsgParams *)p->_this;
        r.transportSpecific = p->transportSpecific;
        r.domainNumber = p->domainNumber;
        r.boundaryHops = p->boundaryHops;
        r.minorVersion = p->minorVersion;
        r.isUnicast = p->isUnicast;
        r.useZeroGet = p->useZeroGet;
        r.rcvSignaling = p->rcvSignaling;
        r.filterSignaling = p->filterSignaling;
        r.rcvSMPTEOrg = p->rcvSMPTEOrg;
        r.sendAuth = p->sendAuth;
        r.rcvAuth = (MsgParams_RcvAuth_e)p->rcvAuth;
        r.implementSpecific = (implementSpecific_e)p->implementSpecific;
        memcpy(r.target.clockIdentity.v, p->target.clockIdentity.v,
            ClockIdentity_t::size());
        r.target.portNumber = p->target.portNumber;
        memcpy(r.self_id.clockIdentity.v, p->self_id.clockIdentity.v,
            ClockIdentity_t::size());
        r.self_id.portNumber = p->self_id.portNumber;
        return r;
    }
    // C interfaces
#define C_SWP(n, v) free(m->n); m->n = v
    static void ptpmgmt_msg_free_wrap(ptpmgmt_msg m)
    {
        if(m != nullptr) {
            delete(BaseMngTlv *)m->sendTlv;
            m->sendTlv = nullptr;
            C_SWP(data, nullptr);
            C_SWP(dataTbl, nullptr);
            C_SWP(dataSig1, nullptr);
            C_SWP(dataSig2, nullptr);
            C_SWP(dataSig3, nullptr);
            if(m->_sa != nullptr) {
                m->_sa->free(m->_sa);
                free(m->_sa);
                m->_sa = nullptr;
            }
        }
    }
    static void ptpmgmt_msg_free(ptpmgmt_msg m)
    {
        if(m != nullptr) {
            delete(Message *)m->_this;
            ptpmgmt_msg_free_wrap(m);
            free(m);
        }
    }
#define C2CPP_void(func)\
    static void ptpmgmt_msg_##func(const_ptpmgmt_msg m) {\
        if(m != nullptr && m->_this != nullptr)\
            ((Message*)m->_this)->func(); }
#define C2CPP_ret(ret, func, def)\
    static ret ptpmgmt_msg_##func(const_ptpmgmt_msg m) {\
        if(m != nullptr && m->_this != nullptr)\
            return ((Message*)m->_this)->func();\
        return def; }
#define C2CPP_cret(func, cast, def)\
    static ptpmgmt_##cast ptpmgmt_msg_##func(const_ptpmgmt_msg m) {\
        if(m != nullptr && m->_this != nullptr)\
            return (ptpmgmt_##cast)((Message*)m->_this)->func();\
        return def; }
#define C2CPP_ret_1(func) \
    bool ptpmgmt_msg_##func(uint8_t arg) { return Message::func(arg); }
#define C2CPP_cret_1(func, cast, arg, def)\
    static ptpmgmt_##cast ptpmgmt_msg_##func(const_ptpmgmt_msg m, size_t arg) {\
        if(m != nullptr && m->_this != nullptr)\
            return (ptpmgmt_##cast)((Message*)m->_this)->func(arg);\
        return def; }
#define C2CPP_ret_c1(func, typ)\
    const char *ptpmgmt_msg_##func(ptpmgmt_##typ##_e arg) {\
        return Message::func##_c((typ##_e)arg); }
#define C2CPP_find(func, _typ)\
    bool ptpmgmt_msg_##func(const char *str, ptpmgmt_##_typ *arg, bool arg2) {\
        if(str != nullptr && arg != nullptr) { _typ x;\
            if(Message::func(str, x, arg2)) {\
                *arg = (ptpmgmt_##_typ)x;\
                return true; } } return false; }
#define C2CPP_port(func, prm)\
    static const ptpmgmt_PortIdentity_t *ptpmgmt_msg_##func(ptpmgmt_msg m) {\
        if(m != nullptr && m->_this != nullptr) {\
            const PortIdentity_t &p = ((Message *)m->_this)->func();\
            ptpmgmt_PortIdentity_t *l = &(m->prm);\
            memcpy(l->clockIdentity.v, p.clockIdentity.v, ClockIdentity_t::size());\
            l->portNumber = p.portNumber;return l;\
        } return nullptr; }
    static ptpmgmt_pMsgParams ptpmgmt_msg_getParams(ptpmgmt_msg m)
    {
        if(m != nullptr) {
            ptpmgmt_pMsgParams p = &(m->_prms);
            cpyMsgParams(p);
            return p;
        }
        return nullptr;
    }
    static bool ptpmgmt_msg_updateParams(ptpmgmt_msg m, ptpmgmt_cpMsgParams prms)
    {
        if(m != nullptr && m->_this != nullptr) {
            MsgParams &p = getMsgParams(prms);
            return ((Message *)m->_this)->updateParams(p);
        }
        return false;
    }
    static bool ptpmgmt_msg_useAuth_cfg(ptpmgmt_msg m, const_ptpmgmt_cfg cfg,
        const char *section)
    {
        if(m != nullptr && m->_this != nullptr && cfg != nullptr &&
            cfg->_this != nullptr) {
            Message *me = (Message *)m->_this;
            ConfigFile &c = *(ConfigFile *)cfg->_this;
            if(section != nullptr)
                return me->useAuth(c, section);
            return me->useAuth(c);
        }
        return false;
    }
    static bool ptpmgmt_msg_useAuth(ptpmgmt_msg m, const_ptpmgmt_safile sa,
        uint8_t spp, uint32_t key)
    {
        if(m != nullptr && m->_this != nullptr && sa != nullptr &&
            sa->_this != nullptr)
            return ((Message *)m->_this)->useAuth(*(const SaFile *)sa->_this, spp,
                    key);
        return false;
    }
    static bool ptpmgmt_msg_changeAuth(ptpmgmt_msg m, uint8_t spp, uint32_t key)
    {
        if(m != nullptr && m->_this != nullptr)
            return ((Message *)m->_this)->changeAuth(spp, key);
        return false;
    }
    static bool ptpmgmt_msg_changeAuthKey(ptpmgmt_msg m, uint32_t key)
    {
        if(m != nullptr && m->_this != nullptr)
            return ((Message *)m->_this)->changeAuth(key);
        return false;
    }
    static bool ptpmgmt_msg_disableAuth(ptpmgmt_msg m)
    {
        if(m != nullptr && m->_this != nullptr)
            return ((Message *)m->_this)->disableAuth();
        return false;
    }
    C2CPP_ret(int, usedAuthSppID, -1)
    C2CPP_ret(uint32_t, usedAuthKeyID, 0)
    static const_ptpmgmt_safile ptpmgmt_msg_getSa(ptpmgmt_msg m)
    {
        if(m != nullptr) {
            if(m->_sa == nullptr && m->_this != nullptr)
                m->_sa = ptpmgmt_safile_alloc_wrap(((Message *)m->_this)->getSa());
            return m->_sa;
        }
        return nullptr;
    }
    C2CPP_ret(bool, haveAuth, false)
    C2CPP_cret(getTlvId, mng_vals_e, PTPMGMT_NULL_PTP_MANAGEMENT)
    C2CPP_cret(getBuildTlvId, mng_vals_e, PTPMGMT_NULL_PTP_MANAGEMENT)
    C2CPP_void(setAllClocks)
    C2CPP_ret(bool, isAllClocks, false)
    static bool ptpmgmt_msg_useConfig(ptpmgmt_msg m, const_ptpmgmt_cfg cfg,
        const char *section)
    {
        if(m != nullptr && m->_this != nullptr && cfg != nullptr &&
            cfg->_this != nullptr) {
            Message *me = (Message *)m->_this;
            ConfigFile &c = *(ConfigFile *)cfg->_this;
            if(section != nullptr)
                return me->useConfig(c, section);
            return me->useConfig(c);
        }
        return false;
    }
    C2CPP_ret_c1(err2str, MNG_PARSE_ERROR)
    C2CPP_ret_c1(type2str, msgType)
    C2CPP_ret_c1(tlv2str, tlvType)
    C2CPP_ret_c1(act2str, actionField)
    C2CPP_ret_c1(mng2str, mng_vals)
    C2CPP_find(findMngID, mng_vals_e)
    C2CPP_ret_c1(errId2str, managementErrorId)
    C2CPP_ret_c1(clkType2str, clockType)
    C2CPP_ret_c1(netProt2str, networkProtocol)
    C2CPP_ret_c1(clockAcc2str, clockAccuracy)
    C2CPP_ret_c1(faultRec2str, faultRecord)
    C2CPP_ret_c1(timeSrc2str, timeSource)
    C2CPP_find(findTimeSrc, timeSource_e)
    C2CPP_ret_c1(portState2str, portState)
    C2CPP_find(findPortState, portState_e)
    C2CPP_ret_c1(delayMech2str, delayMechanism)
    C2CPP_find(findDelayMech, delayMechanism_e)
    C2CPP_ret_c1(smpteLck2str, SMPTEmasterLockingStatus)
    C2CPP_ret_c1(ts2str, linuxptpTimeStamp)
    C2CPP_ret_c1(pwr2str, linuxptpPowerProfileVersion)
    C2CPP_ret_c1(us2str, linuxptpUnicastState)
    C2CPP_ret_1(is_LI_61)
    C2CPP_ret_1(is_LI_59)
    C2CPP_ret_1(is_UTCV)
    C2CPP_ret_1(is_PTP)
    C2CPP_ret_1(is_TTRA)
    C2CPP_ret_1(is_FTRA)
    bool ptpmgmt_msg_isEmpty(ptpmgmt_mng_vals_e id)
    {
        return Message::isEmpty((mng_vals_e)id);
    }
    static bool ptpmgmt_msg_isValidId(const_ptpmgmt_msg m, ptpmgmt_mng_vals_e id)
    {
        if(m != nullptr && m->_this != nullptr)
            return ((Message *)m->_this)->isValidId((mng_vals_e)id);
        return false;
    }
    static bool ptpmgmt_msg_setAction(ptpmgmt_msg m,
        ptpmgmt_actionField_e actionField, ptpmgmt_mng_vals_e tlv_id,
        const void *dataSend)
    {
        if(m != nullptr && m->_this != nullptr) {
            BaseMngTlv *tlv = nullptr;
            if(actionField != PTPMGMT_GET && dataSend != nullptr) {
                tlv = c2cppMngTlv((mng_vals_e) tlv_id, dataSend);
                if(tlv != nullptr) {
                    delete(BaseMngTlv *)m->sendTlv;
                    m->sendTlv = (void *)tlv;
                }
                m->dataSend = dataSend;
            }
            return ((Message *)m->_this)->setAction((actionField_e)actionField,
                    (mng_vals_e) tlv_id, tlv);
        }
        return false;
    }
    C2CPP_void(clearData)
    static ptpmgmt_MNG_PARSE_ERROR_e ptpmgmt_msg_build(const_ptpmgmt_msg m,
        void *buf, size_t bufSize, uint16_t sequence)
    {
        if(m != nullptr && m->_this != nullptr && buf != nullptr)
            return (ptpmgmt_MNG_PARSE_ERROR_e)
                ((Message *)m->_this)->build(buf, bufSize, sequence);
        return PTPMGMT_MNG_PARSE_ERROR_TOO_SMALL;
    }
    C2CPP_cret(getSendAction, actionField_e, PTPMGMT_GET)
    C2CPP_ret(size_t, getMsgLen, 0)
    C2CPP_ret(ssize_t, getMsgPlanedLen, -1)
    static ptpmgmt_MNG_PARSE_ERROR_e ptpmgmt_msg_parse(const_ptpmgmt_msg m,
        const void *buf, ssize_t msgSize)
    {
        if(m != nullptr && m->_this != nullptr && buf != nullptr)
            return (ptpmgmt_MNG_PARSE_ERROR_e)
                ((Message *)m->_this)->parse(buf, msgSize);
        return PTPMGMT_MNG_PARSE_ERROR_UNSUPPORT;
    }
    C2CPP_cret(getReplyAction, actionField_e, PTPMGMT_GET)
    C2CPP_ret(bool, isUnicast, false)
    C2CPP_ret(uint8_t, getPTPProfileSpecific, 0)
    C2CPP_ret(uint16_t, getSequence, 0)
    C2CPP_port(getPeer, _peer)
    C2CPP_port(getTarget, _target)
    C2CPP_ret(uint32_t, getSdoId, 0)
    C2CPP_ret(uint8_t, getDomainNumber, 0)
    C2CPP_ret(uint8_t, getVersionPTP, 0)
    C2CPP_ret(uint8_t, getMinorVersionPTP, 0)
    static const void *ptpmgmt_msg_getData(ptpmgmt_msg m)
    {
        if(m != nullptr && m->_this != nullptr) {
            Message *me = (Message *)m->_this;
            const BaseMngTlv *t = me->getData();
            if(t != nullptr) {
                void *x = nullptr;
                void *tlv = cpp2cMngTlv(me->getTlvId(), t, x);
                if(tlv != nullptr) {
                    C_SWP(data, tlv);
                    C_SWP(dataTbl, x);
                    return tlv;
                }
            }
        }
        return nullptr;
    }
    static const void *ptpmgmt_msg_getSendData(const_ptpmgmt_msg m)
    {
        if(m != nullptr)
            return m->dataSend;
        return nullptr;
    }
    C2CPP_cret(getErrId, managementErrorId_e, PTPMGMT_GENERAL_ERROR)
    static const char *ptpmgmt_msg_getErrDisplay(const_ptpmgmt_msg m)
    {
        if(m != nullptr && m->_this != nullptr)
            return ((Message *)m->_this)->getErrDisplay_c();
        return nullptr;
    }
    C2CPP_ret(bool, isLastMsgSig, false)
    C2CPP_ret(bool, isLastMsgSMPTE, false)
    C2CPP_cret(getType, msgType_e, ptpmgmt_Management)
    C2CPP_cret(getMngType, tlvType_e, PTPMGMT_MANAGEMENT)
    static bool ptpmgmt_msg_traversSigTlvs(ptpmgmt_msg m, void *cookie,
        ptpmgmt_msg_sig_callback callback)
    {
        if(m != nullptr && m->_this != nullptr && callback != nullptr) {
            return ((Message *)m->_this)->traversSigTlvs([&m, cookie, callback](
            const Message &, tlvType_e tlvType, const BaseSigTlv * tlv) {
                void *sig = nullptr;
                if(tlv != nullptr) {
                    void *x = nullptr;
                    void *x2 = nullptr;
                    sig = cpp2cSigTlv(tlvType, tlv, x, x2);
                    if(sig != nullptr) {
                        C_SWP(dataSig1, sig);
                        C_SWP(dataSig2, x);
                        C_SWP(dataSig3, x2);
                    }
                }
                return callback(cookie, m, (ptpmgmt_tlvType_e)tlvType, sig);
            });
        }
        return false;
    }
    C2CPP_ret(size_t, getSigTlvsCount, 0)
    static const void *ptpmgmt_msg_getSigTlv(ptpmgmt_msg m, size_t position)
    {
        if(m != nullptr && m->_this != nullptr) {
            Message *me = (Message *)m->_this;
            const BaseSigTlv *s = me->getSigTlv(position);
            if(s != nullptr) {
                void *x = nullptr;
                void *x2 = nullptr;
                void *sig = cpp2cSigTlv(me->getSigTlvType(position), s, x, x2);
                if(sig != nullptr) {
                    C_SWP(dataSig1, sig);
                    C_SWP(dataSig2, x);
                    C_SWP(dataSig3, x2);
                    return sig;
                }
            }
        }
        return nullptr;
    }
    C2CPP_cret_1(getSigTlvType, tlvType_e, position, PTPMGMT_MANAGEMENT)
    C2CPP_cret_1(getSigMngTlvType, mng_vals_e, position,
        PTPMGMT_NULL_PTP_MANAGEMENT)
    static const void *ptpmgmt_msg_getSigMngTlv(ptpmgmt_msg m, size_t position)
    {
        if(m != nullptr && m->_this != nullptr) {
            Message *me = (Message *)m->_this;
            const BaseMngTlv *t = me->getSigMngTlv(position);
            if(t != nullptr) {
                void *x = nullptr;
                void *tlv = cpp2cMngTlv(me->getSigMngTlvType(position), t, x);
                if(tlv != nullptr) {
                    C_SWP(dataSig1, nullptr);
                    C_SWP(dataSig2, tlv);
                    C_SWP(dataSig3, x);
                    return tlv;
                }
            }
        }
        return nullptr;
    }
    static inline void ptpmgmt_msg_asign_cb(ptpmgmt_msg m)
    {
#define C_ASGN(n) m->n = ptpmgmt_msg_##n
        C_ASGN(getParams);
        C_ASGN(updateParams);
        C_ASGN(useAuth_cfg);
        C_ASGN(useAuth);
        C_ASGN(changeAuth);
        C_ASGN(changeAuthKey);
        C_ASGN(disableAuth);
        C_ASGN(usedAuthSppID);
        C_ASGN(usedAuthKeyID);
        C_ASGN(getSa);
        C_ASGN(haveAuth);
        C_ASGN(getTlvId);
        C_ASGN(getBuildTlvId);
        C_ASGN(setAllClocks);
        C_ASGN(isAllClocks);
        C_ASGN(useConfig);
        C_ASGN(err2str);
        C_ASGN(type2str);
        C_ASGN(tlv2str);
        C_ASGN(act2str);
        C_ASGN(mng2str);
        C_ASGN(findMngID);
        C_ASGN(errId2str);
        C_ASGN(clkType2str);
        C_ASGN(netProt2str);
        C_ASGN(clockAcc2str);
        C_ASGN(faultRec2str);
        C_ASGN(timeSrc2str);
        C_ASGN(findTimeSrc);
        C_ASGN(portState2str);
        C_ASGN(findPortState);
        C_ASGN(delayMech2str);
        C_ASGN(findDelayMech);
        C_ASGN(smpteLck2str);
        C_ASGN(ts2str);
        C_ASGN(pwr2str);
        C_ASGN(us2str);
        C_ASGN(is_LI_61);
        C_ASGN(is_LI_59);
        C_ASGN(is_UTCV);
        C_ASGN(is_PTP);
        C_ASGN(is_TTRA);
        C_ASGN(is_FTRA);
        C_ASGN(isEmpty);
        C_ASGN(isValidId);
        C_ASGN(setAction);
        C_ASGN(clearData);
        C_ASGN(build);
        C_ASGN(getSendAction);
        C_ASGN(getMsgLen);
        C_ASGN(getMsgPlanedLen);
        C_ASGN(parse);
        C_ASGN(getReplyAction);
        C_ASGN(isUnicast);
        C_ASGN(getPTPProfileSpecific);
        C_ASGN(getSequence);
        C_ASGN(getPeer);
        C_ASGN(getTarget);
        C_ASGN(getSdoId);
        C_ASGN(getDomainNumber);
        C_ASGN(getVersionPTP);
        C_ASGN(getMinorVersionPTP);
        C_ASGN(getData);
        C_ASGN(getSendData);
        C_ASGN(getErrId);
        C_ASGN(getErrDisplay);
        C_ASGN(isLastMsgSig);
        C_ASGN(isLastMsgSMPTE);
        C_ASGN(getType);
        C_ASGN(getMngType);
        C_ASGN(traversSigTlvs);
        C_ASGN(getSigTlvsCount);
        C_ASGN(getSigTlv);
        C_ASGN(getSigTlvType);
        C_ASGN(getSigMngTlvType);
        C_ASGN(getSigMngTlv);
        // set MsgParams
        const MsgParams &pm = ((Message *)m->_this)->getParams();
        m->_prms._this = (void *)&pm; // point to actual message parameters
        ptpmgmt_MsgParams_alloc_wrap(&(m->_prms));
    }
    ptpmgmt_msg ptpmgmt_msg_alloc()
    {
        ptpmgmt_msg m = (ptpmgmt_msg)malloc(sizeof(ptpmgmt_msg_t));
        if(m == nullptr)
            return nullptr;
        memset(m, 0, sizeof(ptpmgmt_msg_t));
        m->_this = (void *)(new Message);
        if(m->_this == nullptr) {
            free(m);
            return nullptr;
        }
        m->free = ptpmgmt_msg_free;
        ptpmgmt_msg_asign_cb(m);
        return m;
    }
    ptpmgmt_msg ptpmgmt_msg_alloc_prms(ptpmgmt_cpMsgParams prms)
    {
        if(prms == nullptr || prms->_this == nullptr)
            return nullptr;
        MsgParams &prm = getMsgParams(prms);
        ptpmgmt_msg m = (ptpmgmt_msg)malloc(sizeof(ptpmgmt_msg_t));
        if(m == nullptr)
            return nullptr;
        memset(m, 0, sizeof(ptpmgmt_msg_t));
        m->_this = (void *)(new Message(prm));
        if(m->_this == nullptr) {
            free(m);
            return nullptr;
        }
        m->free = ptpmgmt_msg_free;
        ptpmgmt_msg_asign_cb(m);
        return m;
    }
    ptpmgmt_msg ptpmgmt_msg_alloc_wrap(const Message &msg)
    {
        ptpmgmt_msg m = (ptpmgmt_msg)malloc(sizeof(ptpmgmt_msg_t));
        if(m == nullptr)
            return nullptr;
        memset(m, 0, sizeof(ptpmgmt_msg_t));
        m->_this = (void *)&msg;
        m->free = ptpmgmt_msg_free_wrap;
        ptpmgmt_msg_asign_cb(m);
        return m;
    }
    static const size_t tlv_c_size[] = {
#define _ptpmCaseNA(n) [PTPMGMT_##n] = 0,
#define _ptpmCaseUF(n) [PTPMGMT_##n] = sizeof(ptpmgmt_##n##_t),
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
    };
    static inline void _tlv_mem_clearTlv(ptpmgmt_tlv_mem self)
    {
        free(self->tlv);
        vector<unq_cptr> *nalc = (vector<unq_cptr> *)self->_memHndl;
        if(nalc != nullptr)
            nalc->clear();
    }
    static inline bool _tlv_mem_freeMem(ptpmgmt_tlv_mem self, const void *mem)
    {
        vector<unq_cptr> *nalc = (vector<unq_cptr> *)self->_memHndl;
        if(nalc != nullptr) {
            for(auto it = nalc->begin(); it != nalc->end(); ++it)
                if(it->p == mem) {
                    nalc->erase(it);
                    return true;
                }
        }
        return false;
    }
    static inline void *_tlv_mem_allocTLV(size_t &sz, ptpmgmt_mng_vals_e id)
    {
        if((mng_vals_e)id >= FIRST_MNG_ID && (mng_vals_e)id < LAST_MNG_ID) {
            sz = tlv_c_size[id];
            if(sz > 0)
                return malloc(sz);
        }
        return nullptr;
    }
    static inline bool _tlv_mem_copyText(vector<unq_cptr> &nalc,
        ptpmgmt_PTPText_t &t)
    {
        if(t.lengthField > 0 && t.textField != nullptr) {
            size_t len = t.lengthField;
            char *s = (char *)malloc(len + 1);
            if(s == nullptr)
                return false;
            memcpy(s, t.textField, len);
            s[len] = 0;
            t.textField = s;
            nalc.push_back({s});
            return true;
        }
        return false;
    }
    static inline bool _tlv_mem_copyBloc(vector<unq_cptr> &nalc, void *&p,
        size_t len)
    {
        if(len > 0 && p != nullptr) {
            void *b = malloc(len);
            if(b == nullptr)
                return false;
            memcpy(b, p, len);
            p = b;
            nalc.push_back({b});
            return true;
        }
        return false;
    }
    static inline void *_tlv_mem_reallocMem(ptpmgmt_tlv_mem self, void *memory,
        size_t size)
    {
        vector<unq_cptr> *nalc = (vector<unq_cptr> *)self->_memHndl;
        if(nalc != nullptr) {
            auto it = nalc->begin();
            bool find = false;
            for(; !find && it != nalc->end(); ++it) {
                if(it->p == memory) {
                    find = true;
                    break;
                }
            }
            if(find) {
                void *m = realloc(memory, size);
                if(m != nullptr) {
                    it->p = m;
                    return m;
                }
            }
        }
        return nullptr;
    }
    static void ptpmgmt_tlv_mem_free(ptpmgmt_tlv_mem self)
    {
        if(self != nullptr) {
            free(self->tlv);
            delete(vector<unq_cptr> *)self->_memHndl;
            free(self);
        }
    }
    static void ptpmgmt_tlv_mem_clear(ptpmgmt_tlv_mem self)
    {
        if(self != nullptr) {
            _tlv_mem_clearTlv(self);
            self->tlv = nullptr;
            self->id = PTPMGMT_NULL_PTP_MANAGEMENT;
        }
    }
    static bool ptpmgmt_tlv_mem_newTlv(ptpmgmt_tlv_mem self, ptpmgmt_mng_vals_e id)
    {
        if(self != nullptr) {
            size_t sz;
            void *tlv = _tlv_mem_allocTLV(sz, id);
            if(tlv != nullptr) {
                _tlv_mem_clearTlv(self);
                memset(tlv, 0, sz);
                self->tlv = tlv;
                self->id = id;
                return true;
            }
        }
        return false;
    }
    static bool ptpmgmt_tlv_mem_copyTlv(ptpmgmt_tlv_mem self, ptpmgmt_mng_vals_e id,
        void *tlv)
    {
        if(self == nullptr || tlv == nullptr)
            return false;
        vector<unq_cptr> *nalc = (vector<unq_cptr> *)self->_memHndl;
        if(nalc == nullptr)
            return false;
        size_t sz;
        void *ntlv = _tlv_mem_allocTLV(sz, id);
        if(ntlv == nullptr)
            return false;
        bool ret = true;
        vector<unq_cptr> _nalc;
        memcpy(ntlv, tlv, sz);
        switch(id) {
            case PTPMGMT_CLOCK_DESCRIPTION: {
                auto *a = (ptpmgmt_CLOCK_DESCRIPTION_t *)ntlv;
                void *n1 = a->physicalAddress;
                void *n2 = a->protocolAddress.addressField;
                ret = _tlv_mem_copyText(_nalc, a->physicalLayerProtocol) &&
                    _tlv_mem_copyText(_nalc, a->productDescription) &&
                    _tlv_mem_copyText(_nalc, a->revisionData) &&
                    _tlv_mem_copyText(_nalc, a->userDescription) &&
                    _tlv_mem_copyBloc(_nalc, n1, a->physicalAddressLength) &&
                    _tlv_mem_copyBloc(_nalc, n2, a->protocolAddress.addressLength);
                a->physicalAddress = (uint8_t *)n1;
                a->protocolAddress.addressField = (uint8_t *)n2;
                break;
            }
            case PTPMGMT_USER_DESCRIPTION: {
                auto *a = (ptpmgmt_USER_DESCRIPTION_t *)ntlv;
                ret = _tlv_mem_copyText(_nalc, a->userDescription);
                break;
            }
            case PTPMGMT_ALTERNATE_TIME_OFFSET_NAME: {
                auto *a = (ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t *)ntlv;
                ret = _tlv_mem_copyText(_nalc, a->displayName);
                break;
            }
            case PTPMGMT_PORT_PROPERTIES_NP: {
                auto *a = (ptpmgmt_PORT_PROPERTIES_NP_t *)ntlv;
                ret = _tlv_mem_copyText(_nalc, a->interface);
                break;
            }
            case PTPMGMT_FAULT_LOG: {
                auto *a = (ptpmgmt_FAULT_LOG_t *)ntlv;
                void *n1 = a->faultRecords;
                ret = _tlv_mem_copyBloc(_nalc, n1,
                        a->numberOfFaultRecords * sizeof(ptpmgmt_FaultRecord_t));
                a->faultRecords = (ptpmgmt_FaultRecord_t *)n1;
                for(size_t i = 0; ret && i < a->numberOfFaultRecords; i++)
                    ret = _tlv_mem_copyText(_nalc, a->faultRecords[i].faultName) &&
                        _tlv_mem_copyText(_nalc, a->faultRecords[i].faultValue) &&
                        _tlv_mem_copyText(_nalc,
                            a->faultRecords[i].faultDescription);
                break;
            }
            case PTPMGMT_PATH_TRACE_LIST: {
                ptpmgmt_ClockIdentity_t nClock = { 0 };
                auto *a = (ptpmgmt_PATH_TRACE_LIST_t *)ntlv;
                for(size_t sz = 0; memcmp(a->pathSequence[sz].v, nClock.v,
                        sizeof(nClock)); sz++);
                void *n1 = a->pathSequence;
                ret = _tlv_mem_copyBloc(_nalc, n1, sizeof(nClock) * sz);
                a->pathSequence = (ptpmgmt_ClockIdentity_t *)n1;
                break;
            }
            case PTPMGMT_GRANDMASTER_CLUSTER_TABLE: {
                auto *a = (ptpmgmt_GRANDMASTER_CLUSTER_TABLE_t *)ntlv;
                void *n1 = a->PortAddress;
                ret = _tlv_mem_copyBloc(_nalc, n1,
                        a->actualTableSize * sizeof(ptpmgmt_PortAddress_t));
                a->PortAddress = (ptpmgmt_PortAddress_t *)n1;
                for(size_t i = 0; ret && i < a->actualTableSize; i++) {
                    n1 = a->PortAddress[i].addressField;
                    ret = _tlv_mem_copyBloc(_nalc, n1,
                            a->PortAddress[i].addressLength);
                    a->PortAddress[i].addressField = (uint8_t *)n1;
                }
                break;
            }
            case PTPMGMT_UNICAST_MASTER_TABLE: {
                auto *a = (ptpmgmt_UNICAST_MASTER_TABLE_t *)ntlv;
                void *n1 = a->PortAddress;
                ret = _tlv_mem_copyBloc(_nalc, n1,
                        a->actualTableSize * sizeof(ptpmgmt_PortAddress_t));
                a->PortAddress = (ptpmgmt_PortAddress_t *)n1;
                for(size_t i = 0; ret && i < a->actualTableSize; i++) {
                    n1 = a->PortAddress[i].addressField;
                    ret = _tlv_mem_copyBloc(_nalc, n1,
                            a->PortAddress[i].addressLength);
                    a->PortAddress[i].addressField = (uint8_t *)n1;
                }
                break;
            }
            case PTPMGMT_ACCEPTABLE_MASTER_TABLE: {
                auto *a = (ptpmgmt_ACCEPTABLE_MASTER_TABLE_t *)ntlv;
                void *n1 = a->list;
                ret = _tlv_mem_copyBloc(_nalc, n1,
                        a->actualTableSize * sizeof(ptpmgmt_AcceptableMaster_t));
                a->list = (ptpmgmt_AcceptableMaster_t *)n1;
                break;
            }
            case PTPMGMT_UNICAST_MASTER_TABLE_NP: {
                auto *a = (ptpmgmt_UNICAST_MASTER_TABLE_NP_t *)ntlv;
                void *n1 = a->unicastMasters;
                ret = _tlv_mem_copyBloc(_nalc, n1, a->actualTableSize *
                        sizeof(ptpmgmt_LinuxptpUnicastMaster_t));
                a->unicastMasters = (ptpmgmt_LinuxptpUnicastMaster_t *)n1;
                for(int i = 0; ret && i < a->actualTableSize; i++) {
                    n1 = a->unicastMasters[i].portAddress.addressField;
                    ret = _tlv_mem_copyBloc(_nalc, n1,
                            a->unicastMasters[i].portAddress.addressLength);
                    a->unicastMasters[i].portAddress.addressField = (uint8_t *)n1;
                }
                break;
            }
            default:
                break;
        }
        if(ret) {
            free(self->tlv);
            self->tlv = ntlv;
            self->id = id;
            *nalc = std::move(_nalc);
        } else
            free(ntlv);
        return ret;
    }
    static bool ptpmgmt_tlv_mem_copy(ptpmgmt_tlv_mem self,
        const_ptpmgmt_tlv_mem other)
    {
        if(other != nullptr)
            return ptpmgmt_tlv_mem_copyTlv(self, other->id, other->tlv);
        return false;
    }
    static ptpmgmt_mng_vals_e ptpmgmt_tlv_mem_getID(const_ptpmgmt_tlv_mem self)
    {
        if(self != nullptr)
            return self->id;
        return PTPMGMT_NULL_PTP_MANAGEMENT;
    }
    static void *ptpmgmt_tlv_mem_getTLV(const_ptpmgmt_tlv_mem self)
    {
        if(self != nullptr)
            return self->tlv;
        return nullptr;
    }
    static void *ptpmgmt_tlv_mem_allocMem(ptpmgmt_tlv_mem self, size_t size)
    {
        if(self != nullptr && size > 0 && self->tlv != nullptr) {
            vector<unq_cptr> *nalc = (vector<unq_cptr> *)self->_memHndl;
            if(nalc != nullptr) {
                void *m = malloc(size);
                if(m != nullptr) {
                    memset(m, 0, size);
                    nalc->push_back({nullptr});
                    nalc->back().p = m;
                    return m;
                }
            }
        }
        return nullptr;
    }
    static void *ptpmgmt_tlv_mem_callocMem(ptpmgmt_tlv_mem self, size_t number,
        size_t size)
    {
        if(self != nullptr && number > 0 && size > 0 && self->tlv != nullptr)
            return ptpmgmt_tlv_mem_allocMem(self, number * size);
        return nullptr;
    }
    static void *ptpmgmt_tlv_mem_reallocMem(ptpmgmt_tlv_mem self, void *memory,
        size_t size)
    {
        if(self != nullptr && size > 0 && self->tlv != nullptr) {
            if(memory == nullptr)
                return ptpmgmt_tlv_mem_allocMem(self, size);
            return _tlv_mem_reallocMem(self, memory, size);
        }
        return nullptr;
    }
    static void *ptpmgmt_tlv_mem_recallocMem(ptpmgmt_tlv_mem self, void *memory,
        size_t number, size_t size)
    {
        if(self != nullptr && number > 0 && size > 0 && self->tlv != nullptr)
            return ptpmgmt_tlv_mem_reallocMem(self, memory, number * size);
        return nullptr;
    }
    static inline ptpmgmt_PTPText_t _tlv_mem_allocStringLen(ptpmgmt_tlv_mem self,
        const char *str, size_t len)
    {
        ptpmgmt_PTPText_t a = { 0 };
        if(self != nullptr && self->tlv != nullptr && len > 0 && len <= UINT8_MAX) {
            vector<unq_cptr> *nalc = (vector<unq_cptr> *)self->_memHndl;
            if(nalc != nullptr) {
                char *s = (char *)malloc(len + 1);
                if(s != nullptr) {
                    memcpy(s, str, len);
                    a.lengthField = len;
                    a.textField = s;
                    s[len] = 0;
                    nalc->push_back({s});
                }
            }
        }
        return a;
    }
    static ptpmgmt_PTPText_t ptpmgmt_tlv_mem_allocStringLen(ptpmgmt_tlv_mem self,
        const char *str, size_t len)
    {
        if(str != nullptr)
            return _tlv_mem_allocStringLen(self, str, len);
        return { 0 };
    }
    static ptpmgmt_PTPText_t ptpmgmt_tlv_mem_allocString(ptpmgmt_tlv_mem self,
        const char *str)
    {
        if(str != nullptr)
            return _tlv_mem_allocStringLen(self, str, strlen(str));
        return { 0 };
    }
    static bool inline _tlv_mem_reallocStringLen(ptpmgmt_tlv_mem self,
        struct ptpmgmt_PTPText_t *text, const char *str, size_t len)
    {
        if(text != nullptr) {
            if(text->textField == nullptr || text->lengthField == 0) {
                *text = _tlv_mem_allocStringLen(self, str, len);
                return text->lengthField > 0 && text->textField != nullptr ;
            }
            if(self != nullptr && self->tlv != nullptr && len > 0 &&
                len <= UINT8_MAX) {
                char *tgt = (char *)text->textField;
                if(len > text->lengthField) {
                    tgt = (char *)_tlv_mem_reallocMem(self, tgt, len + 1);
                    if(tgt == nullptr)
                        return false;
                    text->textField = tgt;
                }
                memcpy(tgt, str, len);
                tgt[len] = 0;
                text->lengthField = len;
                return true;
            }
        }
        return false;
    }
    static bool ptpmgmt_tlv_mem_reallocStringLen(ptpmgmt_tlv_mem self,
        struct ptpmgmt_PTPText_t *text, const char *str, size_t len)
    {
        if(str != nullptr)
            return _tlv_mem_reallocStringLen(self, text, str, len);
        return false;
    }
    static bool ptpmgmt_tlv_mem_reallocString(ptpmgmt_tlv_mem self,
        struct ptpmgmt_PTPText_t *text, const char *str)
    {
        if(str != nullptr)
            return _tlv_mem_reallocStringLen(self, text, str, strlen(str));
        return false;
    }
    static bool ptpmgmt_tlv_mem_freeMem(ptpmgmt_tlv_mem self, void *mem)
    {
        if(self != nullptr && mem != nullptr)
            return _tlv_mem_freeMem(self, mem);
        return false;
    }
    static bool ptpmgmt_tlv_mem_freeString(ptpmgmt_tlv_mem self,
        ptpmgmt_PTPText_t *txt)
    {
        if(self != nullptr && txt != nullptr && txt->textField != nullptr &&
            _tlv_mem_freeMem(self, txt->textField)) {
            txt->lengthField = 0;
            txt->textField = nullptr;
            return true;
        }
        return false;
    }
    ptpmgmt_tlv_mem ptpmgmt_tlv_mem_alloc()
    {
        ptpmgmt_tlv_mem m = (ptpmgmt_tlv_mem)malloc(sizeof(ptpmgmt_tlv_mem_t));
        if(m == nullptr)
            return nullptr;
        vector<unq_cptr> *nalc = new vector<unq_cptr>;
        if(nalc == nullptr) {
            free(m);
            return nullptr;
        }
        m->_memHndl = (void *)nalc;
        m->id = PTPMGMT_NULL_PTP_MANAGEMENT;
        m->tlv = nullptr;
#define T_ASGN(n) m->n = ptpmgmt_tlv_mem_##n
        T_ASGN(free);
        T_ASGN(clear);
        T_ASGN(newTlv);
        T_ASGN(copy);
        T_ASGN(copyTlv);
        T_ASGN(getID);
        T_ASGN(getTLV);
        T_ASGN(allocMem);
        T_ASGN(callocMem);
        T_ASGN(reallocMem);
        T_ASGN(recallocMem);
        T_ASGN(allocString);
        T_ASGN(allocStringLen);
        T_ASGN(reallocString);
        T_ASGN(reallocStringLen);
        T_ASGN(freeMem);
        T_ASGN(freeString);
        return m;
    }
}
