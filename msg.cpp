/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* msg.cpp Create and parse PTP managment messages
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 * Created following "IEEE Std 1588-2008", PTP version 2
 */

#include "msg.h"
#include <cstring>
#include <arpa/inet.h>
#include <endian.h>

#define caseItem(a) case a: return #a

const uint8_t ptp_major_ver = 0x2; // low 4 bits, portDS.versionNumber
const uint8_t ptp_minor_ver = 0x0; // IEEE 1588-2019 uses 0x1
const uint8_t ptp_version = (ptp_minor_ver << 4) | ptp_major_ver;
const uint8_t messageType_Management = 0xd; // low 4 bits
const uint8_t controlField_Management = 0x04;
const uint8_t logMessageInterval_Management = 0x7f;
const uint16_t allPorts = UINT16_MAX;

enum flagField_e {
    unicastFlag = 1 << 2,
    PTPProfileSpecific1 = 1 << 5,
    PTPProfileSpecific2 = 1 << 6,
};
enum tlvType_e {
    MANAGEMENT                              = 0x0001,
    MANAGEMENT_ERROR_STATUS                 = 0x0002,
    ORGANIZATION_EXTENSION                  = 0x0003,
    REQUEST_UNICAST_TRANSMISSION            = 0x0004,
    GRANT_UNICAST_TRANSMISSION              = 0x0005,
    CANCEL_UNICAST_TRANSMISSION             = 0x0006,
    ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION = 0x0007,
    PATH_TRACE                              = 0x0008,
    ALTERNATE_TIME_OFFSET_INDICATOR         = 0x0009,
    AUTHENTICATION                          = 0x2000,
    AUTHENTICATION_CHALLENGE                = 0x2001,
    SECURITY_ASSOCIATION_UPDATE             = 0x2002,
    CUM_FREQ_SCALE_FACTOR_OFFSET            = 0x2003,
};
enum allowAction_e { // bits of actionField_e
    A_GET = 1 << GET,
    A_SET = 1 << SET,
    A_COMMAND = 1 << COMMAND,
    A_USE_LINUXPTP = 1 << 5, // Out side of actionField_e
};
enum scope_e {
    s_port,
    s_clock,
};

struct managementMessage_t {
    // Header 34 Octets
    uint8_t        messageType_transportSpecific;
    uint8_t        versionPTP; // minorVersionPTP | versionPTP
    uint16_t       messageLength;
    uint8_t        domainNumber;
    uint8_t        res2;
    uint8_t        flagField[2]; // [1] is always 0 for managment
    uint64_t       correctionField; // always 0 for managment
    uint32_t       res3;
    PortIdentity_t sourcePortIdentity;
    uint16_t       sequenceId;
    uint8_t        controlField;
    uint8_t        logMessageInterval;
    // Management message
    PortIdentity_t targetPortIdentity;
    uint8_t        startingBoundaryHops;
    uint8_t        boundaryHops;
    uint8_t        actionField; // low 4 bits
    uint8_t        res5;
};

const uint16_t tlvSize = 4;
struct managementTLV_t {
    // TLV header 4 Octets
    uint16_t tlvType;      // tlvType_e.MANAGEMENT
    uint16_t lengthField;  // lengthFieldMngBase + dataField length
    // Management part
    uint16_t managementId; // mng_all_vals.value
    // dataField is even length
};
const uint16_t lengthFieldMngBase = sizeof(managementTLV_t) - tlvSize;
struct managementErrorTLV_t {
    // TLV header 4 Octets
    uint16_t tlvType;           // tlvType_e.MANAGEMENT_ERROR_STATUS
    uint16_t lengthField;       // lengthFieldMngErrBase + displayData length
    // Management Error part
    uint16_t managementErrorId; // managementErrorId_e
    uint16_t managementId;      // mng_all_vals.value
    uint32_t reserved;          //
    // displayData              // PTPText
};
const uint16_t lengthFieldMngErrBase = sizeof(managementErrorTLV_t) - tlvSize;
const size_t mngMsgBaseSize = sizeof(managementMessage_t) +
                              sizeof(managementTLV_t);

