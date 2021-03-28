/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief Create and parse PTP management messages
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * Created following "IEEE Std 1588-2008", PTP version 2
 */

#include <cstring>
#include <cmath>
#include <limits>
#include <byteswap.h>
#include <arpa/inet.h>
#include "end.h"
#include "msg.h"

#if defined __GNUC__
/* See:
 * GNU GCC
 * gcc.gnu.org/onlinedocs/gcc-4.0.0/gcc/Type-Attributes.html
 * Keil GNU mode
 * www.keil.com/support/man/docs/armcc/armcc_chr1359125007083.htm
 * www.keil.com/support/man/docs/armclang_ref/armclang_ref_chr1393328521340.htm
 */
#define PACK(__definition__) __definition__ __attribute__((packed))
#elif defined _MSC_VER
// See: http://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
// For MSVC: http://docs.microsoft.com/en-us/cpp/preprocessor/pack
#define PACK(__definition__) __pragma( pack(push, 1) )\
    __definition__ __pragma( pack(pop) )
#else
#error Unknown compiler
#endif

#define caseItem(a) a: return #a

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
const uint16_t u16_sig_bit = 1 << 15;           // bit 16 negative sign bit
const uint64_t u64_sig_bit = (uint64_t)1 << 63; // bit 64 negative sign bit
// constants for IEEE 754 64-bit
// sign bit: 1 bit, exponent: 11 bits, mantissa: 52 bits
// https://en.wikipedia.org/wiki/Double-precision_floating-point_format
const int ieee754_mnt_size = 52; // Exponent location, mantissa size
const int ieee754_exp_size = 11; // Exponent size
const int64_t ieee754_exp_nan = ((int64_t)1 << (ieee754_exp_size - 1)); // 1024
const int64_t ieee754_exp_max = ieee754_exp_nan - 1; // 1023
const int64_t ieee754_exp_sub = -ieee754_exp_max; // -1023 for subnormal
const int64_t ieee754_exp_min = ieee754_exp_sub + 1; // -1022
const int64_t ieee754_exp_bias = ieee754_exp_max; // 1023
// 0x7ff0000000000000
const uint64_t ieee754_exp_mask = (((uint64_t)1 << ieee754_exp_size) - 1) <<
    ieee754_mnt_size;
// 0x0010000000000000
const int64_t ieee754_mnt_base = (int64_t)1 << ieee754_mnt_size;
// 0x000fffffffffffff
const int64_t ieee754_mnt_mask = ieee754_mnt_base - 1;

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
const uint16_t tlvSize = 4;
PACK(struct managementTLV_t {
    // TLV header 4 Octets
    uint16_t tlvType;      // tlvType_e.MANAGEMENT
    uint16_t lengthField;  // lengthFieldMngBase + dataField length
    // Management part
    uint16_t managementId; // mng_all_vals.value
    // dataField is even length
});
const uint16_t lengthFieldMngBase = sizeof(managementTLV_t) - tlvSize;
PACK(struct managementErrorTLV_p {
    uint16_t managementErrorId; // managementErrorId_e
    uint16_t managementId;      // mng_all_vals.value
    uint32_t reserved;
    // displayData              // PTPText
});
const size_t mngMsgBaseSize = sizeof(managementMessage_p) +
    sizeof(managementTLV_t);

