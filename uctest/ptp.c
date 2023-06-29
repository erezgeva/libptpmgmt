/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief PTP classes unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "ptp.h"
#include "err.h" // Verify the error is of old kernel

TestSuite(IfInfoTest, .init = initLibSys);
TestSuite(SysClockTest, .init = initLibSys);
TestSuite(PtpClockTest, .init = initLibSys);

// Tests initUsingName method
// bool initUsingName(ptpmgmt_ifInfo i, const char *ifName)
// bool isInit(ptpmgmt_ifInfo i)
// int ifIndex(ptpmgmt_ifInfo i)
// const char *ifName(ptpmgmt_ifInfo i)
// const uint8_t *mac(ptpmgmt_ifInfo i)
// size_t mac_size(ptpmgmt_ifInfo i)
// int ptpIndex(ptpmgmt_ifInfo i)
Test(IfInfoTest, MethodInitUsingName)
{
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    cr_expect(not(zero(ptr, i)));
    uint8_t a[] = { 1, 2, 3, 4, 5, 6 };
    useTestMode(true);
    bool r1 = i->initUsingName(i, "eth0");
    bool r2 = i->isInit(i);
    int r3 = i->ifIndex(i);
    char *r4 = (char *)i->ifName(i);
    int r5 = memcmp(i->mac(i), a, sizeof a);
    size_t r6 = i->mac_size(i);
    int r7 = i->ptpIndex(i);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(eq(int, r3, 7));
    cr_expect(eq(str, r4, "eth0"));
    cr_expect(zero(int, r5));
    cr_expect(eq(sz, r6, sizeof a));
    cr_expect(eq(int, r7, 3));
    i->free(i);
}

// Tests initUsingName method
// bool initUsingIndex(ptpmgmt_ifInfo i, int ifIndex)
// bool isInit(ptpmgmt_ifInfo i)
// int ifIndex(ptpmgmt_ifInfo i)
// const char *ifName(ptpmgmt_ifInfo i)
// const uint8_t *mac(ptpmgmt_ifInfo i)
// size_t mac_size(ptpmgmt_ifInfo i)
// int ptpIndex(ptpmgmt_ifInfo i)
Test(IfInfoTest, MethodInitUsingIndex)
{
    uint8_t a[] = { 1, 2, 3, 4, 5, 6 };
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    cr_expect(not(zero(ptr, i)));
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = i->isInit(i);
    int r3 = i->ifIndex(i);
    char *r4 = (char *)i->ifName(i);
    int r5 = memcmp(i->mac(i), a, sizeof a);
    size_t r6 = i->mac_size(i);
    int r7 = i->ptpIndex(i);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(eq(int, r3, 7));
    cr_expect(eq(str, r4, "eth0"));
    cr_expect(zero(int, r5));
    cr_expect(eq(sz, r6, sizeof a));
    cr_expect(eq(int, r7, 3));
    i->free(i);
}

// Tests getTime method
// struct timespec getTime()
Test(SysClockTest, MethodGetTime)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc_sys();
    useTestMode(true);
    struct timespec r1 = c->getTime(c);
    struct timespec r2 = c->getTime(c);
    useTestMode(false);
    cr_expect(eq(long, r1.tv_sec, 1));
    cr_expect(eq(ulong, r1.tv_nsec, 0));
    cr_expect(eq(long, r2.tv_sec, 2));
    cr_expect(eq(ulong, r2.tv_nsec, 0));
    c->free(c);
}

// Tests setTime method
// bool setTime(const struct timespec *ts)
Test(SysClockTest, MethodSetTime)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc_sys();
    struct timespec t = {12, 147};
    useTestMode(true);
    bool r1 = c->setTime(c, &t);
    useTestMode(false);
    cr_expect(r1);
    c->free(c);
}

// Tests offsetClock method
// bool offsetClock(int64_t offset)
Test(SysClockTest, MethodOffsetClock)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc_sys();
    useTestMode(true);
    bool r1 = c->offsetClock(c, 17000000029);
    bool r2 = c->offsetClock(c, -19999999963);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    c->free(c);
}

