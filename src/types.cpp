/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2025 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Types, enumerators, and structers used by PTP management messages
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Erez Geva
 *
 */

#include <cmath>
#include "msg.h"
#include "c/msg.h"
#include "timeCvrt.h"
#include "comp.h"

__PTPMGMT_NAMESPACE_BEGIN

const uint16_t allPorts = UINT16_MAX;
const ClockIdentity_t allClocks = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

struct floor_t {
    int64_t intg;
    float_seconds rem;
};
static inline floor_t _floor(float_seconds val)
{
    floor_t ret;
    ret.intg = floorl(val);
    ret.rem = val - ret.intg;
    return ret;
}

float_nanoseconds TimeInterval_t::getInterval() const
{
    return (float_nanoseconds)scaledNanoseconds / 0x10000;
}
int64_t TimeInterval_t::getIntervalInt() const
{
    if(scaledNanoseconds < 0)
        return -((-scaledNanoseconds) >> 16);
    return scaledNanoseconds >> 16;
}
size_t TimeInterval_t::size()
{
    return sizeof scaledNanoseconds;
}
bool TimeInterval_t::isZero() const
{
    return scaledNanoseconds == 0;
}
Timestamp_t::Timestamp_t() : secondsField(0), nanosecondsField(0) {}
Timestamp_t::Timestamp_t(int64_t secs, uint32_t nsecs) : secondsField(secs),
    nanosecondsField(nsecs) {}
