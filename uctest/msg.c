/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief message class wrapper unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "config.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include "msg.h"

// Tests creating message object
// ptpmgmt_msg ptpmgmt_msg_alloc()
Test(MessageTest, MethodEmptyConstructor)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    cr_expect(eq(u16, p->target.portNumber, 0xffff));
    cr_expect(zero(memcmp(p->target.clockIdentity.v,
                "\xff\xff\xff\xff\xff\xff\xff\xff",
                sizeof(struct ptpmgmt_ClockIdentity_t))));
    m->free(m);
}

// Tests creating message object with parameters
// ptpmgmt_msg ptpmgmt_msg_alloc_prms(ptpmgmt_cpMsgParams prms)
Test(MessageTest, MethodConstructor)
{
    ptpmgmt_pMsgParams p1 = ptpmgmt_MsgParams_alloc();
    cr_assert(not(zero(ptr, p1)));
    p1->transportSpecific = 0xf;
    p1->domainNumber = 17;
    p1->boundaryHops = 13;
    p1->isUnicast = false;;
    ptpmgmt_msg m = ptpmgmt_msg_alloc_prms(p1);
    cr_assert(not(zero(ptr, m)));
    ptpmgmt_cpMsgParams p = m->getParams(m);
    cr_expect(eq(u8, p->transportSpecific, 0xf));
    cr_expect(eq(u8, p->domainNumber, 17));
    cr_expect(eq(u8, p->boundaryHops, 13));
    cr_expect(not(p->isUnicast));
    m->free(m);
    p1->free(p1);
}

// Tests get parameters method
// ptpmgmt_pMsgParams getParams(ptpmgmt_msg m)
Test(MessageTest, MethodGetParams)
{
    ptpmgmt_pMsgParams p1 = ptpmgmt_MsgParams_alloc();
    cr_assert(not(zero(ptr, p1)));
    p1->transportSpecific = 0xf;
    p1->domainNumber = 17;
    p1->boundaryHops = 13;
    p1->isUnicast = false;
    p1->sendAuth = true;
    p1->rcvAuth = PTPMGMT_RCV_AUTH_SIG_LAST;
    ptpmgmt_msg m = ptpmgmt_msg_alloc_prms(p1);
    cr_assert(not(zero(ptr, m)));
    ptpmgmt_cpMsgParams p = m->getParams(m);
    cr_expect(eq(u8, p->transportSpecific, p1->transportSpecific));
    cr_expect(eq(u8, p->domainNumber, p1->domainNumber));
    cr_expect(eq(u8, p->boundaryHops, p1->boundaryHops));
    cr_expect(eq(int, p->isUnicast, p1->isUnicast));
    cr_expect(eq(int, p->implementSpecific, p1->implementSpecific));
    cr_expect(eq(u16, p->target.portNumber, p1->target.portNumber));
    cr_expect(zero(memcmp(p->target.clockIdentity.v, p1->target.clockIdentity.v,
                sizeof(struct ptpmgmt_ClockIdentity_t))));
    cr_expect(eq(u16, p->self_id.portNumber, p1->self_id.portNumber));
    cr_expect(zero(memcmp(p->self_id.clockIdentity.v, p1->self_id.clockIdentity.v,
                sizeof(struct ptpmgmt_ClockIdentity_t))));
    cr_expect(eq(int, p->useZeroGet, p1->useZeroGet));
    cr_expect(eq(int, p->rcvSignaling, p1->rcvSignaling));
    cr_expect(eq(int, p->filterSignaling, p1->filterSignaling));
    cr_expect(eq(int, p->rcvSMPTEOrg, p1->rcvSMPTEOrg));
    cr_expect(eq(i8, p->sendAuth, p1->sendAuth));
    cr_expect(eq(i8, p->rcvAuth, p1->rcvAuth));
    m->free(m);
    p1->free(p1);
}

// Tests set parameters method
// bool updateParams(ptpmgmt_msg m, ptpmgmt_cpMsgParams prms)
Test(MessageTest, MethodUpdateParams)
{
    ptpmgmt_pMsgParams p1 = ptpmgmt_MsgParams_alloc();
    cr_assert(not(zero(ptr, p1)));
    p1->transportSpecific = 0xf;
    p1->domainNumber = 17;
    p1->boundaryHops = 13;
    p1->isUnicast = false;;
    p1->sendAuth = true;
    p1->rcvAuth = PTPMGMT_RCV_AUTH_SIG_ALL;
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->updateParams(m, p1));
    ptpmgmt_cpMsgParams p = m->getParams(m);
    cr_expect(eq(u8, p->transportSpecific, p1->transportSpecific));
    cr_expect(eq(u8, p->domainNumber, p1->domainNumber));
    cr_expect(eq(u8, p->boundaryHops, p1->boundaryHops));
    cr_expect(eq(int, p->isUnicast, p1->isUnicast));
    cr_expect(eq(int, p->implementSpecific, p1->implementSpecific));
    cr_expect(eq(u16, p->target.portNumber, p1->target.portNumber));
    cr_expect(zero(memcmp(p->target.clockIdentity.v, p1->target.clockIdentity.v,
                sizeof(struct ptpmgmt_ClockIdentity_t))));
    cr_expect(eq(u16, p->self_id.portNumber, p1->self_id.portNumber));
    cr_expect(zero(memcmp(p->self_id.clockIdentity.v, p1->self_id.clockIdentity.v,
                sizeof(struct ptpmgmt_ClockIdentity_t))));
    cr_expect(eq(int, p->useZeroGet, p1->useZeroGet));
    cr_expect(eq(int, p->rcvSignaling, p1->rcvSignaling));
    cr_expect(eq(int, p->filterSignaling, p1->filterSignaling));
    cr_expect(eq(int, p->rcvSMPTEOrg, p1->rcvSMPTEOrg));
    cr_expect(eq(i8, p->sendAuth, p1->sendAuth));
    cr_expect(eq(i8, p->rcvAuth, p1->rcvAuth));
    m->free(m);
    p1->free(p1);
}

// Tests get parsed TLV ID method
// enum ptpmgmt_mng_vals_e getTlvId(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetTlvId)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_NULL_PTP_MANAGEMENT));
    m->free(m);
}

// Tests get build TLV ID method
// enum ptpmgmt_mng_vals_e getBuildTlvId(const_ptpmgmt_msg m)
Test(MessageTest, MethodBuildGetTlvId)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_NULL_PTP_MANAGEMENT));
    m->free(m);
}

// Tests set using all clocks method
// void setAllClocks(const_ptpmgmt_msg m)
Test(MessageTest, MethodSetAllClocks)
{
    ptpmgmt_pMsgParams p1 = ptpmgmt_MsgParams_alloc();
    cr_assert(not(zero(ptr, p1)));
    p1->target.portNumber = 0x1f1f;
    memcpy(p1->target.clockIdentity.v, "\x1\x2\x3\x4\x5\x6\x7\x8",
        sizeof(struct ptpmgmt_ClockIdentity_t));
    ptpmgmt_msg m = ptpmgmt_msg_alloc_prms(p1);
    cr_assert(not(zero(ptr, m)));
    m->setAllClocks(m);
    ptpmgmt_cpMsgParams p = m->getParams(m);
    cr_expect(eq(u16, p->target.portNumber, 0xffff));
    cr_expect(zero(memcmp(p->target.clockIdentity.v,
                "\xff\xff\xff\xff\xff\xff\xff\xff",
                sizeof(struct ptpmgmt_ClockIdentity_t))));
    m->free(m);
    p1->free(p1);
}

// Tests set using all clocks method
// bool isAllClocks(const_ptpmgmt_msg m)
Test(MessageTest, MethodIsAllClocks)
{
    ptpmgmt_pMsgParams p1 = ptpmgmt_MsgParams_alloc();
    cr_assert(not(zero(ptr, p1)));
    p1->target.portNumber = 0x1f1f;
    memcpy(p1->target.clockIdentity.v, "\x1\x2\x3\x4\x5\x6\x7\x8",
        sizeof(struct ptpmgmt_ClockIdentity_t));
    ptpmgmt_msg m = ptpmgmt_msg_alloc_prms(p1);
    cr_assert(not(zero(ptr, m)));
    cr_expect(not(m->isAllClocks(m)));
    m->setAllClocks(m);
    cr_expect(m->isAllClocks(m));
    m->free(m);
    p1->free(p1);
}

// Tests using configuration file method
// bool useConfig(ptpmgmt_msg m, const_ptpmgmt_cfg cfg, const char *section)
Test(MessageTest, MethodUseConfig)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->useConfig(m, f, "dumm"));
    ptpmgmt_cpMsgParams p = m->getParams(m);
    cr_expect(eq(u8, p->transportSpecific, 9));
    cr_expect(eq(u8, p->domainNumber, 3));
    cr_expect(m->useConfig(m, f, NULL));
    p = m->getParams(m);
    cr_expect(eq(u8, p->transportSpecific, 7));
    cr_expect(eq(u8, p->domainNumber, 5));
    m->free(m);
}

// Tests convert error to string method
// const char *ptpmgmt_msg_err2str(ptpmgmt_MNG_PARSE_ERROR_e err)
// const char *err2str(enum ptpmgmt_MNG_PARSE_ERROR_e err)
Test(MessageTest, MethodErr2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_OK),
            "MNG_PARSE_ERROR_OK"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_MSG),
            "MNG_PARSE_ERROR_MSG"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_SIG),
            "MNG_PARSE_ERROR_SIG"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_SMPTE),
            "MNG_PARSE_ERROR_SMPTE"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_INVALID_ID),
            "MNG_PARSE_ERROR_INVALID_ID"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_INVALID_TLV),
            "MNG_PARSE_ERROR_INVALID_TLV"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_MISMATCH_TLV),
            "MNG_PARSE_ERROR_MISMATCH_TLV"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_SIZE_MISS),
            "MNG_PARSE_ERROR_SIZE_MISS"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_TOO_SMALL),
            "MNG_PARSE_ERROR_TOO_SMALL"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_SIZE),
            "MNG_PARSE_ERROR_SIZE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_VAL),
            "MNG_PARSE_ERROR_VAL"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_HEADER),
            "MNG_PARSE_ERROR_HEADER"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_ACTION),
            "MNG_PARSE_ERROR_ACTION"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_UNSUPPORT),
            "MNG_PARSE_ERROR_UNSUPPORT"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_MEM),
            "MNG_PARSE_ERROR_MEM"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(PTPMGMT_MNG_PARSE_ERROR_AUTH),
            "MNG_PARSE_ERROR_AUTH"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(
                PTPMGMT_MNG_PARSE_ERROR_AUTH_NONE), "MNG_PARSE_ERROR_AUTH_NONE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(
                PTPMGMT_MNG_PARSE_ERROR_AUTH_WRONG), "MNG_PARSE_ERROR_AUTH_WRONG"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_err2str(
                PTPMGMT_MNG_PARSE_ERROR_AUTH_NOKEY), "MNG_PARSE_ERROR_AUTH_NOKEY"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->err2str(PTPMGMT_MNG_PARSE_ERROR_OK),
            "MNG_PARSE_ERROR_OK"));
    m->free(m);
}

