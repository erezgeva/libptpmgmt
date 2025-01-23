/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Json2msg wrapper class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "json.h"

#define ip_v ("\x12\x34\x56\x78")
#define address_v ("\xc4\x7d\x46\x20\xac\xae")
#define clockId ("\xc4\x7d\x46\xff\xfe\x20\xac\xae")
#define allIds ("\xff\xff\xff\xff\xff\xff\xff\xff")

// Tests selectLib method
// bool ptpmgmt_json_selectLib(const char *libName)
Test(Json2msgTest, MethodSelectLib)
{
    // We use static link :-)
    cr_expect(not(ptpmgmt_json_selectLib("jsonc")));
}

// Tests isLibShared method
// bool ptpmgmt_json_isLibShared()
Test(Json2msgTest, MethodIsLibShared)
{
    // We use static link :-)
    cr_expect(not(ptpmgmt_json_isLibShared()));
}

// Tests fromJson method
// bool fromJson(ptpmgmt_json m, const char *json)
Test(Json2msgTest, MethodFromJson)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_expect(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"unicastFlag\":true,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"NULL_PTP_MANAGEMENT\","
            "\"dataField\":null"
            "}"));
    m->free(m);
}

// Tests managementId method
// ptpmgmt_mng_vals_e ptpmgmt_json_managementId(const_ptpmgmt_json m)
Test(Json2msgTest, MethodManagementId)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"unicastFlag\":true,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"SET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\","
            "\"dataField\" :"
            " {"
            "  \"priority1\":153"
            " }"
            "}"));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PRIORITY1));
    m->free(m);
}

// Tests dataField method
// const void *ptpmgmt_json_dataField(ptpmgmt_json m)
Test(Json2msgTest, MethodDataField)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"unicastFlag\":true,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"SET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\","
            "\"dataField\" :"
            " {"
            "  \"priority1\":153"
            " }"
            "}"));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PRIORITY1));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PRIORITY1_t *t = (struct ptpmgmt_PRIORITY1_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(u8, t->priority1, 153));
    m->free(m);
}

// Tests actionField method
// ptpmgmt_actionField_e actionField(const_ptpmgmt_json m)
Test(Json2msgTest, MethodActionField)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"unicastFlag\":true,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_GET));
    m->free(m);
}

// Tests isUnicast method
// bool isUnicast(const_ptpmgmt_json m)
Test(Json2msgTest, MethodIsUnicast)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"unicastFlag\":true,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(m->isUnicast(m));
    cr_expect(m->haveIsUnicast(m));
    m->free(m);
}

// Tests haveIsUnicast method
// bool haveIsUnicast(const_ptpmgmt_json m)
Test(Json2msgTest, MethodHaveIsUnicast)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(not(m->haveIsUnicast(m)));
    m->free(m);
}

// Tests PTPProfileSpecific method
// uint8_t PTPProfileSpecific(const_ptpmgmt_json m)
Test(Json2msgTest, MethodPTPProfileSpecific)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"PTPProfileSpecific\":5,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(eq(u8, m->PTPProfileSpecific(m), 5));
    cr_expect(m->havePTPProfileSpecific(m));
    m->free(m);
}

// Tests havePTPProfileSpecific method
// bool havePTPProfileSpecific(const_ptpmgmt_json m)
Test(Json2msgTest, MethodHavePTPProfileSpecific)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(not(m->havePTPProfileSpecific(m)));
    m->free(m);
}

// Tests domainNumber method
// uint8_t domainNumber(const_ptpmgmt_json m)
Test(Json2msgTest, MethodDomainNumber)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":9,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(eq(int, m->domainNumber(m), 9));
    cr_expect(m->haveDomainNumber(m));
    m->free(m);
}

// Tests haveDomainNumber method
// bool haveDomainNumber(const_ptpmgmt_json m)
Test(Json2msgTest, MethodHaveDomainNumber)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(not(m->haveDomainNumber(m)));
    m->free(m);
}

// Tests versionPTP method
// uint8_t versionPTP() const
Test(Json2msgTest, MethodVersionPTP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(eq(u8, m->versionPTP(m), 2));
    cr_expect(m->haveVersionPTP(m));
    m->free(m);
}

// Tests haveVersionPTP method
// bool haveVersionPTP(const_ptpmgmt_json m)
Test(Json2msgTest, MethodHaveVersionPTP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(not(m->haveVersionPTP(m)));
    m->free(m);
}

// Tests minorVersionPTP method
// uint8_t minorVersionPTP() const
Test(Json2msgTest, MethodMinorVersionPTP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(eq(u8, m->minorVersionPTP(m), 0));
    cr_expect(m->haveMinorVersionPTP(m));
    m->free(m);
}

// Tests haveMinorVersionPTP method
// bool haveMinorVersionPTP(const_ptpmgmt_json m)
Test(Json2msgTest, MethodHaveMinorVersionPTP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(not(m->haveMinorVersionPTP(m)));
    m->free(m);
}

// Tests sequenceId method
// uint16_t sequenceId(const_ptpmgmt_json m)
Test(Json2msgTest, MethodSequenceId)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sequenceId\":15,"
            "\"sdoId\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(eq(u16, m->sequenceId(m), 15));
    cr_expect(m->haveSequenceId(m));
    m->free(m);
}

// Tests haveSequenceId method
// bool haveSequenceId(const_ptpmgmt_json m)
Test(Json2msgTest, MethodHaveSequenceId)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sdoId\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(not(m->haveSequenceId(m)));
    m->free(m);
}

// Tests sdoId method
// uint32_t sdoId(const_ptpmgmt_json m)
Test(Json2msgTest, MethodSdoId)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"sdoId\":1054,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(eq(int, m->sdoId(m), 1054));
    cr_expect(m->haveSdoId(m));
    m->free(m);
}

// Tests haveSdoId method
// bool haveSdoId(const_ptpmgmt_json m)
Test(Json2msgTest, MethodHaveSdoId)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(not(m->haveSdoId(m)));
    m->free(m);
}

// Tests srcPort method
// const struct ptpmgmt_PortIdentity_t *srcPort(ptpmgmt_json m)
Test(Json2msgTest, MethodSrcPort)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":7"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_assert(m->haveSrcPort(m));
    const struct ptpmgmt_PortIdentity_t *port = m->srcPort(m);
    cr_expect(zero(memcmp(port->clockIdentity.v, clockId, 8)));
    cr_expect(eq(u16, port->portNumber, 7));
    m->free(m);
}

// Tests haveSrcPort method
// bool haveSrcPort(const_ptpmgmt_json m)
Test(Json2msgTest, MethodHaveSrcPort)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"messageType\":\"Management\","
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(not(m->haveSrcPort(m)));
    m->free(m);
}

// Tests dstPort method
// const struct ptpmgmt_PortIdentity_t *dstPort(ptpmgmt_json m)
Test(Json2msgTest, MethodDstPort)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"messageType\":\"Management\","
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_assert(m->haveDstPort(m));
    const struct ptpmgmt_PortIdentity_t *port = m->dstPort(m);
    cr_expect(zero(memcmp(port->clockIdentity.v, allIds, 8)));
    cr_expect(eq(u16, port->portNumber, 0xffff));
    m->free(m);
}

// Tests haveDstPort method
// bool ptpmgmt_json_haveDstPort(const_ptpmgmt_json m)
Test(Json2msgTest, MethodHaveDstPort)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"messageType\":\"Management\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"actionField\":\"GET\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    cr_expect(not(m->haveDstPort(m)));
    m->free(m);
}

// Tests minimum to pass
Test(Json2msgTest, Minimum)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"GET\","
            "\"managementId\":\"NULL_PTP_MANAGEMENT\"}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_GET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_NULL_PTP_MANAGEMENT));
    m->free(m);
}

