/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief Parse and build PTP management TLVs
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 */

#ifndef __PTPMGMT_MSG_DEF_H
#define __PTPMGMT_MSG_DEF_H

#include "proc.h" /* Structures for management TLVs */
#include "sig.h" /* Structures for signaling TLVs */

__PTPMGMT_NAMESPACE_BEGIN

/**
 * @brief Handle PTP management message
 * @details
 *  Handle parse and build of a PTP management massage.
 *  Handle TLV specific dataField by calling a specific call-back per TLV id
 */
struct MsgProc {
    bool              m_build; /* true on build */
    uint8_t          *m_cur;
    ssize_t           m_left;
    size_t            m_size;  /* TLV data size on build */
    MNG_PARSE_ERROR_e m_err; /* Last TLV err */
    uint8_t           reserved; /* Used for reserved values */

    MNG_PARSE_ERROR_e call_tlv_data(mng_vals_e id, BaseMngTlv *&tlv);
    MNG_PARSE_ERROR_e parseSig();

#define _ptpmCaseUF(n) bool n##_f(n##_t &data);
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    /* Per tlv ID call-back for parse or build or both */
#include "ids.h"
    /* Parse functions for signalling messages */
#define _ptpmParseFunc(n) bool n##_f(n##_t &data)
    _ptpmParseFunc(MANAGEMENT_ERROR_STATUS);
    _ptpmParseFunc(ORGANIZATION_EXTENSION);
    _ptpmParseFunc(PATH_TRACE);
    _ptpmParseFunc(ALTERNATE_TIME_OFFSET_INDICATOR);
    _ptpmParseFunc(ENHANCED_ACCURACY_METRICS);
    _ptpmParseFunc(L1_SYNC);
    _ptpmParseFunc(PORT_COMMUNICATION_AVAILABILITY);
    _ptpmParseFunc(PROTOCOL_ADDRESS);
    _ptpmParseFunc(SLAVE_RX_SYNC_TIMING_DATA);
    _ptpmParseFunc(SLAVE_RX_SYNC_COMPUTED_DATA);
    _ptpmParseFunc(SLAVE_TX_EVENT_TIMESTAMPS);
    _ptpmParseFunc(CUMULATIVE_RATE_RATIO);
    _ptpmParseFunc(SLAVE_DELAY_TIMING_DATA_NP);
#undef _ptpmParseFunc

    /* Parsing functions */
    void move(size_t val) {
        m_cur += val;
        m_left -= val;
        m_size += val;
    }
    /* val in network order */
    bool proc(uint8_t &val);
    bool proc(uint16_t &val);
    bool proc(uint32_t &val);
    bool proc48(uint64_t &val);
    bool proc(uint64_t &val);
    bool proc(int8_t &val);
    bool proc(int16_t &val);
    bool proc(int32_t &val);
    bool proc48(int64_t &val);
    bool proc(int64_t &val);
    bool proc(Float64_t &val);
    bool proc(std::string &str, uint16_t len);
    bool proc(Binary &bin, uint16_t len);
    bool proc(uint8_t *val, size_t len);
    /* For Enumerators using 8 bits */
    template <typename T> bool procE8(T &val);
    bool proc(clockAccuracy_e &val) { return procE8(val); }
    bool proc(faultRecord_e &val) { return procE8(val); }
    bool proc(timeSource_e &val) { return procE8(val); }
    bool proc(portState_e &val) { return procE8(val); }
    bool proc(msgType_e &val) { return procE8(val); }
    bool proc(linuxptpTimeStamp_e &val) { return procE8(val); }
    /* For Enumerators using 16 bits */
    template <typename T> bool procE16(T &val);
    bool proc(networkProtocol_e &val) { return procE16(val); }
    bool proc(linuxptpPowerProfileVersion_e &val) { return procE16(val); }
    bool proc(TimeInterval_t &v);
    bool proc(Timestamp_t &d);
    bool proc(ClockIdentity_t &v);
    bool proc(PortIdentity_t &d);
    bool proc(PortAddress_t &d);
    bool proc(ClockQuality_t &d);
    bool proc(PTPText_t &d);
    bool proc(FaultRecord_t &d);
    bool proc(AcceptableMaster_t &d);
    bool proc(linuxptpUnicastState_e &val) { return procE16(val); }
    bool proc(LinuxptpUnicastMaster_t &d);
    bool proc(SLAVE_RX_SYNC_TIMING_DATA_rec_t &rec);
    bool proc(SLAVE_RX_SYNC_COMPUTED_DATA_rec_t &rec);
    bool proc(SLAVE_TX_EVENT_TIMESTAMPS_rec_t &rec);
    bool proc(SLAVE_DELAY_TIMING_DATA_NP_rec_t &rec);
    bool procFlags(uint8_t &flags, const uint8_t flagsMask);
    /* linuxptp PORT_STATS_NP statistics use little endian */
    bool procLe(uint64_t &val);
    /* list proccess with count */
    template <typename T> bool vector_f(uint32_t count, std::vector<T> &vec);
    /* countless list proccess */
    template <typename T> bool vector_o(std::vector<T> &vec);
};

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_MSG_DEF_H */