// Tests getFreq method
// double getFreq()
Test(SysClockTest, MethodGetFreq)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc_sys();
    useTestMode(true);
    double f = c->getFreq(c);
    useTestMode(false);
    cr_expect(epsilon_eq(ldbl, f, 77600000003.875732l, 0.000001l));
    c->free(c);
}

// Tests setFreq method
// bool setFreq(double freq)
Test(SysClockTest, MethodSetFreq)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc_sys();
    useTestMode(true);
    bool r1 = c->setFreq(c, 376000000.0l);
    useTestMode(false);
    cr_expect(r1);
    c->free(c);
}

// Tests setPhase method
// bool setPhase(int64_t offset)
Test(SysClockTest, MethodSetPhase)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc_sys();
    useTestMode(true);
    bool r1 = c->setPhase(c, 218150012.0l);
    useTestMode(false);
    cr_expect(r1);
    c->free(c);
}

// Tests isCharFile method
// bool ptpmgmt_clock_isCharFile(const char *file)
Test(PtpClockTest, MethodIsCharFile)
{
    useTestMode(true);
    bool r1 = ptpmgmt_clock_isCharFile("/dev/ptp0");
    useTestMode(false);
    cr_expect(r1);
}

// Tests initUsingDevice method
// bool initUsingDevice(const char *device, bool readonly)
// bool isInit()
// const char *device()
Test(PtpClockTest, MethodInitUsingDevice)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingDevice(c, "ptp0", false);
    bool r2 = c->isInit(c);
    char *r3 = (char *)c->device(c);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(eq(str, r3, "/dev/ptp0"));
    c->free(c);
}

// Tests initUsingIndex method
// bool initUsingIndex(int ptpIndex, bool readonly)
// clockid_t clkId()
// int getFd()
// int fileno()
// int ptpIndex()
Test(PtpClockTest, initUsingIndex)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    bool r2 = c->isInit(c);
    char *r3 = (char *)c->device(c);
    clockid_t r4 = c->clkId(c);
    int r5 = c->getFd(c);
    int r6 = c->fileno(c);
    int r7 = c->ptpIndex(c);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(eq(str, r3, "/dev/ptp0"));
    cr_expect(lt(int, r4, 0));
    cr_expect(gt(int, r5, 2));
    cr_expect(eq(int, r5, r6));
    cr_expect(eq(int, r7, 0));
    c->free(c);
}

// Tests init readonly
Test(PtpClockTest, initReadOnly)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 1, true);
    bool r2 = c->isInit(c);
    char *r3 = (char *)c->device(c);
    clockid_t r4 = c->clkId(c);
    int r5 = c->getFd(c);
    int r6 = c->fileno(c);
    int r7 = c->ptpIndex(c);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(eq(str, r3, "/dev/ptp1"));
    cr_expect(lt(int, r4, 0));
    cr_expect(gt(int, r5, 2));
    cr_expect(eq(int, r5, r6));
    cr_expect(eq(int, r7, 1));
    c->free(c);
}

// Tests getTime method
// struct timespec getTime()
Test(PtpClockTest, MethodGetTime)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 1, true);
    struct timespec r2 = c->getTime(c);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(eq(long, r2.tv_sec, 17));
    cr_expect(eq(ulong, r2.tv_nsec, 567));
    c->free(c);
}

// Tests setTime method
// bool setTime(const struct timespec *ts)
Test(PtpClockTest, MethodSetTime)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    struct timespec t = {19, 351};
    bool r2 = c->setTime(c, &t);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    c->free(c);
}

// Tests offsetClock method
// bool offsetClock(int64_t offset)
Test(PtpClockTest, MethodOffsetClock)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    bool r2 = c->offsetClock(c, 93000000571);
    bool r3 = c->offsetClock(c, -19999999963);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    c->free(c);
}

// Tests getFreq method
// double getFreq()
Test(PtpClockTest, MethodGetFreq)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 1, true);
    double f = c->getFreq(c);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(eq(ldbl, f, 9.979248046875l));
    c->free(c);
}

