/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Set and get the configuration of time base
 *
 * @author Song Yoong Siang <yoong.siang.song@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "pub/clkmgr/timebase_configs.h"
#include "common/timebase.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

PTPCfg::PTPCfg(const string &ifName, uint8_t transportSpecific,
    uint8_t domainNumber, int ifIndex, int ptpIndex) :
    m_ifName(ifName), m_transportSpecific(transportSpecific),
    m_domainNumber(domainNumber), m_ifIndex(ifIndex), m_ptpIndex(ptpIndex)
{
}
const string &PTPCfg::ifName() const
{
    return m_ifName;
}
const char *PTPCfg::ifName_c() const
{
    return m_ifName.c_str();
}
int PTPCfg::ifIndex() const
{
    return m_ifIndex;
}
int PTPCfg::ptpIndex() const
{
    return m_ptpIndex;
}
uint8_t PTPCfg::transportSpecific() const
{
    return m_transportSpecific;
}
uint8_t PTPCfg::domainNumber() const
{
    return m_domainNumber;
}
TimeBaseRecord::TimeBaseRecord(size_t index, const string &name) :
    m_index(index), m_name(name)
{
}
void TimeBaseRecord::setPtp(PTPCfg &&ptp)
{
    m_ptp = std::move(ptp);
    m_have_ptp = true;
}
size_t TimeBaseRecord::index() const
{
    return m_index;
}
const string &TimeBaseRecord::name() const
{
    return m_name;
}
const char *TimeBaseRecord::name_c() const
{
    return m_name.c_str();
}
bool TimeBaseRecord::havePtp() const
{
    return m_have_ptp;
}
const PTPCfg &TimeBaseRecord::ptp() const
{
    return m_ptp;
}
bool TimeBaseRecord::haveSysClock() const
{
    return m_have_sys;
}
TimeBaseConfigurations::iterator::iterator(
    const map<size_t, TimeBaseRecord>::const_iterator &_it) : it(_it)
{
}
TimeBaseConfigurations::iterator &TimeBaseConfigurations::iterator::operator++()
{
    ++it;
    return *this;
}
TimeBaseConfigurations::iterator
&TimeBaseConfigurations::iterator::operator++(int)
{
    it++;
    return *this;
}
const TimeBaseRecord &TimeBaseConfigurations::iterator::operator*()
{
    return it->second;
}
bool TimeBaseConfigurations::iterator::operator!=(iterator &o)
{
    return it != o.it;
}
TimeBaseConfigurations::iterator TimeBaseConfigurations::begin()
{
    return TimeBaseConfigurations::iterator(getInstWr().m_cfgs.begin());
}
TimeBaseConfigurations::iterator TimeBaseConfigurations::end()
{
    return TimeBaseConfigurations::iterator(getInstWr().m_cfgs.end());
}
TimeBaseConfigurations &TimeBaseConfigurations::getInstWr()
{
    static TimeBaseConfigurations instance;
    return instance;
}
const TimeBaseConfigurations &TimeBaseConfigurations::getInstance()
{
    return getInstWr();
}
void TimeBaseConfigurations::addTimeBaseCfg(const TimeBaseCfg &cfg)
{
    TimeBaseRecord r(cfg.timeBaseIndex, cfg.timeBaseName);
    if(cfg.interfaceName[0] != 0)
        r.setPtp(PTPCfg(cfg.interfaceName, cfg.transportSpecific,
                cfg.domainNumber));
    if(cfg.haveSys)
        r.m_have_sys = true;
    if(cfg.havePtp)
        r.m_have_ptp = true;
    getInstWr().m_cfgs[cfg.timeBaseIndex] = std::move(r);
}
const TimeBaseRecord &TimeBaseConfigurations::getRecord(size_t timeBaseIndex)
{
    static TimeBaseRecord dummy(0, "");
    return isTimeBaseIndexPresent(timeBaseIndex) ?
        getInstWr().m_cfgs.at(timeBaseIndex) : dummy;
}
bool TimeBaseConfigurations::isTimeBaseIndexPresent(size_t timeBaseIndex)
{
    return getInstWr().m_cfgs.count(timeBaseIndex) > 0;
}
bool TimeBaseConfigurations::BaseNameToBaseIndex(const string &timeBaseName,
    size_t &timeBaseIndex)
{
    for(const auto &it : getInstWr().m_cfgs)
        if(it.second.name() == timeBaseName) {
            timeBaseIndex = it.first;
            return true;
        }
    return false;
}
size_t TimeBaseConfigurations::size()
{
    return getInstWr().m_cfgs.size();
}

//////////////////////////////////
// C API
//////////////////////////////////

extern "C" {

#define getPtp(n, d)\
    if(TimeBaseConfigurations::isTimeBaseIndexPresent(timeBaseIndex)) {\
        const TimeBaseRecord &r = TimeBaseConfigurations::getRecord(timeBaseIndex);\
        if(r.havePtp()) return r.ptp().n();\
    } return d

    bool clkmgr_isTimeBaseIndexPresent(size_t timeBaseIndex)
    {
        return TimeBaseConfigurations::isTimeBaseIndexPresent(timeBaseIndex);
    }
    const char *clkmgr_ifName(size_t timeBaseIndex)
    {
        if(TimeBaseConfigurations::isTimeBaseIndexPresent(timeBaseIndex)) {
            const TimeBaseRecord &r =
                TimeBaseConfigurations::getRecord(timeBaseIndex);
            if(r.havePtp())
                return r.ptp().ifName().c_str();
        }
        return nullptr;
    }
    int clkmgr_ifIndex(size_t timeBaseIndex)
    {
        getPtp(ifIndex, -1);
    }
    int clkmgr_ptpIndex(size_t timeBaseIndex)
    {
        getPtp(ptpIndex, -1);
    }
    uint8_t clkmgr_transportSpecific(size_t timeBaseIndex)
    {
        getPtp(transportSpecific, 1);
    }
    uint8_t clkmgr_domainNumber(size_t timeBaseIndex)
    {
        getPtp(domainNumber, 0);
    }
    const char *clkmgr_timeBaseName(size_t timeBaseIndex)
    {
        if(TimeBaseConfigurations::isTimeBaseIndexPresent(timeBaseIndex))
            return TimeBaseConfigurations::getRecord(timeBaseIndex).name().c_str();
        return nullptr;
    }
    bool clkmgr_havePtp(size_t timeBaseIndex)
    {
        if(TimeBaseConfigurations::isTimeBaseIndexPresent(timeBaseIndex))
            return TimeBaseConfigurations::getRecord(timeBaseIndex).havePtp();
        return false;
    }
    bool clkmgr_haveSysClock(size_t timeBaseIndex)
    {
        if(TimeBaseConfigurations::isTimeBaseIndexPresent(timeBaseIndex))
            return TimeBaseConfigurations::getRecord(timeBaseIndex).haveSysClock();
        return false;
    }
    bool clkmgr_BaseNameToBaseIndex(const char *timeBaseName, size_t *timeBaseIndex)
    {
        return timeBaseIndex != nullptr &&
            TimeBaseConfigurations::BaseNameToBaseIndex(timeBaseName,
                *timeBaseIndex);
    }

};