// Tests minimum set to pass
Test(Json2msgTest, MinimumSet)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PRIORITY1\",\"dataField\":{"
            " \"priority1\":1"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PRIORITY1));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PRIORITY1_t *t = (struct ptpmgmt_PRIORITY1_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->priority1, 1));
    m->free(m);
}

// Tests setting Message parameters
Test(Json2msgTest, MessageParameters)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{"
            "\"actionField\":\"GET\","
            "\"managementId\":\"NULL_PTP_MANAGEMENT\","
            "\"domainNumber\":9,"
            "\"unicastFlag\":true,"
            "\"sdoId\":704,"
            "\"sourcePortIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":7"
            "},"
            "\"targetPortIdentity\":"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "}"
            "}"));
    cr_expect(eq(int, m->domainNumber(m), 9));
    cr_expect(m->haveDomainNumber(m));
    cr_expect(m->isUnicast(m));
    cr_expect(m->haveIsUnicast(m));
    struct ptpmgmt_PortIdentity_t p_s;
    memcpy(p_s.clockIdentity.v, clockId, 8);
    p_s.portNumber = 7;
    struct ptpmgmt_PortIdentity_t p_d;
    memcpy(p_d.clockIdentity.v, allIds, 8);
    p_d.portNumber = 0xffff;
    cr_assert(m->haveSrcPort(m));
    cr_expect(zero(memcmp(&p_s, m->srcPort(m),
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_assert(m->haveDstPort(m));
    cr_expect(zero(memcmp(&p_d, m->dstPort(m),
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(eq(int, m->sdoId(m), 704));
    cr_expect(m->haveSdoId(m));
    ptpmgmt_pMsgParams p = ptpmgmt_MsgParams_alloc();
    p->transportSpecific = m->sdoId(m) >> 4; // Only major is supported
    p->domainNumber = m->domainNumber(m);
    p->isUnicast = m->isUnicast(m);
    memcpy(&(p->target), &p_d, sizeof(struct ptpmgmt_PortIdentity_t));
    memcpy(&(p->self_id), &p_s, sizeof(struct ptpmgmt_PortIdentity_t));
    cr_expect(eq(int, p->transportSpecific, 44));
    cr_expect(eq(int, p->domainNumber, 9));
    cr_expect(p->isUnicast);
    cr_expect(zero(memcmp(&p_d, &(p->target),
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(zero(memcmp(&p_s, &(p->self_id),
                sizeof(struct ptpmgmt_PortIdentity_t))));
    p->free(p);
    m->free(m);
}

// Tests CLOCK_DESCRIPTION management ID
Test(Json2msgTest, CLOCK_DESCRIPTION)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"CLOCK_DESCRIPTION\",\"dataField\":{"
            "\"clockType\":32768,"
            "\"physicalLayerProtocol\":\"IEEE 802.3\","
            "\"physicalAddress\":\"c4:7d:46:20:ac:ae\","
            "\"protocolAddress\":"
            "{"
            "  \"networkProtocol\":\"IEEE_802_3\","
            "  \"addressField\":\"c4:7d:46:20:ac:ae\""
            "},"
            "\"manufacturerIdentity\":\"01:02:03\","
            "\"productDescription\":\";;\","
            "\"revisionData\":\";;\","
            "\"userDescription\":\"test123\","
            "\"profileIdentity\":\"01:02:03:04:05:06\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_CLOCK_DESCRIPTION));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_CLOCK_DESCRIPTION_t *t = (struct
            ptpmgmt_CLOCK_DESCRIPTION_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->clockType, ptpmgmt_ordinaryClock));
    cr_assert(eq(str, (char *)t->physicalLayerProtocol.textField, "IEEE 802.3"));
    cr_expect(eq(int, t->physicalAddressLength, 6));
    cr_expect(zero(memcmp(t->physicalAddress, address_v, 6)));
    cr_expect(eq(int, t->protocolAddress.networkProtocol, ptpmgmt_IEEE_802_3));
    cr_expect(eq(u16, t->protocolAddress.addressLength, 6));
    cr_expect(zero(memcmp(t->protocolAddress.addressField, address_v, 6)));
    cr_expect(eq(int, t->manufacturerIdentity[0], 1));
    cr_expect(eq(int, t->manufacturerIdentity[1], 2));
    cr_expect(eq(int, t->manufacturerIdentity[2], 3));
    cr_assert(eq(str, (char *)t->productDescription.textField, ";;"));
    cr_assert(eq(str, (char *)t->revisionData.textField, ";;"));
    cr_assert(eq(str, (char *)t->userDescription.textField, "test123"));
    cr_expect(zero(memcmp(t->profileIdentity, "\x1\x2\x3\x4\x5\x6", 6)));
    m->free(m);
}

// Tests USER_DESCRIPTION management ID
// bool setAction(const_ptpmgmt_json m, ptpmgmt_msg m)
Test(Json2msgTest, USER_DESCRIPTION)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"USER_DESCRIPTION\",\"dataField\":{"
            "\"userDescription\":\"test123\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_USER_DESCRIPTION));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_USER_DESCRIPTION_t *t =
        (struct ptpmgmt_USER_DESCRIPTION_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_assert(eq(str, (char *)t->userDescription.textField, "test123"));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests INITIALIZE management ID
Test(Json2msgTest, INITIALIZE)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"COMMAND\","
            "\"managementId\":\"INITIALIZE\",\"dataField\":{"
            "\"initializationKey\":4660"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_COMMAND));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_INITIALIZE));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_INITIALIZE_t *t = (struct ptpmgmt_INITIALIZE_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->initializationKey, 0x1234));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests FAULT_LOG management ID
Test(Json2msgTest, FAULT_LOG)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"FAULT_LOG\",\"dataField\":{"
            "\"numberOfFaultRecords\":2,"
            "\"faultRecords\":"
            "["
            "  {"
            "    \"faultRecordLength\":50,"
            "    \"faultTime\":9.000709000,"
            "    \"severityCode\":\"Critical\","
            "    \"faultName\":\"error 1\","
            "    \"faultValue\":\"test123\","
            "    \"faultDescription\":\"This is first record\""
            "  },"
            "  {"
            "    \"faultRecordLength\":55,"
            "    \"faultTime\":1791.003019000,"
            "    \"severityCode\":\"Warning\","
            "    \"faultName\":\"error 2\","
            "    \"faultValue\":\"test321\","
            "    \"faultDescription\":\"This is the second record\""
            "  }"
            "]"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_FAULT_LOG));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_FAULT_LOG_t *t = (struct ptpmgmt_FAULT_LOG_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->numberOfFaultRecords, 2));
    cr_expect(eq(int, t->faultRecords[0].faultRecordLength, 50));
    cr_expect(eq(u64, t->faultRecords[0].faultTime.secondsField, 9));
    cr_expect(eq(u32, t->faultRecords[0].faultTime.nanosecondsField, 709000));
    cr_expect(eq(int, t->faultRecords[0].severityCode, ptpmgmt_F_Critical));
    cr_expect(eq(str, (char *)t->faultRecords[0].faultName.textField, "error 1"));
    cr_expect(eq(str, (char *)t->faultRecords[0].faultValue.textField, "test123"));
    cr_expect(eq(str, (char *)t->faultRecords[0].faultDescription.textField,
            "This is first record"));
    cr_expect(eq(int, t->faultRecords[1].faultRecordLength, 55));
    cr_expect(eq(u64, t->faultRecords[1].faultTime.secondsField, 1791));
    cr_expect(ge(u32, t->faultRecords[1].faultTime.nanosecondsField, 3019000 - 1));
    cr_expect(le(u32, t->faultRecords[1].faultTime.nanosecondsField, 3019000 + 1));
    cr_expect(eq(int, t->faultRecords[1].severityCode, ptpmgmt_F_Warning));
    cr_expect(eq(str, (char *)t->faultRecords[1].faultName.textField, "error 2"));
    cr_expect(eq(str, (char *)t->faultRecords[1].faultValue.textField, "test321"));
    cr_expect(eq(str, (char *)t->faultRecords[1].faultDescription.textField,
            "This is the second record"));
    m->free(m);
}