// Tests setFreq method
// bool setFreq(double freq)
Test(PtpClockTest, MethodSetFreq)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    bool r2 = c->setFreq(c, 234780.0l);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    c->free(c);
}

// Tests setPhase method
// bool setPhase(int64_t offset)
Test(PtpClockTest, MethodSetPhase)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    bool r2 = c->setPhase(c, 265963.0l);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    c->free(c);
}

// Tests setTimeFromSys method
// bool setTimeFromSys()
Test(PtpClockTest, MethodSetTimeFromSys)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    bool r2 = c->setTimeFromSys(c);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    c->free(c);
}

// Tests setTimeToSys method
// bool setTimeToSys()
Test(PtpClockTest, MethodSetTimeToSys)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    bool r2 = c->setTimeToSys(c);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    c->free(c);
}

// Tests fetchCaps method
// bool fetchCaps(PtpCaps_t &caps)
Test(PtpClockTest, MethodFetchCaps)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    struct ptp_clock_caps caps;
    bool r2 = c->fetchCaps(c, &caps);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(eq(int, caps.max_adj, 17));
    cr_expect(eq(int, caps.n_alarm, 32));
    cr_expect(eq(int, caps.n_ext_ts, 48));
    cr_expect(eq(int, caps.n_per_out, 75));
    cr_expect(gt(int, caps.pps, 0));
    cr_expect(eq(int, caps.n_pins, 12));
    cr_expect(gt(int, caps.cross_timestamping, 0));
    cr_expect(zero(int, caps.adjust_phase));
    c->free(c);
}

// Tests readPin method
// bool readPin(unsigned int index, PtpPin_t &pin)
Test(PtpClockTest, MethodReadPin)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    struct ptp_pin_desc pin;
    bool r2 = c->readPin(c, 1, &pin);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(eq(int, pin.index, 1));
    cr_expect(eq(str, pin.name, "pin desc"));
    cr_expect(eq(int, pin.chan, 19));
    cr_expect(eq(int, pin.func, PTP_PF_PHYSYNC));
    c->free(c);
}

// Tests writePin method
// bool writePin(PtpPin_t &pin)
Test(PtpClockTest, MethodWritePin)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    struct ptp_pin_desc pin = {
        .index = 2,
        .func = PTP_PF_PEROUT,
        .chan = 23
    };
    bool r2 = c->writePin(c, &pin);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    c->free(c);
}

// Tests ExternTSEbable method
// bool ExternTSEbable(unsigned int index, uint8_t flags)
Test(PtpClockTest, MethodExternTSEbable)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    bool r2 = c->ExternTSEbable(c, 7, PTP_RISING_EDGE);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    c->free(c);
}

// Tests ExternTSDisable method
// bool ExternTSDisable(unsigned int index)
Test(PtpClockTest, MethodExternTSDisable)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    bool r2 = c->ExternTSDisable(c, 9);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    c->free(c);
}

// Tests setPinPeriod method
// bool setPinPeriod(unsigned int index, PtpPinPeriodDef_t times, uint8_t flags)
Test(PtpClockTest, MethodSetPinPeriod)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    struct ptp_perout_request times;
    times.period.sec = 76;
    times.period.nsec = 154;
    bool r2 = c->setPinPeriod(c, 11, &times);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    c->free(c);
}

// Tests setPtpPpsEvent method
// bool setPtpPpsEvent(bool enable)
Test(PtpClockTest, MethodSetPtpPpsEvent)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    bool r2 = c->setPtpPpsEvent(c, true);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    c->free(c);
}

// Tests samplePtpSys method
// bool samplePtpSys(size_t count, std::vector<PtpSample_t> &samples)
Test(PtpClockTest, MethodSamplePtpSys)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    struct ptp_sys_offset samples;
    bool r2 = c->samplePtpSys(c, 5, &samples);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(eq(int, samples.n_samples, 2));
    cr_expect(eq(long, samples.ts[0].sec, 11));
    cr_expect(eq(ulong, samples.ts[0].nsec, 33));
    cr_expect(eq(long, samples.ts[1].sec, 22));
    cr_expect(eq(ulong, samples.ts[1].nsec, 44));
    cr_expect(eq(long, samples.ts[2].sec, 71));
    cr_expect(eq(ulong, samples.ts[2].nsec, 63));
    cr_expect(eq(long, samples.ts[3].sec, 62));
    cr_expect(eq(ulong, samples.ts[3].nsec, 84));
    c->free(c);
}