// tests convert message type to string method
// const char *ptpmgmt_msg_type2str(ptpmgmt_msgType_e type)
// const char *type2str(enum ptpmgmt_msgType_e type)
Test(MessageTest, MethodType2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_type2str(ptpmgmt_Sync), "Sync"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_type2str(ptpmgmt_Delay_Req),
            "Delay_Req"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_type2str(ptpmgmt_Pdelay_Req),
            "Pdelay_Req"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_type2str(ptpmgmt_Pdelay_Resp),
            "Pdelay_Resp"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_type2str(ptpmgmt_Follow_Up),
            "Follow_Up"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_type2str(ptpmgmt_Delay_Resp),
            "Delay_Resp"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_type2str(ptpmgmt_Pdelay_Resp_Follow_Up),
            "Pdelay_Resp_Follow_Up"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_type2str(ptpmgmt_Announce), "Announce"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_type2str(ptpmgmt_Signaling),
            "Signaling"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_type2str(ptpmgmt_Management),
            "Management"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->type2str(ptpmgmt_Management), "Management"));
    m->free(m);
}

// tests convert TLV type to string method
// const char *ptpmgmt_msg_tlv2str(ptpmgmt_tlvType_e type)
// const char *tlv2str(enum ptpmgmt_tlvType_e type)
Test(MessageTest, MethodTlv2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_tlv2str(PTPMGMT_MANAGEMENT),
            "MANAGEMENT"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_tlv2str(PTPMGMT_MANAGEMENT_ERROR_STATUS),
            "MANAGEMENT_ERROR_STATUS"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_tlv2str(PTPMGMT_ORGANIZATION_EXTENSION),
            "ORGANIZATION_EXTENSION"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_tlv2str(PTPMGMT_REQUEST_UNICAST_TRANSMISSION),
            "REQUEST_UNICAST_TRANSMISSION"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_tlv2str(PTPMGMT_GRANT_UNICAST_TRANSMISSION),
            "GRANT_UNICAST_TRANSMISSION"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_tlv2str(PTPMGMT_CANCEL_UNICAST_TRANSMISSION),
            "CANCEL_UNICAST_TRANSMISSION"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_tlv2str(
                PTPMGMT_ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION),
            "ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_tlv2str(PTPMGMT_PATH_TRACE),
            "PATH_TRACE"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_tlv2str(PTPMGMT_ALTERNATE_TIME_OFFSET_INDICATOR),
            "ALTERNATE_TIME_OFFSET_INDICATOR"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_tlv2str(PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE),
            "ORGANIZATION_EXTENSION_PROPAGATE"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_tlv2str(PTPMGMT_ENHANCED_ACCURACY_METRICS),
            "ENHANCED_ACCURACY_METRICS"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_tlv2str(
                PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE),
            "ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_tlv2str(PTPMGMT_L1_SYNC), "L1_SYNC"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_tlv2str(PTPMGMT_PORT_COMMUNICATION_AVAILABILITY),
            "PORT_COMMUNICATION_AVAILABILITY"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_tlv2str(PTPMGMT_PROTOCOL_ADDRESS),
            "PROTOCOL_ADDRESS"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_tlv2str(PTPMGMT_SLAVE_RX_SYNC_TIMING_DATA),
            "SLAVE_RX_SYNC_TIMING_DATA"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_tlv2str(PTPMGMT_SLAVE_RX_SYNC_COMPUTED_DATA),
            "SLAVE_RX_SYNC_COMPUTED_DATA"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_tlv2str(PTPMGMT_SLAVE_TX_EVENT_TIMESTAMPS),
            "SLAVE_TX_EVENT_TIMESTAMPS"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_tlv2str(PTPMGMT_CUMULATIVE_RATE_RATIO),
            "CUMULATIVE_RATE_RATIO"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_tlv2str(PTPMGMT_TLV_PAD), "PAD"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_tlv2str(PTPMGMT_AUTHENTICATION),
            "AUTHENTICATION"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_tlv2str(PTPMGMT_SLAVE_DELAY_TIMING_DATA_NP),
            "SLAVE_DELAY_TIMING_DATA_NP"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->tlv2str(PTPMGMT_MANAGEMENT), "MANAGEMENT"));
    m->free(m);
}

// tests convert action to string
// const char *ptpmgmt_msg_act2str(ptpmgmt_actionField_e action)
// const char *act2str(enum ptpmgmt_actionField_e action)
Test(MessageTest, MethodAct2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_act2str(PTPMGMT_GET), "GET"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_act2str(PTPMGMT_SET), "SET"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_act2str(PTPMGMT_RESPONSE), "RESPONSE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_act2str(PTPMGMT_COMMAND), "COMMAND"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_act2str(PTPMGMT_ACKNOWLEDGE),
            "ACKNOWLEDGE"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->act2str(PTPMGMT_GET), "GET"));
    m->free(m);
}

// tests convert management id to string
// static const char *mng2str(mng_vals_e id)
// const char *ptpmgmt_msg_mng2str(ptpmgmt_mng_vals_e id)
// const char *mng2str(enum ptpmgmt_mng_vals_e id)
Test(MessageTest, MethodMng2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_NULL_PTP_MANAGEMENT),
            "NULL_PTP_MANAGEMENT"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_CLOCK_DESCRIPTION),
            "CLOCK_DESCRIPTION"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_USER_DESCRIPTION),
            "USER_DESCRIPTION"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_SAVE_IN_NON_VOLATILE_STORAGE),
            "SAVE_IN_NON_VOLATILE_STORAGE"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_RESET_NON_VOLATILE_STORAGE),
            "RESET_NON_VOLATILE_STORAGE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_INITIALIZE),
            "INITIALIZE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_FAULT_LOG), "FAULT_LOG"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_FAULT_LOG_RESET),
            "FAULT_LOG_RESET"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_DEFAULT_DATA_SET),
            "DEFAULT_DATA_SET"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_CURRENT_DATA_SET),
            "CURRENT_DATA_SET"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PARENT_DATA_SET),
            "PARENT_DATA_SET"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_TIME_PROPERTIES_DATA_SET),
            "TIME_PROPERTIES_DATA_SET"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PORT_DATA_SET),
            "PORT_DATA_SET"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PRIORITY1), "PRIORITY1"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PRIORITY2), "PRIORITY2"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_DOMAIN), "DOMAIN"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_SLAVE_ONLY),
            "SLAVE_ONLY"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_LOG_ANNOUNCE_INTERVAL),
            "LOG_ANNOUNCE_INTERVAL"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_ANNOUNCE_RECEIPT_TIMEOUT),
            "ANNOUNCE_RECEIPT_TIMEOUT"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_LOG_SYNC_INTERVAL),
            "LOG_SYNC_INTERVAL"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_VERSION_NUMBER),
            "VERSION_NUMBER"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_ENABLE_PORT),
            "ENABLE_PORT"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_DISABLE_PORT),
            "DISABLE_PORT"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_TIME), "TIME"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_CLOCK_ACCURACY),
            "CLOCK_ACCURACY"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_UTC_PROPERTIES),
            "UTC_PROPERTIES"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_TRACEABILITY_PROPERTIES),
            "TRACEABILITY_PROPERTIES"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_TIMESCALE_PROPERTIES),
            "TIMESCALE_PROPERTIES"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_UNICAST_NEGOTIATION_ENABLE),
            "UNICAST_NEGOTIATION_ENABLE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PATH_TRACE_LIST),
            "PATH_TRACE_LIST"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PATH_TRACE_ENABLE),
            "PATH_TRACE_ENABLE"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_GRANDMASTER_CLUSTER_TABLE),
            "GRANDMASTER_CLUSTER_TABLE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_UNICAST_MASTER_TABLE),
            "UNICAST_MASTER_TABLE"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_UNICAST_MASTER_MAX_TABLE_SIZE),
            "UNICAST_MASTER_MAX_TABLE_SIZE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_ACCEPTABLE_MASTER_TABLE),
            "ACCEPTABLE_MASTER_TABLE"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_ACCEPTABLE_MASTER_TABLE_ENABLED),
            "ACCEPTABLE_MASTER_TABLE_ENABLED"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_ACCEPTABLE_MASTER_MAX_TABLE_SIZE),
            "ACCEPTABLE_MASTER_MAX_TABLE_SIZE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_ALTERNATE_MASTER),
            "ALTERNATE_MASTER"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_ALTERNATE_TIME_OFFSET_ENABLE),
            "ALTERNATE_TIME_OFFSET_ENABLE"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_ALTERNATE_TIME_OFFSET_NAME),
            "ALTERNATE_TIME_OFFSET_NAME"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_ALTERNATE_TIME_OFFSET_MAX_KEY),
            "ALTERNATE_TIME_OFFSET_MAX_KEY"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_ALTERNATE_TIME_OFFSET_PROPERTIES),
            "ALTERNATE_TIME_OFFSET_PROPERTIES"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_TRANSPARENT_CLOCK_PORT_DATA_SET),
            "TRANSPARENT_CLOCK_PORT_DATA_SET"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_LOG_MIN_PDELAY_REQ_INTERVAL),
            "LOG_MIN_PDELAY_REQ_INTERVAL"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_TRANSPARENT_CLOCK_DEFAULT_DATA_SET),
            "TRANSPARENT_CLOCK_DEFAULT_DATA_SET"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PRIMARY_DOMAIN),
            "PRIMARY_DOMAIN"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_DELAY_MECHANISM),
            "DELAY_MECHANISM"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(
                PTPMGMT_EXTERNAL_PORT_CONFIGURATION_ENABLED),
            "EXTERNAL_PORT_CONFIGURATION_ENABLED"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_MASTER_ONLY),
            "MASTER_ONLY"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_HOLDOVER_UPGRADE_ENABLE),
            "HOLDOVER_UPGRADE_ENABLE"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_EXT_PORT_CONFIG_PORT_DATA_SET),
            "EXT_PORT_CONFIG_PORT_DATA_SET"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_TIME_STATUS_NP),
            "TIME_STATUS_NP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_GRANDMASTER_SETTINGS_NP),
            "GRANDMASTER_SETTINGS_NP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PORT_DATA_SET_NP),
            "PORT_DATA_SET_NP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_SUBSCRIBE_EVENTS_NP),
            "SUBSCRIBE_EVENTS_NP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PORT_PROPERTIES_NP),
            "PORT_PROPERTIES_NP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PORT_STATS_NP),
            "PORT_STATS_NP"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP),
            "SYNCHRONIZATION_UNCERTAIN_NP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PORT_SERVICE_STATS_NP),
            "PORT_SERVICE_STATS_NP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_UNICAST_MASTER_TABLE_NP),
            "UNICAST_MASTER_TABLE_NP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PORT_HWCLOCK_NP),
            "PORT_HWCLOCK_NP"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_POWER_PROFILE_SETTINGS_NP),
            "POWER_PROFILE_SETTINGS_NP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_CMLDS_INFO_NP),
            "CMLDS_INFO_NP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_mng2str(PTPMGMT_PORT_CORRECTIONS_NP),
            "PORT_CORRECTIONS_NP"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_mng2str(PTPMGMT_EXTERNAL_GRANDMASTER_PROPERTIES_NP),
            "EXTERNAL_GRANDMASTER_PROPERTIES_NP"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->mng2str(PTPMGMT_INITIALIZE), "INITIALIZE"));
    m->free(m);
}

