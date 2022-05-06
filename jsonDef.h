/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief JSON base structure used by both build and parse of JSON
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 *
 * @note Internal use header.
 *       Not used by SWIG, Doxygen, or in development package
 */

#ifndef __PTPMGMT_JSON_DEFS_H
#define __PTPMGMT_JSON_DEFS_H

#include "json.h"

namespace ptpmgmt
{

struct JsonProc {
    bool procData(mng_vals_e managementId, const BaseMngTlv *&data);
#define _ptpmProcType(type) \
    virtual bool procValue(const char *name, type &val) = 0;
    _ptpmProcType(uint8_t)
    _ptpmProcType(uint16_t)
    _ptpmProcType(uint32_t)
    _ptpmProcType(uint64_t)
    _ptpmProcType(int8_t)
    _ptpmProcType(int16_t)
    _ptpmProcType(int32_t)
    _ptpmProcType(int64_t)
    _ptpmProcType(float)
    _ptpmProcType(double)
    _ptpmProcType(long double)
    _ptpmProcType(networkProtocol_e)
    _ptpmProcType(clockAccuracy_e)
    _ptpmProcType(faultRecord_e)
    _ptpmProcType(timeSource_e)
    _ptpmProcType(portState_e)
    _ptpmProcType(linuxptpTimeStamp_e)
    _ptpmProcType(linuxptpPowerProfileVersion_e)
    _ptpmProcType(linuxptpUnicastState_e)
    _ptpmProcType(TimeInterval_t)
    _ptpmProcType(Timestamp_t)
    _ptpmProcType(ClockIdentity_t)
    _ptpmProcType(PortIdentity_t)
    _ptpmProcType(PortAddress_t)
    _ptpmProcType(ClockQuality_t)
    _ptpmProcType(PTPText_t)
    _ptpmProcType(FaultRecord_t)
    _ptpmProcType(AcceptableMaster_t)
    _ptpmProcType(LinuxptpUnicastMaster_t)
    virtual bool procBinary(const char *name, Binary &val, uint16_t &len) = 0;
    virtual bool procBinary(const char *name, uint8_t *val, size_t len) = 0;
    virtual bool procFlag(const char *name, uint8_t &flags, int mask) = 0;
#define _ptpmProcVector(type) \
    virtual bool procArray(const char *name, std::vector<type> &val) = 0;
    _ptpmProcVector(ClockIdentity_t)
    _ptpmProcVector(PortAddress_t)
    _ptpmProcVector(FaultRecord_t)
    _ptpmProcVector(AcceptableMaster_t)
    _ptpmProcVector(LinuxptpUnicastMaster_t)
};

}; /* namespace ptpmgmt */

#endif /* __PTPMGMT_JSON_DEFS_H */
