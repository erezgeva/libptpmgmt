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
#include <time.h>

__CLKMGR_NAMESPACE_BEGIN

using namespace std;

//              0      1     2      3
enum LogLevel { ERROR, INFO, DEBUG, TRACE };

static LogLevel currentLogLevel = INFO;
static bool useSyslog = false;
static bool verbose = true;

static double getMonotonicTime()
{
    timespec timeSpec;
    if(clock_gettime(CLOCK_MONOTONIC, &timeSpec) == -1) {
        perror("clock_gettime failed");
        return -1;
    }
    double seconds = timeSpec.tv_sec;
    double nanoseconds = timeSpec.tv_nsec / 1e9;
    return seconds + nanoseconds;
}

void PrintStartLog(const char *me)
{
    openlog(me, LOG_PID, LOG_DAEMON);
    useSyslog = true;
}
void PrintStopLog()
{
    closelog();
}

void setLogLevel(int level)
{
    currentLogLevel = (LogLevel)level;
}

void setVerbose(bool isVerbose)
{
    verbose = isVerbose;
}

void _PrintError(const string &msg, uint16_t line, const char *file,
    const char *func, int errnum)
{
    const char *ebuf = nullptr;
    if(errnum != 0) {
        ebuf = strerrordesc_np(errnum);
        if(ebuf == nullptr)
            ebuf = "NA";
    } else
        ebuf = "";
    if(useSyslog)
        syslog(LOG_ERR, "Error: %s %s at line %u in %s: %s",
            msg.c_str(), ebuf, line, file, func);
    if(verbose) {
        fprintf(stderr, "[clkmgr][%.3f] Error: %s %s at line %u in %s: %s\n",
            getMonotonicTime(), msg.c_str(), ebuf, line, file, func);
        fflush(stderr);
    }
}

void _PrintDebug(const string &msg, uint16_t line, const char *file,
    const char *func)
{
    if(currentLogLevel < DEBUG)
        return;
    if(useSyslog)
        syslog(LOG_DEBUG, "Debug: %s at line %u in %s: %s",
            msg.c_str(), line, file, func);
    if(verbose) {
        fprintf(stderr, "[clkmgr][%.3f] Debug: %s at line %u in %s: %s\n",
            getMonotonicTime(), msg.c_str(), line, file, func);
        fflush(stderr);
    }
}

void _PrintInfo(const string &msg, uint16_t line, const char *file,
    const char *func)
{
    if(currentLogLevel < INFO)
        return;
    if(useSyslog)
        syslog(LOG_INFO, "Info: %s at line %u in %s: %s",
            msg.c_str(), line, file, func);
    if(verbose) {
        fprintf(stderr, "[clkmgr][%.3f] Info: %s at line %u in %s: %s\n",
            getMonotonicTime(), msg.c_str(), line, file, func);
        fflush(stderr);
    }
}

#define HEX_DIGITS_PER_LINE 16
void _DumpOctetArray(string msg, const uint8_t *arr,
    size_t length, uint16_t line, const char *file, const char *func)
{
    if(currentLogLevel < TRACE)
        return;
    char buf[2000];
    string str;
    snprintf(buf, sizeof buf, "Trace: %s at line %u in %s:%s",
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
        fprintf(stderr, "[clkmgr][%.3f] %s\n", getMonotonicTime(), str.c_str());
        fflush(stderr);
    }
}

__CLKMGR_NAMESPACE_END