// tests convert string to management id
// bool ptpmgmt_msg_findMngID(const char *str, ptpmgmt_mng_vals_e *id, bool exact)
// bool findMngID(const char *str, enum ptpmgmt_mng_vals_e *id, bool exact)
Test(MessageTest, MethodFindMngID)
{
    enum ptpmgmt_mng_vals_e i;
    cr_expect(ptpmgmt_msg_findMngID("FAULT_LOG", &i, true));
    cr_expect(eq(int, i, PTPMGMT_FAULT_LOG));
    cr_expect(ptpmgmt_msg_findMngID("Null", &i, false));
    cr_expect(eq(int, i, PTPMGMT_NULL_PTP_MANAGEMENT));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->findMngID("UTC_PROPERTIES", &i, true));
    cr_expect(eq(int, i, PTPMGMT_UTC_PROPERTIES));
    m->free(m);
}

// tests convert management error to string
// const char *ptpmgmt_msg_errId2str(ptpmgmt_managementErrorId_e err)
// const char *errId2str(enum ptpmgmt_managementErrorId_e err)
Test(MessageTest, MethodErrId2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_errId2str(PTPMGMT_RESPONSE_TOO_BIG),
            "RESPONSE_TOO_BIG"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_errId2str(PTPMGMT_NO_SUCH_ID),
            "NO_SUCH_ID"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_errId2str(PTPMGMT_WRONG_LENGTH),
            "WRONG_LENGTH"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_errId2str(PTPMGMT_WRONG_VALUE),
            "WRONG_VALUE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_errId2str(PTPMGMT_NOT_SETABLE),
            "NOT_SETABLE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_errId2str(PTPMGMT_NOT_SUPPORTED),
            "NOT_SUPPORTED"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_errId2str(PTPMGMT_GENERAL_ERROR),
            "GENERAL_ERROR"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->errId2str(PTPMGMT_GENERAL_ERROR),
            "GENERAL_ERROR"));
    m->free(m);
}

// tests convert clock type to string
// const char *ptpmgmt_msg_clkType2str(ptpmgmt_clockType_e type)
// const char *clkType2str(enum ptpmgmt_clockType_e type)
Test(MessageTest, MethodClkType2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_clkType2str(ptpmgmt_ordinaryClock),
            "ordinaryClock"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_clkType2str(ptpmgmt_boundaryClock),
            "boundaryClock"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_clkType2str(ptpmgmt_p2pTransparentClock),
            "p2pTransparentClock"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_clkType2str(ptpmgmt_e2eTransparentClock),
            "e2eTransparentClock"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_clkType2str(ptpmgmt_managementClock),
            "managementClock"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->clkType2str(ptpmgmt_ordinaryClock),
            "ordinaryClock"));
    m->free(m);
}

// tests convert network protocol to string
// const char *ptpmgmt_msg_netProt2str(ptpmgmt_networkProtocol_e protocol)
// const char *netProt2str(enum ptpmgmt_networkProtocol_e protocol)
Test(MessageTest, MethodNetProt2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_netProt2str(ptpmgmt_UDP_IPv4),
            "UDP_IPv4"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_netProt2str(ptpmgmt_UDP_IPv6),
            "UDP_IPv6"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_netProt2str(ptpmgmt_IEEE_802_3),
            "IEEE_802_3"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_netProt2str(ptpmgmt_DeviceNet),
            "DeviceNet"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_netProt2str(ptpmgmt_ControlNet),
            "ControlNet"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_netProt2str(ptpmgmt_PROFINET),
            "PROFINET"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->netProt2str(ptpmgmt_UDP_IPv4), "UDP_IPv4"));
    m->free(m);
}

// tests convert clock accuracy to string
// const char *ptpmgmt_msg_clockAcc2str(ptpmgmt_clockAccuracy_e value)
// const char *clockAcc2str(enum ptpmgmt_clockAccuracy_e value)
Test(MessageTest, MethodClockAcc2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_1ps),
            "within_1ps"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_2_5ps),
            "within_2_5ps"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_10ps),
            "within_10ps"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_25ps),
            "within_25ps"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_100ps),
            "within_100ps"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_250ps),
            "within_250ps"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_1ns),
            "within_1ns"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_2_5ns),
            "within_2_5ns"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_10ns),
            "within_10ns"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_25ns),
            "within_25ns"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_100ns),
            "within_100ns"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_250ns),
            "within_250ns"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_1us),
            "within_1us"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_2_5us),
            "within_2_5us"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_10us),
            "within_10us"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_25us),
            "within_25us"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_100us),
            "within_100us"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_250us),
            "within_250us"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_1ms),
            "within_1ms"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_2_5ms),
            "within_2_5ms"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_10ms),
            "within_10ms"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_25ms),
            "within_25ms"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_100ms),
            "within_100ms"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_250ms),
            "within_250ms"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_1s),
            "within_1s"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_within_10s),
            "within_10s"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_more_10s),
            "more_10s"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_clockAcc2str(ptpmgmt_Accurate_Unknown),
            "Unknown"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->clockAcc2str(ptpmgmt_Accurate_Unknown),
            "Unknown"));
    m->free(m);
}

// tests convert fault record severity code to string
// const char *ptpmgmt_msg_faultRec2str(ptpmgmt_faultRecord_e code)
// const char *faultRec2str(enum ptpmgmt_faultRecord_e code)
Test(MessageTest, MethodFaultRec2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_faultRec2str(ptpmgmt_F_Emergency),
            "Emergency"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_faultRec2str(ptpmgmt_F_Alert), "Alert"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_faultRec2str(ptpmgmt_F_Critical),
            "Critical"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_faultRec2str(ptpmgmt_F_Error), "Error"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_faultRec2str(ptpmgmt_F_Warning),
            "Warning"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_faultRec2str(ptpmgmt_F_Notice),
            "Notice"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_faultRec2str(ptpmgmt_F_Informational),
            "Informational"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_faultRec2str(ptpmgmt_F_Debug), "Debug"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->faultRec2str(ptpmgmt_F_Debug), "Debug"));
    m->free(m);
}

// tests convert time source to string
// const char *ptpmgmt_msg_timeSrc2str(ptpmgmt_timeSource_e type)
// const char *timeSrc2str(enum ptpmgmt_timeSource_e type)
Test(MessageTest, MethodTimeSrc2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_timeSrc2str(PTPMGMT_ATOMIC_CLOCK),
            "ATOMIC_CLOCK"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_timeSrc2str(PTPMGMT_GNSS), "GNSS"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_timeSrc2str(PTPMGMT_GPS), "GNSS"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_timeSrc2str(PTPMGMT_TERRESTRIAL_RADIO),
            "TERRESTRIAL_RADIO"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_timeSrc2str(PTPMGMT_SERIAL_TIME_CODE),
            "SERIAL_TIME_CODE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_timeSrc2str(PTPMGMT_PTP), "PTP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_timeSrc2str(PTPMGMT_NTP), "NTP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_timeSrc2str(PTPMGMT_HAND_SET),
            "HAND_SET"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_timeSrc2str(PTPMGMT_OTHER), "OTHER"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_timeSrc2str(PTPMGMT_INTERNAL_OSCILLATOR),
            "INTERNAL_OSCILLATOR"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->timeSrc2str(PTPMGMT_GPS), "GNSS"));
    m->free(m);
}

// tests convert string to time source type
// bool ptpmgmt_msg_findTimeSrc(const char *str, ptpmgmt_timeSource_e *type,
//     bool exact)
// bool findTimeSrc(const char *str, enum ptpmgmt_timeSource_e *type, bool exact)
Test(MessageTest, MethodFindTimeSrc)
{
    enum ptpmgmt_timeSource_e t;
    cr_expect(ptpmgmt_msg_findTimeSrc("ATOMIC_CLOCK", &t, true));
    cr_expect(eq(int, t, PTPMGMT_ATOMIC_CLOCK));
    cr_expect(ptpmgmt_msg_findTimeSrc("Atomic", &t, false));
    cr_expect(eq(int, t, PTPMGMT_ATOMIC_CLOCK));
    cr_expect(ptpmgmt_msg_findTimeSrc("GNSS", &t, true));
    cr_expect(eq(int, t, PTPMGMT_GNSS));
    cr_expect(ptpmgmt_msg_findTimeSrc("Gnss", &t, false));
    cr_expect(eq(int, t, PTPMGMT_GNSS));
    cr_expect(ptpmgmt_msg_findTimeSrc("GPS", &t, true));
    cr_expect(eq(int, t, PTPMGMT_GPS));
    cr_expect(ptpmgmt_msg_findTimeSrc("Gps", &t, false));
    cr_expect(eq(int, t, PTPMGMT_GPS));
    cr_expect(ptpmgmt_msg_findTimeSrc("TERRESTRIAL_RADIO", &t, true));
    cr_expect(eq(int, t, PTPMGMT_TERRESTRIAL_RADIO));
    cr_expect(ptpmgmt_msg_findTimeSrc("Terrestrial_Radio", &t, false));
    cr_expect(eq(int, t, PTPMGMT_TERRESTRIAL_RADIO));
    cr_expect(ptpmgmt_msg_findTimeSrc("Radio", &t, false));
    cr_expect(eq(int, t, PTPMGMT_TERRESTRIAL_RADIO));
    cr_expect(ptpmgmt_msg_findTimeSrc("Terres", &t, false));
    cr_expect(eq(int, t, PTPMGMT_TERRESTRIAL_RADIO));
    cr_expect(ptpmgmt_msg_findTimeSrc("SERIAL_TIME_CODE", &t, true));
    cr_expect(eq(int, t, PTPMGMT_SERIAL_TIME_CODE));
    cr_expect(ptpmgmt_msg_findTimeSrc("Serial_Time_Code", &t, false));
    cr_expect(eq(int, t, PTPMGMT_SERIAL_TIME_CODE));
    cr_expect(ptpmgmt_msg_findTimeSrc("Serial", &t, false));
    cr_expect(eq(int, t, PTPMGMT_SERIAL_TIME_CODE));
    cr_expect(ptpmgmt_msg_findTimeSrc("PTP", &t, true));
    cr_expect(eq(int, t, PTPMGMT_PTP));
    cr_expect(ptpmgmt_msg_findTimeSrc("Ptp", &t, false));
    cr_expect(eq(int, t, PTPMGMT_PTP));
    cr_expect(ptpmgmt_msg_findTimeSrc("NTP", &t, true));
    cr_expect(eq(int, t, PTPMGMT_NTP));
    cr_expect(ptpmgmt_msg_findTimeSrc("Ntp", &t, false));
    cr_expect(eq(int, t, PTPMGMT_NTP));
    cr_expect(ptpmgmt_msg_findTimeSrc("HAND_SET", &t, true));
    cr_expect(eq(int, t, PTPMGMT_HAND_SET));
    cr_expect(ptpmgmt_msg_findTimeSrc("Hand_set", &t, false));
    cr_expect(eq(int, t, PTPMGMT_HAND_SET));
    cr_expect(ptpmgmt_msg_findTimeSrc("Hand", &t, false));
    cr_expect(eq(int, t, PTPMGMT_HAND_SET));
    cr_expect(ptpmgmt_msg_findTimeSrc("OTHER", &t, true));
    cr_expect(eq(int, t, PTPMGMT_OTHER));
    cr_expect(ptpmgmt_msg_findTimeSrc("Other", &t, false));
    cr_expect(eq(int, t, PTPMGMT_OTHER));
    cr_expect(ptpmgmt_msg_findTimeSrc("INTERNAL_OSCILLATOR", &t, true));
    cr_expect(eq(int, t, PTPMGMT_INTERNAL_OSCILLATOR));
    cr_expect(ptpmgmt_msg_findTimeSrc("Internal_oscillator", &t, false));
    cr_expect(eq(int, t, PTPMGMT_INTERNAL_OSCILLATOR));
    cr_expect(ptpmgmt_msg_findTimeSrc("oscil", &t, false));
    cr_expect(eq(int, t, PTPMGMT_INTERNAL_OSCILLATOR));
    cr_expect(ptpmgmt_msg_findTimeSrc("Intern", &t, false));
    cr_expect(eq(int, t, PTPMGMT_INTERNAL_OSCILLATOR));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->findTimeSrc("Other", &t, false));
    cr_expect(eq(int, t, PTPMGMT_OTHER));
    m->free(m);
    // Confirm compatibility
    cr_expect(eq(int, PTPMGMT_GPS, PTPMGMT_GNSS));
}

