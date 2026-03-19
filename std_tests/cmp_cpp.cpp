/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */
#include <ptpmgmt/err.h>
#include <ptpmgmt/init.h>
#include <ptpmgmt/json.h>
#include <ptpmgmt/msgCall.h>
#include <ptpmgmt/msg.h>
#include <ptpmgmt/ptp.h>
#include <ptpmgmt/timeCvrt.h>
#include <ptpmgmt/ver.h>
#include <clockmanager.h>
int main(void)
{
    printf("compile version: %d.%d = %s (0x%d)\n", LIBPTPMGMT_VER_MAJ,
        LIBPTPMGMT_VER_MIN, LIBPTPMGMT_VER, LIBPTPMGMT_VER_VAL);
#ifdef __cplusplus
    clkmgr::PTPClockSubscription ptp4lSub;
    printf("running version: %d.%d = %s (0x%d)\n", ptpmgmt::getVersionMajor(),
        ptpmgmt::getVersionMinor(), ptpmgmt::getVersion(),
        ptpmgmt::getVersionValue());
#else
    Clkmgr_ClockSyncData *syncData = clkmgr_constructClockSyncDataInstance();
    printf("running version: %d.%d = %s (0x%d)\n", ptpmgmt_getVersionMajor(),
        ptpmgmt_getVersionMinor(), ptpmgmt_getVersion(), ptpmgmt_getVersionValue());
    clkmgr_destroyClockSyncDataInstance(syncData);
    printf("All events %d\n", CLKMGR_PTP_EVENT_ALL);
#endif
    return 0;
}