string Timestamp_t::string() const
{
    char buf[200];
    snprintf(buf, sizeof buf, "%ju.%.9u", secondsField, nanosecondsField);
    return buf;
}
Timestamp_t::Timestamp_t(const timespec &ts)
{
    secondsField = ts.tv_sec;
    nanosecondsField = ts.tv_nsec;
}
void Timestamp_t::toTimespec(timespec &ts) const
{
    ts.tv_sec = secondsField;
    ts.tv_nsec = nanosecondsField;
}
Timestamp_t::operator timespec() const
{
    timespec ts;
    toTimespec(ts);
    return ts;
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
Timestamp_t::operator timeval() const
{
    timeval tv;
    toTimeval(tv);
    return tv;
}
Timestamp_t::Timestamp_t(float_seconds seconds)
{
    fromFloat(seconds);
}
void Timestamp_t::fromFloat(float_seconds seconds)
{
    floor_t ret = _floor(seconds);
    secondsField = ret.intg;
    nanosecondsField = ret.rem * NSEC_PER_SEC;
}
float_seconds Timestamp_t::toFloat() const
{
    return (float_seconds)nanosecondsField / NSEC_PER_SEC + secondsField;
}
void Timestamp_t::fromNanoseconds(uint64_t nanoseconds)
{
    lldiv_t d = lldiv((long long)nanoseconds, (long long)NSEC_PER_SEC);
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
bool Timestamp_t::eq(const Timestamp_t &ts) const
{
    return secondsField == ts.secondsField &&
        nanosecondsField == ts.nanosecondsField;
}
bool Timestamp_t::eq(float_seconds seconds) const
{
    // We use unsigned, negitive can not be equal
    if(seconds < 0)
        return false;
    uint64_t secs = floorl(seconds);
    if(secondsField == secs) {
        uint32_t nano = (seconds - secs) * NSEC_PER_SEC;
        uint32_t diff = nano > nanosecondsField ?
            nano - nanosecondsField : nanosecondsField - nano;
        // printf("secs %lu diff %d\n", secs, diff);
        if(diff < 10)
            return true;
    }
    return false;
}
bool Timestamp_t::less(const Timestamp_t &ts) const
{
    return secondsField < ts.secondsField ||
        (secondsField == ts.secondsField &&
            nanosecondsField < ts.nanosecondsField);
}
bool Timestamp_t::less(float_seconds seconds) const
{
    // As we use unsigned, we are always bigger than negitive
    if(seconds < 0)
        return false;
    uint64_t secs = floorl(seconds);
    if(secondsField == secs)
        return nanosecondsField < (seconds - secs) * NSEC_PER_SEC;
    return secondsField < secs;
}
size_t Timestamp_t::size()
{
    return sizeof_UInteger48_t + sizeof nanosecondsField;
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
Timestamp_t &Timestamp_t::add(float_seconds seconds)
{
    floor_t ret = _floor(seconds);
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
Timestamp_t &Timestamp_t::subt(float_seconds seconds)
{
    return add(-seconds);
}
bool Timestamp_t::isZero() const
{
    return secondsField == 0 && nanosecondsField == 0;
}
string ClockIdentity_t::string() const
{
    char buf[25];
    snprintf(buf, sizeof buf, "%02x%02x%02x.%02x%02x.%02x%02x%02x",
        v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
    return buf;
}
void ClockIdentity_t::clear(int val)
{
    memset(v, val, 8);
}
bool ClockIdentity_t::eq(const ClockIdentity_t &rhs) const
{
    return memcmp(v, rhs.v, size()) == 0;
}
bool ClockIdentity_t::less(const ClockIdentity_t &rhs) const
{
    return memcmp(v, rhs.v, size()) < 0;
}
bool ClockIdentity_t::eq(const Binary &bin) const
{
    return bin.size() == size() &&
        memcmp(v, bin.get(), size()) == 0;
}
size_t ClockIdentity_t::size() { return sizeof v; }
string PortIdentity_t::string() const
{
    std::string ret = clockIdentity.string();
    ret += "-";
    ret += to_string(portNumber);
    return ret;
}
void PortIdentity_t::clear()
{
    clockIdentity.clear();
    portNumber = 0;
}
bool PortIdentity_t::eq(const PortIdentity_t &rhs) const
{
    return clockIdentity == rhs.clockIdentity && portNumber == rhs.portNumber;
}
bool PortIdentity_t::less(const PortIdentity_t &rhs) const
{
    return clockIdentity == rhs.clockIdentity ?
        portNumber < rhs.portNumber : clockIdentity < rhs.clockIdentity;
}
size_t PortIdentity_t::size()
{
    return ClockIdentity_t::size() + sizeof portNumber;
}
string PortAddress_t::string() const
{
    switch(networkProtocol) {
        case UDP_IPv4:
            FALLTHROUGH;
        case UDP_IPv6:
            return addressField.toIp();
        case IEEE_802_3:
            break;
        case DeviceNet:
            break;
        case ControlNet:
            break;
        case PROFINET:
            break;
    }
    return addressField.toId();
}
bool PortAddress_t::eq(const PortAddress_t &rhs) const
{
    return networkProtocol == rhs.networkProtocol &&
        addressField == rhs.addressField;
}
bool PortAddress_t::less(const PortAddress_t &rhs) const
{
    return networkProtocol == rhs.networkProtocol ? addressField <
        rhs.addressField : networkProtocol < rhs.networkProtocol;
}
size_t PortAddress_t::size() const
{
    return sizeof networkProtocol + sizeof addressLength +
        addressField.length();
}
size_t ClockQuality_t::size()
{
    return sizeof clockClass + sizeof clockAccuracy +
        sizeof offsetScaledLogVariance;
}
size_t PTPText_t::size() const
{
    return sizeof lengthField + textField.length();
}
const char *PTPText_t::string() const
{
    return textField.c_str();
}
size_t FaultRecord_t::size() const
{
    return sizeof faultRecordLength + faultTime.size() + sizeof severityCode +
        faultName.size() + faultValue.size() + faultDescription.size();
}
size_t AcceptableMaster_t::size()
{
    return PortIdentity_t::size() + sizeof alternatePriority1;
}
size_t LinuxptpUnicastMaster_t::size() const
{
    return portIdentity.size() + clockQuality.size() +
        sizeof selected + sizeof portState + sizeof priority1 +
        sizeof priority2 + portAddress.size();
}
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
size_t MsgParams::countSigTlvs() const
{
    return allowSigTlvs.size();
}
MsgParams::MsgParams() :
    transportSpecific(0),
    domainNumber(0),
    boundaryHops(1),
    minorVersion(0),
    isUnicast(true),
    implementSpecific(linuxptp),
    target{allClocks, allPorts},
    m_init(self_id),
    useZeroGet(true),
    rcvSignaling(false),
    filterSignaling(true),
    rcvSMPTEOrg(true),
    sendAuth(true),
    rcvAuth(RCV_AUTH_ALL)
{
}

__PTPMGMT_NAMESPACE_END

__PTPMGMT_NAMESPACE_USE;

__PTPMGMT_C_BEGIN

// C interfaces
static void ptpmgmt_MsgParams_free(ptpmgmt_pMsgParams m)
{
    if(m != nullptr) {
        delete(MsgParams *)m->_this;
        free(m);
    }
}
static void ptpmgmt_MsgParams_free_wrap(ptpmgmt_pMsgParams m)
{
}
void cpyMsgParams(ptpmgmt_pMsgParams p)
{
    MsgParams &r = *(MsgParams *)p->_this;
    p->transportSpecific = r.transportSpecific;
    p->domainNumber = r.domainNumber;
    p->boundaryHops = r.boundaryHops;
    p->minorVersion = r.minorVersion;
    p->isUnicast = r.isUnicast;
    p->useZeroGet = r.useZeroGet;
    p->rcvSignaling = r.rcvSignaling;
    p->filterSignaling = r.filterSignaling;
    p->rcvSMPTEOrg = r.rcvSMPTEOrg;
    p->sendAuth = r.sendAuth;
    p->rcvAuth = (ptpmgmt_MsgParams_RcvAuth_e)r.rcvAuth;
    p->implementSpecific = (ptpmgmt_implementSpecific_e)r.implementSpecific;
    memcpy(p->target.clockIdentity.v, r.target.clockIdentity.v,
        ClockIdentity_t::size());
    p->target.portNumber = r.target.portNumber;
    memcpy(p->self_id.clockIdentity.v, r.self_id.clockIdentity.v,
        ClockIdentity_t::size());
    p->self_id.portNumber = r.self_id.portNumber;
}
static void ptpmgmt_allowSigTlv(ptpmgmt_pMsgParams m, ptpmgmt_tlvType_e type)
{
    if(m != nullptr && m->_this != nullptr)
        ((MsgParams *)m->_this)->allowSigTlv((tlvType_e)type);
}
static void ptpmgmt_removeSigTlv(ptpmgmt_pMsgParams m, ptpmgmt_tlvType_e type)
{
    if(m != nullptr && m->_this != nullptr)
        ((MsgParams *)m->_this)->removeSigTlv((tlvType_e)type);
}
static bool ptpmgmt_isSigTlv(ptpmgmt_cpMsgParams m, ptpmgmt_tlvType_e type)
{
    if(m != nullptr && m->_this != nullptr)
        return ((MsgParams *)m->_this)->isSigTlv((tlvType_e)type);
    return false;
}
static size_t ptpmgmt_countSigTlvs(ptpmgmt_cpMsgParams m)
{
    if(m != nullptr && m->_this != nullptr)
        return ((MsgParams *)m->_this)->countSigTlvs();
    return 0;
}
static inline void ptpmgmt_MsgParams_asign_cb(ptpmgmt_pMsgParams m)
{
    m->allowSigTlv  = ptpmgmt_allowSigTlv;
    m->removeSigTlv = ptpmgmt_removeSigTlv;
    m->isSigTlv     = ptpmgmt_isSigTlv;
    m->countSigTlvs = ptpmgmt_countSigTlvs;
    cpyMsgParams(m);
}
ptpmgmt_pMsgParams ptpmgmt_MsgParams_alloc()
{
    ptpmgmt_pMsgParams m =
        (ptpmgmt_pMsgParams)malloc(sizeof(ptpmgmt_MsgParams));
    if(m == nullptr)
        return nullptr;
    m->_this = (void *)(new MsgParams);
    if(m->_this == nullptr) {
        free(m);
        return nullptr;
    }
    ptpmgmt_MsgParams_asign_cb(m);
    m->free = ptpmgmt_MsgParams_free;
    return m;
}
void ptpmgmt_MsgParams_alloc_wrap(ptpmgmt_pMsgParams p)
{
    ptpmgmt_MsgParams_asign_cb(p);
    p->free = ptpmgmt_MsgParams_free_wrap;
}

__PTPMGMT_C_END