const managementId_t message::mng_all_vals[] = {
    #define A(n, v, sc, a, sz, f)\
        [n] = {.value = 0x##v, .scope = s_##sc, .allowed = a, .size = sz},
    #include "ids.h"
};
MNG_PARSE_ERROR_e message::call_tlv_data()
{
// No function needed
#define caseNA(n)\
    case n:\
        return MNG_PARSE_ERROR_OK;
// Call build or parser
#define caseUF(n)\
    case n:\
        if (m_build) {\
            if (n##_f(*(n##_t *)m_dataSend))\
                return m_err;\
        } else {\
            n##_t *t = new n##_t;\
            if (t == nullptr)\
                return MNG_PARSE_ERROR_MEM;\
            m_dataGet = std::move(std::unique_ptr<baseData>(t));\
            if (n##_f(*t))\
                return m_err;\
        }\
        break;
    // The default error on build or parsing
    m_err = MNG_PARSE_ERROR_TOO_SMALL;
    switch(m_tlv_id)
    {
        #define A(n, v, sc, a, sz, f) case##f(n)
        #include "ids.h"
        default:
            return MNG_PARSE_ERROR_UNSUPPORT;
    }
    // The mng ID is not supported yet
    return MNG_PARSE_ERROR_OK;
}

inline uint16_t hton16(uint16_t val){return htobe16(val);} // = htons
inline uint16_t ntoh16(uint16_t val){return be16toh(val);} // = ntohs
inline uint32_t hton32(uint32_t val){return htobe32(val);} // = htonl
inline uint32_t ntoh32(uint32_t val){return be32toh(val);} // = ntohl
inline uint64_t hton64(uint64_t val){return htobe64(val);}
inline uint64_t ntoh64(uint64_t val){return be64toh(val);}

bool message::findTlvId(uint16_t val)
{
    uint16_t value = ntoh16(val);
    switch(value)
    {
        #define A(n, v, sc, a, sz, f) case 0x##v: m_tlv_id = n; return true;
        #include "ids.h"
        default:
            return false;
    }
}
bool message::checkReplyAction(uint8_t actionField)
{
    uint8_t allowed = mng_all_vals[m_tlv_id].allowed;
    if (actionField == ACKNOWLEDGE)
        return allowed & A_COMMAND;
    else if (actionField == RESPONSE)
        return allowed & (A_SET | A_GET);
    return false;
}
bool message::allowedAction(mng_vals_e id, actionField_e action)
{
    if (id < FIRST_MNG_ID || id > LAST_MNG_ID || mng_all_vals[id].size == -1)
        return false;
    if (!m_prms.useLinuxPTPTlvs && mng_all_vals[id].allowed & A_USE_LINUXPTP)
        return false;
    return mng_all_vals[id].allowed & (1 << action);
}
static inline bool verifyAction(actionField_e action)
{
    switch(action)
    {
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
    m_prms{
        .boundaryHops = 1,
        .isUnicast = true,
        .portNumber = allPorts,
        .useLinuxPTPTlvs = true,
    },
    m_tlv_id(FIRST_MNG_ID),
    m_actionField(GET),
    m_msgLen(0),
    m_sequence(0),
    m_peer{{0,0},0},
    m_isUnicast(true),
    m_dataSend(nullptr)
{
}
message::message(msgParams prms) :
    m_prms(prms),
    m_tlv_id(FIRST_MNG_ID),
    m_actionField(GET),
    m_msgLen(0),
    m_sequence(0),
    m_peer{{0,0},0},
    m_isUnicast(true),
    m_dataSend(nullptr)
{
    if (m_prms.transportSpecific > 0xf)
        m_prms.transportSpecific = 0;
}
ssize_t message::getMsgPlanedLen()
{
    ssize_t ret = dataFieldSize();
    if (ret < 0)
        return ret;
    // Function return:
    //  -1  tlv (m_tlv_id) is not supported
    //  -2  tlv (m_tlv_id) can not be calculate
    //      * GET only TLV or no data to send (no m_dataSend)
    return ret + mngMsgBaseSize;
}
bool message::updateParams(msgParams prms)
{
    if (prms.transportSpecific > 0xf)
        return false;
    m_prms = prms;
    return true;
}
bool message::isEmpty(mng_vals_e id)
{
    if (id >= FIRST_MNG_ID && id <= LAST_MNG_ID && mng_all_vals[id].size == 0)
        return true;
    return false;
}

bool message::setAction(actionField_e actionField, mng_vals_e tlv_id)
{
    if (!verifyAction(actionField) ||
        !allowedAction(tlv_id, actionField))
        return false;
    if (actionField != GET && mng_all_vals[tlv_id].size != 0)
        return false; // SET and COMMAND need dataSend
    m_actionField = actionField;
    m_tlv_id = tlv_id;
    m_dataSend = nullptr;
    return true;
}
bool message::setAction(actionField_e actionField, mng_vals_e tlv_id,
                        baseData &dataSend)
{
    if (!verifyAction(actionField) ||
        !allowedAction(tlv_id, actionField))
        return false;
    m_actionField = actionField;
    m_tlv_id = tlv_id;
    if (tlv_id > FIRST_MNG_ID && actionField != GET &&
        mng_all_vals[tlv_id].size != 0)
        m_dataSend = &dataSend;
    else
        m_dataSend = nullptr;
    return true;
}
MNG_PARSE_ERROR_e message::build(const void *buf, size_t bufSize,
                               uint16_t sequence)
{
    if (bufSize < mngMsgBaseSize)
        return MNG_PARSE_ERROR_TOO_SMALL;
    managementMessage_t *msg = (managementMessage_t*)buf;
    memset(msg, 0, sizeof(managementMessage_t));
    msg->messageType_transportSpecific = (messageType_Management |
        (m_prms.transportSpecific << 4)) & UINT8_MAX;
    msg->versionPTP = ptp_version;
    msg->domainNumber = m_prms.domainNumber;
    if (m_prms.isUnicast)
        msg->flagField[0] |= unicastFlag;
    msg->sequenceId = hton16(sequence);
    msg->controlField = controlField_Management;
    msg->logMessageInterval = logMessageInterval_Management;
    msg->startingBoundaryHops = m_prms.boundaryHops;
    msg->boundaryHops = m_prms.boundaryHops;
    msg->actionField = m_actionField;
    memset(msg->targetPortIdentity.clockIdentity, UINT8_MAX, ClockIdentity_s);
    msg->targetPortIdentity.portNumber = hton16(m_prms.portNumber);
    memcpy(msg->sourcePortIdentity.clockIdentity, m_prms.self_id.clockIdentity,
           ClockIdentity_s);
    msg->sourcePortIdentity.portNumber = hton16(m_prms.self_id.portNumber);
    if (!allowedAction(m_tlv_id, m_actionField))
        return MNG_PARSE_ERROR_INVALID_ID;
    managementTLV_t *tlv = (managementTLV_t *)(msg + 1);
    tlv->tlvType = hton16(MANAGEMENT);
    tlv->managementId = hton16(mng_all_vals[m_tlv_id].value);
    m_size = 0;
    m_cur = (uint8_t*)(tlv + 1); // point on dataField
    size_t size = mngMsgBaseSize;
    m_left = bufSize - size;
    if (m_actionField != GET && m_dataSend != nullptr) {
        m_build = true;
        // Ensure reserve fileds are zero
        reserved = reserved2 = reserved3 = 0;
        MNG_PARSE_ERROR_e err = call_tlv_data();
        if (err != MNG_PARSE_ERROR_OK)
            return err;
        // Add 'reserve' at end of message
        reserved = 0;
        if ((m_size & 1) && u8(reserved)) // length need to be even
            return MNG_PARSE_ERROR_TOO_SMALL;
        size += m_size;
    }
    tlv->lengthField = hton16(lengthFieldMngBase + m_size);
    if (size & 1) // length need to be even
        return MNG_PARSE_ERROR_SIZE;
    m_msgLen = size;
    msg->messageLength = hton16(size);
    return MNG_PARSE_ERROR_OK;
}
MNG_PARSE_ERROR_e message::parse(const void *buf, size_t msgSize)
{
    if (msgSize < sizeof(managementMessage_t) + tlvSize)
        return MNG_PARSE_ERROR_TOO_SMALL;
    managementMessage_t *msg = (managementMessage_t*)buf;
    uint8_t ptp_maj = msg->versionPTP & 0xf;
    uint8_t ptp_min = msg->versionPTP >> 4;
    if (msg->messageType_transportSpecific != ((messageType_Management |
        (m_prms.transportSpecific << 4)) & UINT8_MAX) ||
        ptp_maj != ptp_major_ver || ptp_min < ptp_minor_ver ||
        msg->domainNumber != m_prms.domainNumber ||
        msg->controlField != controlField_Management ||
        msg->logMessageInterval != logMessageInterval_Management)
        return MNG_PARSE_ERROR_HEADER;
    m_isUnicast = msg->flagField[0] & unicastFlag;
    m_sequence = ntoh16(msg->sequenceId);
    uint8_t actionField = 0xf & msg->actionField;
    if (actionField != RESPONSE && actionField != ACKNOWLEDGE)
        return MNG_PARSE_ERROR_ACTION;
    m_peer.portNumber = ntoh16(msg->sourcePortIdentity.portNumber);
    memcpy(m_peer.clockIdentity, msg->sourcePortIdentity.clockIdentity,
           ClockIdentity_s);
    uint16_t *cur = (uint16_t *)(msg + 1);
    uint16_t tlvType = ntoh16(*cur);
    m_build = false;
    ssize_t size = msgSize - sizeof(managementMessage_t);
    if (MANAGEMENT_ERROR_STATUS == tlvType)
    {
        if (size < (ssize_t)sizeof(managementErrorTLV_t))
            return MNG_PARSE_ERROR_TOO_SMALL;
        size -= sizeof(managementErrorTLV_t);
        managementErrorTLV_t *errTlv = (managementErrorTLV_t*)cur;
        if (!findTlvId(errTlv->managementId))
            return MNG_PARSE_ERROR_INVALID_ID;
        if (!checkReplyAction(actionField))
            return MNG_PARSE_ERROR_ACTION;
        m_errorId = ntoh16(errTlv->managementErrorId);
        m_left = ntoh16(errTlv->lengthField);
        // check minimum size and even
        if (m_left < lengthFieldMngErrBase || m_left & 1)
            return MNG_PARSE_ERROR_TOO_SMALL;
        m_left -= lengthFieldMngErrBase;
        m_cur = (uint8_t *)(errTlv + 1);
        // Check displayData size
        if (size < m_left)
            return MNG_PARSE_ERROR_TOO_SMALL;
        if (m_left > 1 && PTPText_f(m_errorDisplay))
            return MNG_PARSE_ERROR_TOO_SMALL;
        return MNG_PARSE_ERROR_MSG;
    }
    else if (MANAGEMENT != tlvType)
        return MNG_PARSE_ERROR_INVALID_TLV;

    if (size < (ssize_t)sizeof(managementTLV_t))
        return MNG_PARSE_ERROR_TOO_SMALL;
    managementTLV_t *tlv = (managementTLV_t*)cur;
    size -= sizeof(managementTLV_t);
    if (!findTlvId(tlv->managementId))
        return MNG_PARSE_ERROR_INVALID_ID;
    if (!checkReplyAction(actionField))
        return MNG_PARSE_ERROR_ACTION;
    m_left = ntoh16(tlv->lengthField);
    // Check minimum size and even
    if (m_left < lengthFieldMngBase || m_left & 1)
        return MNG_PARSE_ERROR_TOO_SMALL;
    m_left -= lengthFieldMngBase;
    if (m_left == 0)
        return MNG_PARSE_ERROR_OK;
    m_cur = (uint8_t *)(tlv + 1);
    if (size < m_left) // Check dataField size
        return MNG_PARSE_ERROR_TOO_SMALL;
    return call_tlv_data();
}
void message::setAllPorts()
{
    m_prms.portNumber = allPorts;
}
bool message::isAllPorts()
{
    return m_prms.portNumber == allPorts;
}
bool message::useConfig(configFile &cfg, const char *section)
{

    uint8_t transportSpecific = cfg.transportSpecific(section);
    if (transportSpecific > 0xf)
        return false;
    m_prms.transportSpecific = transportSpecific;
    m_prms.domainNumber = cfg.domainNumber(section);
    return true;
}
bool message::useConfig(configFile &cfg, std::string &section)
{
    uint8_t transportSpecific = cfg.transportSpecific(section);
    if (transportSpecific > 0xf)
        return false;
    m_prms.transportSpecific = transportSpecific;
    m_prms.domainNumber = cfg.domainNumber();
    return true;
}
const char *message::c2str_c(MNG_PARSE_ERROR_e err)
{
    switch(err)
    {
        caseItem(MNG_PARSE_ERROR_OK);
        caseItem(MNG_PARSE_ERROR_MSG);
        caseItem(MNG_PARSE_ERROR_INVALID_ID);
        caseItem(MNG_PARSE_ERROR_INVALID_TLV);
        caseItem(MNG_PARSE_ERROR_SIZE_MISS);
        caseItem(MNG_PARSE_ERROR_TOO_SMALL);
        caseItem(MNG_PARSE_ERROR_SIZE);
        caseItem(MNG_PARSE_ERROR_VAL);
        caseItem(MNG_PARSE_ERROR_HEADER);
        caseItem(MNG_PARSE_ERROR_ACTION);
        caseItem(MNG_PARSE_ERROR_UNSUPPORT);
        caseItem(MNG_PARSE_ERROR_MEM);
        default: return "unknown";
    }
}
const char *message::c2str_c(mng_vals_e id)
{
    switch(id)
    {
        #define A(n, v, sc, a, sz, f) case n: return #n;
        #include "ids.h"
        default:
            if (id < FIRST_MNG_ID || id > LAST_MNG_ID)
                return "out of range";
            return "unknown";
    }
}
const char *message::c2str_c(managementErrorId_e err)
{
    switch(err)
    {
        caseItem(RESPONSE_TOO_BIG);
        caseItem(NO_SUCH_ID);
        caseItem(WRONG_LENGTH);
        caseItem(WRONG_VALUE);
        caseItem(NOT_SETABLE);
        caseItem(NOT_SUPPORTED);
        caseItem(GENERAL_ERROR);
        default: return "unknown";
    }
}
const char *message::c2str_c(networkProtocol_e val)
{
    switch(val)
    {
        caseItem(UDP_IPv4);
        caseItem(UDP_IPv6);
        caseItem(IEEE_802_3);
        caseItem(DeviceNet);
        caseItem(ControlNet);
        caseItem(PROFINET);
        default: return "unknown";
    }
}
const char *message::c2str_c(clockAccuracy_e val)
{
    switch(val)
    {
        caseItem(Accurate_within_1ps);
        caseItem(Accurate_within_2_5ps);
        caseItem(Accurate_within_10ps);
        caseItem(Accurate_within_25ps);
        caseItem(Accurate_within_100ps);
        caseItem(Accurate_within_250ps);
        caseItem(Accurate_within_1ns);
        caseItem(Accurate_within_2_5ns);
        caseItem(Accurate_within_10ns);
        caseItem(Accurate_within_25ns);
        caseItem(Accurate_within_100ns);
        caseItem(Accurate_within_250ns);
        caseItem(Accurate_within_1us);
        caseItem(Accurate_within_2_5us);
        caseItem(Accurate_within_10us);
        caseItem(Accurate_within_25us);
        caseItem(Accurate_within_100us);
        caseItem(Accurate_within_250us);
        caseItem(Accurate_within_1ms);
        caseItem(Accurate_within_2_5ms);
        caseItem(Accurate_within_10ms);
        caseItem(Accurate_within_25ms);
        caseItem(Accurate_within_100ms);
        caseItem(Accurate_within_250ms);
        caseItem(Accurate_within_1s);
        caseItem(Accurate_within_10s);
        caseItem(Accurate_more_10s);
        caseItem(Accurate_Unknown);
        default:
            if (val < Accurate_within_1ps)
                return "small 1ps";
            return "unknown val";
    }
}
const char *message::c2str_c(faultRecord_e val)
{
    switch(val)
    {
        caseItem(Emergency);
        caseItem(Alert);
        caseItem(Critical);
        caseItem(Error);
        caseItem(Warning);
        caseItem(Notice);
        caseItem(Informational);
        caseItem(Debug);
        default: return "unknown fault record";
    }
}
const char *message::c2str_c(timeSource_e val)
{
    switch(val)
    {
        caseItem(ATOMIC_CLOCK);
        caseItem(GPS);
        caseItem(TERRESTRIAL_RADIO);
        caseItem(SERIAL_TIME_CODE);
        caseItem(PTP);
        caseItem(NTP);
        caseItem(HAND_SET);
        caseItem(OTHER);
        caseItem(INTERNAL_OSCILLATOR);
        default: return "unknown clock";
    }
}
const char *message::c2str_c(portState_e val)
{
    switch(val)
    {
        caseItem(INITIALIZING);
        caseItem(FAULTY);
        caseItem(DISABLED);
        caseItem(LISTENING);
        caseItem(PRE_MASTER);
        caseItem(MASTER);
        caseItem(PASSIVE);
        caseItem(UNCALIBRATED);
        caseItem(SLAVE);
        default: return "unknown state";
    }
}
const char *message::c2str_c(linuxptpTimesTamp_e val)
{
    switch(val)
    {
        caseItem(TS_SOFTWARE);
        caseItem(TS_HARDWARE);
        caseItem(TS_LEGACY_HW);
        caseItem(TS_ONESTEP);
        caseItem(TS_P2P1STEP);
        default: return "unknown";
    }
}
std::string message::c2str(const Timestamp_t &v)
{
    char buf[200];
    snprintf(buf, sizeof(buf), "%ju.%.9u",
             v.secondsField, v.nanosecondsField);
    return buf;
};
std::string message::c2str(const ClockIdentity_t &pt)
{
    char buf[25];
    snprintf(buf, sizeof(buf), "%02x%02x%02x.%02x%02x.%02x%02x%02x",
         pt[0], pt[1], pt[2], pt[3], pt[4], pt[5], pt[6], pt[7]);
    return buf;
}
std::string message::c2str(const PortIdentity_t &val)
{
    std::string ret = c2str(val.clockIdentity);
    ret += "-";
    ret += std::to_string(val.portNumber);
    return ret;
}
std::string message::b2str(const uint8_t *id, size_t len)
{
    std::string ret;
    char buf[10];
    if (len < 1)
        return "";
    snprintf(buf, sizeof(buf), "%02x", *id);
    ret = buf;
    for(len--;len > 0;len--) {
        snprintf(buf, sizeof(buf), ":%02x", *++id);
        ret += buf;
    }
    return ret;
}
std::string message::b2str(const std::string &id)
{
    return b2str((const uint8_t*)id.c_str(), id.length());
}
std::string message::ipv42str(const std::string &id)
{
    char buf[INET_ADDRSTRLEN];
    return inet_ntop(AF_INET, (in_addr*)id.c_str(), buf, sizeof(buf));
}
std::string message::ipv62str(const std::string &id)
{
    char buf[INET6_ADDRSTRLEN];
    return inet_ntop(AF_INET6, (in6_addr*)id.c_str(), buf, sizeof(buf));
}
std::string message::c2str(const PortAddress_t &d)
{
    switch(d.networkProtocol)
    {
        case UDP_IPv4:
            return ipv42str(d.addressField);
        case UDP_IPv6:
            return ipv62str(d.addressField);
        case IEEE_802_3:
        case DeviceNet:
        case ControlNet:
        case PROFINET:
        default:
            return b2str(d.addressField);
    }
}
bool message::u8(uint8_t &val)
{
    if (m_left < 1)
        return true;
    if (m_build)
        *m_cur = val;
    else
        val = *m_cur;
    move(1);
    return false;
}
bool message::u16(uint16_t &val)
{
    if (m_left < 2)
        return true;
    if (m_build)
        *(uint16_t*)m_cur = hton16(val);
    else
        val = ntoh16(*(uint16_t*)m_cur);
    move(2);
    return false;
}
bool message::u32(uint32_t &val)
{
    if (m_left < 4)
        return true;
    if (m_build)
        *(uint32_t*)m_cur = hton32(val);
    else
        val = ntoh32(*(uint32_t*)m_cur);
    move(4);
    return false;
}
bool message::u48(uint64_t &val)
{
    uint16_t high;
    uint32_t low;
    if (m_build) {
        if (val > UINT48_MAX) {
            m_err = MNG_PARSE_ERROR_VAL;
            return true;
        }
        high = (val >> 32) & UINT16_MAX;
        low = val & UINT32_MAX;
    }
    if (u16(high) || u32(low))
        return true;
    if (!m_build)
        val = low | ((uint64_t)high << 32);
    return false;
}
bool message::u64(uint64_t &val)
{
    if (m_left < 8)
        return true;
    if (m_build)
        *(uint64_t*)m_cur = hton64(val);
    else
        val = ntoh64(*(uint64_t*)m_cur);
    move(8);
    return false;
}
bool message::i8(int8_t &val)
{
    if (m_left < 1)
        return true;
    if (m_build)
        *(int8_t*)m_cur = val;
    else
        val = *(int8_t*)m_cur;
    move(1);
    return false;
}
bool message::i16(int16_t &val)
{
    if (m_left < 2)
        return true;
    if (m_build)
        *(uint16_t*)m_cur = hton16((uint16_t)val);
    else
        val = (int16_t)ntoh16(*(uint16_t*)m_cur);
    move(2);
    return false;
}
bool message::i32(int32_t &val)
{
    if (m_left < 4)
        return true;
    if (m_build)
        *(uint32_t*)m_cur = hton32((uint32_t)val);
    else
        val = (int32_t)ntoh32(*(uint32_t*)m_cur);
    move(4);
    return false;
}
bool message::i48(int64_t &val)
{
    uint16_t high;
    uint32_t low;
    if (m_build) {
        if (val < INT48_MIN || val > INT48_MAX) {
            m_err = MNG_PARSE_ERROR_VAL;
            return true;
        }
        high = (val >> 32) & INT16_MAX;
        if (val < 0) // Add sign bit for negative
            high |= (INT16_MAX + 1);
        low = val & UINT32_MAX;
    }
    if (u16(high) || u32(low))
        return true;
    if (!m_build)
    {
        uint64_t ret = low | (((uint64_t)high & INT16_MAX) << 32);
        if (high & (INT16_MAX + 1)) // Add sign bit for negative
            ret |= UINT64_C(0x8000000000000000); // 64 bits negative sign bit
         val = (int64_t)ret;
    }
    return false;
}
bool message::i64(int64_t &val)
{
    if (m_left < 8)
        return true;
    if (m_build)
        *(uint64_t*)m_cur = hton64((uint64_t)val);
    else
        val = (int64_t)ntoh64(*(uint64_t*)m_cur);
    move(8);
    return false;
}
bool message::buffer(std::string &str, uint16_t len)
{
    if (m_build) // On build ignore len variable
        len = str.length();
    if (m_left < (ssize_t)len)
        return true;
    if (m_build)
        memcpy(m_cur, str.c_str(), len);
    else
        str = std::string((char *)m_cur, len);
    move(len);
    return false;
}
bool message::buffer(uint8_t *val, size_t len)
{
    if (m_left < (ssize_t)len)
        return true;
    if (m_build)
        memcpy(m_cur, val, len);
    else
        memcpy(val, m_cur, len);
    move(len);
    return false;
}
bool message::TimeInterval_f(TimeInterval_t &v)
{
    return u64(v.scaledNanoseconds);
}
bool message::Timestamp_f(Timestamp_t &d)
{
    return u48(d.secondsField) || u32(d.nanosecondsField);
}
bool message::ClockIdentity_f(ClockIdentity_t &v)
{
    return buffer(v, ClockIdentity_s);
}
bool message::PortIdentity_f(PortIdentity_t &d)
{
    return ClockIdentity_f(d.clockIdentity) || u16(d.portNumber);
}
bool message::PortAddress_f(PortAddress_t &d)
{
    uint16_t net = d.networkProtocol;
    d.addressLength =  d.addressField.length();
    if (u16(net) || u16(d.addressLength) ||
        buffer(d.addressField, d.addressLength))
        return true;
    d.networkProtocol = (networkProtocol_e)net;
    return false;
}
bool message::ClockQuality_f(ClockQuality_t &d)
{
    uint8_t acc = d.clockAccuracy;
    if (u8(d.clockClass) || u8(acc) || u16(d.offsetScaledLogVariance))
        return true;
    d.clockAccuracy = (clockAccuracy_e)acc;
    return false;
}
bool message::PTPText_f(PTPText_t &d)
{
    d.lengthField = d.textField.length();
    return u8(d.lengthField) || buffer(d.textField, d.lengthField);
}
bool message::FaultRecord_f(FaultRecord_t &d)
{
    uint8_t code;
    if (m_build)
        code = d.severityCode;
    if (u16(d.faultRecordLength) || Timestamp_f(d.faultTime) || u8(code) ||
        PTPText_f(d.faultName) || PTPText_f(d.faultValue) ||
        PTPText_f(d.faultDescription))
        return true;

    if (d.faultRecordLength != 16 + d.faultName.lengthField +
        d.faultValue.lengthField + d.faultDescription.lengthField) {
        m_err = MNG_PARSE_ERROR_SIZE_MISS;
        return true;
    }
    if (!m_build)
        d.severityCode = (faultRecord_e)code;
    return false;
}
bool message::AcceptableMaster_f(AcceptableMaster_t &d)
{
    return PortIdentity_f(d.acceptablePortIdentity) ||
           u8(d.alternatePriority1);
}
size_t message::PortAddress_l(PortAddress_t &d)
{
    return 4 + d.addressField.length();
}
size_t message::PTPText_l(PTPText_t &d)
{
    return 1 + d.textField.length();
}
size_t message::FaultRecord_l(FaultRecord_t &d)
{
    return 3 + sizeof(Timestamp_t) + PTPText_l(d.faultName) +
           PTPText_l(d.faultValue) + PTPText_l(d.faultDescription);
}
/* Must be here, must be in library binary and not header source code! */
const char *message::getVersion()
{
    // Need 2 levels to stringify macros value instead of macro name
    #define stringify(s) #s
    #define VER_STR(a, b) stringify(a) "." stringify(b)
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
