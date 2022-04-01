/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Impleament linuxptp pmc tool using the libpmc library
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PMC_TOOL_H
#define __PMC_TOOL_H

#include "msg.h"

/** print error message */
#define PMCERR(msg) fprintf(stderr, msg "\n")
/** dump */
#define DUMPS(format, ...) printf(format, __VA_ARGS__)
/** dump end of line */
#define DUMPNL printf("\n");

static const char toksep[] = " \t\n\r"; /* while spaces */

/**
 * Dump message managment TLV
 * @param[in] msg object
 * @param[in] data optional TLV to dump
 */
void call_dump(Message &msg, BaseMngTlv *data);
/**
 * Dump a signaling message TLV
 * @param[in] msg object
 * @param[in] tlvType TLV type
 * @param[in] tlv to dump
 * @return always false, as true will stop the travers
 */
bool call_dumpSig(const Message &msg, tlvType_e tlvType, const BaseSigTlv *tlv);
/**
 * Build a message managment TLV
 * @param[in] msg object
 * @param[in] id managment TLV ID
 * @param[in] save pointer to last strtok_r call
 * @return always false, as true will stop the travers
 */
BaseMngTlv *call_data(Message &msg, mng_vals_e id, char *save);

#endif /* __PMC_TOOL_H */