// tests convert port state to string
// const char *ptpmgmt_msg_portState2str(enum ptpmgmt_portState_e state)
// const char *portState2str(enum ptpmgmt_portState_e state)
Test(MessageTest, MethodPortState2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_portState2str(PTPMGMT_INITIALIZING),
            "INITIALIZING"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_portState2str(PTPMGMT_FAULTY), "FAULTY"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_portState2str(PTPMGMT_DISABLED),
            "DISABLED"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_portState2str(PTPMGMT_LISTENING),
            "LISTENING"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_portState2str(PTPMGMT_PRE_MASTER),
            "PRE_TIME_TRANSMITTER"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_portState2str(PTPMGMT_PRE_TIME_TRANSMITTER),
            "PRE_TIME_TRANSMITTER"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_portState2str(PTPMGMT_MASTER),
            "TIME_TRANSMITTER"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_portState2str(PTPMGMT_TIME_TRANSMITTER),
            "TIME_TRANSMITTER"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_portState2str(PTPMGMT_PASSIVE),
            "PASSIVE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_portState2str(PTPMGMT_UNCALIBRATED),
            "UNCALIBRATED"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_portState2str(PTPMGMT_SLAVE),
            "TIME_RECEIVER"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_portState2str(PTPMGMT_TIME_RECEIVER),
            "TIME_RECEIVER"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->portState2str(PTPMGMT_PASSIVE), "PASSIVE"));
    m->free(m);
}

// tests convert string to port state
// bool ptpmgmt_msg_findPortState(const char *str, enum ptpmgmt_portState_e *state,
//     bool caseSens)
// bool findPortState(const char *str, enum ptpmgmt_portState_e *state,
//     bool caseSens)
Test(MessageTest, MethodFindPortState)
{
    enum ptpmgmt_portState_e s;
    cr_expect(ptpmgmt_msg_findPortState("INITIALIZING", &s, true));
    cr_expect(eq(int, s, PTPMGMT_INITIALIZING));
    cr_expect(ptpmgmt_msg_findPortState("Initializing", &s, false));
    cr_expect(eq(int, s, PTPMGMT_INITIALIZING));
    cr_expect(ptpmgmt_msg_findPortState("FAULTY", &s, true));
    cr_expect(eq(int, s, PTPMGMT_FAULTY));
    cr_expect(ptpmgmt_msg_findPortState("Faulty", &s, false));
    cr_expect(eq(int, s, PTPMGMT_FAULTY));
    cr_expect(ptpmgmt_msg_findPortState("DISABLED", &s, true));
    cr_expect(eq(int, s, PTPMGMT_DISABLED));
    cr_expect(ptpmgmt_msg_findPortState("Disabled", &s, false));
    cr_expect(eq(int, s, PTPMGMT_DISABLED));
    cr_expect(ptpmgmt_msg_findPortState("LISTENING", &s, true));
    cr_expect(eq(int, s, PTPMGMT_LISTENING));
    cr_expect(ptpmgmt_msg_findPortState("Listening", &s, false));
    cr_expect(eq(int, s, PTPMGMT_LISTENING));
    cr_expect(ptpmgmt_msg_findPortState("PRE_MASTER", &s, true));
    cr_expect(eq(int, s, PTPMGMT_PRE_MASTER));
    cr_expect(ptpmgmt_msg_findPortState("Pre_Master", &s, false));
    cr_expect(eq(int, s, PTPMGMT_PRE_MASTER));
    cr_expect(ptpmgmt_msg_findPortState("PRE_TIME_TRANSMITTER", &s, true));
    cr_expect(eq(int, s, PTPMGMT_PRE_TIME_TRANSMITTER));
    cr_expect(ptpmgmt_msg_findPortState("Pre_Time_Transmitter", &s, false));
    cr_expect(eq(int, s, PTPMGMT_PRE_TIME_TRANSMITTER));
    cr_expect(ptpmgmt_msg_findPortState("MASTER", &s, true));
    cr_expect(eq(int, s, PTPMGMT_MASTER));
    cr_expect(ptpmgmt_msg_findPortState("Master", &s, false));
    cr_expect(eq(int, s, PTPMGMT_MASTER));
    cr_expect(ptpmgmt_msg_findPortState("TIME_TRANSMITTER", &s, true));
    cr_expect(eq(int, s, PTPMGMT_TIME_TRANSMITTER));
    cr_expect(ptpmgmt_msg_findPortState("Time_Transmitter", &s, false));
    cr_expect(eq(int, s, PTPMGMT_TIME_TRANSMITTER));
    cr_expect(ptpmgmt_msg_findPortState("PASSIVE", &s, true));
    cr_expect(eq(int, s, PTPMGMT_PASSIVE));
    cr_expect(ptpmgmt_msg_findPortState("Passive", &s, false));
    cr_expect(eq(int, s, PTPMGMT_PASSIVE));
    cr_expect(ptpmgmt_msg_findPortState("UNCALIBRATED", &s, true));
    cr_expect(eq(int, s, PTPMGMT_UNCALIBRATED));
    cr_expect(ptpmgmt_msg_findPortState("Uncalibrated", &s, false));
    cr_expect(eq(int, s, PTPMGMT_UNCALIBRATED));
    cr_expect(ptpmgmt_msg_findPortState("SLAVE", &s, true));
    cr_expect(eq(int, s, PTPMGMT_SLAVE));
    cr_expect(ptpmgmt_msg_findPortState("Slave", &s, false));
    cr_expect(eq(int, s, PTPMGMT_SLAVE));
    cr_expect(ptpmgmt_msg_findPortState("TIME_RECEIVER", &s, true));
    cr_expect(eq(int, s, PTPMGMT_TIME_RECEIVER));
    cr_expect(ptpmgmt_msg_findPortState("Time_Receiver", &s, false));
    cr_expect(eq(int, s, PTPMGMT_TIME_RECEIVER));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->findPortState("Slave", &s, false));
    m->free(m);
    // Confirm compatibility
    cr_expect(eq(int, PTPMGMT_PRE_MASTER, PTPMGMT_PRE_TIME_TRANSMITTER));
    cr_expect(eq(int, PTPMGMT_MASTER, PTPMGMT_TIME_TRANSMITTER));
    cr_expect(eq(int, PTPMGMT_SLAVE, PTPMGMT_TIME_RECEIVER));
}

// tests convert delay mechanism to string
// const char *ptpmgmt_msg_delayMech2str(enum ptpmgmt_delayMechanism_e type)
// const char *delayMech2str(enum ptpmgmt_delayMechanism_e type)
Test(MessageTest, MethodDelayMech2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_delayMech2str(PTPMGMT_AUTO), "AUTO"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_delayMech2str(PTPMGMT_E2E), "E2E"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_delayMech2str(PTPMGMT_P2P), "P2P"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_delayMech2str(PTPMGMT_NO_MECHANISM),
            "NO_MECHANISM"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_delayMech2str(PTPMGMT_COMMON_P2P),
            "COMMON_P2P"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_delayMech2str(PTPMGMT_SPECIAL),
            "SPECIAL"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->delayMech2str(PTPMGMT_E2E), "E2E"));
    m->free(m);
}

// tests convert string to delay mechanism
// bool ptpmgmt_msg_findDelayMech(const char *str,
//     enum ptpmgmt_delayMechanism_e *type, bool exact)
// bool findDelayMech(const char *str, enum ptpmgmt_delayMechanism_e *type,
//     bool exact)
Test(MessageTest, MethodFindDelayMech)
{
    enum ptpmgmt_delayMechanism_e t;
    cr_expect(ptpmgmt_msg_findDelayMech("AUTO", &t, true));
    cr_expect(eq(int, t, PTPMGMT_AUTO));
    cr_expect(ptpmgmt_msg_findDelayMech("Auto", &t, false));
    cr_expect(eq(int, t, PTPMGMT_AUTO));
    cr_expect(ptpmgmt_msg_findDelayMech("E2E", &t, true));
    cr_expect(eq(int, t, PTPMGMT_E2E));
    cr_expect(ptpmgmt_msg_findDelayMech("e2e", &t, false));
    cr_expect(eq(int, t, PTPMGMT_E2E));
    cr_expect(ptpmgmt_msg_findDelayMech("P2P", &t, true));
    cr_expect(eq(int, t, PTPMGMT_P2P));
    cr_expect(ptpmgmt_msg_findDelayMech("p2p", &t, false));
    cr_expect(eq(int, t, PTPMGMT_P2P));
    cr_expect(ptpmgmt_msg_findDelayMech("NO_MECHANISM", &t, true));
    cr_expect(eq(int, t, PTPMGMT_NO_MECHANISM));
    cr_expect(ptpmgmt_msg_findDelayMech("No_mechanism", &t, false));
    cr_expect(eq(int, t, PTPMGMT_NO_MECHANISM));
    cr_expect(ptpmgmt_msg_findDelayMech("COMMON_P2P", &t, true));
    cr_expect(eq(int, t, PTPMGMT_COMMON_P2P));
    cr_expect(ptpmgmt_msg_findDelayMech("Common_p2p", &t, false));
    cr_expect(eq(int, t, PTPMGMT_COMMON_P2P));
    cr_expect(ptpmgmt_msg_findDelayMech("SPECIAL", &t, true));
    cr_expect(eq(int, t, PTPMGMT_SPECIAL));
    cr_expect(ptpmgmt_msg_findDelayMech("Special", &t, false));
    cr_expect(eq(int, t, PTPMGMT_SPECIAL));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->findDelayMech("p2p", &t, false));
    cr_expect(eq(int, t, PTPMGMT_P2P));
    m->free(m);
}