// Tests DEFAULT_DATA_SET management ID
Test(Json2msgTest, DEFAULT_DATA_SET)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"DEFAULT_DATA_SET\",\"dataField\":{"
            "\"twoStepFlag\":true,"
            "\"slaveOnly\":true,"
            "\"numberPorts\":1,"
            "\"priority1\":153,"
            "\"clockQuality\":"
            "{"
            "  \"clockClass\":255,"
            "  \"clockAccuracy\":\"Unknown\","
            "  \"offsetScaledLogVariance\":65535"
            "},"
            "\"priority2\":137,"
            "\"clockIdentity\":\"c47d46.fffe.20acae\","
            "\"domainNumber\":0"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_DEFAULT_DATA_SET));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_DEFAULT_DATA_SET_t *t =
        (struct ptpmgmt_DEFAULT_DATA_SET_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->flags, 0x3));
    cr_expect(eq(int, t->numberPorts, 1));
    cr_expect(eq(int, t->priority1, 153));
    cr_expect(eq(int, t->clockQuality.clockClass, 255));
    cr_expect(eq(int, t->clockQuality.clockAccuracy, ptpmgmt_Accurate_Unknown));
    cr_expect(eq(u16, t->clockQuality.offsetScaledLogVariance, 0xffff));
    cr_expect(eq(int, t->priority2, 137));
    cr_expect(zero(memcmp(t->clockIdentity.v, clockId, 8)));
    cr_expect(eq(int, t->domainNumber, 0));
    m->free(m);
}

// Tests CURRENT_DATA_SET management ID
Test(Json2msgTest, CURRENT_DATA_SET)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"CURRENT_DATA_SET\",\"dataField\":{"
            "\"stepsRemoved\":4660,"
            "\"offsetFromMaster\":3607462104733586053,"
            "\"meanPathDelay\":650212710990086400"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_CURRENT_DATA_SET));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_CURRENT_DATA_SET_t *t =
        (struct ptpmgmt_CURRENT_DATA_SET_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(u16, t->stepsRemoved, 0x1234));
    cr_expect(eq(i64, t->offsetFromMaster.scaledNanoseconds, 0x321047abcd541285LL));
    cr_expect(eq(i64, t->meanPathDelay.scaledNanoseconds, 0x0906050403020100LL));
    m->free(m);
}

// Tests PARENT_DATA_SET management ID
Test(Json2msgTest, PARENT_DATA_SET)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PARENT_DATA_SET\",\"dataField\":{"
            "\"parentPortIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":5"
            "},"
            "\"parentStats\":true,"
            "\"observedParentOffsetScaledLogVariance\":65535,"
            "\"observedParentClockPhaseChangeRate\":2147483647,"
            "\"grandmasterPriority1\":255,"
            "\"grandmasterClockQuality\":"
            "{"
            "  \"clockClass\":255,"
            "  \"clockAccuracy\":\"Unknown\","
            "  \"offsetScaledLogVariance\":65535"
            "},"
            "\"grandmasterPriority2\":255,"
            "\"grandmasterIdentity\":\"c47d46.fffe.20acae\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PARENT_DATA_SET));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PARENT_DATA_SET_t *t =
        (struct ptpmgmt_PARENT_DATA_SET_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    struct ptpmgmt_PortIdentity_t portIdentity;
    memcpy(portIdentity.clockIdentity.v, clockId, 8);
    portIdentity.portNumber = 5;
    cr_expect(zero(memcmp(&(t->parentPortIdentity), &portIdentity,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(eq(int, t->flags, 1));
    cr_expect(eq(int, t->observedParentOffsetScaledLogVariance, 0xffff));
    cr_expect(eq(int, t->observedParentClockPhaseChangeRate, 0x7fffffff));
    cr_expect(eq(int, t->grandmasterPriority1, 255));
    cr_expect(eq(int, t->grandmasterClockQuality.clockClass, 255));
    cr_expect(eq(int, t->grandmasterClockQuality.clockAccuracy,
            ptpmgmt_Accurate_Unknown));
    cr_expect(eq(u16, t->grandmasterClockQuality.offsetScaledLogVariance, 0xffff));
    cr_expect(eq(int, t->grandmasterPriority2, 255));
    cr_expect(zero(memcmp(t->grandmasterIdentity.v, clockId, 8)));
    m->free(m);
}

// Tests TIME_PROPERTIES_DATA_SET management ID
Test(Json2msgTest, TIME_PROPERTIES_DATA_SET)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"TIME_PROPERTIES_DATA_SET\",\"dataField\":{"
            "\"currentUtcOffset\":37,"
            "\"leap61\":false,"
            "\"leap59\":false,"
            "\"currentUtcOffsetValid\":false,"
            "\"ptpTimescale\":true,"
            "\"timeTraceable\":false,"
            "\"frequencyTraceable\":false,"
            "\"timeSource\":\"INTERNAL_OSCILLATOR\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_TIME_PROPERTIES_DATA_SET));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_TIME_PROPERTIES_DATA_SET_t *t =
        (struct ptpmgmt_TIME_PROPERTIES_DATA_SET_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->currentUtcOffset, 37));
    cr_expect(eq(int, t->flags, PTPMGMT_F_PTP)); // ptpTimescale bit
    cr_expect(eq(int, t->timeSource, PTPMGMT_INTERNAL_OSCILLATOR));
    m->free(m);
}

// Tests PORT_DATA_SET management ID
Test(Json2msgTest, PORT_DATA_SET)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_DATA_SET\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"portState\":\"LISTENING\","
            "\"logMinDelayReqInterval\":0,"
            "\"peerMeanPathDelay\":0,"
            "\"logAnnounceInterval\":1,"
            "\"announceReceiptTimeout\":3,"
            "\"logSyncInterval\":0,"
            "\"delayMechanism\":\"P2P\","
            "\"logMinPdelayReqInterval\":0,"
            "\"versionNumber\":2"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PORT_DATA_SET));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PORT_DATA_SET_t *t = (struct ptpmgmt_PORT_DATA_SET_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    struct ptpmgmt_PortIdentity_t portIdentity;
    memcpy(portIdentity.clockIdentity.v, clockId, 8);
    portIdentity.portNumber = 1;
    cr_expect(zero(memcmp(&(t->portIdentity), &portIdentity,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(eq(int, t->portState, PTPMGMT_LISTENING));
    cr_expect(eq(int, t->logMinDelayReqInterval, 0));
    cr_expect(eq(i64, t->peerMeanPathDelay.scaledNanoseconds, 0));
    cr_expect(eq(int, t->logAnnounceInterval, 1));
    cr_expect(eq(int, t->announceReceiptTimeout, 3));
    cr_expect(eq(int, t->logSyncInterval, 0));
    cr_expect(eq(int, t->delayMechanism, PTPMGMT_P2P));
    cr_expect(eq(int, t->logMinPdelayReqInterval, 0));
    cr_expect(eq(int, t->versionNumber, 2));
    m->free(m);
};

// Tests PRIORITY1 management ID
Test(Json2msgTest, PRIORITY1)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PRIORITY1\",\"dataField\":{"
            "\"priority1\":153"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PRIORITY1));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PRIORITY1_t *t = (struct ptpmgmt_PRIORITY1_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->priority1, 153));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests PRIORITY2 management ID
Test(Json2msgTest, PRIORITY2)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PRIORITY2\",\"dataField\":{"
            "\"priority2\":137"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PRIORITY2));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PRIORITY2_t *t = (struct ptpmgmt_PRIORITY2_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    cr_expect(eq(int, t->priority2, 137));
    m->free(m);
}