const managementId_t message::mng_all_vals[] = {
#define A(n, v, sc, a, sz, f)\
    [n] = {.value = 0x##v, .scope = s_##sc, .allowed = a, .size = sz},
#include "ids.h"
};
MNG_PARSE_ERROR_e message::call_tlv_data(mng_vals_e id, baseMngTlv *&tlv)
{
#define A(n, v, sc, a, sz, f) case##f(n);
#define caseNA(n) case n: return MNG_PARSE_ERROR_OK
#define caseUF(n) case n:\
        if(m_build) {\
            if(n##_f(*(n##_t *)tlv))\
                return m_err;\
        } else {\
            n##_t *t = new n##_t;\
            if(t == nullptr)\
                return MNG_PARSE_ERROR_MEM;\
            if(n##_f(*t)) {\
                delete t;\
                return m_err;\
            }\
            tlv = t;\
        } break
    // The default error on build or parsing
    m_err = MNG_PARSE_ERROR_TOO_SMALL;
    switch(id) {
#include "ids.h"
        default:
            return MNG_PARSE_ERROR_UNSUPPORT;
    }
    // The mng ID is not supported yet
    return MNG_PARSE_ERROR_OK;
}

bool message::findTlvId(uint16_t val, mng_vals_e &rid, implementSpecific_e spec)
{
    mng_vals_e id;
#define A(n, v, sc, a, sz, f) case 0x##v: id = n; break;
    uint16_t value = net_to_cpu16(val);
    switch(value) {
#include "ids.h"
        default:
            return false;
    }
    /* block linuxptp is not used */
    if(spec != linuxptp && mng_all_vals[id].allowed & A_USE_LINUXPTP)
        return false;
    rid = id;
    return true;
}
bool message::checkReplyAction(uint8_t actionField)
{
    uint8_t allowed = mng_all_vals[m_tlv_id].allowed;
    if(actionField == ACKNOWLEDGE)
        return allowed & A_COMMAND;
    else if(actionField == RESPONSE)
        return allowed & (A_SET | A_GET);
    return false;
}
bool message::allowedAction(mng_vals_e id, actionField_e action)
{
    if(id < FIRST_MNG_ID || id > LAST_MNG_ID || mng_all_vals[id].size == -1)
        return false;
    if(m_prms.implementSpecific != linuxptp &&
        mng_all_vals[id].allowed & A_USE_LINUXPTP)
        return false;
    return mng_all_vals[id].allowed & (1 << action);
}
static inline bool verifyAction(actionField_e action)
{
    switch(action) {
        case GET:
        case SET:
        case COMMAND:
            return true;
        default:
            break;
    }
    return false;
}

message::message() :
    m_sendAction(GET),
    m_msgLen(0),
    m_dataSend(nullptr),
    m_sequence(0),
    m_isUnicast(true),
    m_replyAction(RESPONSE),
    m_tlv_id(FIRST_MNG_ID),
    m_peer{0},
    m_target{0}
{
    m_prms = {
        .boundaryHops = 1,
        .isUnicast = true,
        .implementSpecific = linuxptp,
        .filterSignaling = true,
    };
    setAllClocks();
}
message::message(msgParams prms) :
    m_sendAction(GET),
    m_msgLen(0),
    m_dataSend(nullptr),
    m_sequence(0),
    m_isUnicast(true),
    m_replyAction(RESPONSE),
    m_tlv_id(FIRST_MNG_ID),
    m_prms(prms),
    m_peer{0},
    m_target{0}
{
    if(m_prms.transportSpecific > 0xf)
        m_prms.transportSpecific = 0;
}
ssize_t message::getMsgPlanedLen() const
{
    // That should not happen, precaution
    if(m_tlv_id < FIRST_MNG_ID || m_tlv_id > LAST_MNG_ID)
        return -1; // Not supported
    // GET do not send dataField payload!
    if(m_sendAction == GET)
        return mngMsgBaseSize;
    ssize_t ret = mng_all_vals[m_tlv_id].size;
    if(ret == -2) { // variable length TLV
        if(m_dataSend == nullptr)
            return -2;         // can not calculate without data
        ret = dataFieldSize(); // Calculate variable length
    }
    if(ret < 0)
        return ret;
    // Function return:
    //  -1  tlv (m_tlv_id) is not supported
    //  -2  tlv (m_tlv_id) can not be calculate
    //      * GET only TLV or no data to send (no m_dataSend)
    if(ret & 1) // Ensure even size for calculated size
        ret++;
    return ret + mngMsgBaseSize;
    // return total length of to the message to be send
}
bool message::updateParams(msgParams prms)
{
    if(prms.transportSpecific > 0xf)
        return false;
    m_prms = prms;
    return true;
}
bool message::isEmpty(mng_vals_e id)
{
    if(id >= FIRST_MNG_ID && id <= LAST_MNG_ID && mng_all_vals[id].size == 0)
        return true;
    return false;
}

bool message::setAction(actionField_e actionField, mng_vals_e tlv_id)
{
    if(!verifyAction(actionField) || !allowedAction(tlv_id, actionField))
        return false;
    if(actionField != GET && mng_all_vals[tlv_id].size != 0)
        return false; // SET and COMMAND need dataSend
    m_sendAction = actionField;
    m_tlv_id = tlv_id;
    m_dataSend = nullptr;
    return true;
}
bool message::setAction(actionField_e actionField, mng_vals_e tlv_id,
    baseMngTlv &dataSend)
{
    if(!verifyAction(actionField) || !allowedAction(tlv_id, actionField))
        return false;
    m_sendAction = actionField;
    m_tlv_id = tlv_id;
    if(tlv_id > FIRST_MNG_ID && actionField != GET &&
        mng_all_vals[tlv_id].size != 0)
        m_dataSend = &dataSend;
    else
        m_dataSend = nullptr;
    return true;
}
MNG_PARSE_ERROR_e message::build(void *buf, size_t bufSize, uint16_t sequence)
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
    if(!allowedAction(m_tlv_id, m_sendAction))
        return MNG_PARSE_ERROR_INVALID_ID;
    managementTLV_t *tlv = (managementTLV_t *)(msg + 1);
    tlv->tlvType = cpu_to_net16(MANAGEMENT);
    tlv->managementId = cpu_to_net16(mng_all_vals[m_tlv_id].value);
    m_size = 0;
    m_cur = (uint8_t *)(tlv + 1); // point on dataField
    size_t size = mngMsgBaseSize;
    m_left = bufSize - size;
    if(m_sendAction != GET && m_dataSend != nullptr) {
        m_build = true;
        // Ensure reserve fields are zero
        reserved = 0;
        MNG_PARSE_ERROR_e err = call_tlv_data(m_tlv_id, m_dataSend);
        if(err != MNG_PARSE_ERROR_OK)
            return err;
        // Add 'reserve' at end of message
        reserved = 0;
        if((m_size & 1) && proc(reserved)) // length need to be even
            return MNG_PARSE_ERROR_TOO_SMALL;
        size += m_size;
    }
    tlv->lengthField = cpu_to_net16(lengthFieldMngBase + m_size);
    if(size & 1) // length need to be even
        return MNG_PARSE_ERROR_SIZE;
    m_msgLen = size;
    msg->messageLength = cpu_to_net16(size);
    return MNG_PARSE_ERROR_OK;
}
MNG_PARSE_ERROR_e message::parse(void *buf, ssize_t msgSize)
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
            if(msgSize < (ssize_t)sizeof(managementMessage_p) + tlvSize)
                return MNG_PARSE_ERROR_TOO_SMALL;
            if(msg->controlField != controlFieldMng)
                return MNG_PARSE_ERROR_HEADER;
            break;
        default:
            return MNG_PARSE_ERROR_HEADER;
    }
    Nibble_t ptp_maj = msg->versionPTP & 0xf;
    //Nibble_t ptp_min = msg->versionPTP >> 4;
    if(ptp_maj != ptp_major_ver ||
        msg->logMessageInterval != logMessageIntervalDef)
        return MNG_PARSE_ERROR_HEADER;
    m_sdoId = msg->minorSdoId | ((msg->messageType_majorSdoId & 0xf0) << 4);
    m_domainNumber = msg->domainNumber;
    m_isUnicast = msg->flagField[0] & unicastFlag;
    m_sequence = net_to_cpu16(msg->sequenceId);
    m_peer.portNumber = net_to_cpu16(msg->sourcePortIdentity.portNumber);
    memcpy(m_peer.clockIdentity.v, msg->sourcePortIdentity.clockIdentity.v,
        m_peer.clockIdentity.size());
    // Exist in both Management and signaling
    m_target.portNumber = net_to_cpu16(msg->targetPortIdentity.portNumber);
    memcpy(m_target.clockIdentity.v, msg->targetPortIdentity.clockIdentity.v,
        m_target.clockIdentity.size());
    m_build = false;
    if(m_type == Signaling) {
        m_cur = (uint8_t *)buf + sigBaseSize;
        m_size = msgSize - sigBaseSize; // pass left to parseSig()
        return parseSig();
    }
    // Management message part
    uint8_t actionField = 0xf & msg->actionField;
    if(actionField != RESPONSE && actionField != ACKNOWLEDGE)
        return MNG_PARSE_ERROR_ACTION;
    m_replyAction = (actionField_e)actionField;
    uint16_t *cur = (uint16_t *)(msg + 1);
    uint16_t tlvType = net_to_cpu16(*cur++);
    m_left = net_to_cpu16(*cur++); // lengthField
    ssize_t size = msgSize - sizeof(managementMessage_p) - tlvSize;
    if(MANAGEMENT_ERROR_STATUS == tlvType) {
        if(size < (ssize_t)sizeof(managementErrorTLV_p))
            return MNG_PARSE_ERROR_TOO_SMALL;
        size -= sizeof(managementErrorTLV_p);
        managementErrorTLV_p *errTlv = (managementErrorTLV_p *)cur;
        if(!findTlvId(errTlv->managementId, m_tlv_id, m_prms.implementSpecific))
            return MNG_PARSE_ERROR_INVALID_ID;
        if(!checkReplyAction(actionField))
            return MNG_PARSE_ERROR_ACTION;
        m_errorId = net_to_cpu16(errTlv->managementErrorId);
        // check minimum size and even
        if(m_left < (ssize_t)sizeof(managementErrorTLV_p) || m_left & 1)
            return MNG_PARSE_ERROR_TOO_SMALL;
        m_left -= sizeof(managementErrorTLV_p);
        m_cur = (uint8_t *)(errTlv + 1);
        // Check displayData size
        if(size < m_left)
            return MNG_PARSE_ERROR_TOO_SMALL;
        if(m_left > 1 && proc(m_errorDisplay))
            return MNG_PARSE_ERROR_TOO_SMALL;
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
    if(m_left < lengthFieldMngBase || m_left & 1)
        return MNG_PARSE_ERROR_TOO_SMALL;
    m_left -= lengthFieldMngBase;
    if(m_left == 0)
        return MNG_PARSE_ERROR_OK;
    m_cur = (uint8_t *)cur;
    if(size < m_left) // Check dataField size
        return MNG_PARSE_ERROR_TOO_SMALL;
    baseMngTlv *tlv;
    MNG_PARSE_ERROR_e err = call_tlv_data(m_tlv_id, tlv);
    if(err != MNG_PARSE_ERROR_OK)
        return err;
    m_dataGet = std::move(std::unique_ptr<baseMngTlv>(tlv));
    return MNG_PARSE_ERROR_OK;
}
#define caseBuildAct(n) {\
        n##_t *t = new n##_t;\
        if(t == nullptr)\
            return MNG_PARSE_ERROR_MEM;\
        if(n##_f(*t)) {\
            delete t;\
            return m_err;\
        }\
        tlv = t;\
        break;\
    }
