/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief Create and parse PTP managment messages
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * Created following "IEEE Std 1588-2008", PTP version 2
 */

#include <cstring>
#include <arpa/inet.h>
#include "end.h"
#include "msg.h"

#define caseItem(a) a: return #a

const uint8_t ptp_major_ver = 0x2; // low 4 bits, portDS.versionNumber
const uint8_t ptp_minor_ver = 0x0; // IEEE 1588-2019 uses 0x1
const uint8_t ptp_version = (ptp_minor_ver << 4) | ptp_major_ver;
const uint8_t messageType_Management = 0xd; // low 4 bits
const uint8_t controlField_Management = 0x04;
const uint8_t logMessageInterval_Management = 0x7f;
const uint16_t allPorts = UINT16_MAX;
const ClockIdentity_t allClocks = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const uint16_t u16_sig_bit = 1 << 15;           // 16 bits negative sign bit
const uint64_t u64_sig_bit = (uint64_t)1 << 63; // 64 bits negative sign bit

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
enum scope_e : uint8_t {
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
    uint32_t reserved;
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
#define A(n, v, sc, a, sz, f) case##f(n)
#define caseNA(n) case n: return MNG_PARSE_ERROR_OK;
#define caseUF(n) case n:\
        if (m_build) { if (n##_f(*(n##_t *)m_dataSend)) return m_err;\
        } else {\
            n##_t *t = new n##_t;\
            if (t == nullptr) return MNG_PARSE_ERROR_MEM;\
            m_dataGet = std::move(std::unique_ptr<baseData>(t));\
            if (n##_f(*t)) return m_err;\
        } break;
    // The default error on build or parsing
    m_err = MNG_PARSE_ERROR_TOO_SMALL;
    switch(m_tlv_id) {
#include "ids.h"
        default:
            return MNG_PARSE_ERROR_UNSUPPORT;
    }
    // The mng ID is not supported yet
    return MNG_PARSE_ERROR_OK;
}

bool message::findTlvId(uint16_t val)
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
    if(!m_prms.useLinuxPTPTlvs && mng_all_vals[id].allowed & A_USE_LINUXPTP)
        return false;
    m_tlv_id = id;
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
    if(!m_prms.useLinuxPTPTlvs && mng_all_vals[id].allowed & A_USE_LINUXPTP)
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
    m_tlv_id(FIRST_MNG_ID),
    m_sendAction(GET),
    m_replyAction(RESPONSE),
    m_msgLen(0),
    m_sequence(0),
    m_isUnicast(true),
    m_dataSend(nullptr),
    m_peer{0}
{
    m_prms = {
        .boundaryHops = 1,
        .isUnicast = true,
        .useLinuxPTPTlvs = true,
    };
    setAllClocks();
}
message::message(msgParams prms) :
    m_tlv_id(FIRST_MNG_ID),
    m_sendAction(GET),
    m_replyAction(RESPONSE),
    m_msgLen(0),
    m_sequence(0),
    m_isUnicast(true),
    m_dataSend(nullptr),
    m_prms(prms),
    m_peer{0}
{
    if(m_prms.transportSpecific > 0xf)
        m_prms.transportSpecific = 0;
}
ssize_t message::getMsgPlanedLen()
{
    // That should not happen, precaustion
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
    // return total length of to the message to be send send
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
    baseData &dataSend)
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
    managementMessage_t *msg = (managementMessage_t *)buf;
    *msg = {0};
    msg->messageType_transportSpecific = (messageType_Management |
            (m_prms.transportSpecific << 4)) & UINT8_MAX;
    msg->versionPTP = ptp_version;
    msg->domainNumber = m_prms.domainNumber;
    if(m_prms.isUnicast)
        msg->flagField[0] |= unicastFlag;
    msg->sequenceId = cpu_to_net16(sequence);
    msg->controlField = controlField_Management;
    msg->logMessageInterval = logMessageInterval_Management;
    msg->startingBoundaryHops = m_prms.boundaryHops;
    msg->boundaryHops = m_prms.boundaryHops;
    msg->actionField = m_sendAction;
    msg->targetPortIdentity.clockIdentity = m_prms.target.clockIdentity;
    msg->targetPortIdentity.portNumber = cpu_to_net16(m_prms.target.portNumber);
    msg->sourcePortIdentity.clockIdentity = m_prms.self_id.clockIdentity;
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
        // Ensure reserve fileds are zero
        reserved = 0;
        MNG_PARSE_ERROR_e err = call_tlv_data();
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
    if(msgSize < (ssize_t)sizeof(managementMessage_t) + tlvSize)
        return MNG_PARSE_ERROR_TOO_SMALL;
    managementMessage_t *msg = (managementMessage_t *)buf;
    uint8_t ptp_maj = msg->versionPTP & 0xf;
    uint8_t ptp_min = msg->versionPTP >> 4;
    if(msg->messageType_transportSpecific != ((messageType_Management |
                (m_prms.transportSpecific << 4)) & UINT8_MAX) ||
        ptp_maj != ptp_major_ver || ptp_min < ptp_minor_ver ||
        msg->domainNumber != m_prms.domainNumber ||
        msg->controlField != controlField_Management ||
        msg->logMessageInterval != logMessageInterval_Management)
        return MNG_PARSE_ERROR_HEADER;
    m_isUnicast = msg->flagField[0] & unicastFlag;
    m_sequence = net_to_cpu16(msg->sequenceId);
    uint8_t actionField = 0xf & msg->actionField;
    if(actionField != RESPONSE && actionField != ACKNOWLEDGE)
        return MNG_PARSE_ERROR_ACTION;
    m_replyAction = (actionField_e)actionField;
    m_peer.portNumber = net_to_cpu16(msg->sourcePortIdentity.portNumber);
    m_peer.clockIdentity = msg->sourcePortIdentity.clockIdentity;
    uint16_t *cur = (uint16_t *)(msg + 1);
    uint16_t tlvType = net_to_cpu16(*cur);
    m_build = false;
    ssize_t size = msgSize - sizeof(managementMessage_t);
    if(MANAGEMENT_ERROR_STATUS == tlvType) {
        if(size < (ssize_t)sizeof(managementErrorTLV_t))
            return MNG_PARSE_ERROR_TOO_SMALL;
        size -= sizeof(managementErrorTLV_t);
        managementErrorTLV_t *errTlv = (managementErrorTLV_t *)cur;
        if(!findTlvId(errTlv->managementId))
            return MNG_PARSE_ERROR_INVALID_ID;
        if(!checkReplyAction(actionField))
            return MNG_PARSE_ERROR_ACTION;
        m_errorId = net_to_cpu16(errTlv->managementErrorId);
        m_left = net_to_cpu16(errTlv->lengthField);
        // check minimum size and even
        if(m_left < lengthFieldMngErrBase || m_left & 1)
            return MNG_PARSE_ERROR_TOO_SMALL;
        m_left -= lengthFieldMngErrBase;
        m_cur = (uint8_t *)(errTlv + 1);
        // Check displayData size
        if(size < m_left)
            return MNG_PARSE_ERROR_TOO_SMALL;
        if(m_left > 1 && proc(m_errorDisplay))
            return MNG_PARSE_ERROR_TOO_SMALL;
        return MNG_PARSE_ERROR_MSG;
    } else if(MANAGEMENT != tlvType)
        return MNG_PARSE_ERROR_INVALID_TLV;
    if(size < (ssize_t)sizeof(managementTLV_t))
        return MNG_PARSE_ERROR_TOO_SMALL;
    managementTLV_t *tlv = (managementTLV_t *)cur;
    size -= sizeof(managementTLV_t);
    if(!findTlvId(tlv->managementId))
        return MNG_PARSE_ERROR_INVALID_ID;
    if(!checkReplyAction(actionField))
        return MNG_PARSE_ERROR_ACTION;
    m_left = net_to_cpu16(tlv->lengthField);
    // Check minimum size and even
    if(m_left < lengthFieldMngBase || m_left & 1)
        return MNG_PARSE_ERROR_TOO_SMALL;
    m_left -= lengthFieldMngBase;
    if(m_left == 0)
        return MNG_PARSE_ERROR_OK;
    m_cur = (uint8_t *)(tlv + 1);
    if(size < m_left) // Check dataField size
        return MNG_PARSE_ERROR_TOO_SMALL;
    return call_tlv_data();
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
bool message::proc(std::string &str, uint16_t len)
{
    if(m_build) // On build ignore len variable
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
    if(m_build) // On build ignore len variable
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