// Tests DOMAIN management ID
Test(Json2msgTest, DOMAIN)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"DOMAIN\",\"dataField\":{"
            "\"domainNumber\":7"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_DOMAIN));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_DOMAIN_t *t = (struct ptpmgmt_DOMAIN_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->domainNumber, 7));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests SLAVE_ONLY management ID
Test(Json2msgTest, SLAVE_ONLY)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"SLAVE_ONLY\",\"dataField\":{"
            "\"slaveOnly\":true"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_SLAVE_ONLY));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_SLAVE_ONLY_t *t = (struct ptpmgmt_SLAVE_ONLY_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->flags, 1));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests LOG_ANNOUNCE_INTERVAL management ID
Test(Json2msgTest, LOG_ANNOUNCE_INTERVAL)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"LOG_ANNOUNCE_INTERVAL\",\"dataField\":{"
            "\"logAnnounceInterval\":1"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_LOG_ANNOUNCE_INTERVAL));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_LOG_ANNOUNCE_INTERVAL_t *t =
        (struct ptpmgmt_LOG_ANNOUNCE_INTERVAL_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->logAnnounceInterval, 1));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests ANNOUNCE_RECEIPT_TIMEOUT management ID
Test(Json2msgTest, ANNOUNCE_RECEIPT_TIMEOUT)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"ANNOUNCE_RECEIPT_TIMEOUT\",\"dataField\":{"
            "\"announceReceiptTimeout\":3"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_ANNOUNCE_RECEIPT_TIMEOUT));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_ANNOUNCE_RECEIPT_TIMEOUT_t *t =
        (struct ptpmgmt_ANNOUNCE_RECEIPT_TIMEOUT_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->announceReceiptTimeout, 3));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests LOG_SYNC_INTERVAL management ID
Test(Json2msgTest, LOG_SYNC_INTERVAL)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"LOG_SYNC_INTERVAL\",\"dataField\":{"
            "\"logSyncInterval\":7"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_LOG_SYNC_INTERVAL));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_LOG_SYNC_INTERVAL_t *t =
        (struct ptpmgmt_LOG_SYNC_INTERVAL_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->logSyncInterval, 7));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests VERSION_NUMBER management ID
Test(Json2msgTest, VERSION_NUMBER)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"VERSION_NUMBER\",\"dataField\":{"
            "\"versionNumber\":2"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_VERSION_NUMBER));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_VERSION_NUMBER_t *t = (struct ptpmgmt_VERSION_NUMBER_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->versionNumber, 2));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests TIME management ID
Test(Json2msgTest, TIME)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"TIME\",\"dataField\":{"
            "\"currentTime\":13.150000000"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_TIME));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_TIME_t *t = (struct ptpmgmt_TIME_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(u64, t->currentTime.secondsField, 13));
    cr_expect(eq(u32, t->currentTime.nanosecondsField, 150000000));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests CLOCK_ACCURACY management ID
Test(Json2msgTest, CLOCK_ACCURACY)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"CLOCK_ACCURACY\",\"dataField\":{"
            "\"clockAccuracy\":\"Unknown\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_CLOCK_ACCURACY));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_CLOCK_ACCURACY_t *t = (struct ptpmgmt_CLOCK_ACCURACY_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->clockAccuracy, ptpmgmt_Accurate_Unknown));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests UTC_PROPERTIES management ID
Test(Json2msgTest, UTC_PROPERTIES)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"UTC_PROPERTIES\",\"dataField\":{"
            "\"currentUtcOffset\":-21555,"
            "\"leap61\":true,"
            "\"leap59\":true,"
            "\"currentUtcOffsetValid\":true"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_UTC_PROPERTIES));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_UTC_PROPERTIES_t *t = (struct ptpmgmt_UTC_PROPERTIES_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->currentUtcOffset, -0x5433));
    cr_expect(eq(int, t->flags, 7));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests TRACEABILITY_PROPERTIES management ID
Test(Json2msgTest, TRACEABILITY_PROPERTIES)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"TRACEABILITY_PROPERTIES\",\"dataField\":{"
            "\"timeTraceable\":true,"
            "\"frequencyTraceable\":true"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_TRACEABILITY_PROPERTIES));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_TRACEABILITY_PROPERTIES_t *t =
        (struct ptpmgmt_TRACEABILITY_PROPERTIES_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->flags, PTPMGMT_F_TTRA | PTPMGMT_F_FTRA));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests TIMESCALE_PROPERTIES management ID
Test(Json2msgTest, TIMESCALE_PROPERTIES)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"TIMESCALE_PROPERTIES\",\"dataField\":{"
            "\"ptpTimescale\":true,"
            "\"timeSource\":\"HAND_SET\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_TIMESCALE_PROPERTIES));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_TIMESCALE_PROPERTIES_t *t =
        (const struct ptpmgmt_TIMESCALE_PROPERTIES_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->flags, PTPMGMT_F_PTP));
    cr_expect(eq(int, t->timeSource, PTPMGMT_HAND_SET));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests UNICAST_NEGOTIATION_ENABLE management ID
Test(Json2msgTest, UNICAST_NEGOTIATION_ENABLE)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"UNICAST_NEGOTIATION_ENABLE\",\"dataField\":{"
            "\"unicastNegotiationPortDS\":true"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_UNICAST_NEGOTIATION_ENABLE));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_UNICAST_NEGOTIATION_ENABLE_t *t =
        (struct ptpmgmt_UNICAST_NEGOTIATION_ENABLE_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->flags, 1));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests PATH_TRACE_LIST management ID
Test(Json2msgTest, PATH_TRACE_LIST)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PATH_TRACE_LIST\",\"dataField\":{"
            "\"pathSequence\":"
            "["
            "  \"c47d46.fffe.20acae\","
            "  \"0c0413.610b.4a0c4a\""
            "]"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PATH_TRACE_LIST));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PATH_TRACE_LIST_t *t =
        (struct ptpmgmt_PATH_TRACE_LIST_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(zero(memcmp(t->pathSequence[0].v, clockId, 8)));
    cr_expect(zero(memcmp(t->pathSequence[1].v,
                "\xc\x4\x13\x61\xb\x4a\xc\x4a", 8)));
    m->free(m);
}

// Tests PATH_TRACE_ENABLE management ID
Test(Json2msgTest, PATH_TRACE_ENABLE)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PATH_TRACE_ENABLE\",\"dataField\":{"
            "\"pathTraceDS\":true"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PATH_TRACE_ENABLE));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PATH_TRACE_ENABLE_t *t =
        (struct ptpmgmt_PATH_TRACE_ENABLE_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->flags, 1));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests GRANDMASTER_CLUSTER_TABLE management ID
Test(Json2msgTest, GRANDMASTER_CLUSTER_TABLE)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"GRANDMASTER_CLUSTER_TABLE\",\"dataField\":{"
            "\"logQueryInterval\":-19,"
            "\"actualTableSize\":2,"
            "\"PortAddress\":"
            "["
            "  {"
            "    \"networkProtocol\":\"IEEE_802_3\","
            "    \"addressField\":\"c4:7d:46:20:ac:ae\""
            "  },"
            "  {"
            "    \"networkProtocol\":\"UDP_IPv4\","
            "    \"addressField\":\"12:34:56:78\""
            "  }"
            "]"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_GRANDMASTER_CLUSTER_TABLE));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_GRANDMASTER_CLUSTER_TABLE_t *t =
        (struct ptpmgmt_GRANDMASTER_CLUSTER_TABLE_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->logQueryInterval, -19));
    cr_expect(eq(int, t->actualTableSize, 2));
    cr_expect(eq(int, t->PortAddress[0].networkProtocol, ptpmgmt_IEEE_802_3));
    cr_expect(eq(u16, t->PortAddress[0].addressLength, 6));
    cr_expect(zero(memcmp(t->PortAddress[0].addressField, address_v, 6)));
    cr_expect(eq(int, t->PortAddress[1].networkProtocol, ptpmgmt_UDP_IPv4));
    cr_expect(eq(u16, t->PortAddress[1].addressLength, 4));
    cr_expect(zero(memcmp(t->PortAddress[1].addressField, ip_v, 4)));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests UNICAST_MASTER_TABLE management ID