#define caseBuild(n) n: caseBuildAct(n)
MNG_PARSE_ERROR_e message::parseSig()
{
    ssize_t leftAll = m_size;
    m_sigTlvs.clear(); // remove old TLVs
    while(leftAll >= tlvSize) {
        uint16_t *cur = (uint16_t *)m_cur;
        tlvType_e tlvType = (tlvType_e)net_to_cpu16(*cur++);
        uint16_t lengthField = net_to_cpu16(*cur);
        m_cur += tlvSize;
        leftAll -= tlvSize;
        if(lengthField > leftAll)
            return MNG_PARSE_ERROR_TOO_SMALL;
        leftAll -= lengthField;
        // Check signalling filter
        if(m_prms.filterSignaling && m_prms.allowSigTlvs.count(tlvType) == 0) {
            // And TLV not in filter is skiped
            m_cur += lengthField;
            continue;
        }
        m_left = lengthField; // for build functions
        // The default error on build or parsing
        m_err = MNG_PARSE_ERROR_TOO_SMALL;
        baseSigTlv *tlv = nullptr;
        switch(tlvType) {
            case ORGANIZATION_EXTENSION_PROPAGATE:
            case ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE:
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
            case MANAGEMENT: {
                if(m_left < 2)
                    return MNG_PARSE_ERROR_TOO_SMALL;
                mng_vals_e tlv_id;
                // managementId
                bool ret = findTlvId(*(uint16_t *)m_cur, tlv_id,
                        m_prms.implementSpecific);
                m_cur += 2;
                m_left -= 2;
                // Ignore empty and unknown management TLVs
                if(ret && m_left > 0) {
                    baseMngTlv *mtlv;
                    MNG_PARSE_ERROR_e err = call_tlv_data(tlv_id, mtlv);
                    if(err != MNG_PARSE_ERROR_OK)
                        return err;
                    MANAGEMENT_t *d = new MANAGEMENT_t;
                    if(d == nullptr)
                        return MNG_PARSE_ERROR_MEM;
                    d->tlv_id = tlv_id;
                    d->tlvData = std::move(std::unique_ptr<baseMngTlv>(mtlv));
                    tlv = d;
                }
                break;
            }
            case SLAVE_DELAY_TIMING_DATA_NP:
                if(m_prms.implementSpecific == linuxptp)
                    caseBuildAct(SLAVE_DELAY_TIMING_DATA_NP);
                break;
            default: // Ignore TLV
                break;
        }
        if(m_left > 0)
            m_cur += m_left;
        if(tlv != nullptr) {
            sigTlv rec(tlvType);
            auto it = m_sigTlvs.insert(m_sigTlvs.end(), rec);
            it->tlv = std::move(std::unique_ptr<baseSigTlv>(tlv));
        };
    }
    return MNG_PARSE_ERROR_SIG; // We have signaling message
}
bool message::traversSigTlvs(std::function<bool (const message &msg,
        tlvType_e tlvType, baseSigTlv *tlv)> callback) const
{
    if(m_type == Signaling)
        for(auto tlv : m_sigTlvs)
            if(callback(*this, tlv.tlvType, tlv.tlv.get()))
                return true;
    return false;
}
size_t message::getSigTlvsCount() const
{
    if(m_type == Signaling)
        return m_sigTlvs.size();
    return 0;
}
baseSigTlv *message::getSigTlv(size_t pos) const
{
    if(m_type == Signaling && pos < m_sigTlvs.size())
        return m_sigTlvs[pos].tlv.get();
    return nullptr;
}
tlvType_e message::getSigTlvType(size_t pos) const
{
    if(m_type == Signaling && pos < m_sigTlvs.size())
        return m_sigTlvs[pos].tlvType;
    return (tlvType_e)0; // unknown
}
mng_vals_e message::getSigMngTlvType(size_t pos) const
{
    if(m_type == Signaling && pos < m_sigTlvs.size() &&
        m_sigTlvs[pos].tlvType == MANAGEMENT) {
        MANAGEMENT_t *mng = (MANAGEMENT_t *)m_sigTlvs[pos].tlv.get();
        return mng->tlv_id;
    }
    return NULL_PTP_MANAGEMENT;
}
baseMngTlv *message::getSigMngTlv(size_t pos) const
{
    if(m_type == Signaling && pos < m_sigTlvs.size() &&
        m_sigTlvs[pos].tlvType == MANAGEMENT) {
        MANAGEMENT_t *mng = (MANAGEMENT_t *)m_sigTlvs[pos].tlv.get();
        return mng->tlvData.get();
    }
    return nullptr;
}
void message::setAllClocks()
{
    m_prms.target.portNumber = allPorts;
    m_prms.target.clockIdentity = allClocks;
}
bool message::isAllClocks()
{
    return m_prms.target.portNumber == allPorts &&
        memcmp(&m_prms.target.clockIdentity, &allClocks, sizeof(allClocks)) == 0;
}
bool message::useConfig(configFile &cfg, std::string section)
{
    uint8_t transportSpecific = cfg.transportSpecific(section);
    if(transportSpecific > 0xf)
        return false;
    m_prms.transportSpecific = transportSpecific;
    m_prms.domainNumber = cfg.domainNumber();
    return true;
}
const char *message::err2str_c(MNG_PARSE_ERROR_e err)
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
const char *message::tlv2str_c(tlvType_e type)
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
const char *message::act2str_c(actionField_e action)
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
const char *message::mng2str_c(mng_vals_e id)
{
#define A(n, v, sc, a, sz, f) case n: return #n;
    switch(id) {
#include "ids.h"
        default:
            if(id < FIRST_MNG_ID || id > LAST_MNG_ID)
                return "out of range";
            return "unknown";
    }
}
const char *message::errId2str_c(managementErrorId_e err)
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
const char *message::clkType2str_c(clockType_e val)
{
    switch(val) {
        case caseItem(ordinaryClock);
        case caseItem(boundaryClock);
        case caseItem(p2pTransparentClock);
        case caseItem(e2eTransparentClock);
        case caseItem(management);
        default:
            return "unknown";
    }
}
const char *message::netProt2str_c(networkProtocol_e val)
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
const char *message::clockAcc2str_c(clockAccuracy_e val)
{
    switch(val) {
        case caseItem(Accurate_within_1ps);
        case caseItem(Accurate_within_2_5ps);
        case caseItem(Accurate_within_10ps);
        case caseItem(Accurate_within_25ps);
        case caseItem(Accurate_within_100ps);
        case caseItem(Accurate_within_250ps);
        case caseItem(Accurate_within_1ns);
        case caseItem(Accurate_within_2_5ns);
        case caseItem(Accurate_within_10ns);
        case caseItem(Accurate_within_25ns);
        case caseItem(Accurate_within_100ns);
        case caseItem(Accurate_within_250ns);
        case caseItem(Accurate_within_1us);
        case caseItem(Accurate_within_2_5us);
        case caseItem(Accurate_within_10us);
        case caseItem(Accurate_within_25us);
        case caseItem(Accurate_within_100us);
        case caseItem(Accurate_within_250us);
        case caseItem(Accurate_within_1ms);
        case caseItem(Accurate_within_2_5ms);
        case caseItem(Accurate_within_10ms);
        case caseItem(Accurate_within_25ms);
        case caseItem(Accurate_within_100ms);
        case caseItem(Accurate_within_250ms);
        case caseItem(Accurate_within_1s);
        case caseItem(Accurate_within_10s);
        case caseItem(Accurate_more_10s);
        case caseItem(Accurate_Unknown);
        default:
            if(val < Accurate_within_1ps)
                return "small 1ps";
            return "unknown val";
    }
}
const char *message::faultRec2str_c(faultRecord_e val)
{
    switch(val) {
        case caseItem(Emergency);
        case caseItem(Alert);
        case caseItem(Critical);
        case caseItem(Error);
        case caseItem(Warning);
        case caseItem(Notice);
        case caseItem(Informational);
        case caseItem(Debug);
        default:
            return "unknown fault record";
    }
}
const char *message::timeSrc2str_c(timeSource_e val)
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
const char *message::portState2str_c(portState_e val)
{
    switch(val) {
        case caseItem(INITIALIZING);
        case caseItem(FAULTY);
        case caseItem(DISABLED);
        case caseItem(LISTENING);
        case caseItem(PRE_MASTER);
        case caseItem(MASTER);
        case caseItem(PASSIVE);
        case caseItem(UNCALIBRATED);
        case caseItem(CLIENT);
        default:
            return "unknown state";
    }
}
const char *message::ts2str_c(linuxptpTimeStamp_e val)
{
    switch(val) {
        case caseItem(TS_SOFTWARE);
        case caseItem(TS_HARDWARE);
        case caseItem(TS_LEGACY_HW);
        case caseItem(TS_ONESTEP);
        case caseItem(TS_P2P1STEP);
        default:
            return "unknown";
    }
}
std::string Timestamp_t::str() const
{
    char buf[200];
    snprintf(buf, sizeof(buf), "%ju.%.9u", secondsField, nanosecondsField);
    return buf;
};
std::string ClockIdentity_t::str() const
{
    char buf[25];
    snprintf(buf, sizeof(buf), "%02x%02x%02x.%02x%02x.%02x%02x%02x",
        v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
    return buf;
}
std::string PortIdentity_t::str() const
{
    std::string ret = clockIdentity.str();
    ret += "-";
    ret += std::to_string(portNumber);
    return ret;
}
std::string PortAddress_t::str() const
{
    switch(networkProtocol) {
        case UDP_IPv4:
        case UDP_IPv6:
            return addressField.toIp();
        case IEEE_802_3:
        case DeviceNet:
        case ControlNet:
        case PROFINET:
        default:
            return addressField.toId();
    }
}
bool message::proc(uint8_t &val)
{
    if(m_left < 1)
        return true;
    if(m_build)
        *m_cur = val;
    else
        val = *m_cur;
    move(1);
    return false;
}
bool message::proc(uint16_t &val)
{
    if(m_left < 2)
        return true;
    if(m_build)
        *(uint16_t *)m_cur = cpu_to_net16(val);
    else
        val = net_to_cpu16(*(uint16_t *)m_cur);
    move(2);
    return false;
}
bool message::proc(uint32_t &val)
{
    if(m_left < 4)
        return true;
    if(m_build)
        *(uint32_t *)m_cur = cpu_to_net32(val);
    else
        val = net_to_cpu32(*(uint32_t *)m_cur);
    move(4);
    return false;
}
bool message::proc48(uint64_t &val)
{
    uint16_t high;
    uint32_t low;
    if(m_build) {
        if(val > UINT48_MAX) {
            m_err = MNG_PARSE_ERROR_VAL;
            return true;
        }
        high = (val >> 32) & UINT16_MAX;
        low = val & UINT32_MAX;
    }
    if(proc(high) || proc(low))
        return true;
    if(!m_build)
        val = low | ((uint64_t)high << 32);
    return false;
}
bool message::proc(uint64_t &val)
{
    if(m_left < 8)
        return true;
    if(m_build)
        *(uint64_t *)m_cur = cpu_to_net64(val);
    else
        val = net_to_cpu64(*(uint64_t *)m_cur);
    move(8);
    return false;
}
bool message::proc(int8_t &val)
{
    if(m_left < 1)
        return true;
    if(m_build)
        *(int8_t *)m_cur = val;
    else
        val = *(int8_t *)m_cur;
    move(1);
    return false;
}
bool message::proc(int16_t &val)
{
    if(m_left < 2)
        return true;
    if(m_build)
        *(uint16_t *)m_cur = cpu_to_net16((uint16_t)val);
    else
        val = (int16_t)net_to_cpu16(*(uint16_t *)m_cur);
    move(2);
    return false;
}
bool message::proc(int32_t &val)
{
    if(m_left < 4)
        return true;
    if(m_build)
        *(uint32_t *)m_cur = cpu_to_net32((uint32_t)val);
    else
        val = (int32_t)net_to_cpu32(*(uint32_t *)m_cur);
    move(4);
    return false;
}
bool message::proc48(int64_t &val)
{
    uint16_t high;
    uint32_t low;
    if(m_build) {
        if(val < INT48_MIN || val > INT48_MAX) {
            m_err = MNG_PARSE_ERROR_VAL;
            return true;
        }
        high = (val >> 32) & INT16_MAX;
        if(val < 0) // Add sign bit for negative
            high |= u16_sig_bit;
        low = val & UINT32_MAX;
    }
    if(proc(high) || proc(low))
        return true;
    if(!m_build) {
        uint64_t ret = low | (((uint64_t)high & INT16_MAX) << 32);
        if(high & u16_sig_bit) // Add sign bit for negative
            ret |= u64_sig_bit;
        val = (int64_t)ret;
    }
    return false;
}
bool message::proc(int64_t &val)
{
    if(m_left < 8)
        return true;
    if(m_build)
        *(uint64_t *)m_cur = cpu_to_net64((uint64_t)val);
    else
        val = (int64_t)net_to_cpu64(*(uint64_t *)m_cur);
    move(8);
    return false;
}

bool message::proc(Float64_t &val)
{
    // Float64_t
    // Using IEEE 754 64-bit floating-point
    if(m_left < 8)
        return true;
    uint64_t num;
    int64_t mnt, exp;
    int sizeMod;
    int ordMod;
    // see ieee754.h
    // Most processors support IEEE 754
    if(sizeof(uint64_t) == sizeof(Float64_t)) {
        #if __FLOAT_WORD_ORDER == __BIG_ENDIAN
        sizeMod = 1; // Float64_t is 64 bits IEEE 754
        ordMod = 1; // float is big endian (network order)
        #elif __FLOAT_WORD_ORDER == __BYTE_ORDER
        sizeMod = 1; // Float64_t is 64 bits IEEE 754
        ordMod = 0; // use host order
        #elif __FLOAT_WORD_ORDER == __LITTLE_ENDIAN
        sizeMod = 1; // Float64_t is 64 bits IEEE 754
        ordMod = 2; // float is little endian
        #else
        sizeMod = 0; // calculate IEEE 754
        ordMod = 0; // when calculate always use host order
        #endif
    } else {
        sizeMod = 0; // calculate IEEE 754
        ordMod = 0; // when calculate always use host order
    }
    if(m_build) {
        if(sizeMod == 1) // Float64_t is 64 bits IEEE 754
            memcpy(&num, &val, sizeof(uint64_t));
        else {
            /* For processors that do not support IEEE 754
             *  or endian is not clear
             * The computed float is in host order
             */
            // Move negative sign bit
            if(std::signbit(val)) {
                num = u64_sig_bit; // add sign bit
                if(std::isfinite(val))
                    val = fabsl(val);
            } else
                num = 0;
            double norm;
            switch(std::fpclassify(val)) {
                case FP_NAN: // Not a number
                    exp = ieee754_exp_nan;
                    mnt = 1; // Any positive goes
                    break;
                case FP_INFINITE: // Infinity
                    exp = ieee754_exp_nan;
                    mnt = 0;
                    break;
                case FP_ZERO: // Zero
                    exp = ieee754_exp_sub;
                    mnt = 0;
                    break;
                default:
                case FP_NORMAL:
                    exp = (int64_t)floorl(log2l(val));
                    if(exp > ieee754_exp_max)
                        return true; // Number is too big
                    else if(exp >= ieee754_exp_min) {
                        norm = val / exp2l(exp);
                        if(norm >= 1) {
                            mnt = (int64_t)floorl(norm * ieee754_mnt_base -
                                    ieee754_mnt_base);
                            if(mnt < 0 || mnt >= ieee754_mnt_base) {
                                fprintf(stderr, "wrong calculation of float, "
                                    "mnt out of range\n");
                                return true; // wrong calculation
                            }
                            break; // Break normal
                        }
                        fprintf(stderr, "wrong calculation of float, "
                            "norm is too small\n");
                    }
                // Fall to subnormal
                case FP_SUBNORMAL: // Subnormal number
                    exp = ieee754_exp_sub;
                    norm = val / exp2l(ieee754_exp_min);
                    mnt = (int64_t)floorl(norm * ieee754_mnt_base);
                    if(mnt < 0 || mnt >= ieee754_mnt_base) {
                        fprintf(stderr, "wrong calculation of float, "
                            "mnt out of range for subnormal\n");
                        return true; // wrong calculation
                    }
                    // For very small number use the minimum subnormal
                    // As zero is used by zero only!
                    if(mnt == 0)
                        mnt = 1;
                    break;
            }
            // Add exponent value and mantissa
            num |= ((exp + ieee754_exp_bias) << ieee754_mnt_size) |
                (mnt & ieee754_mnt_mask);
        }
    }
    if(ordMod == 1) { // float is big endian (network order)
        if(m_build)
            *(uint64_t *)m_cur = num;
        else
            num = *(uint64_t *)m_cur;
        move(8);
    } else if(ordMod == 2) { // float is little endian, swap to network order
        if(m_build)
            *(uint64_t *)m_cur = bswap_64(num);
        else
            num = bswap_64(*(uint64_t *)m_cur);
        move(8);
    } else if(proc(num)) // host order to network order
        return true;
    if(!m_build) {
        if(sizeMod == 1) // Float64_t is 64 bits IEEE 754
            memcpy(&val, &num, sizeof(uint64_t));
        else {
            /* For processors that do not support IEEE 754
             *  or endian is not clear
             * The computed float is in host order
             */
            exp = (int64_t)((num & ieee754_exp_mask) >> ieee754_mnt_size) -
                ieee754_exp_bias;
            mnt = num & ieee754_mnt_mask;
            if(exp == ieee754_exp_nan) {
                if(mnt == 0) // infinity
                    val = HUGE_VALL;
                else // NaN
                    val = std::numeric_limits<Float64_t>::quiet_NaN();
            } else if(exp == ieee754_exp_sub) // Subnormal or zero
                val = exp2l(ieee754_exp_min) * mnt / ieee754_mnt_base;
            else // Normal
                val = exp2l(exp) * (mnt + ieee754_mnt_base) / ieee754_mnt_base;
            if(num & u64_sig_bit) // Negative
                val = std::copysign(val, -1);
        }
    }
    move(8);
    return false;
}
bool message::proc(std::string &str, uint16_t len)
{
    if(m_build) // On build ignore length variable
        len = str.length();
    if(m_left < (ssize_t)len)
        return true;
    if(m_build)
        memcpy(m_cur, str.c_str(), len);
    else
        str = std::string((char *)m_cur, len);
    move(len);
    return false;
}
bool message::proc(binary &bin, uint16_t len)
{
    if(m_build) // On build ignore length variable
        len = bin.length();
    if(m_left < (ssize_t)len)
        return true;
    if(m_build)
        bin.copy(m_cur);
    else
        bin.set(m_cur, len);
    move(len);
    return false;
}
bool message::proc(uint8_t *val, size_t len)
{
    if(m_left < (ssize_t)len)
        return true;
    if(m_build)
        memcpy(m_cur, val, len);
    else
        memcpy(val, m_cur, len);
    move(len);
    return false;
}
bool message::proc(networkProtocol_e &val)
{
    uint16_t v = val;
    bool ret = proc(v);
    val = (networkProtocol_e)v;
    return ret;
}
bool message::proc(clockAccuracy_e &val)
{
    uint8_t v = val;
    bool ret = proc(v);
    val = (clockAccuracy_e)v;
    return ret;
}
bool message::proc(faultRecord_e &val)
{
    uint8_t v = val;
    bool ret = proc(v);
    val = (faultRecord_e)v;
    return ret;
}
bool message::proc(timeSource_e &val)
{
    uint8_t v = val;
    bool ret = proc(v);
    val = (timeSource_e)v;
    return ret;
}
bool message::proc(portState_e &val)
{
    uint8_t v = val;
    bool ret = proc(v);
    val = (portState_e)v;
    return ret;
}
bool message::proc(msgType_e &val)
{
    uint8_t v = val;
    bool ret = proc(v);
    val = (msgType_e)v;
    return ret;
}
bool message::proc(linuxptpTimeStamp_e &val)
{
    uint8_t v = val;
    bool ret = proc(v);
    val = (linuxptpTimeStamp_e)v;
    return ret;
}
bool message::proc(TimeInterval_t &v)
{
    return proc(v.scaledNanoseconds);
}
bool message::proc(Timestamp_t &d)
{
    return proc48(d.secondsField) || proc(d.nanosecondsField);
}
bool message::proc(ClockIdentity_t &v)
{
    return proc(v.v, sizeof(ClockIdentity_t));
}
bool message::proc(PortIdentity_t &d)
{
    return proc(d.clockIdentity) || proc(d.portNumber);
}
bool message::proc(PortAddress_t &d)
{
    d.addressLength = d.addressField.length();
    return proc(d.networkProtocol) || proc(d.addressLength) ||
        proc(d.addressField, d.addressLength);
}
bool message::proc(ClockQuality_t &d)
{
    return proc(d.clockClass) || proc(d.clockAccuracy) ||
        proc(d.offsetScaledLogVariance);
}
bool message::proc(PTPText_t &d)
{
    d.lengthField = d.textField.length();
    return proc(d.lengthField) || proc(d.textField, d.lengthField);
}
bool message::proc(FaultRecord_t &d)
{
    if(proc(d.faultRecordLength) || proc(d.faultTime) || proc(d.severityCode) ||
        proc(d.faultName) || proc(d.faultValue) || proc(d.faultDescription))
        return true;
    if(d.faultRecordLength != 16 + d.faultName.lengthField +
        d.faultValue.lengthField + d.faultDescription.lengthField) {
        m_err = MNG_PARSE_ERROR_SIZE_MISS;
        return true;
    }
    return false;
}
bool message::proc(AcceptableMaster_t &d)
{
    return proc(d.acceptablePortIdentity) || proc(d.alternatePriority1);
}
bool message::procFlags(uint8_t &flags, const uint8_t flagsMask)
{
    if(m_build) {
        if(flagsMask > 1) // Ensure we use proper bits
            flags &= flagsMask;
        else if(flags > 0) // We have single flag, any positive goes
            flags = 1;
    }
    return proc(flags);
}
bool message::procLe(uint64_t &val)
{
    if(m_left < 8)
        return true;
    if(m_build)
        *(uint64_t *)m_cur = cpu_to_le64(val);
    else
        val = le_to_cpu64(*(uint64_t *)m_cur);
    move(8);
    return false;
}
// Need 2 levels to stringify macros value instead of macro name
#define stringify(s) #s
#define VER_STR(a, b) stringify(a) "." stringify(b)
/* Must be here, must be in library binary and not header source code! */
const char *message::getVersion()
{
    return VER_STR(VER_MAJ, VER_MIN);
}
int message::getVersionMajor()
{
    return VER_MAJ;
}
int message::getVersionMinor()
{
    return VER_MIN;
}
