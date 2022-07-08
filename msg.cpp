/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Create and parse PTP management messages
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * Created following "IEEE Std 1588-2008", PTP version 2
 */

#include <cmath>
#include "err.h"
#include "end.h"
#include "msg.h"
#include "msgProc.h"
#include "timeCvrt.h"
#include "comp.h"

namespace ptpmgmt
{

struct floor_t {
    int64_t intg;
    long double rem;
};
static inline floor_t _floor(long double val)
{
    floor_t ret;
    ret.intg = floorl(val);
    ret.rem = val - ret.intg;
    return ret;
}

const uint8_t ptp_major_ver = 0x2; // low Nibble, portDS.versionNumber
const uint8_t ptp_minor_ver = 0x0; // IEEE 1588-2019 uses 0x1
const uint8_t ptp_version = (ptp_minor_ver << 4) | ptp_major_ver;
const uint8_t controlFieldMng = 0x04; // For Management
// For Pdelay_Req, Pdelay_Resp, Pdelay_Resp_Follow_Up, Announce, Signaling
const uint8_t controlFieldDef = 0x05;
// For Delay_Req, Signaling, Management, Pdelay_Resp, Pdelay_Resp_Follow_Up
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
    s_port,
    s_clock,
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
    Nibble_t       messageType_majorSdoId; // minorSdoId == transportSpecific;
    Nibble_t       versionPTP; // minorVersionPTP | versionPTP
    UInteger16_t   messageLength;
    UInteger8_t    domainNumber;
    UInteger8_t    minorSdoId;
    Octet_t        flagField[2]; // [1] is always 0 for management
    Integer64_t    correctionField; // always 0 for management
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

void MsgParams::allowSigTlv(tlvType_e type)
{
    allowSigTlvs[type] = true;
}
void MsgParams::removeSigTlv(tlvType_e type)
{
    allowSigTlvs.erase(type);
}
bool MsgParams::isSigTlv(tlvType_e type) const
{
    return allowSigTlvs.count(type) > 0;
}

// Shortcuts for the ids table
#define use_GSC A_GET | A_SET | A_COMMAND
#define use_GS  A_GET | A_SET
#define use_GL  A_GET | A_USE_LINUXPTP
#define use_GSL A_GET | A_SET | A_USE_LINUXPTP
const ManagementId_t Message::mng_all_vals[] = {
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
    uint8_t allowed = mng_all_vals[m_tlv_id].allowed;
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
            FALLTHROUGH;
        case SET:
            FALLTHROUGH;
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
    m_sendAction(GET),
    m_msgLen(0),
    m_dataSend(nullptr),
    m_sequence(0),
    m_isUnicast(true),
    m_replyAction(RESPONSE),
    m_tlv_id(NULL_PTP_MANAGEMENT),
    m_prms{0},
    m_peer{0},
    m_target{0}
{
    m_prms.boundaryHops = 1;
    m_prms.isUnicast = true;
    m_prms.implementSpecific = linuxptp;
    m_prms.filterSignaling = true;
    setAllClocks();
}
Message::Message(const MsgParams &prms) :
    m_sendAction(GET),
    m_msgLen(0),
    m_dataSend(nullptr),
    m_sequence(0),
    m_isUnicast(true),
    m_replyAction(RESPONSE),
    m_tlv_id(NULL_PTP_MANAGEMENT),
    m_prms(prms),
    m_peer{0},
    m_target{0}
{
    if(m_prms.transportSpecific > 0xf)
        m_prms.transportSpecific = 0;
}
ssize_t Message::getMsgPlanedLen() const
{
    // That should not happen, precaution
    if(m_tlv_id < FIRST_MNG_ID || m_tlv_id >= LAST_MNG_ID)
        return -1; // Not supported
    const BaseMngTlv *data = nullptr;
    if(m_sendAction != GET)
        data = m_dataSend;
    else if(m_prms.useZeroGet)
        return mngMsgBaseSize; // GET with zero dataField!
    ssize_t ret = mng_all_vals[m_tlv_id].size;
    if(ret == -2) { // variable length TLV
        if(data == nullptr && m_sendAction != GET)
            return -2; // SET and COMMAND must have data
        ret = dataFieldSize(data); // Calculate variable length
    }
    if(ret < 0)
        return ret;
    // Function return:
    //  -1  tlv (m_tlv_id) is not supported
    //  -2  tlv (m_tlv_id) can not be calculate
    if(ret & 1) // Ensure even size for calculated size
        ret++;
    return ret + mngMsgBaseSize;
    // return total length of to the message to be send
}
bool Message::updateParams(const MsgParams &prms)
{
    if(prms.transportSpecific > 0xf)
        return false;
    m_prms = prms;
    return true;
}
bool Message::isEmpty(mng_vals_e id)
{
    return id >= FIRST_MNG_ID && id < LAST_MNG_ID && mng_all_vals[id].size == 0;
}
bool Message::isValidId(mng_vals_e id)
{
    if(id < FIRST_MNG_ID || id >= LAST_MNG_ID)
        return false;
    if(m_prms.implementSpecific != linuxptp &&
        mng_all_vals[id].allowed & A_USE_LINUXPTP)
        return false;
    return true;
}
bool Message::setAction(actionField_e actionField, mng_vals_e tlv_id,
    const BaseMngTlv *dataSend)
{
    if(!allowedAction(tlv_id, actionField))
        return false;
    if(tlv_id > FIRST_MNG_ID && actionField != GET &&
        mng_all_vals[tlv_id].size != 0) {
        if(dataSend == nullptr)
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
    if(buf == nullptr)
        return MNG_PARSE_ERROR_TOO_SMALL;
    if(bufSize < mngMsgBaseSize)
        return MNG_PARSE_ERROR_TOO_SMALL;
    managementMessage_p *msg = (managementMessage_p *)buf;
    *msg = {0};
    msg->messageType_majorSdoId = (Management |
            (m_prms.transportSpecific << 4)) & UINT8_MAX;
    msg->versionPTP = ptp_version;
    msg->domainNumber = m_prms.domainNumber;
    if(m_prms.isUnicast)
        msg->flagField[0] |= unicastFlag;
    msg->sequenceId = cpu_to_net16(sequence);
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
    mp.m_size = 0;
    mp.m_cur = (uint8_t *)(tlv + 1); // point on dataField
    size_t size = mngMsgBaseSize;
    mp.m_left = bufSize - size;
    ssize_t tlvSize = mng_all_vals[m_tlv_id].size;
    if(m_sendAction != GET && m_dataSend != nullptr) {
        mp.m_build = true;
        // Ensure reserve fields are zero
        mp.reserved = 0;
        // call_tlv_data() do not change data on build,
        // but does on parsing!
        BaseMngTlv *data = const_cast<BaseMngTlv *>(m_dataSend);
        MNG_PARSE_ERROR_e err = mp.call_tlv_data(m_tlv_id, data);
        if(err != MNG_PARSE_ERROR_OK)
            return err;
        // Add 'reserve' at end of message
        mp.reserved = 0;
        if((mp.m_size & 1) && mp.proc(mp.reserved)) // length need to be even
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
        mp.m_size = tlvSize;
        memset(mp.m_cur, 0, mp.m_size);
        // mp.m_cur += mp.m_size; // As m_cur is not used anymore
    }
    size += mp.m_size;
    if(size & 1) // length need to be even
        return MNG_PARSE_ERROR_SIZE;
    tlv->lengthField = cpu_to_net16(lengthFieldMngBase + mp.m_size);
    m_msgLen = size;
    msg->messageLength = cpu_to_net16(size);
    return MNG_PARSE_ERROR_OK;
}
MNG_PARSE_ERROR_e Message::parse(const void *buf, ssize_t msgSize)
{
    if(msgSize < sigBaseSize)
        return MNG_PARSE_ERROR_TOO_SMALL;
    managementMessage_p *msg = (managementMessage_p *)buf;
    m_type = (msgType_e)(msg->messageType_majorSdoId & 0xf);
    switch(m_type) {
        case Signaling:
            if(!m_prms.rcvSignaling)
                return MNG_PARSE_ERROR_HEADER;
            if(msg->controlField != controlFieldDef)
                return MNG_PARSE_ERROR_HEADER;
            break;
        case Management:
            if(msgSize < (ssize_t)sizeof(managementMessage_p) + tlvSizeHdr)
                return MNG_PARSE_ERROR_TOO_SMALL;
            if(msg->controlField != controlFieldMng)
                return MNG_PARSE_ERROR_HEADER;
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
    // Exist in both Management and signaling
    m_target.portNumber = net_to_cpu16(msg->targetPortIdentity.portNumber);
    memcpy(m_target.clockIdentity.v, msg->targetPortIdentity.clockIdentity.v,
        m_target.clockIdentity.size());
    MsgProc mp;
    mp.m_build = false;
    if(m_type == Signaling) {
        mp.m_cur = (uint8_t *)buf + sigBaseSize;
        mp.m_size = msgSize - sigBaseSize; // pass left to parseSig()
        return parseSig(&mp);
    }
    // Management message part
    uint8_t actionField = 0xf & msg->actionField;
    if(actionField != RESPONSE && actionField != ACKNOWLEDGE)
        return MNG_PARSE_ERROR_ACTION;
    m_replyAction = (actionField_e)actionField;
    uint16_t *cur = (uint16_t *)(msg + 1);
    uint16_t tlvType = net_to_cpu16(*cur++);
    mp.m_left = net_to_cpu16(*cur++); // lengthField
    ssize_t size = msgSize - sizeof(managementMessage_p) - tlvSizeHdr;
    if(MANAGEMENT_ERROR_STATUS == tlvType) {
        if(size < (ssize_t)sizeof(managementErrorTLV_p))
            return MNG_PARSE_ERROR_TOO_SMALL;
        size -= sizeof(managementErrorTLV_p);
        managementErrorTLV_p *errTlv = (managementErrorTLV_p *)cur;
        if(!findTlvId(errTlv->managementId, m_tlv_id, m_prms.implementSpecific))
            return MNG_PARSE_ERROR_INVALID_ID;
        if(!checkReplyAction(actionField))
            return MNG_PARSE_ERROR_ACTION;
        m_errorId = (managementErrorId_e)net_to_cpu16(errTlv->managementErrorId);
        // check minimum size and even
        if(mp.m_left < (ssize_t)sizeof(managementErrorTLV_p) || mp.m_left & 1)
            return MNG_PARSE_ERROR_TOO_SMALL;
        mp.m_left -= sizeof(managementErrorTLV_p);
        mp.m_cur = (uint8_t *)(errTlv + 1);
        // Check displayData size
        if(size < mp.m_left)
            return MNG_PARSE_ERROR_TOO_SMALL;
        if(mp.m_left > 1 && mp.proc(m_errorDisplay))
            return MNG_PARSE_ERROR_TOO_SMALL;
        m_mngType = MANAGEMENT_ERROR_STATUS;
        return MNG_PARSE_ERROR_MSG;
    } else if(MANAGEMENT != tlvType)
        return MNG_PARSE_ERROR_INVALID_TLV;
    if(size < (ssize_t)sizeof(uint16_t))
        return MNG_PARSE_ERROR_TOO_SMALL;
    size -= sizeof(uint16_t);
    if(!findTlvId(*cur++, m_tlv_id, m_prms.implementSpecific)) // managementId
        return MNG_PARSE_ERROR_INVALID_ID;
    if(!checkReplyAction(actionField))
        return MNG_PARSE_ERROR_ACTION;
    // Check minimum size and even
    if(mp.m_left < lengthFieldMngBase || mp.m_left & 1)
        return MNG_PARSE_ERROR_TOO_SMALL;
    mp.m_left -= lengthFieldMngBase;
    if(mp.m_left == 0)
        return MNG_PARSE_ERROR_OK;
    mp.m_cur = (uint8_t *)cur;
    if(size < mp.m_left) // Check dataField size
        return MNG_PARSE_ERROR_TOO_SMALL;
    BaseMngTlv *tlv;
    MNG_PARSE_ERROR_e err = mp.call_tlv_data(m_tlv_id, tlv);
    if(err != MNG_PARSE_ERROR_OK)
        return err;
    m_dataGet.reset(tlv);
    m_mngType = MANAGEMENT;
    return MNG_PARSE_ERROR_OK;
}
MNG_PARSE_ERROR_e Message::parse(const Buf &buf, ssize_t msgSize)
{
    // That should not happens!
    // As user used too big size in the recieve function
    // But if it does, we need protection!
    return (ssize_t)buf.size() < msgSize ? MNG_PARSE_ERROR_TOO_SMALL :
        parse(buf.get(), msgSize);
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
MNG_PARSE_ERROR_e Message::parseSig(MsgProc *pMp)
{
    MsgProc &mp = *pMp;
    ssize_t leftAll = mp.m_size;
    m_sigTlvs.clear(); // remove old TLVs
    while(leftAll >= tlvSizeHdr) {
        uint16_t *cur = (uint16_t *)mp.m_cur;
        tlvType_e tlvType = (tlvType_e)net_to_cpu16(*cur++);
        uint16_t lengthField = net_to_cpu16(*cur);
        mp.m_cur += tlvSizeHdr;
        leftAll -= tlvSizeHdr;
        if(lengthField > leftAll)
            return MNG_PARSE_ERROR_TOO_SMALL;
        leftAll -= lengthField;
        // Check signalling filter
        if(m_prms.filterSignaling && m_prms.allowSigTlvs.count(tlvType) == 0) {
            // TLV not in filter is skiped
            mp.m_cur += lengthField;
            continue;
        }
        mp.m_left = lengthField; // for build functions
        // The default error on build or parsing
        mp.m_err = MNG_PARSE_ERROR_TOO_SMALL;
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
                if(mp.m_left < (ssize_t)sizeof(managementErrorTLV_p))
                    return MNG_PARSE_ERROR_TOO_SMALL;
                errTlv = (managementErrorTLV_p *)mp.m_cur;
                if(findTlvId(errTlv->managementId, managementId,
                        m_prms.implementSpecific)) {
                    MANAGEMENT_ERROR_STATUS_t *d = new MANAGEMENT_ERROR_STATUS_t;
                    if(d == nullptr)
                        return MNG_PARSE_ERROR_MEM;
                    mp.m_cur += sizeof(managementErrorTLV_p);
                    mp.m_left -= sizeof(managementErrorTLV_p);
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
        if(tlv != nullptr) {
            sigTlv rec(tlvType);
            auto it = m_sigTlvs.insert(m_sigTlvs.end(), rec);
            // Put the tlv to the object inside the vector
            it->set(tlv);
        };
    }
    return MNG_PARSE_ERROR_SIG; // We have signaling message
}
bool Message::traversSigTlvs(std::function<bool (const Message &msg,
        tlvType_e tlvType, const BaseSigTlv *tlv)> callback) const
{
    if(m_type == Signaling)
        for(const auto &tlv : m_sigTlvs) {
            if(callback(*this, tlv.tlvType, tlv.get()))
                return true;
        }
    return false;
}
size_t Message::getSigTlvsCount() const
{
    return m_type == Signaling ? m_sigTlvs.size() : 0;
}
const BaseSigTlv *Message::getSigTlv(size_t pos) const
{
    return m_type == Signaling && pos < m_sigTlvs.size() ?
        m_sigTlvs[pos].get() : nullptr;
}
tlvType_e Message::getSigTlvType(size_t pos) const
{
    return m_type == Signaling && pos < m_sigTlvs.size() ?
        m_sigTlvs[pos].tlvType : (tlvType_e)0;
}
mng_vals_e Message::getSigMngTlvType(size_t pos) const
{
    if(m_type == Signaling && pos < m_sigTlvs.size() &&
        m_sigTlvs[pos].tlvType == MANAGEMENT) {
        const MANAGEMENT_t *mng = (MANAGEMENT_t *)m_sigTlvs[pos].get();
        return mng->managementId;
    }
    return NULL_PTP_MANAGEMENT;
}
const BaseMngTlv *Message::getSigMngTlv(size_t pos) const
{
    if(m_type == Signaling && pos < m_sigTlvs.size() &&
        m_sigTlvs[pos].tlvType == MANAGEMENT) {
        const MANAGEMENT_t *mng = (MANAGEMENT_t *)m_sigTlvs[pos].get();
        return mng->tlvData.get();
    }
    return nullptr;
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
bool Message::useConfig(const ConfigFile &cfg, const std::string &section)
{
    uint8_t transportSpecific = cfg.transportSpecific(section);
    if(transportSpecific > 0xf)
        return false;
    m_prms.transportSpecific = transportSpecific;
    m_prms.domainNumber = cfg.domainNumber();
    return true;
}
const char *Message::err2str_c(MNG_PARSE_ERROR_e err)
{
    switch(err) {
        case caseItem(MNG_PARSE_ERROR_OK);
        case caseItem(MNG_PARSE_ERROR_MSG);
        case caseItem(MNG_PARSE_ERROR_INVALID_ID);
        case caseItem(MNG_PARSE_ERROR_INVALID_TLV);
        case caseItem(MNG_PARSE_ERROR_SIZE_MISS);
        case caseItem(MNG_PARSE_ERROR_TOO_SMALL);
        case caseItem(MNG_PARSE_ERROR_SIZE);
        case caseItem(MNG_PARSE_ERROR_VAL);
        case caseItem(MNG_PARSE_ERROR_HEADER);
        case caseItem(MNG_PARSE_ERROR_ACTION);
        case caseItem(MNG_PARSE_ERROR_UNSUPPORT);
        case caseItem(MNG_PARSE_ERROR_MEM);
        default:
            return "unknown";
    }
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
        default:
            return "unknown message type";
    }
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
        default:
            return "unknown TLV";
    }
}
const char *Message::act2str_c(actionField_e action)
{
    switch(action) {
        case caseItem(GET);
        case caseItem(SET);
        case caseItem(RESPONSE);
        case caseItem(COMMAND);
        case caseItem(ACKNOWLEDGE);
        default:
            return "unknown";
    }
}
const char *Message::mng2str_c(mng_vals_e id)
{
    switch(id) {
#define A(n, v, sc, a, sz, f) case n: return #n;
#include "ids.h"
        default:
            if(id < FIRST_MNG_ID || id >= LAST_MNG_ID)
                return "out of range";
            return "unknown";
    }
}
const bool Message::findMngID(const std::string &str, mng_vals_e &id,
    bool exact)
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
        default:
            return "unknown";
    }
}
const char *Message::clkType2str_c(clockType_e val)
{
    switch(val) {
        case caseItem(ordinaryClock);
        case caseItem(boundaryClock);
        case caseItem(p2pTransparentClock);
        case caseItem(e2eTransparentClock);
        case caseItem(managementClock);
        default:
            return "unknown";
    }
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
        default:
            return "unknown";
    }
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
        default:
            return "unknown fault record";
    }
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
        default:
            return "unknown clock";
    }
}
const bool Message::findTimeSrc(const std::string &str, timeSource_e &type,
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
        default:
            return "unknown state";
    }
}
const bool Message::findPortState(const std::string &str, portState_e &state,
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
#undef PROC_STR
    return false;
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
        default:
            return "unknown";
    }
}
const char *Message::pwr2str_c(linuxptpPowerProfileVersion_e ver)
{
    const size_t off = 21; // Remove prefix 'IEEE_C37_238_VERSION_'
    switch(ver) {
        case caseItemOff(IEEE_C37_238_VERSION_NONE);
        case caseItemOff(IEEE_C37_238_VERSION_2011);
        case caseItemOff(IEEE_C37_238_VERSION_2017);
        default:
            return "unknown state";
    }
}
const char *Message::us2str_c(linuxptpUnicastState_e state)
{
    const size_t off = 3; // Remove prefix 'UC_'
    switch(state) {
        case caseItemOff(UC_WAIT);
        case caseItemOff(UC_HAVE_ANN);
        case caseItemOff(UC_NEED_SYDY);
        case caseItemOff(UC_HAVE_SYDY);
        default:
            return "???";
    }
}
std::string Timestamp_t::string() const
{
    char buf[200];
    snprintf(buf, sizeof buf, "%ju.%.9u", secondsField, nanosecondsField);
    return buf;
}
Timestamp_t::Timestamp_t(const timeval &tv)
{
    secondsField = tv.tv_sec;
    nanosecondsField = tv.tv_usec * NSEC_PER_USEC;
}
void Timestamp_t::toTimeval(timeval &tv) const
{
    tv.tv_sec = secondsField;
    tv.tv_usec = nanosecondsField / NSEC_PER_USEC;
}
void Timestamp_t::fromFloat(long double seconds)
{
    auto ret = _floor(seconds);
    secondsField = ret.intg;
    nanosecondsField = ret.rem * NSEC_PER_SEC;
}
long double Timestamp_t::toFloat() const
{
    return (long double)nanosecondsField / NSEC_PER_SEC + secondsField;
}
void Timestamp_t::fromNanoseconds(uint64_t nanosecondsField)
{
    auto d = div((long long)nanosecondsField, (long long)NSEC_PER_SEC);
    while(d.rem < 0) {
        d.quot--;
        d.rem += NSEC_PER_SEC;
    };
    secondsField = d.quot;
    nanosecondsField = d.rem;
}
uint64_t Timestamp_t::toNanoseconds() const
{
    return nanosecondsField + secondsField * NSEC_PER_SEC;
}
bool Timestamp_t::eq(long double seconds) const
{
    uint64_t secs = floorl(seconds);
    if(secondsField == secs)
        return nanosecondsField == (seconds - secs) * NSEC_PER_SEC;
    return false;
}
bool Timestamp_t::less(long double seconds) const
{
    uint64_t secs = floorl(seconds);
    if(secondsField == secs)
        return nanosecondsField < (seconds - secs) * NSEC_PER_SEC;
    return secondsField < secs;
}
Timestamp_t &normNano(Timestamp_t *ts)
{
    while(ts->nanosecondsField >= NSEC_PER_SEC) {
        ts->nanosecondsField -= NSEC_PER_SEC;
        ts->secondsField++;
    }
    return *ts;
}
Timestamp_t &Timestamp_t::add(const Timestamp_t &ts)
{
    secondsField += ts.secondsField;
    nanosecondsField += ts.nanosecondsField;
    return normNano(this);
}
Timestamp_t &Timestamp_t::add(long double seconds)
{
    auto ret = _floor(seconds);
    secondsField += ret.intg;
    nanosecondsField += ret.rem * NSEC_PER_SEC;
    return normNano(this);
}
Timestamp_t &Timestamp_t::subt(const Timestamp_t &ts)
{
    secondsField -= ts.secondsField;
    while(nanosecondsField < ts.nanosecondsField) {
        nanosecondsField += NSEC_PER_SEC;
        secondsField--;
    }
    nanosecondsField -= ts.nanosecondsField;
    return normNano(this);
}
std::string ClockIdentity_t::string() const
{
    char buf[25];
    snprintf(buf, sizeof buf, "%02x%02x%02x.%02x%02x.%02x%02x%02x",
        v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
    return buf;
}
std::string PortIdentity_t::string() const
{
    std::string ret = clockIdentity.string();
    ret += "-";
    ret += std::to_string(portNumber);
    return ret;
}
bool PortIdentity_t::less(const PortIdentity_t &rhs) const
{
    return clockIdentity == rhs.clockIdentity ?
        portNumber < rhs.portNumber : clockIdentity < rhs.clockIdentity;
}
std::string PortAddress_t::string() const
{
    switch(networkProtocol) {
        case UDP_IPv4:
            FALLTHROUGH;
        case UDP_IPv6:
            return addressField.toIp();
        case IEEE_802_3:
            FALLTHROUGH;
        case DeviceNet:
            FALLTHROUGH;
        case ControlNet:
            FALLTHROUGH;
        case PROFINET:
            FALLTHROUGH;
        default:
            return addressField.toId();
    }
}
bool PortAddress_t::less(const PortAddress_t &rhs) const
{
    return networkProtocol == rhs.networkProtocol ?  addressField <
        rhs.addressField : networkProtocol < rhs.networkProtocol;
}

}; /* namespace ptpmgmt */