Test(Json2msgTest, UNICAST_MASTER_TABLE)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"UNICAST_MASTER_TABLE\",\"dataField\":{"
            "\"logQueryInterval\":-19,"
            "\"actualTableSize\":2,"
            "\"PortAddress\":"
            "["
            "  {"
            "    \"networkProtocol\":\"IEEE_802_3\","
            "    \"addressField\":\"c4:7d:46:20:ac:ae\""
            "  },"
            "  {"
            "    \"networkProtocol\":\"UDP_IPv4\","
            "    \"addressField\":\"12:34:56:78\""
            "  }"
            "]"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_UNICAST_MASTER_TABLE));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_UNICAST_MASTER_TABLE_t *t =
        (const struct ptpmgmt_UNICAST_MASTER_TABLE_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->logQueryInterval, -19));
    cr_expect(eq(int, t->actualTableSize, 2));
    cr_expect(eq(int, t->PortAddress[0].networkProtocol, ptpmgmt_IEEE_802_3));
    cr_expect(eq(u16, t->PortAddress[0].addressLength, 6));
    cr_expect(zero(memcmp(t->PortAddress[0].addressField, address_v, 6)));
    cr_expect(eq(int, t->PortAddress[1].networkProtocol, ptpmgmt_UDP_IPv4));
    cr_expect(eq(u16, t->PortAddress[1].addressLength, 4));
    cr_expect(zero(memcmp(t->PortAddress[1].addressField, ip_v, 4)));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests UNICAST_MASTER_MAX_TABLE_SIZE management ID
Test(Json2msgTest, UNICAST_MASTER_MAX_TABLE_SIZE)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"UNICAST_MASTER_MAX_TABLE_SIZE\",\"dataField\":{"
            "\"maxTableSize\":8515"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_UNICAST_MASTER_MAX_TABLE_SIZE));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_UNICAST_MASTER_MAX_TABLE_SIZE_t *t =
        (struct ptpmgmt_UNICAST_MASTER_MAX_TABLE_SIZE_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->maxTableSize, 0x2143));
    m->free(m);
}

// Tests ACCEPTABLE_MASTER_TABLE management ID
Test(Json2msgTest, ACCEPTABLE_MASTER_TABLE)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"ACCEPTABLE_MASTER_TABLE\",\"dataField\":{"
            "\"actualTableSize\":2,"
            "\"list\":"
            "["
            "  {"
            "    \"acceptablePortIdentity\":"
            "    {"
            "      \"clockIdentity\":\"c47d46.fffe.20acae\","
            "      \"portNumber\":1"
            "    },"
            "    \"alternatePriority1\":127"
            "  },"
            "  {"
            "    \"acceptablePortIdentity\":"
            "    {"
            "      \"clockIdentity\":\"090807.0605.040107\","
            "      \"portNumber\":2"
            "    },"
            "    \"alternatePriority1\":111"
            "  }"
            "]"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_ACCEPTABLE_MASTER_TABLE));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_t *t =
        (struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->actualTableSize, 2));
    struct ptpmgmt_PortIdentity_t a0;
    memcpy(a0.clockIdentity.v, clockId, 8);
    a0.portNumber = 1;
    cr_expect(zero(memcmp(&(t->list[0].acceptablePortIdentity), &a0,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(eq(int, t->list[0].alternatePriority1, 127));
    struct ptpmgmt_PortIdentity_t a1;
    memcpy(a1.clockIdentity.v, "\x9\x8\x7\x6\x5\x4\x1\x7", 8);
    a1.portNumber = 2;
    cr_expect(zero(memcmp(&(t->list[1].acceptablePortIdentity), &a1,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(eq(int, t->list[1].alternatePriority1, 111));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests ACCEPTABLE_MASTER_TABLE_ENABLED management ID
Test(Json2msgTest, ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"ACCEPTABLE_MASTER_TABLE_ENABLED\",\"dataField\":{"
            "\"acceptableMasterPortDS\":true"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_ACCEPTABLE_MASTER_TABLE_ENABLED));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_ENABLED_t *t =
        (struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_ENABLED_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->flags, 1));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests ACCEPTABLE_MASTER_MAX_TABLE_SIZE management ID
Test(Json2msgTest, ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"ACCEPTABLE_MASTER_MAX_TABLE_SIZE\",\"dataField\":{"
            "\"maxTableSize\":26554"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m),
            PTPMGMT_ACCEPTABLE_MASTER_MAX_TABLE_SIZE));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t *t =
        (struct ptpmgmt_ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->maxTableSize, 0x67ba));
    m->free(m);
}

// Tests ALTERNATE_MASTER management ID
Test(Json2msgTest, ALTERNATE_MASTER)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"ALTERNATE_MASTER\",\"dataField\":{"
            "\"transmitAlternateMulticastSync\":true,"
            "\"logAlternateMulticastSyncInterval\":-17,"
            "\"numberOfAlternateMasters\":210"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_ALTERNATE_MASTER));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_ALTERNATE_MASTER_t *t =
        (struct ptpmgmt_ALTERNATE_MASTER_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->flags, 1));
    cr_expect(eq(int, t->logAlternateMulticastSyncInterval, -17));
    cr_expect(eq(int, t->numberOfAlternateMasters, 210));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests ALTERNATE_TIME_OFFSET_ENABLE management ID
Test(Json2msgTest, ALTERNATE_TIME_OFFSET_ENABLE)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"ALTERNATE_TIME_OFFSET_ENABLE\",\"dataField\":{"
            "\"keyField\":7,"
            "\"alternateTimescaleOffsetsDS\":true"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_ALTERNATE_TIME_OFFSET_ENABLE));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_ALTERNATE_TIME_OFFSET_ENABLE_t *t =
        (struct ptpmgmt_ALTERNATE_TIME_OFFSET_ENABLE_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->keyField, 7));
    cr_expect(eq(int, t->flags, 1));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests ALTERNATE_TIME_OFFSET_NAME management ID
Test(Json2msgTest, ALTERNATE_TIME_OFFSET_NAME)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"ALTERNATE_TIME_OFFSET_NAME\",\"dataField\":{"
            "\"keyField\":11,"
            "\"displayName\":\"123\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_ALTERNATE_TIME_OFFSET_NAME));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t *t =
        (struct ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->keyField, 11));
    cr_assert(eq(str, (char *)t->displayName.textField, "123"));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests ALTERNATE_TIME_OFFSET_MAX_KEY management ID
Test(Json2msgTest, ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"ALTERNATE_TIME_OFFSET_MAX_KEY\",\"dataField\":{"
            "\"maxKey\":9"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_ALTERNATE_TIME_OFFSET_MAX_KEY));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_ALTERNATE_TIME_OFFSET_MAX_KEY_t *t =
        (struct ptpmgmt_ALTERNATE_TIME_OFFSET_MAX_KEY_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->maxKey, 9));
    m->free(m);
}

