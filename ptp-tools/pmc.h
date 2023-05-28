/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Impleament linuxptp pmc tool using the libptpmgmt library
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_PMC_TOOL_H
#define __PTPMGMT_PMC_TOOL_H

#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include "init.h"
#include "msg.h"
#include "msgCall.h"
#include "err.h"

using namespace ptpmgmt;

/** print error message */
#define PMCERR(msg) fprintf(stderr, msg "\n")
/** dump */
#define DUMPS(format, ...) printf(format, __VA_ARGS__)
/** dump end of line */
#define DUMPNL puts("\n");
/** print library error */
#define PMCLERR {\
    if(Error::isError())\
        fprintf(stderr, "%s\n", Error::getError().c_str());}while(0)

static const char toksep[] = " \t\n\r"; /* while spaces */

/**
 * Dump message management TLV
 * @param[in] msg object
 * @param[in] id optional TLV ID to dump
 * @param[in] data optional TLV to dump
 */
void call_dump(Message &msg, mng_vals_e id = NULL_PTP_MANAGEMENT,
    BaseMngTlv *data = nullptr);
/**
 * Dump a signaling message TLV
 * @param[in] msg object
 * @param[in] tlvType TLV type
 * @param[in] tlv to dump
 * @return always false, as true will stop the travers
 */
bool call_dumpSig(const Message &msg, tlvType_e tlvType, const BaseSigTlv *tlv);
/**
 * Build a message management TLV
 * @param[in] msg object
 * @param[in] action management TLV action
 * @param[in] id management TLV ID
 * @param[in] save pointer to last strtok_r call
 * @return true on success
 */
bool call_data(Message &msg, actionField_e action, mng_vals_e id, char *save);
/**
 * Send current management TLV
 * @return true on success
 */
bool sendAction();

#endif /* __PTPMGMT_PMC_TOOL_H */