// tests convert SMPTE clock locking state to string
// const char *ptpmgmt_msg_smpteLck2str(enum ptpmgmt_SMPTEmasterLockingStatus_e
//     state)
// const char *smpteLck2str(enum ptpmgmt_SMPTEmasterLockingStatus_e state)
Test(MessageTest, MethodSmpteLck2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_smpteLck2str(PTPMGMT_SMPTE_NOT_IN_USE),
            "NOT_IN_USE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_smpteLck2str(PTPMGMT_SMPTE_FREE_RUN),
            "FREE_RUN"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_smpteLck2str(PTPMGMT_SMPTE_COLD_LOCKING),
            "COLD_LOCKING"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_smpteLck2str(PTPMGMT_SMPTE_WARM_LOCKING),
            "WARM_LOCKING"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_smpteLck2str(PTPMGMT_SMPTE_LOCKED),
            "LOCKED"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->smpteLck2str(PTPMGMT_SMPTE_LOCKED), "LOCKED"));
    m->free(m);
}

// tests convert linuxptp time stamp type to string
// const char *ptpmgmt_msg_ts2str(enum ptpmgmt_linuxptpTimeStamp_e type)
// const char *ts2str(enum ptpmgmt_linuxptpTimeStamp_e type)
Test(MessageTest, MethodTs2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_ts2str(PTPMGMT_TS_SOFTWARE), "SOFTWARE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_ts2str(PTPMGMT_TS_HARDWARE), "HARDWARE"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_ts2str(PTPMGMT_TS_LEGACY_HW),
            "LEGACY_HW"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_ts2str(PTPMGMT_TS_ONESTEP), "ONESTEP"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_ts2str(PTPMGMT_TS_P2P1STEP), "P2P1STEP"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->ts2str(PTPMGMT_TS_ONESTEP), "ONESTEP"));
    m->free(m);
}

// tests convert linuxptp power profile version to string
// const char *ptpmgmt_msg_pwr2str(enum ptpmgmt_linuxptpPowerProfileVersion_e ver)
// const char *pwr2str(enum ptpmgmt_linuxptpPowerProfileVersion_e ver)
Test(MessageTest, MethodPwr2str)
{
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_pwr2str(PTPMGMT_IEEE_C37_238_VERSION_NONE),
            "NONE"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_pwr2str(PTPMGMT_IEEE_C37_238_VERSION_2011),
            "2011"));
    cr_expect(eq(str,
            (char *)ptpmgmt_msg_pwr2str(PTPMGMT_IEEE_C37_238_VERSION_2017),
            "2017"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->pwr2str(PTPMGMT_IEEE_C37_238_VERSION_2017),
            "2017"));
    m->free(m);
}

// tests convert linuxptp master unicasy state to string
// const char *ptpmgmt_msg_us2str(enum ptpmgmt_linuxptpUnicastState_e state)
// const char *us2str(enum ptpmgmt_linuxptpUnicastState_e state)
Test(MessageTest, MethodUs2str)
{
    cr_expect(eq(str, (char *)ptpmgmt_msg_us2str(PTPMGMT_UC_WAIT), "WAIT"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_us2str(PTPMGMT_UC_HAVE_ANN), "HAVE_ANN"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_us2str(PTPMGMT_UC_NEED_SYDY),
            "NEED_SYDY"));
    cr_expect(eq(str, (char *)ptpmgmt_msg_us2str(PTPMGMT_UC_HAVE_SYDY),
            "HAVE_SYDY"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(str, (char *)m->us2str(PTPMGMT_UC_WAIT), "WAIT"));
    m->free(m);
}

// Test if leap 61 seconds flag is enabled method
// bool ptpmgmt_msg_is_LI_61(uint8_t flags)
// bool is_LI_61(uint8_t flags)
Test(MessageTest, MethodIs_LI_61)
{
    cr_expect(ptpmgmt_msg_is_LI_61(PTPMGMT_F_LI_61));
    cr_expect(ptpmgmt_msg_is_LI_61(0xff));
    cr_expect(not(ptpmgmt_msg_is_LI_61(0)));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->is_LI_61(0xff));
    m->free(m);
}

// Test if leap 59 seconds flag is enabled
// bool ptpmgmt_msg_is_LI_59(uint8_t flags)
// bool is_LI_59(uint8_t flags)
Test(MessageTest, MethodIs_LI_59)
{
    cr_expect(ptpmgmt_msg_is_LI_59(PTPMGMT_F_LI_59));
    cr_expect(ptpmgmt_msg_is_LI_59(0xff));
    cr_expect(not(ptpmgmt_msg_is_LI_59(0)));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->is_LI_59(0xff));
    m->free(m);
}

// Test if UTC offset is valid flag is enabled
// bool ptpmgmt_msg_is_UTCV(uint8_t flags)
// bool is_UTCV(uint8_t flags)
Test(MessageTest, MethodIs_UTCV)
{
    cr_expect(ptpmgmt_msg_is_UTCV(PTPMGMT_F_UTCV));
    cr_expect(ptpmgmt_msg_is_UTCV(0xff));
    cr_expect(not(ptpmgmt_msg_is_UTCV(0)));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->is_UTCV(0xff));
    m->free(m);
}

// Test if is PTP instance flag is enabled
// bool ptpmgmt_msg_is_PTP(uint8_t flags)
// bool is_PTP(uint8_t flags)
Test(MessageTest, MethodIs_PTP)
{
    cr_expect(ptpmgmt_msg_is_PTP(PTPMGMT_F_PTP));
    cr_expect(ptpmgmt_msg_is_PTP(0xff));
    cr_expect(not(ptpmgmt_msg_is_PTP(0)));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->is_PTP(0xff));
    m->free(m);
}

// Test if timescale is traceable flag is enabled
// bool ptpmgmt_msg_is_TTRA(uint8_t flags)
// bool is_TTRA(uint8_t flags)
Test(MessageTest, MethodIs_TTRA)
{
    cr_expect(ptpmgmt_msg_is_TTRA(PTPMGMT_F_TTRA));
    cr_expect(ptpmgmt_msg_is_TTRA(0xff));
    cr_expect(not(ptpmgmt_msg_is_TTRA(0)));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->is_TTRA(0xff));
    m->free(m);
}

// Test if frequency is traceable flag is enabled
// bool ptpmgmt_msg_is_FTRA(uint8_t flags)
// bool is_FTRA(uint8_t flags)
Test(MessageTest, MethodIs_FTRA)
{
    cr_expect(ptpmgmt_msg_is_FTRA(PTPMGMT_F_FTRA));
    cr_expect(ptpmgmt_msg_is_FTRA(0xff));
    cr_expect(not(ptpmgmt_msg_is_FTRA(0)));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->is_FTRA(0xff));
    m->free(m);
}

// test if management TLV id uses empty dataField
// bool ptpmgmt_msg_isEmpty(enum ptpmgmt_mng_vals_e id)
// bool isEmpty(enum ptpmgmt_mng_vals_e id)
Test(MessageTest, MethodIsEmpty)
{
    cr_expect(ptpmgmt_msg_isEmpty(PTPMGMT_NULL_PTP_MANAGEMENT));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_CLOCK_DESCRIPTION)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_USER_DESCRIPTION)));
    cr_expect(ptpmgmt_msg_isEmpty(PTPMGMT_SAVE_IN_NON_VOLATILE_STORAGE));
    cr_expect(ptpmgmt_msg_isEmpty(PTPMGMT_RESET_NON_VOLATILE_STORAGE));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_INITIALIZE)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_FAULT_LOG)));
    cr_expect(ptpmgmt_msg_isEmpty(PTPMGMT_FAULT_LOG_RESET));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_DEFAULT_DATA_SET)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_CURRENT_DATA_SET)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PARENT_DATA_SET)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_TIME_PROPERTIES_DATA_SET)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PORT_DATA_SET)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PRIORITY1)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PRIORITY2)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_DOMAIN)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_SLAVE_ONLY)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_LOG_ANNOUNCE_INTERVAL)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_ANNOUNCE_RECEIPT_TIMEOUT)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_LOG_SYNC_INTERVAL)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_VERSION_NUMBER)));
    cr_expect(ptpmgmt_msg_isEmpty(PTPMGMT_ENABLE_PORT));
    cr_expect(ptpmgmt_msg_isEmpty(PTPMGMT_DISABLE_PORT));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_TIME)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_CLOCK_ACCURACY)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_UTC_PROPERTIES)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_TRACEABILITY_PROPERTIES)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_TIMESCALE_PROPERTIES)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_UNICAST_NEGOTIATION_ENABLE)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PATH_TRACE_LIST)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PATH_TRACE_ENABLE)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_GRANDMASTER_CLUSTER_TABLE)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_UNICAST_MASTER_TABLE)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_UNICAST_MASTER_MAX_TABLE_SIZE)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_ACCEPTABLE_MASTER_TABLE)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_ACCEPTABLE_MASTER_TABLE_ENABLED)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_ACCEPTABLE_MASTER_MAX_TABLE_SIZE)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_ALTERNATE_MASTER)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_ALTERNATE_TIME_OFFSET_ENABLE)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_ALTERNATE_TIME_OFFSET_NAME)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_ALTERNATE_TIME_OFFSET_MAX_KEY)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_ALTERNATE_TIME_OFFSET_PROPERTIES)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_TRANSPARENT_CLOCK_PORT_DATA_SET)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_LOG_MIN_PDELAY_REQ_INTERVAL)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_TRANSPARENT_CLOCK_DEFAULT_DATA_SET)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PRIMARY_DOMAIN)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_DELAY_MECHANISM)));
    cr_expect(not(
            ptpmgmt_msg_isEmpty(PTPMGMT_EXTERNAL_PORT_CONFIGURATION_ENABLED)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_MASTER_ONLY)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_HOLDOVER_UPGRADE_ENABLE)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_EXT_PORT_CONFIG_PORT_DATA_SET)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_TIME_STATUS_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_GRANDMASTER_SETTINGS_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PORT_DATA_SET_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_SUBSCRIBE_EVENTS_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PORT_PROPERTIES_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PORT_STATS_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PORT_SERVICE_STATS_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_UNICAST_MASTER_TABLE_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PORT_HWCLOCK_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_POWER_PROFILE_SETTINGS_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_CMLDS_INFO_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_PORT_CORRECTIONS_NP)));
    cr_expect(not(ptpmgmt_msg_isEmpty(PTPMGMT_EXTERNAL_GRANDMASTER_PROPERTIES_NP)));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(not(m->isEmpty(PTPMGMT_PORT_HWCLOCK_NP)));
    m->free(m);
}

// Test if management TLV is valid for use method
// bool isValidId(const_ptpmgmt_msg m, enum ptpmgmt_mng_vals_e id)
Test(MessageTest, MethodIsValidId)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->isValidId(m, PTPMGMT_PRIORITY1));
    cr_expect(m->isValidId(m, PTPMGMT_PRIORITY2));
    cr_expect(m->isValidId(m, PTPMGMT_DOMAIN));
    cr_expect(m->isValidId(m, PTPMGMT_SLAVE_ONLY));
    cr_expect(m->isValidId(m, PTPMGMT_VERSION_NUMBER));
    cr_expect(m->isValidId(m, PTPMGMT_ENABLE_PORT));
    cr_expect(m->isValidId(m, PTPMGMT_DISABLE_PORT));
    cr_expect(m->isValidId(m, PTPMGMT_TIME));
    cr_expect(m->isValidId(m, PTPMGMT_CLOCK_ACCURACY));
    cr_expect(m->isValidId(m, PTPMGMT_UTC_PROPERTIES));
    cr_expect(m->isValidId(m, PTPMGMT_TIME_STATUS_NP));
    cr_expect(m->isValidId(m, PTPMGMT_PORT_DATA_SET_NP));
    cr_expect(m->isValidId(m, PTPMGMT_PORT_PROPERTIES_NP));
    cr_expect(m->isValidId(m, PTPMGMT_PORT_STATS_NP));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    cr_expect(m->updateParams(m, p));
    cr_expect(not(m->isValidId(m, PTPMGMT_TIME_STATUS_NP)));
    cr_expect(not(m->isValidId(m, PTPMGMT_PORT_DATA_SET_NP)));
    cr_expect(not(m->isValidId(m, PTPMGMT_PORT_PROPERTIES_NP)));
    cr_expect(not(m->isValidId(m, PTPMGMT_PORT_STATS_NP)));
    m->free(m);
}