// Tests ALTERNATE_TIME_OFFSET_PROPERTIES management ID
Test(Json2msgTest, ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"ALTERNATE_TIME_OFFSET_PROPERTIES\",\"dataField\":{"
            "\"keyField\":13,"
            "\"currentOffset\":-2145493247,"
            "\"jumpSeconds\":-2147413249,"
            "\"timeOfNextJump\":159585821399185"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m),
            PTPMGMT_ALTERNATE_TIME_OFFSET_PROPERTIES));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_ALTERNATE_TIME_OFFSET_PROPERTIES_t *t =
        (struct ptpmgmt_ALTERNATE_TIME_OFFSET_PROPERTIES_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(u8, t->keyField, 13));
    cr_expect(eq(u32, t->currentOffset, -2145493247));
    cr_expect(eq(u32, t->jumpSeconds, -2147413249));
    cr_expect(eq(u64, t->timeOfNextJump, 0x912478321891LL));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests TRANSPARENT_CLOCK_PORT_DATA_SET management ID
Test(Json2msgTest, TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"TRANSPARENT_CLOCK_PORT_DATA_SET\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"transparentClockPortDS\":true,"
            "\"logMinPdelayReqInterval\":-21,"
            "\"peerMeanPathDelay\":-2524141968232996095"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_TRANSPARENT_CLOCK_PORT_DATA_SET));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_TRANSPARENT_CLOCK_PORT_DATA_SET_t *t =
        (struct ptpmgmt_TRANSPARENT_CLOCK_PORT_DATA_SET_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    struct ptpmgmt_PortIdentity_t portIdentity;
    memcpy(portIdentity.clockIdentity.v, clockId, 8);
    portIdentity.portNumber = 1;
    cr_expect(zero(memcmp(&(t->portIdentity), &portIdentity,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(eq(int, t->flags, 1));
    cr_expect(eq(int, t->logMinPdelayReqInterval, -21));
    cr_expect(eq(i64, t->peerMeanPathDelay.scaledNanoseconds,
            0xdcf87240dcd12301LL));
    m->free(m);
}

// Tests LOG_MIN_PDELAY_REQ_INTERVAL management ID
Test(Json2msgTest, LOG_MIN_PDELAY_REQ_INTERVAL)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"LOG_MIN_PDELAY_REQ_INTERVAL\",\"dataField\":{"
            "\"logMinPdelayReqInterval\":9"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_LOG_MIN_PDELAY_REQ_INTERVAL));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_LOG_MIN_PDELAY_REQ_INTERVAL_t *t =
        (struct ptpmgmt_LOG_MIN_PDELAY_REQ_INTERVAL_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->logMinPdelayReqInterval, 9));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests TRANSPARENT_CLOCK_DEFAULT_DATA_SET management ID
Test(Json2msgTest, TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"TRANSPARENT_CLOCK_DEFAULT_DATA_SET\","
            "\"dataField\":{"
            "\"clockIdentity\":\"c47d46.fffe.20acae\","
            "\"numberPorts\":6010,"
            "\"delayMechanism\":\"NO_MECHANISM\","
            "\"primaryDomain\":18"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m),
            PTPMGMT_TRANSPARENT_CLOCK_DEFAULT_DATA_SET));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t *t =
        (struct ptpmgmt_TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(zero(memcmp(t->clockIdentity.v, clockId, 8)));
    cr_expect(eq(u16, t->numberPorts, 0x177a));
    cr_expect(eq(int, t->delayMechanism, PTPMGMT_NO_MECHANISM));
    cr_expect(eq(u8, t->primaryDomain, 18));
    m->free(m);
}

// Tests PRIMARY_DOMAIN management ID
Test(Json2msgTest, PRIMARY_DOMAIN)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PRIMARY_DOMAIN\",\"dataField\":{"
            "\"primaryDomain\":17"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PRIMARY_DOMAIN));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PRIMARY_DOMAIN_t *t = (struct ptpmgmt_PRIMARY_DOMAIN_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->primaryDomain, 17));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests DELAY_MECHANISM management ID
Test(Json2msgTest, DELAY_MECHANISM)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"DELAY_MECHANISM\",\"dataField\":{"
            "\"delayMechanism\":\"P2P\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_DELAY_MECHANISM));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_DELAY_MECHANISM_t *t =
        (struct ptpmgmt_DELAY_MECHANISM_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->delayMechanism, PTPMGMT_P2P));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests EXTERNAL_PORT_CONFIGURATION_ENABLED management ID
Test(Json2msgTest, EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"EXTERNAL_PORT_CONFIGURATION_ENABLED\","
            "\"dataField\":{"
            "\"externalPortConfiguration\":true"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m),
            PTPMGMT_EXTERNAL_PORT_CONFIGURATION_ENABLED));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_EXTERNAL_PORT_CONFIGURATION_ENABLED_t *t =
        (struct ptpmgmt_EXTERNAL_PORT_CONFIGURATION_ENABLED_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->flags, 1));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests MASTER_ONLY management ID
Test(Json2msgTest, MASTER_ONLY)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"MASTER_ONLY\",\"dataField\":{"
            "\"masterOnly\":true"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_MASTER_ONLY));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_MASTER_ONLY_t *t = (struct ptpmgmt_MASTER_ONLY_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->flags, 1));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests HOLDOVER_UPGRADE_ENABLE management ID
Test(Json2msgTest, HOLDOVER_UPGRADE_ENABLE)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"HOLDOVER_UPGRADE_ENABLE\",\"dataField\":{"
            "\"holdoverUpgradeDS\":true"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_HOLDOVER_UPGRADE_ENABLE));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_HOLDOVER_UPGRADE_ENABLE_t *t =
        (struct ptpmgmt_HOLDOVER_UPGRADE_ENABLE_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->flags, 1));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests EXT_PORT_CONFIG_PORT_DATA_SET management ID
Test(Json2msgTest, EXT_PORT_CONFIG_PORT_DATA_SET)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"EXT_PORT_CONFIG_PORT_DATA_SET\",\"dataField\":{"
            "\"acceptableMasterPortDS\":true,"
            "\"desiredState\":\"PASSIVE\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_EXT_PORT_CONFIG_PORT_DATA_SET));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_EXT_PORT_CONFIG_PORT_DATA_SET_t *t =
        (struct ptpmgmt_EXT_PORT_CONFIG_PORT_DATA_SET_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(u8, t->flags, 1));
    cr_expect(eq(int, t->desiredState, PTPMGMT_PASSIVE));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests TIME_STATUS_NP management ID
Test(Json2msgTest, TIME_STATUS_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"TIME_STATUS_NP\",\"dataField\":{"
            "\"master_offset\":0,"
            "\"ingress_time\":0,"
            "\"cumulativeScaledRateOffset\":0,"
            "\"scaledLastGmPhaseChange\":0,"
            "\"gmTimeBaseIndicator\":0,"
            "\"nanoseconds_msb\":0,"
            "\"nanoseconds_lsb\":0,"
            "\"fractional_nanoseconds\":0,"
            "\"gmPresent\":0,"
            "\"gmIdentity\":\"c47d46.fffe.20acae\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_TIME_STATUS_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_TIME_STATUS_NP_t *t = (struct ptpmgmt_TIME_STATUS_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->master_offset, 0));
    cr_expect(eq(int, t->ingress_time, 0));
    cr_expect(eq(int, t->cumulativeScaledRateOffset, 0));
    cr_expect(eq(int, t->scaledLastGmPhaseChange, 0));
    cr_expect(eq(int, t->gmTimeBaseIndicator, 0));
    cr_expect(eq(int, t->nanoseconds_msb, 0));
    cr_expect(eq(int, t->nanoseconds_lsb, 0));
    cr_expect(eq(int, t->fractional_nanoseconds, 0));
    cr_expect(eq(int, t->gmPresent, 0));
    cr_expect(zero(memcmp(t->gmIdentity.v, clockId, 8)));
    m->free(m);
}

