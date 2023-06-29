/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief types structures unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "types.h"

/*****************************************************************************/
// Unit tests for struct MsgParams

// Tests allow a signal TLV method
// void allowSigTlv(ptpmgmt_pMsgParams m, enum ptpmgmt_tlvType_e type)
// void free(ptpmgmt_pMsgParams m)
Test(MsgParamsTest, MethodAllowSigTlv)
{
    ptpmgmt_pMsgParams m = ptpmgmt_MsgParams_alloc();
    m->allowSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION);
    cr_expect(m->isSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION));
    m->free(m);
}

// Tests remove a signal TLV method
// void removeSigTlv(ptpmgmt_pMsgParams m, enum ptpmgmt_tlvType_e type)
Test(MsgParamsTest, MethodRemoveSigTlv)
{
    ptpmgmt_pMsgParams m = ptpmgmt_MsgParams_alloc();
    m->allowSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION);
    cr_expect(m->isSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION));
    m->allowSigTlv(m, PTPMGMT_MANAGEMENT_ERROR_STATUS);
    m->removeSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION);
    cr_expect(not(m->isSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION)));
    m->free(m);
}

// Tests query if a signal TLV present method
// bool isSigTlv(ptpmgmt_cpMsgParams m, enum ptpmgmt_tlvType_e type)
Test(MsgParamsTest, MethodIsSigTlv)
{
    ptpmgmt_pMsgParams m = ptpmgmt_MsgParams_alloc();
    m->allowSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION);
    m->allowSigTlv(m, PTPMGMT_MANAGEMENT_ERROR_STATUS);
    cr_expect(m->isSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION));
    cr_expect(m->isSigTlv(m, PTPMGMT_MANAGEMENT_ERROR_STATUS));
    cr_expect(not(m->isSigTlv(m, PTPMGMT_REQUEST_UNICAST_TRANSMISSION)));
    m->free(m);
}

// Tests count of signal TLVs method
// size_t countSigTlvs(ptpmgmt_cpMsgParams m)
Test(MsgParamsTest, MethodCountSigTlv)
{
    ptpmgmt_pMsgParams m = ptpmgmt_MsgParams_alloc();
    m->filterSignaling = true;
    m->allowSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION);
    m->allowSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE);
    m->allowSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE);
    cr_expect(m->isSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION));
    cr_expect(m->isSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE));
    cr_expect(m->isSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE));
    cr_expect(eq(sz, m->countSigTlvs(m), 3));
    // Filter the middle TLV :-)
    m->removeSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE);
    cr_expect(m->isSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION));
    cr_expect(not(m->isSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE)));
    cr_expect(m->isSigTlv(m, PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE));
    cr_expect(eq(sz, m->countSigTlvs(m), 2));
    m->free(m);
}