// Tests extSamplePtpSys method
// bool extSamplePtpSys(size_t count, std::vector<PtpSampleExt_t> &samples)
Test(PtpClockTest, MethodExtSamplePtpSys)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    struct ptp_sys_offset_extended samples;
    bool ret = c->extSamplePtpSys(c, 7, &samples);
    if(!ret) {
        cr_expect(eq(str, (char *)ptpmgmt_err_getMsg(),
                "Old kernel, PTP_SYS_OFFSET_EXTENDED ioctl is not supported"));
        return;
    }
    useTestMode(false);
    cr_expect(r1);
    cr_expect(eq(int, samples.n_samples, 2));
    cr_expect(eq(long, samples.ts[0][0].sec, 11));
    cr_expect(eq(ulong, samples.ts[0][0].nsec, 33));
    cr_expect(eq(long, samples.ts[0][1].sec, 22));
    cr_expect(eq(ulong, samples.ts[0][1].nsec, 44));
    cr_expect(eq(long, samples.ts[0][2].sec, 84));
    cr_expect(eq(ulong, samples.ts[0][2].nsec, 91));
    cr_expect(eq(long, samples.ts[1][0].sec, 71));
    cr_expect(eq(ulong, samples.ts[1][0].nsec, 63));
    cr_expect(eq(long, samples.ts[1][1].sec, 62));
    cr_expect(eq(ulong, samples.ts[1][1].nsec, 84));
    cr_expect(eq(long, samples.ts[1][2].sec, 45));
    cr_expect(eq(ulong, samples.ts[1][2].nsec, 753));
    c->free(c);
}

// Tests preciseSamplePtpSys method
// bool preciseSamplePtpSys(PtpSamplePrecise_t &sample)
Test(PtpClockTest, MethodPreciseSamplePtpSys)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    struct ptp_sys_offset_precise sample;
    bool r2 = c->preciseSamplePtpSys(c, &sample);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(eq(long, sample.device.sec, 17));
    cr_expect(eq(ulong, sample.device.nsec, 135));
    cr_expect(eq(long, sample.sys_realtime.sec, 415));
    cr_expect(eq(ulong, sample.sys_realtime.nsec, 182));
    cr_expect(eq(long, sample.sys_monoraw.sec, 9413));
    cr_expect(eq(ulong, sample.sys_monoraw.nsec, 3654));
    c->free(c);
}

// Tests readEvent method
// bool readEvent(PtpEvent_t &event)
Test(PtpClockTest, MethodReadEvent)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    struct ptp_extts_event event;
    bool r2 = c->readEvent(c, &event);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(eq(int, event.index, 19));
    cr_expect(eq(long, event.t.sec, 123));
    cr_expect(eq(ulong, event.t.nsec, 712));
    c->free(c);
}

// Tests readEvents method
// bool readEvents(std::vector<PtpEvent_t> &events, size_t max)
Test(PtpClockTest, MethodReadEvents)
{
    ptpmgmt_clock c = ptpmgmt_clock_alloc();
    useTestMode(true);
    bool r1 = c->initUsingIndex(c, 0, false);
    struct ptp_extts_event events[10];
    size_t s = 10;
    bool r2 = c->readEvents(c, events, &s);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(eq(sz, s, 3));
    cr_expect(eq(int, events[0].index, 2));
    cr_expect(eq(long, events[0].t.sec, 34));
    cr_expect(eq(ulong, events[0].t.nsec, 7856));
    cr_expect(eq(int, events[1].index, 6));
    cr_expect(eq(long, events[1].t.sec, 541));
    cr_expect(eq(ulong, events[1].t.nsec, 468));
    cr_expect(eq(int, events[2].index, 3));
    cr_expect(eq(long, events[2].t.sec, 1587));
    cr_expect(eq(ulong, events[2].t.nsec, 12));
    c->free(c);
}