// Test set management TLV to build
// bool setAction(ptpmgmt_msg m, enum ptpmgmt_actionField_e actionField,
//     enum ptpmgmt_mng_vals_e tlv_id, const void *dataSend)
Test(MessageTest, MethodSetAction)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_GET));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 0x7f;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_SET));
    m->clearData(m);
    m->free(m);
}

// Test clear Data
// void clearData(const_ptpmgmt_msg m)
Test(MessageTest, MethodClearData)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 0x7f;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_SET));
    m->clearData(m);
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_GET));
    m->free(m);
}

// Test build from memory buffer
// enum ptpmgmt_MNG_PARSE_ERROR_e build(const_ptpmgmt_msg m, void *buf,
//     size_t bufSize, uint16_t sequence)
Test(MessageTest, MethodBuildVoid)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 0x7f;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_SET));
    ssize_t plen = m->getMsgPlanedLen(m);
    cr_expect(eq(int, plen, 56));
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 137),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getMsgLen(m), plen));
    // byte 4 is message length
    // Second byte is the PTP version
    uint8_t ret[56] = { 13, buf[1], 0, 56, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x89, 4, 0x7f, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1, 1, 1, 0, 0, 1, 0, 4, 0x20,
            5, 0x7f
        };
    cr_expect(zero(memcmp(buf, ret, sizeof ret)));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, plen), PTPMGMT_MNG_PARSE_ERROR_OK));
    m->free(m);
}

// Test clear Data
// enum ptpmgmt_actionField_e getSendAction(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetSendAction)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_GET));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 0x7f;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_SET));
    m->free(m);
}

// Test get build message length
// size_t getMsgLen(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetMsgLen)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 0x7f;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_SET));
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getMsgLen(m), 56));
    m->free(m);
}

// Test build using buffer object
// ssize_t getMsgPlanedLen(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetMsgPlanedLen)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_SET));
    cr_expect(eq(int, m->getMsgPlanedLen(m), 56));
    m->free(m);
}

// Test parse from memory buffer
// enum ptpmgmt_MNG_PARSE_ERROR_e parse(const_ptpmgmt_msg m, const void *buf,
//     ssize_t msgSize)
Test(MessageTest, MethodParseVoid)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 0x7f;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_SET));
    ssize_t plen = m->getMsgPlanedLen(m);
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 137),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, plen), PTPMGMT_MNG_PARSE_ERROR_OK));
    m->free(m);
}

// Test parse using buffer object
// enum ptpmgmt_actionField_e getReplyAction(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetReplyAction)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 137),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 56), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getReplyAction(m), PTPMGMT_RESPONSE));
    m->free(m);
}

// Test if message is unicast
// bool isUnicast(const_ptpmgmt_msg m)
Test(MessageTest, MethodIsUnicast)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(m->isUnicast(m));
    p->isUnicast = false;
    cr_expect(m->updateParams(m, p));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(not(m->isUnicast(m)));
    m->free(m);
}

// Test get PTP Profile Specific
// uint8_t getPTPProfileSpecific(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetPTPProfileSpecific)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[60];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getPTPProfileSpecific(m), 0));
    m->free(m);
}

// Test get message sequence
// uint16_t getSequence(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetSequence)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[60];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 0x4231),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getSequence(m), 0x4231));
    m->free(m);
}

// Test get peer port address of parsed message
// const struct ptpmgmt_PortIdentity_t *getPeer(ptpmgmt_msg m)
Test(MessageTest, MethodGetPeer)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    struct ptpmgmt_PortIdentity_t t;
    t.portNumber = 0x3184;
    memcpy(t.clockIdentity.v, "\x1\x2\x3\x4\x5\x6\x7\x8",
        sizeof(struct ptpmgmt_ClockIdentity_t));
    memcpy(&p->self_id, &t, sizeof(struct ptpmgmt_PortIdentity_t));
    cr_expect(m->updateParams(m, p));
    uint8_t buf[60];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(zero(memcmp(m->getPeer(m), &t,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    m->free(m);
}

// Test get target port address of parsed message
// const struct ptpmgmt_PortIdentity_t *getTarget(ptpmgmt_msg m)
Test(MessageTest, MethodGetTarget)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    struct ptpmgmt_PortIdentity_t t;
    t.portNumber = 0x3184;
    memcpy(t.clockIdentity.v, "\x1\x2\x3\x4\x5\x6\x7\x8",
        sizeof(struct ptpmgmt_ClockIdentity_t));
    memcpy(&p->target, &t, sizeof(struct ptpmgmt_PortIdentity_t));
    cr_expect(m->updateParams(m, p));
    uint8_t buf[60];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(zero(memcmp(m->getTarget(m), &t,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    m->free(m);
}

// Test get transport specific ID of parsed message
// uint32_t getSdoId(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetSdoId)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    p->transportSpecific = 14;
    cr_expect(m->updateParams(m, p));
    uint8_t buf[60];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getSdoId(m), 14 * 0x100));
    m->free(m);
}

// Test get domain number of parsed message
// uint8_t getDomainNumber(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetDomainNumber)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    p->domainNumber = 7;
    cr_expect(m->updateParams(m, p));
    uint8_t buf[60];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getDomainNumber(m), 7));
    m->free(m);
}

// Test get PTP version of parsed message
// uint8_t getVersionPTP(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetVersionPTP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[60];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getVersionPTP(m), 2)); // PTP 2
    m->free(m);
}

// Test get PTP minor version of parsed message
// uint8_t getMinorVersionPTP(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetMinorVersionPTP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[60];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    // Currently we use PTP 2->0, but we may upgrade->
    cr_expect(eq(int, m->getVersionPTP(m), 2));
    cr_expect(ge(int, m->getMinorVersionPTP(m), 0));
    m->free(m);
}

// Test get tlv data of parsed message
// const void *getData() const
Test(MessageTest, MethodGetData)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 137;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 56), PTPMGMT_MNG_PARSE_ERROR_OK));
    const void *data = m->getData(m);
    cr_assert(not(zero(ptr, (void *)data)));
    const struct ptpmgmt_PRIORITY1_t *p1 = (const struct ptpmgmt_PRIORITY1_t *)data;
    cr_expect(eq(int, p1->priority1, p.priority1));
    m->free(m);
}

// Test get send tlv data
// const void *getSendData(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetSendData)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(ptr, (void *)m->getSendData(m), &p));
    m->free(m);
}

// Test get error of parsed message
// enum ptpmgmt_managementErrorId_e getErrId(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetErrId)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[80];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // Follow location IEEE "PTP management message"
    // Change to actionField to response of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    // Create error managementTLV
    uint16_t *mt = (uint16_t *)(buf + 48);
    char displayData[] = "test 123";
    uint8_t l = sizeof displayData - 1; // textField size exclude the null
    int n = l + 1; // lengthField + textField
    n += n & 1;
    mt[0] = htons(PTPMGMT_MANAGEMENT_ERROR_STATUS); // tlvType
    mt[1] = htons(8 + n); // lengthField
    mt[3] = mt[2]; // cp managementId from "MANAGEMENT TLV"
    mt[2] = htons(PTPMGMT_WRONG_VALUE); // managementErrorId
    mt[4] = 0; // reserved
    mt[5] = 0; // reserved
    uint8_t *d = buf + 48 + 12; // displayData
    d[0] = l; // displayData->lengthField
    buf[3] = 48 + 12 + n; // header.messageLength
    memcpy(d + 1, displayData, l); // displayData->textField
    cr_expect(eq(int, m->parse(m, buf, 48 + 12 + n), PTPMGMT_MNG_PARSE_ERROR_MSG));
    cr_expect(eq(int, m->getErrId(m), PTPMGMT_WRONG_VALUE));
    m->free(m);
}

// Test get error message of mangment error TLV parsed message
// const char *getErrDisplay(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetErrDisplayC)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // Follow location IEEE "PTP management message"
    // Change to actionField to response of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    // Create error managementTLV
    uint16_t *mt = (uint16_t *)(buf + 48);
    char displayData[] = "test 123";
    uint8_t l = sizeof displayData - 1; // textField size exclude the null
    int n = l + 1; // lengthField + textField
    n += n & 1;
    mt[0] = htons(PTPMGMT_MANAGEMENT_ERROR_STATUS); // tlvType
    mt[1] = htons(8 + n); // lengthField
    mt[3] = mt[2]; // cp managementId from "MANAGEMENT TLV"
    mt[2] = htons(PTPMGMT_WRONG_VALUE); // managementErrorId
    mt[4] = 0; // reserved
    mt[5] = 0; // reserved
    uint8_t *d = buf + 48 + 12; // displayData
    d[0] = l; // displayData->lengthField
    buf[3] = 48 + 12 + n; // header.messageLength
    memcpy(d + 1, displayData, l); // displayData->textField
    cr_expect(eq(int, m->parse(m, buf, 48 + 12 + n), PTPMGMT_MNG_PARSE_ERROR_MSG));
    cr_expect(eq(str, (char *)m->getErrDisplay(m), displayData));
    m->free(m);
}