// Tests GRANDMASTER_SETTINGS_NP management ID
Test(Json2msgTest, GRANDMASTER_SETTINGS_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"GRANDMASTER_SETTINGS_NP\",\"dataField\":{"
            "\"clockQuality\":"
            "{"
            "  \"clockClass\":255,"
            "  \"clockAccuracy\":\"Unknown\","
            "  \"offsetScaledLogVariance\":65535"
            "},"
            "\"currentUtcOffset\":37,"
            "\"leap61\":false,"
            "\"leap59\":false,"
            "\"currentUtcOffsetValid\":false,"
            "\"ptpTimescale\":true,"
            "\"timeTraceable\":false,"
            "\"frequencyTraceable\":false,"
            "\"timeSource\":\"INTERNAL_OSCILLATOR\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_GRANDMASTER_SETTINGS_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_GRANDMASTER_SETTINGS_NP_t *t =
        (struct ptpmgmt_GRANDMASTER_SETTINGS_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->clockQuality.clockClass, 255));
    cr_expect(eq(int, t->clockQuality.clockAccuracy, ptpmgmt_Accurate_Unknown));
    cr_expect(eq(u16, t->clockQuality.offsetScaledLogVariance, 0xffff));
    cr_expect(eq(int, t->currentUtcOffset, 37));
    cr_expect(eq(int, t->flags, PTPMGMT_F_PTP));
    cr_expect(eq(int, t->timeSource, PTPMGMT_INTERNAL_OSCILLATOR));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests PORT_DATA_SET_NP management ID
Test(Json2msgTest, PORT_DATA_SET_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_DATA_SET_NP\",\"dataField\":{"
            "\"neighborPropDelayThresh\":20000000,"
            "\"asCapable\":1"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PORT_DATA_SET_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PORT_DATA_SET_NP_t *t =
        (struct ptpmgmt_PORT_DATA_SET_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->neighborPropDelayThresh, 20000000));
    cr_expect(eq(int, t->asCapable, 1));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests SUBSCRIBE_EVENTS_NP management ID
Test(Json2msgTest, SUBSCRIBE_EVENTS_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"SUBSCRIBE_EVENTS_NP\",\"dataField\":{"
            "\"duration\":4660,"
            "\"NOTIFY_PORT_STATE\":true,"
            "\"NOTIFY_TIME_SYNC\":true,"
            "\"NOTIFY_PARENT_DATA_SET\":true,"
            "\"NOTIFY_CMLDS\":true"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_SUBSCRIBE_EVENTS_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t *t =
        (struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->duration, 0x1234));
    cr_expect(ptpmgmt_getEvent_lnp(t, PTPMGMT_NOTIFY_PORT_STATE));
    cr_expect(ptpmgmt_getEvent_lnp(t, PTPMGMT_NOTIFY_TIME_SYNC));
    cr_expect(ptpmgmt_getEvent_lnp(t, PTPMGMT_NOTIFY_PARENT_DATA_SET));
    cr_expect(ptpmgmt_getEvent_lnp(t, PTPMGMT_NOTIFY_CMLDS));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests PORT_PROPERTIES_NP management ID
