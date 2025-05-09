/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Utility functions for printing
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/print.hpp"

#include <cstring>
#include <syslog.h>
#include <rtpi/mutex.hpp>

__CLKMGR_NAMESPACE_USE;

using namespace std;

static LogLevel currentLogLevel = INFO;
static bool useSyslog = false;
static bool verbose = true;
static rtpi::mutex errMutex;

void clkmgr::PrintStartLog(const char *me)
{
    openlog(me, LOG_PID, LOG_DAEMON);
    useSyslog = true;
}
void clkmgr::PrintStopLog()
{
    closelog();
}

void clkmgr::setLogLevel(LogLevel level)
{
    currentLogLevel = level;
}

void clkmgr::setVerbose(bool isVerbose)
{
    verbose = isVerbose;
}

void clkmgr::_PrintError(string msg, uint16_t line, const char *file,
    const char *func, errno_type errnum)
{
    string ebuf;
    if(errnum != (errno_type)(-1)) {
        unique_lock<rtpi::mutex> lck(errMutex);
        ebuf = strerror(errnum);
    }
    if(useSyslog)
        syslog(LOG_ERR, "*** Error: %s %s at line %u in %s: %s",
            msg.c_str(), ebuf.c_str(), line, file, func);
    if(verbose) {
        fprintf(stderr, "*** Error: %s %s at line %u in %s: %s\n",
            msg.c_str(), ebuf.c_str(), line, file, func);
        fflush(stderr);
    }
}

void clkmgr::_PrintDebug(string msg, uint16_t line, const char *file,
    const char *func)
{
    if(currentLogLevel < DEBUG)
        return;
    if(useSyslog)
        syslog(LOG_DEBUG, "*** Debug: %s at line %u in %s: %s",
            msg.c_str(), line, file, func);
    if(verbose) {
        fprintf(stderr, "*** Debug: %s at line %u in %s: %s\n",
            msg.c_str(), line, file, func);
        fflush(stderr);
    }
}

void clkmgr::_PrintInfo(string msg, uint16_t line, const char *file,
    const char *func)
{
    if(currentLogLevel < INFO)
        return;
    if(useSyslog)
        syslog(LOG_INFO, "* Info: %s at line %u in %s: %s",
            msg.c_str(), line, file, func);
    if(verbose) {
        fprintf(stderr, "* Info: %s at line %u in %s: %s\n",
            msg.c_str(), line, file, func);
        fflush(stderr);
    }
}

#define HEX_DIGITS_PER_LINE 16
void clkmgr::_DumpOctetArray(string msg, const uint8_t *arr,
    size_t length, uint16_t line, const char *file, const char *func)
{
    if(currentLogLevel < TRACE)
        return;
    char buf[2000];
    string str;
    snprintf(buf, sizeof buf, "*   Info: %s at line %u in %s:%s",
        msg.c_str(), line, file, func);
    str = buf;
    for(size_t i = 0; i < length; i++) {
        snprintf(buf, sizeof buf, "%s0x%.2x",
            i % HEX_DIGITS_PER_LINE ? " " : "\n", arr[i]);
        str += buf;
    }
    if(useSyslog)
        syslog(LOG_DEBUG, "%s", str.c_str());
    if(verbose) {
        fprintf(stderr, "%s\n", str.c_str());
        fflush(stderr);
    }
}