// Test get error message of mangment error TLV parsed message
// bool isLastMsgSig(const_ptpmgmt_msg m)
Test(MessageTest, MethodIsLastMsgSig)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    // MNG msg = 36 + 10 targetPortIdentity + 4 = 48
    // MNG msg 48 + 6 Mng TLV + 2 PTPMGMT_PRIORITY1 TLV = 56
    cr_expect(eq(int, m->parse(m, buf, 56), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(not(m->isLastMsgSig(m)));
    // signaling = 36 header + 10 targetPortIdentity = 44
    // signaling MSG 44 + 6 Mng TLV + 2 PTPMGMT_PRIORITY1 TLV = 52
    buf[0] = (buf[0] & 0xf0) | ptpmgmt_Signaling; // messageType
    buf[3] = 52; // header.messageLength
    buf[32] = 5; // controlField
    // Move the 8 bytes of Mng TLV
    for(int i = 0; i < 8; i++)
        buf[44 + i] = buf[48 + i];
    ptpmgmt_pMsgParams mp = m->getParams(m);
    mp->rcvSignaling = true;
    cr_expect(m->updateParams(m, mp));
    // valueField already have Mng TLV at the proper place :-)
    cr_expect(eq(int, m->parse(m, buf, 52), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    m->free(m);
}

// Test get parsed PTP message type
// enum ptpmgmt_msgType_e getType(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetType)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[60];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getType(m), ptpmgmt_Management));
    m->free(m);
}

// Test get parsed PTP management message management type
// enum ptpmgmt_tlvType_e getMngType(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetMngType)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[60];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, m->parse(m, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getMngType(m), PTPMGMT_MANAGEMENT));
    m->free(m);
}

// Test travers signalling message of TLVs
// bool traversSigTlvs(ptpmgmt_msg m, void *cookie,
//     ptpmgmt_msg_sig_callback callback)
// bool ptpmgmt_msg_sig_callback(void *cookie, const_ptpmgmt_msg m,
//     enum ptpmgmt_tlvType_e tlvType, const void *tlv)
struct cookie_t { int val; };
static bool verifyPr1(void *cookie, const_ptpmgmt_msg m,
    enum ptpmgmt_tlvType_e tlvType, const void *tlv)
{
    // Verify cookie value
    if(((struct cookie_t *)cookie)->val != 0xfefe)
        return false;
    struct ptpmgmt_MANAGEMENT_t *mng = (struct ptpmgmt_MANAGEMENT_t *)tlv;
    struct ptpmgmt_PRIORITY1_t *p1 = NULL;
    if(mng != NULL && mng->managementId == PTPMGMT_PRIORITY1)
        p1 = (struct ptpmgmt_PRIORITY1_t *)mng->tlvData;
    return tlvType == PTPMGMT_MANAGEMENT && p1 != NULL && p1->priority1 == 137;
}
Test(MessageTest, MethodTraversSigTlvs)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 137;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    // MNG msg = 36 + 10 targetPortIdentity + 4 = 48
    // MNG msg 48 + 6 Mng TLV + 2 PRIORITY1 TLV = 56
    // signalling = 36 header + 10 targetPortIdentity = 44
    // signalling MSG 44 + 6 Mng TLV + 2 PRIORITY1 TLV = 52
    buf[0] = (buf[0] & 0xf0) | ptpmgmt_Signaling; // messageType
    buf[3] = 52; // header.messageLength
    buf[32] = 5; // controlField
    // Move the 8 bytes of Mng TLV
    for(int i = 0; i < 8; i++)
        buf[44 + i] = buf[48 + i];
    ptpmgmt_pMsgParams mp = m->getParams(m);
    mp->rcvSignaling = true;
    mp->filterSignaling = false;
    cr_expect(m->updateParams(m, mp));
    // valueField already have Mng TLV at the proper place :-)
    cr_expect(eq(int, m->parse(m, buf, 52), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(eq(int, m->getSigTlvsCount(m), 1));
    struct cookie_t a = { 0xfefe };
    cr_expect(m->traversSigTlvs(m, &a, verifyPr1));
}

// Test get number of TLVs in a PTP signaling message
// size_t getSigTlvsCount(const_ptpmgmt_msg m)
Test(MessageTest, MethodGetSigTlvsCount)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 137;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    // MNG msg = 36 + 10 targetPortIdentity + 4 = 48
    // MNG msg 48 + 6 Mng TLV + 2 PTPMGMT_PRIORITY1 TLV = 56
    // signaling = 36 header + 10 targetPortIdentity = 44
    // signaling MSG 44 + 6 Mng TLV + 2 PTPMGMT_PRIORITY1 TLV = 52
    buf[0] = (buf[0] & 0xf0) | ptpmgmt_Signaling; // messageType
    buf[3] = 52; // header.messageLength
    buf[32] = 5; // controlField
    // Move the 8 bytes of Mng TLV
    for(int i = 0; i < 8; i++)
        buf[44 + i] = buf[48 + i];
    ptpmgmt_pMsgParams mp = m->getParams(m);
    mp->rcvSignaling = true;
    mp->filterSignaling = false;
    cr_expect(m->updateParams(m, mp));
    // valueField already have Mng TLV at the proper place :-)
    cr_expect(eq(int, m->parse(m, buf, 52), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(eq(int, m->getSigTlvsCount(m), 1));
    m->free(m);
}

// Test get a TLV and get its type from a PTP signaling message
// const void *getSigTlv(const_ptpmgmt_msg m, size_t position)
// enum ptpmgmt_tlvType_e getSigTlvType(const_ptpmgmt_msg m, size_t position)
Test(MessageTest, MethodGetSigTlv)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 137;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    // MNG msg = 36 + 10 targetPortIdentity + 4 = 48
    // MNG msg 48 + 6 Mng TLV + 2 PTPMGMT_PRIORITY1 TLV = 56
    // signaling = 36 header + 10 targetPortIdentity = 44
    // signaling MSG 44 + 6 Mng TLV + 2 PTPMGMT_PRIORITY1 TLV = 52
    buf[0] = (buf[0] & 0xf0) | ptpmgmt_Signaling; // messageType
    buf[3] = 52; // header.messageLength
    buf[32] = 5; // controlField
    // Move the 8 bytes of Mng TLV
    for(int i = 0; i < 8; i++)
        buf[44 + i] = buf[48 + i];
    ptpmgmt_pMsgParams mp = m->getParams(m);
    mp->rcvSignaling = true;
    mp->filterSignaling = false;
    cr_expect(m->updateParams(m, mp));
    // valueField already have Mng TLV at the proper place :-)
    cr_expect(eq(int, m->parse(m, buf, 52), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(eq(int, m->getSigTlvType(m, 0), PTPMGMT_MANAGEMENT));
    const void *tlv = m->getSigTlv(m, 0);
    cr_assert(not(zero(ptr, (void *)tlv)));
    const struct ptpmgmt_MANAGEMENT_t *mng =
        (const struct ptpmgmt_MANAGEMENT_t *)tlv;
    cr_assert(not(zero(ptr, (void *)mng)));
    cr_expect(eq(int, mng->managementId, PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->getSigMngTlvType(m, 0), PTPMGMT_PRIORITY1));
    const struct ptpmgmt_PRIORITY1_t *p1 = (const struct ptpmgmt_PRIORITY1_t *)
        mng->tlvData;
    cr_assert(not(zero(ptr, (void *)p1)));
    cr_expect(eq(int, p1->priority1, 137));
    m->free(m);
}

// Test get a management TLV and its type from a PTP signaling message
// const void *getSigMngTlv(ptpmgmt_msg m, size_t position)
// enum ptpmgmt_mng_vals_e getSigMngTlvType(const_ptpmgmt_msg m, size_t position)
Test(MessageTest, MethodGetSigMngTlvType)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 137;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    uint8_t buf[70];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    // MNG msg = 36 + 10 targetPortIdentity + 4 = 48
    // MNG msg 48 + 6 Mng TLV + 2 PTPMGMT_PRIORITY1 TLV = 56
    // signaling = 36 header + 10 targetPortIdentity = 44
    // signaling MSG 44 + 6 Mng TLV + 2 PTPMGMT_PRIORITY1 TLV = 52
    buf[0] = (buf[0] & 0xf0) | ptpmgmt_Signaling; // messageType
    buf[3] = 52; // header.messageLength
    buf[32] = 5; // controlField
    // Move the 8 bytes of Mng TLV
    for(int i = 0; i < 8; i++)
        buf[44 + i] = buf[48 + i];
    ptpmgmt_pMsgParams mp = m->getParams(m);
    mp->rcvSignaling = true;
    mp->filterSignaling = false;
    cr_expect(m->updateParams(m, mp));
    // valueField already have Mng TLV at the proper place :-)
    cr_expect(eq(int, m->parse(m, buf, 52), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(eq(int, m->getSigMngTlvType(m, 0), PTPMGMT_PRIORITY1));
    const struct ptpmgmt_PRIORITY1_t *p1 = (const struct ptpmgmt_PRIORITY1_t *)
        m->getSigMngTlv(m, 0);
    cr_assert(not(zero(ptr, (void *)p1)));
    cr_expect(eq(int, p1->priority1, 137));
    m->free(m);
}

// ptpmgmt_tlv_mem ptpmgmt_tlv_mem_alloc()
// enum ptpmgmt_mng_vals_e getID(const_ptpmgmt_tlv_mem self)
// enum ptpmgmt_mng_vals_e id
// void *getTLV(const_ptpmgmt_tlv_mem self)
// void *tlv
// bool newTlv(ptpmgmt_tlv_mem self, enum ptpmgmt_mng_vals_e ID)
// void free(ptpmgmt_tlv_mem self)
Test(TlvMem, newTlv)
{
    ptpmgmt_tlv_mem t = ptpmgmt_tlv_mem_alloc();
    cr_assert(not(zero(ptr, t)));
    cr_expect(eq(int, t->getID(t), PTPMGMT_NULL_PTP_MANAGEMENT));
    cr_expect(eq(int, t->id, PTPMGMT_NULL_PTP_MANAGEMENT));
    cr_assert(zero(ptr, t->getTLV(t)));
    cr_assert(zero(ptr, t->tlv));
    cr_expect(t->newTlv(t, PTPMGMT_USER_DESCRIPTION));
    cr_expect(eq(int, t->getID(t), PTPMGMT_USER_DESCRIPTION));
    cr_expect(eq(int, t->id, PTPMGMT_USER_DESCRIPTION));
    cr_assert(not(zero(ptr, t->getTLV(t))));
    cr_assert(not(zero(ptr, t->tlv)));
    t->free(t);
}

// bool copyTlv(ptpmgmt_tlv_mem self, enum ptpmgmt_mng_vals_e ID, void *tlv)
// void clear(ptpmgmt_tlv_mem self)
Test(TlvMem, copyTlv)
{
    ptpmgmt_tlv_mem t = ptpmgmt_tlv_mem_alloc();
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t v = {
        .keyField = 54,
        .displayName.lengthField = 4,
        .displayName.textField = "AaBb"
    };
    cr_expect(t->copyTlv(t, PTPMGMT_ALTERNATE_TIME_OFFSET_NAME, &v));
    cr_expect(eq(int, t->getID(t), PTPMGMT_ALTERNATE_TIME_OFFSET_NAME));
    void *p = t->getTLV(t);
    cr_assert(not(zero(ptr, p)));
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t *v2 = (struct
            ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t *)p;
    cr_assert(not(zero(ptr, v2)));
    cr_expect(ne(ptr, v2, &v));
    cr_expect(ne(ptr, (char *)v2->displayName.textField,
            (char *)v.displayName.textField));
    cr_expect(eq(int, v2->keyField, 54));
    cr_expect(eq(u8, v2->displayName.lengthField, 4));
    cr_expect(eq(str, (char *)v2->displayName.textField, "AaBb"));
    t->clear(t);
    cr_expect(eq(int, t->getID(t), PTPMGMT_NULL_PTP_MANAGEMENT));
    cr_assert(zero(ptr, t->getTLV(t)));
    t->free(t);
}

// bool copy(ptpmgmt_tlv_mem self, const_ptpmgmt_tlv_mem other)
// void *allocMem(ptpmgmt_tlv_mem self, size_t size)
// struct ptpmgmt_PTPText_t allocString(ptpmgmt_tlv_mem self, const char *str)
// struct ptpmgmt_PTPText_t allocStringLen(ptpmgmt_tlv_mem self, const char *str,
//     size_t len)
Test(TlvMem, copy)
{
    ptpmgmt_tlv_mem t1 = ptpmgmt_tlv_mem_alloc();
    cr_expect(t1->newTlv(t1, PTPMGMT_CLOCK_DESCRIPTION));
    void *p = t1->getTLV(t1);
    cr_assert(not(zero(ptr, p)));
    struct ptpmgmt_CLOCK_DESCRIPTION_t *v = (struct ptpmgmt_CLOCK_DESCRIPTION_t *)p;
    v->clockType = 1687;
    v->physicalLayerProtocol = t1->allocString(t1, "test12");
    v->physicalAddressLength = 4;
    v->physicalAddress = t1->allocMem(t1, 4);
    memcpy(v->physicalAddress, "\x1\x2\x3\x4", 4);
    v->protocolAddress.networkProtocol = ptpmgmt_UDP_IPv4;
    v->protocolAddress.addressLength = 4;
    v->protocolAddress.addressField = (uint8_t *)"\x12\x0\x0\x31";
    v->manufacturerIdentity[0] = 0x24;
    v->manufacturerIdentity[1] = 0x35;
    v->manufacturerIdentity[2] = 0x78;
    v->productDescription = t1->allocStringLen(t1, "Description", 4);
    v->revisionData = t1->allocString(t1, "rev");
    v->userDescription = t1->allocString(t1, "des123");
    v->profileIdentity[0] = 0x32;
    v->profileIdentity[1] = 0x25;
    v->profileIdentity[2] = 0x12;
    v->profileIdentity[3] = 0x63;
    v->profileIdentity[4] = 0x4d;
    v->profileIdentity[5] = 0x27;
    ptpmgmt_tlv_mem t2 = ptpmgmt_tlv_mem_alloc();
    cr_expect(t2->copy(t2, t1));
    t1->clear(t1);
    struct ptpmgmt_CLOCK_DESCRIPTION_t *v2 = (struct ptpmgmt_CLOCK_DESCRIPTION_t *)
        t2->getTLV(t2);
    cr_assert(not(zero(ptr, v2)));
    cr_expect(eq(int, t2->getID(t2), PTPMGMT_CLOCK_DESCRIPTION));
    cr_expect(eq(u16, v2->clockType, 1687));
    cr_expect(eq(u8, v2->physicalLayerProtocol.lengthField, 6));
    cr_expect(eq(str, (char *)v2->physicalLayerProtocol.textField, "test12"));
    cr_expect(eq(u16, v2->physicalAddressLength, 4));
    cr_expect(zero(memcmp(v2->physicalAddress, "\x1\x2\x3\x4", 4)));
    cr_expect(eq(int, v2->protocolAddress.networkProtocol, ptpmgmt_UDP_IPv4));
    cr_expect(eq(u16, v2->protocolAddress.addressLength, 4));
    cr_expect(zero(memcmp(v2->protocolAddress.addressField, "\x12\x0\x0\x31", 4)));
    cr_expect(zero(memcmp(v2->manufacturerIdentity, "\x24\x35\x78", 3)));
    cr_expect(eq(u8, v2->productDescription.lengthField, 4));
    cr_expect(eq(str, (char *)v2->productDescription.textField, "Desc"));
    cr_expect(eq(u8, v2->revisionData.lengthField, 3));
    cr_expect(eq(str, (char *)v2->revisionData.textField, "rev"));
    cr_expect(eq(u8, v2->userDescription.lengthField, 6));
    cr_expect(eq(str, (char *)v2->userDescription.textField, "des123"));
    cr_expect(zero(memcmp(v2->profileIdentity, "\x32\x25\x12\x63\x4d\x27", 6)));
    t1->free(t1);
    t2->free(t2);
}

// void *callocMem(ptpmgmt_tlv_mem self, size_t number, size_t size)
// void *recallocMem(ptpmgmt_tlv_mem self, void *memory, size_t number, size_t size)
Test(TlvMem, callocMem)
{
    ptpmgmt_tlv_mem t = ptpmgmt_tlv_mem_alloc();
    cr_expect(t->newTlv(t, PTPMGMT_ACCEPTABLE_MASTER_TABLE));
    void *p = t->getTLV(t);
    cr_assert(not(zero(ptr, p)));
    struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_t *v = (struct
            ptpmgmt_ACCEPTABLE_MASTER_TABLE_t *)p;
    v->actualTableSize = 3;
    v->list = t->callocMem(t, 3, sizeof(struct ptpmgmt_AcceptableMaster_t));
    v->list[0].alternatePriority1 = 1;
    memcpy(v->list[0].acceptablePortIdentity.clockIdentity.v,
        "\x1\x0\x1\x0\x1\x0\x1\x0", 8);
    v->list[0].acceptablePortIdentity.portNumber = 1;
    v->list[1].alternatePriority1 = 2;
    memcpy(v->list[1].acceptablePortIdentity.clockIdentity.v,
        "\x2\x0\x2\x0\x2\x0\x2\x0", 8);
    v->list[1].acceptablePortIdentity.portNumber = 2;
    v->list[2].alternatePriority1 = 3;
    memcpy(v->list[2].acceptablePortIdentity.clockIdentity.v,
        "\x3\x0\x3\x0\x3\x0\x3\x0", 8);
    v->list[2].acceptablePortIdentity.portNumber = 3;
    void *n = t->recallocMem(t, v->list, 4,
            sizeof(struct ptpmgmt_AcceptableMaster_t));
    cr_assert(not(zero(ptr, n)));
    v->actualTableSize = 4;
    v->list = n;
    v->list[3].alternatePriority1 = 4;
    memcpy(v->list[3].acceptablePortIdentity.clockIdentity.v,
        "\x4\x0\x4\x0\x4\x0\x4\x0", 8);
    v->list[3].acceptablePortIdentity.portNumber = 4;
    ptpmgmt_tlv_mem t2 = ptpmgmt_tlv_mem_alloc();
    cr_expect(t2->copy(t2, t));
    struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_t *v2 = (struct
            ptpmgmt_ACCEPTABLE_MASTER_TABLE_t *)t2->getTLV(t2);
    cr_assert(not(zero(ptr, v2)));
    cr_expect(eq(int, t2->getID(t2), PTPMGMT_ACCEPTABLE_MASTER_TABLE));
    cr_expect(eq(i16, v->actualTableSize, 4));
    cr_expect(eq(u8, v->list[0].alternatePriority1, 1));
    cr_expect(zero(memcmp(v->list[0].acceptablePortIdentity.clockIdentity.v,
                "\x1\x0\x1\x0\x1\x0\x1\x0", 8)));
    cr_expect(eq(u16, v->list[0].acceptablePortIdentity.portNumber, 1));
    cr_expect(eq(u8, v->list[1].alternatePriority1, 2));
    cr_expect(zero(memcmp(v->list[1].acceptablePortIdentity.clockIdentity.v,
                "\x2\x0\x2\x0\x2\x0\x2\x0", 8)));
    cr_expect(eq(u16, v->list[1].acceptablePortIdentity.portNumber, 2));
    cr_expect(eq(u8, v->list[2].alternatePriority1, 3));
    cr_expect(zero(memcmp(v->list[2].acceptablePortIdentity.clockIdentity.v,
                "\x3\x0\x3\x0\x3\x0\x3\x0", 8)));
    cr_expect(eq(u16, v->list[2].acceptablePortIdentity.portNumber, 3));
    cr_expect(eq(u8, v->list[3].alternatePriority1, 4));
    cr_expect(zero(memcmp(v->list[3].acceptablePortIdentity.clockIdentity.v,
                "\x4\x0\x4\x0\x4\x0\x4\x0", 8)));
    cr_expect(eq(u16, v->list[3].acceptablePortIdentity.portNumber, 4));
    t->free(t);
    t2->free(t2);
}

// bool reallocString(ptpmgmt_tlv_mem self, struct ptpmgmt_PTPText_t *text,
//     const char *str)
// bool reallocStringLen(ptpmgmt_tlv_mem self, struct ptpmgmt_PTPText_t *text,
//     const char *str, size_t len)
// bool freeString(ptpmgmt_tlv_mem self, struct ptpmgmt_PTPText_t *txt)
Test(TlvMem, reallocString)
{
    ptpmgmt_tlv_mem t = ptpmgmt_tlv_mem_alloc();
    cr_expect(t->newTlv(t, PTPMGMT_USER_DESCRIPTION));
    void *p = t->getTLV(t);
    cr_assert(not(zero(ptr, p)));
    struct ptpmgmt_USER_DESCRIPTION_t *v = (struct ptpmgmt_USER_DESCRIPTION_t *)p;
    cr_expect(t->reallocString(t, &v->userDescription, "desc 12"));
    cr_expect(eq(u8, v->userDescription.lengthField, 7));
    cr_expect(eq(str, (char *)v->userDescription.textField, "desc 12"));
    char *a1 = (char *)v->userDescription.textField;
    cr_expect(t->reallocStringLen(t, &v->userDescription, "123456", 4));
    cr_expect(eq(ptr, a1, (char *)v->userDescription.textField));
    cr_expect(eq(u8, v->userDescription.lengthField, 4));
    cr_expect(eq(str, (char *)v->userDescription.textField, "1234"));
    cr_expect(t->reallocStringLen(t, &v->userDescription, "1234567890", 10));
    cr_expect(eq(u8, v->userDescription.lengthField, 10));
    cr_expect(eq(str, (char *)v->userDescription.textField, "1234567890"));
    cr_expect(t->freeString(t, &v->userDescription));
    cr_expect(eq(u8, v->userDescription.lengthField, 0));
    cr_expect(zero(ptr, (char *)v->userDescription.textField));
    t->free(t);
}

// void *reallocMem(ptpmgmt_tlv_mem self, void *memory, size_t size)
// bool freeMem(ptpmgmt_tlv_mem self, void *mem)
Test(TlvMem, reallocMem)
{
    ptpmgmt_tlv_mem t = ptpmgmt_tlv_mem_alloc();
    cr_expect(t->newTlv(t, PTPMGMT_UNICAST_MASTER_TABLE));
    struct ptpmgmt_UNICAST_MASTER_TABLE_t *v = (struct
            ptpmgmt_UNICAST_MASTER_TABLE_t *)t->getTLV(t);
    cr_assert(not(zero(ptr, v)));
    v->logQueryInterval = 5; //Integer8_t
    v->actualTableSize = 2; // UInteger16_t
    v->PortAddress = t->callocMem(t, 2, sizeof(struct ptpmgmt_PortAddress_t));
    v->PortAddress[0].networkProtocol = ptpmgmt_UDP_IPv4;
    v->PortAddress[0].addressLength = 4; // UInteger16_t
    v->PortAddress[0].addressField = t->reallocMem(t,
            v->PortAddress[0].addressField, 4);
    cr_expect(not(zero(ptr, v->PortAddress[0].addressField)));
    v->PortAddress[1].networkProtocol = ptpmgmt_UDP_IPv6;
    v->PortAddress[1].addressLength = 6;
    v->PortAddress[1].addressField = t->reallocMem(t,
            v->PortAddress[0].addressField, 6);
    cr_expect(not(zero(ptr, v->PortAddress[1].addressField)));
    cr_expect(t->freeMem(t, v->PortAddress));
    t->free(t);
}