Test(Json2msgTest, PORT_PROPERTIES_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_PROPERTIES_NP\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"portState\":\"LISTENING\","
            "\"timestamping\":\"HARDWARE\","
            "\"interface\":\"enp0s25\""
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PORT_PROPERTIES_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PORT_PROPERTIES_NP_t *t =
        (struct ptpmgmt_PORT_PROPERTIES_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    struct ptpmgmt_PortIdentity_t portIdentity;
    memcpy(portIdentity.clockIdentity.v, clockId, 8);
    portIdentity.portNumber = 1;
    cr_expect(zero(memcmp(&(t->portIdentity), &portIdentity,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(eq(int, t->portState, PTPMGMT_LISTENING));
    cr_expect(eq(int, t->timestamping, PTPMGMT_TS_HARDWARE));
    cr_assert(eq(str, (char *)t->interface.textField, "enp0s25"));
    m->free(m);
}

// Tests PORT_STATS_NP management ID
Test(Json2msgTest, PORT_STATS_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_STATS_NP\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"rx_Sync\":0,"
            "\"rx_Delay_Req\":0,"
            "\"rx_Pdelay_Req\":0,"
            "\"rx_Pdelay_Resp\":0,"
            "\"rx_Follow_Up\":0,"
            "\"rx_Delay_Resp\":0,"
            "\"rx_Pdelay_Resp_Follow_Up\":0,"
            "\"rx_Announce\":0,"
            "\"rx_Signaling\":0,"
            "\"rx_Management\":0,"
            "\"tx_Sync\":0,"
            "\"tx_Delay_Req\":0,"
            "\"tx_Pdelay_Req\":63346,"
            "\"tx_Pdelay_Resp\":0,"
            "\"tx_Follow_Up\":0,"
            "\"tx_Delay_Resp\":0,"
            "\"tx_Pdelay_Resp_Follow_Up\":0,"
            "\"tx_Announce\":0,"
            "\"tx_Signaling\":0,"
            "\"tx_Management\":0"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PORT_STATS_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PORT_STATS_NP_t *t = (struct ptpmgmt_PORT_STATS_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    struct ptpmgmt_PortIdentity_t portIdentity;
    memcpy(portIdentity.clockIdentity.v, clockId, 8);
    portIdentity.portNumber = 1;
    cr_expect(zero(memcmp(&(t->portIdentity), &portIdentity,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(eq(int, t->rxMsgType[PTPMGMT_STAT_SYNC], 0));
    cr_expect(eq(int, t->rxMsgType[PTPMGMT_STAT_DELAY_REQ], 0));
    cr_expect(eq(int, t->rxMsgType[PTPMGMT_STAT_PDELAY_REQ], 0));
    cr_expect(eq(int, t->rxMsgType[PTPMGMT_STAT_PDELAY_RESP], 0));
    cr_expect(eq(int, t->rxMsgType[PTPMGMT_STAT_FOLLOW_UP], 0));
    cr_expect(eq(int, t->rxMsgType[PTPMGMT_STAT_DELAY_RESP], 0));
    cr_expect(eq(int, t->rxMsgType[PTPMGMT_STAT_PDELAY_RESP_FOLLOW_UP], 0));
    cr_expect(eq(int, t->rxMsgType[PTPMGMT_STAT_ANNOUNCE], 0));
    cr_expect(eq(int, t->rxMsgType[PTPMGMT_STAT_SIGNALING], 0));
    cr_expect(eq(int, t->rxMsgType[PTPMGMT_STAT_MANAGEMENT], 0));
    cr_expect(eq(int, t->txMsgType[PTPMGMT_STAT_SYNC], 0));
    cr_expect(eq(int, t->txMsgType[PTPMGMT_STAT_DELAY_REQ], 0));
    cr_expect(eq(int, t->txMsgType[PTPMGMT_STAT_PDELAY_REQ], 63346));
    cr_expect(eq(int, t->txMsgType[PTPMGMT_STAT_PDELAY_RESP], 0));
    cr_expect(eq(int, t->txMsgType[PTPMGMT_STAT_FOLLOW_UP], 0));
    cr_expect(eq(int, t->txMsgType[PTPMGMT_STAT_DELAY_RESP], 0));
    cr_expect(eq(int, t->txMsgType[PTPMGMT_STAT_PDELAY_RESP_FOLLOW_UP], 0));
    cr_expect(eq(int, t->txMsgType[PTPMGMT_STAT_ANNOUNCE], 0));
    cr_expect(eq(int, t->txMsgType[PTPMGMT_STAT_SIGNALING], 0));
    cr_expect(eq(int, t->txMsgType[PTPMGMT_STAT_MANAGEMENT], 0));
    m->free(m);
}

// Tests SYNCHRONIZATION_UNCERTAIN_NP management ID
Test(Json2msgTest, SYNCHRONIZATION_UNCERTAIN_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"SYNCHRONIZATION_UNCERTAIN_NP\",\"dataField\":{"
            "\"val\":255"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_SYNCHRONIZATION_UNCERTAIN_NP_t *t =
        (struct ptpmgmt_SYNCHRONIZATION_UNCERTAIN_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->val, PTPMGMT_SYNC_UNCERTAIN_DONTCARE));
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, msg));
    msg->free(msg);
    m->free(m);
}

// Tests PORT_SERVICE_STATS_NP management ID
Test(Json2msgTest, PORT_SERVICE_STATS_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_SERVICE_STATS_NP\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"announce_timeout\":9041,"
            "\"sync_timeout\":0,"
            "\"delay_timeout\":63346,"
            "\"unicast_service_timeout\":0,"
            "\"unicast_request_timeout\":0,"
            "\"master_announce_timeout\":0,"
            "\"master_sync_timeout\":0,"
            "\"qualification_timeout\":0,"
            "\"sync_mismatch\":0,"
            "\"followup_mismatch\":0"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PORT_SERVICE_STATS_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PORT_SERVICE_STATS_NP_t *t =
        (struct ptpmgmt_PORT_SERVICE_STATS_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    struct ptpmgmt_PortIdentity_t portIdentity;
    memcpy(portIdentity.clockIdentity.v, clockId, 8);
    portIdentity.portNumber = 1;
    cr_expect(zero(memcmp(&(t->portIdentity), &portIdentity,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(eq(int, t->announce_timeout, 9041));
    cr_expect(eq(int, t->sync_timeout, 0));
    cr_expect(eq(int, t->delay_timeout, 63346));
    cr_expect(eq(int, t->unicast_service_timeout, 0));
    cr_expect(eq(int, t->unicast_request_timeout, 0));
    cr_expect(eq(int, t->master_announce_timeout, 0));
    cr_expect(eq(int, t->master_sync_timeout, 0));
    cr_expect(eq(int, t->qualification_timeout, 0));
    cr_expect(eq(int, t->sync_mismatch, 0));
    cr_expect(eq(int, t->followup_mismatch, 0));
    m->free(m);
}

// Tests UNICAST_MASTER_TABLE_NP management ID
Test(Json2msgTest, UNICAST_MASTER_TABLE_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"UNICAST_MASTER_TABLE_NP\",\"dataField\":{"
            "\"actualTableSize\":1,"
            "\"unicastMasters\":"
            "["
            "  {"
            "    \"portIdentity\":"
            "    {"
            "      \"clockIdentity\":\"c47d46.fffe.20acae\","
            "      \"portNumber\":1"
            "    },"
            "    \"clockQuality\":"
            "    {"
            "      \"clockClass\":255,"
            "      \"clockAccuracy\":\"Unknown\","
            "      \"offsetScaledLogVariance\":65535"
            "    },"
            "    \"selected\":true,"
            "    \"portState\":\"HAVE_SYDY\","
            "    \"priority1\":126,"
            "    \"priority2\":134,"
            "    \"portAddress\":"
            "    {"
            "      \"networkProtocol\":\"IEEE_802_3\","
            "      \"addressField\":\"c4:7d:46:20:ac:ae\""
            "    }"
            "  }"
            "]"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_UNICAST_MASTER_TABLE_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_UNICAST_MASTER_TABLE_NP_t *t =
        (struct ptpmgmt_UNICAST_MASTER_TABLE_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->actualTableSize, 1));
    struct ptpmgmt_PortIdentity_t portIdentity;
    memcpy(portIdentity.clockIdentity.v, clockId, 8);
    portIdentity.portNumber = 1;
    cr_expect(zero(memcmp(&(t->unicastMasters[0].portIdentity), &portIdentity,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(eq(int, t->unicastMasters[0].clockQuality.clockClass, 255));
    cr_expect(eq(int, t->unicastMasters[0].clockQuality.clockAccuracy,
            ptpmgmt_Accurate_Unknown));
    cr_expect(eq(u16, t->unicastMasters[0].clockQuality.offsetScaledLogVariance,
            0xffff));
    cr_expect(t->unicastMasters[0].selected);
    cr_expect(eq(int, t->unicastMasters[0].portState, PTPMGMT_UC_HAVE_SYDY));
    cr_expect(eq(int, t->unicastMasters[0].priority1, 126));
    cr_expect(eq(int, t->unicastMasters[0].priority2, 134));
    cr_expect(eq(int, t->unicastMasters[0].portAddress.networkProtocol,
            ptpmgmt_IEEE_802_3));
    cr_expect(eq(u16, t->unicastMasters[0].portAddress.addressLength, 6));
    cr_expect(zero(memcmp(t->unicastMasters[0].portAddress.addressField, address_v,
                6)));
    m->free(m);
}

// Tests PORT_HWCLOCK_NP management ID
Test(Json2msgTest, PORT_HWCLOCK_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_HWCLOCK_NP\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"phc_index\":1,"
            "\"flags\":7"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_PORT_HWCLOCK_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_PORT_HWCLOCK_NP_t *t =
        (struct ptpmgmt_PORT_HWCLOCK_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    struct ptpmgmt_PortIdentity_t portIdentity;
    memcpy(portIdentity.clockIdentity.v, clockId, 8);
    portIdentity.portNumber = 1;
    cr_expect(zero(memcmp(&(t->portIdentity), &portIdentity,
                sizeof(struct ptpmgmt_PortIdentity_t))));
    cr_expect(eq(i32, t->phc_index, 1));
    cr_expect(eq(u8, t->flags, 7));
    m->free(m);
}

// Tests POWER_PROFILE_SETTINGS_NP management ID
Test(Json2msgTest, POWER_PROFILE_SETTINGS_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"POWER_PROFILE_SETTINGS_NP\",\"dataField\":{"
            "\"version\":\"2011\","
            "\"grandmasterID\":56230,"
            "\"grandmasterTimeInaccuracy\":4124796349,"
            "\"networkTimeInaccuracy\":3655058877,"
            "\"totalTimeInaccuracy\":4223530875"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_POWER_PROFILE_SETTINGS_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_POWER_PROFILE_SETTINGS_NP_t *t =
        (struct ptpmgmt_POWER_PROFILE_SETTINGS_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(int, t->version, PTPMGMT_IEEE_C37_238_VERSION_2011));
    cr_expect(eq(int, t->grandmasterID, 56230));
    cr_expect(eq(int, t->grandmasterTimeInaccuracy, 4124796349));
    cr_expect(eq(int, t->networkTimeInaccuracy, 3655058877));
    cr_expect(eq(int, t->totalTimeInaccuracy, 4223530875));
    m->free(m);
}

// Tests CMLDS_INFO_NP management ID
Test(Json2msgTest, CMLDS_INFO_NP)
{
    ptpmgmt_json m = ptpmgmt_json_alloc();
    cr_assert(m->fromJson(m, "{\"actionField\":\"SET\","
            "\"managementId\":\"CMLDS_INFO_NP\",\"dataField\":{"
            "\"meanLinkDelay\":201548321,"
            "\"scaledNeighborRateRatio\":1842,"
            "\"as_capable\":1"
            "}}"));
    cr_expect(eq(int, m->actionField(m), PTPMGMT_SET));
    cr_expect(eq(int, m->managementId(m), PTPMGMT_CMLDS_INFO_NP));
    const void *d = m->dataField(m);
    cr_assert(not(zero(ptr, (void *)d)));
    const struct ptpmgmt_CMLDS_INFO_NP_t *t =
        (struct ptpmgmt_CMLDS_INFO_NP_t *)d;
    cr_assert(not(zero(ptr, (void *)t)));
    cr_expect(eq(i64, t->meanLinkDelay.scaledNanoseconds, 201548321LL));
    cr_expect(eq(i32, t->scaledNeighborRateRatio, 1842));
    cr_expect(eq(i32, t->as_capable, 1));
    m->free(m);
}
