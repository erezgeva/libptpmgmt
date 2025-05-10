/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Signal handling utilities
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/print.hpp"
#include "common/sighandler.hpp"

#include <signal.h>

static const int INTR_SIGNAL = SIGUSR1;

__CLKMGR_NAMESPACE_USE;

using namespace std;

static inline bool SIGADDSET(sigset_t *sigset, int sig)
{
    return sigaddset(sigset, sig) == 0;
}

static inline bool AddStopSignal(sigset_t *sigset)
{
    return SIGADDSET(sigset, SIGINT) &&
        SIGADDSET(sigset, SIGHUP) &&
        SIGADDSET(sigset, SIGTERM) &&
        SIGADDSET(sigset, SIGQUIT) &&
        SIGADDSET(sigset, SIGALRM) &&
        SIGADDSET(sigset, SIGABRT);
}

static inline bool AddInterruptSignal(sigset_t *sigset)
{
    return SIGADDSET(sigset, SIGUSR1) &&
        SIGADDSET(sigset, SIGUSR2);
}

static inline bool GenerateBlockSigset(sigset_t *block)
{
    return sigemptyset(block) == 0 &&
        AddStopSignal(block) &&
        AddInterruptSignal(block);
}

static inline bool GenerateWaitSigset(sigset_t *wait)
{
    return sigemptyset(wait) == 0 &&
        AddStopSignal(wait);
}

__CLKMGR_NAMESPACE_BEGIN;

bool BlockStopSignal()
{
    sigset_t blockSigset;
    return GenerateBlockSigset(&blockSigset) &&
        pthread_sigmask(SIG_BLOCK, &blockSigset, nullptr) == 0;
}

bool WaitForStopSignal()
{
    sigset_t waitSigset;
    if(!GenerateWaitSigset(&waitSigset))
        return false;
    PrintDebug("Waiting for Interrupt Signal");
    int cause;
    if(sigwait(&waitSigset, &cause) == -1) {
        PrintErrorCode("Waiting for Interrupt Signal");
        return false;
    }
    PrintDebug("Received Interrupt Signal");
    return true;
}

// Empty callback for signalling
static void NullSigaction(int sig, siginfo_t *siginfo, void *ctx)
{
}

bool EnableSyscallInterruptSignal()
{
    struct sigaction intrSigaction;
    intrSigaction.sa_sigaction = NullSigaction;
    if(sigemptyset(&intrSigaction.sa_mask) != 0)
        return false;
    intrSigaction.sa_flags = SA_SIGINFO;
    sigset_t unblockSigset;
    return sigaction(INTR_SIGNAL, &intrSigaction, nullptr) == 0 &&
        sigemptyset(&unblockSigset) == 0 &&
        SIGADDSET(&unblockSigset, INTR_SIGNAL) &&
        pthread_sigmask(SIG_UNBLOCK, &unblockSigset, nullptr) == 0;
}

bool SendSyscallInterruptSignal(thread &t)
{
    int ret = pthread_kill(t.native_handle(), INTR_SIGNAL);
    if(ret != 0) {
        PrintError("pthread_kill()", ret);
        return false;
    }
    return true;
}

__CLKMGR_NAMESPACE_END;
