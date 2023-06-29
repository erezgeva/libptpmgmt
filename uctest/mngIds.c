/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Verify PTP management TLV IDs unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "msg.h" // We use message class to test managment IDs behavier
#include "mngIds.h"

Test(MngIDsTest, NULL_PTP_MANAGEMENT)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    char dumm[1];
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_NULL_PTP_MANAGEMENT, NULL));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_NULL_PTP_MANAGEMENT, dumm));
    cr_expect(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_NULL_PTP_MANAGEMENT, NULL));
    m->free(m);
}

Test(MngIDsTest, CLOCK_DESCRIPTION)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_CLOCK_DESCRIPTION, NULL));
    struct ptpmgmt_CLOCK_DESCRIPTION_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_CLOCK_DESCRIPTION_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_CLOCK_DESCRIPTION, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_CLOCK_DESCRIPTION,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, USER_DESCRIPTION)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_USER_DESCRIPTION, NULL));
    struct ptpmgmt_USER_DESCRIPTION_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_USER_DESCRIPTION_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_USER_DESCRIPTION, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_USER_DESCRIPTION,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, SAVE_IN_NON_VOLATILE_STORAGE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    char dumm[1];
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_SAVE_IN_NON_VOLATILE_STORAGE,
                NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_SAVE_IN_NON_VOLATILE_STORAGE,
                dumm)));
    cr_expect(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_SAVE_IN_NON_VOLATILE_STORAGE,
            NULL));
    m->free(m);
}

Test(MngIDsTest, RESET_NON_VOLATILE_STORAGE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    char dumm[1];
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_RESET_NON_VOLATILE_STORAGE,
                NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_RESET_NON_VOLATILE_STORAGE,
                dumm)));
    cr_expect(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_RESET_NON_VOLATILE_STORAGE,
            NULL));
    m->free(m);
}

Test(MngIDsTest, INITIALIZE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_INITIALIZE, NULL)));
    struct ptpmgmt_INITIALIZE_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_INITIALIZE_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_INITIALIZE, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_INITIALIZE, NULL)));
    cr_expect(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_INITIALIZE, &t));
    m->free(m);
}

Test(MngIDsTest, FAULT_LOG)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_FAULT_LOG, NULL));
    struct ptpmgmt_FAULT_LOG_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_FAULT_LOG_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_FAULT_LOG, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_FAULT_LOG, NULL)));
    m->free(m);
}

Test(MngIDsTest, FAULT_LOG_RESET)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    char dumm[1];
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_FAULT_LOG_RESET, NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_FAULT_LOG_RESET, dumm)));
    cr_expect(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_FAULT_LOG_RESET, NULL));
    m->free(m);
}

Test(MngIDsTest, DEFAULT_DATA_SET)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_DEFAULT_DATA_SET, NULL));
    struct ptpmgmt_DEFAULT_DATA_SET_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_DEFAULT_DATA_SET_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_DEFAULT_DATA_SET, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_DEFAULT_DATA_SET,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, CURRENT_DATA_SET)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_CURRENT_DATA_SET, NULL));
    struct ptpmgmt_CURRENT_DATA_SET_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_CURRENT_DATA_SET_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_CURRENT_DATA_SET, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_CURRENT_DATA_SET,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, PARENT_DATA_SET)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PARENT_DATA_SET, NULL));
    struct ptpmgmt_PARENT_DATA_SET_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PARENT_DATA_SET_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PARENT_DATA_SET, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PARENT_DATA_SET, NULL)));
    m->free(m);
}

Test(MngIDsTest, TIME_PROPERTIES_DATA_SET)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_TIME_PROPERTIES_DATA_SET, NULL));
    struct ptpmgmt_TIME_PROPERTIES_DATA_SET_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_TIME_PROPERTIES_DATA_SET_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_TIME_PROPERTIES_DATA_SET,
                &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_TIME_PROPERTIES_DATA_SET,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, PORT_DATA_SET)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PORT_DATA_SET, NULL));
    struct ptpmgmt_PORT_DATA_SET_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PORT_DATA_SET_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_DATA_SET, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PORT_DATA_SET, NULL)));
    m->free(m);
}

Test(MngIDsTest, PRIORITY1)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    struct ptpmgmt_PRIORITY1_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PRIORITY1_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PRIORITY1, NULL)));
    m->free(m);
}

Test(MngIDsTest, PRIORITY2)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIORITY2, NULL));
    struct ptpmgmt_PRIORITY2_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PRIORITY2_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY2, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PRIORITY2, NULL)));
    m->free(m);
}

Test(MngIDsTest, DOMAIN)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_DOMAIN, NULL));
    struct ptpmgmt_DOMAIN_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_DOMAIN_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_DOMAIN, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_DOMAIN, NULL)));
    m->free(m);
}

Test(MngIDsTest, SLAVE_ONLY)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_SLAVE_ONLY, NULL));
    struct ptpmgmt_SLAVE_ONLY_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_SLAVE_ONLY_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_SLAVE_ONLY, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_SLAVE_ONLY, NULL)));
    m->free(m);
}

Test(MngIDsTest, LOG_ANNOUNCE_INTERVAL)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_LOG_ANNOUNCE_INTERVAL, NULL));
    struct ptpmgmt_LOG_ANNOUNCE_INTERVAL_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_LOG_ANNOUNCE_INTERVAL_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_LOG_ANNOUNCE_INTERVAL, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_LOG_ANNOUNCE_INTERVAL,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, ANNOUNCE_RECEIPT_TIMEOUT)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_ANNOUNCE_RECEIPT_TIMEOUT, NULL));
    struct ptpmgmt_ANNOUNCE_RECEIPT_TIMEOUT_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_ANNOUNCE_RECEIPT_TIMEOUT_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ANNOUNCE_RECEIPT_TIMEOUT, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_ANNOUNCE_RECEIPT_TIMEOUT,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, LOG_SYNC_INTERVAL)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_LOG_SYNC_INTERVAL, NULL));
    struct ptpmgmt_LOG_SYNC_INTERVAL_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_LOG_SYNC_INTERVAL_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_LOG_SYNC_INTERVAL, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_LOG_SYNC_INTERVAL,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, VERSION_NUMBER)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_VERSION_NUMBER, NULL));
    struct ptpmgmt_VERSION_NUMBER_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_VERSION_NUMBER_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_VERSION_NUMBER, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_VERSION_NUMBER, NULL)));
    m->free(m);
}

Test(MngIDsTest, ENABLE_PORT)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    char dumm[1];
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_ENABLE_PORT, NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_ENABLE_PORT, dumm)));
    cr_expect(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_ENABLE_PORT, NULL));
    m->free(m);
}

Test(MngIDsTest, DISABLE_PORT)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    char dumm[1];
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_DISABLE_PORT, NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_DISABLE_PORT, dumm)));
    cr_expect(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_DISABLE_PORT, NULL));
    m->free(m);
}

Test(MngIDsTest, TIME)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_TIME, NULL));
    struct ptpmgmt_TIME_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_TIME_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_TIME, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_TIME, NULL)));
    m->free(m);
}

Test(MngIDsTest, CLOCK_ACCURACY)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_CLOCK_ACCURACY, NULL));
    struct ptpmgmt_CLOCK_ACCURACY_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_CLOCK_ACCURACY_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_CLOCK_ACCURACY, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_CLOCK_ACCURACY, NULL)));
    m->free(m);
}

Test(MngIDsTest, UTC_PROPERTIES)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_UTC_PROPERTIES, NULL));
    struct ptpmgmt_UTC_PROPERTIES_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_UTC_PROPERTIES_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_UTC_PROPERTIES, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_UTC_PROPERTIES, NULL)));
    m->free(m);
}

Test(MngIDsTest, TRACEABILITY_PROPERTIES)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_TRACEABILITY_PROPERTIES, NULL));
    struct ptpmgmt_TRACEABILITY_PROPERTIES_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_TRACEABILITY_PROPERTIES_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_TRACEABILITY_PROPERTIES, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_TRACEABILITY_PROPERTIES,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, TIMESCALE_PROPERTIES)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_TIMESCALE_PROPERTIES, NULL));
    struct ptpmgmt_TIMESCALE_PROPERTIES_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_TIMESCALE_PROPERTIES_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_TIMESCALE_PROPERTIES, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_TIMESCALE_PROPERTIES,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, UNICAST_NEGOTIATION_ENABLE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_UNICAST_NEGOTIATION_ENABLE,
            NULL));
    struct ptpmgmt_UNICAST_NEGOTIATION_ENABLE_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_UNICAST_NEGOTIATION_ENABLE_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_UNICAST_NEGOTIATION_ENABLE, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_UNICAST_NEGOTIATION_ENABLE, NULL)));
    m->free(m);
}

Test(MngIDsTest, PATH_TRACE_LIST)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PATH_TRACE_LIST, NULL));
    struct ptpmgmt_PATH_TRACE_LIST_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PATH_TRACE_LIST_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PATH_TRACE_LIST, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PATH_TRACE_LIST, NULL)));
    m->free(m);
}

Test(MngIDsTest, PATH_TRACE_ENABLE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PATH_TRACE_ENABLE, NULL));
    struct ptpmgmt_PATH_TRACE_ENABLE_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PATH_TRACE_ENABLE_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PATH_TRACE_ENABLE, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PATH_TRACE_ENABLE,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, GRANDMASTER_CLUSTER_TABLE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_GRANDMASTER_CLUSTER_TABLE,
            NULL));
    struct ptpmgmt_GRANDMASTER_CLUSTER_TABLE_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_GRANDMASTER_CLUSTER_TABLE_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_GRANDMASTER_CLUSTER_TABLE, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_GRANDMASTER_CLUSTER_TABLE, NULL)));
    m->free(m);
}

Test(MngIDsTest, UNICAST_MASTER_TABLE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_UNICAST_MASTER_TABLE, NULL));
    struct ptpmgmt_UNICAST_MASTER_TABLE_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_UNICAST_MASTER_TABLE_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_UNICAST_MASTER_TABLE, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_UNICAST_MASTER_TABLE,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, UNICAST_MASTER_MAX_TABLE_SIZE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_UNICAST_MASTER_MAX_TABLE_SIZE,
            NULL));
    struct ptpmgmt_UNICAST_MASTER_MAX_TABLE_SIZE_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_UNICAST_MASTER_MAX_TABLE_SIZE_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET,
                PTPMGMT_UNICAST_MASTER_MAX_TABLE_SIZE, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_UNICAST_MASTER_MAX_TABLE_SIZE, NULL)));
    m->free(m);
}

Test(MngIDsTest, ACCEPTABLE_MASTER_TABLE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_ACCEPTABLE_MASTER_TABLE, NULL));
    struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ACCEPTABLE_MASTER_TABLE, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_ACCEPTABLE_MASTER_TABLE,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_ACCEPTABLE_MASTER_TABLE_ENABLED,
            NULL));
    struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_ENABLED_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_ENABLED_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ACCEPTABLE_MASTER_TABLE_ENABLED,
            &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_ACCEPTABLE_MASTER_TABLE_ENABLED, NULL)));
    m->free(m);
}

Test(MngIDsTest, ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_ACCEPTABLE_MASTER_MAX_TABLE_SIZE,
            NULL));
    struct ptpmgmt_ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET,
                PTPMGMT_ACCEPTABLE_MASTER_MAX_TABLE_SIZE, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_ACCEPTABLE_MASTER_MAX_TABLE_SIZE, NULL)));
    m->free(m);
}

Test(MngIDsTest, ALTERNATE_MASTER)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_ALTERNATE_MASTER, NULL));
    struct ptpmgmt_ALTERNATE_MASTER_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_ALTERNATE_MASTER_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ALTERNATE_MASTER, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_ALTERNATE_MASTER,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, ALTERNATE_TIME_OFFSET_ENABLE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_ALTERNATE_TIME_OFFSET_ENABLE,
            NULL));
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_ENABLE_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_ALTERNATE_TIME_OFFSET_ENABLE_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ALTERNATE_TIME_OFFSET_ENABLE,
            &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_ALTERNATE_TIME_OFFSET_ENABLE, NULL)));
    m->free(m);
}

Test(MngIDsTest, ALTERNATE_TIME_OFFSET_NAME)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_ALTERNATE_TIME_OFFSET_NAME,
            NULL));
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ALTERNATE_TIME_OFFSET_NAME, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_ALTERNATE_TIME_OFFSET_NAME, NULL)));
    m->free(m);
}

Test(MngIDsTest, ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_ALTERNATE_TIME_OFFSET_MAX_KEY,
            NULL));
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_MAX_KEY_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_ALTERNATE_TIME_OFFSET_MAX_KEY_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET,
                PTPMGMT_ALTERNATE_TIME_OFFSET_MAX_KEY, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_ALTERNATE_TIME_OFFSET_MAX_KEY, NULL)));
    m->free(m);
}

Test(MngIDsTest, ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_ALTERNATE_TIME_OFFSET_PROPERTIES,
            NULL));
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_PROPERTIES_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_ALTERNATE_TIME_OFFSET_PROPERTIES_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ALTERNATE_TIME_OFFSET_PROPERTIES,
            &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_ALTERNATE_TIME_OFFSET_PROPERTIES, NULL)));
    m->free(m);
}

Test(MngIDsTest, TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_TRANSPARENT_CLOCK_PORT_DATA_SET,
            NULL));
    struct ptpmgmt_TRANSPARENT_CLOCK_PORT_DATA_SET_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_TRANSPARENT_CLOCK_PORT_DATA_SET_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET,
                PTPMGMT_TRANSPARENT_CLOCK_PORT_DATA_SET, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_TRANSPARENT_CLOCK_PORT_DATA_SET, NULL)));
    m->free(m);
}

Test(MngIDsTest, LOG_MIN_PDELAY_REQ_INTERVAL)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_LOG_MIN_PDELAY_REQ_INTERVAL,
            NULL));
    struct ptpmgmt_LOG_MIN_PDELAY_REQ_INTERVAL_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_LOG_MIN_PDELAY_REQ_INTERVAL_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_LOG_MIN_PDELAY_REQ_INTERVAL,
            &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_LOG_MIN_PDELAY_REQ_INTERVAL, NULL)));
    m->free(m);
}

Test(MngIDsTest, TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET,
            PTPMGMT_TRANSPARENT_CLOCK_DEFAULT_DATA_SET, NULL));
    struct ptpmgmt_TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET,
                PTPMGMT_TRANSPARENT_CLOCK_DEFAULT_DATA_SET, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_TRANSPARENT_CLOCK_DEFAULT_DATA_SET, NULL)));
    m->free(m);
}

Test(MngIDsTest, PRIMARY_DOMAIN)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PRIMARY_DOMAIN, NULL));
    struct ptpmgmt_PRIMARY_DOMAIN_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PRIMARY_DOMAIN_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIMARY_DOMAIN, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PRIMARY_DOMAIN, NULL)));
    m->free(m);
}

Test(MngIDsTest, DELAY_MECHANISM)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_DELAY_MECHANISM, NULL));
    struct ptpmgmt_DELAY_MECHANISM_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_DELAY_MECHANISM_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_DELAY_MECHANISM, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_DELAY_MECHANISM, NULL)));
    m->free(m);
}

Test(MngIDsTest, EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET,
            PTPMGMT_EXTERNAL_PORT_CONFIGURATION_ENABLED, NULL));
    struct ptpmgmt_EXTERNAL_PORT_CONFIGURATION_ENABLED_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_EXTERNAL_PORT_CONFIGURATION_ENABLED_t));
    cr_expect(m->setAction(m, PTPMGMT_SET,
            PTPMGMT_EXTERNAL_PORT_CONFIGURATION_ENABLED, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_EXTERNAL_PORT_CONFIGURATION_ENABLED, NULL)));
    m->free(m);
}

Test(MngIDsTest, MASTER_ONLY)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_MASTER_ONLY, NULL));
    struct ptpmgmt_MASTER_ONLY_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_MASTER_ONLY_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_MASTER_ONLY, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_MASTER_ONLY, NULL)));
    m->free(m);
}

Test(MngIDsTest, HOLDOVER_UPGRADE_ENABLE)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_HOLDOVER_UPGRADE_ENABLE, NULL));
    struct ptpmgmt_HOLDOVER_UPGRADE_ENABLE_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_HOLDOVER_UPGRADE_ENABLE_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_HOLDOVER_UPGRADE_ENABLE, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_HOLDOVER_UPGRADE_ENABLE,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, EXT_PORT_CONFIG_PORT_DATA_SET)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_EXT_PORT_CONFIG_PORT_DATA_SET,
            NULL));
    struct ptpmgmt_EXT_PORT_CONFIG_PORT_DATA_SET_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_EXT_PORT_CONFIG_PORT_DATA_SET_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_EXT_PORT_CONFIG_PORT_DATA_SET,
            &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_EXT_PORT_CONFIG_PORT_DATA_SET, NULL)));
    m->free(m);
}

Test(MngIDsTest, TIME_STATUS_NP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_TIME_STATUS_NP, NULL));
    struct ptpmgmt_TIME_STATUS_NP_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_TIME_STATUS_NP_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_TIME_STATUS_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_TIME_STATUS_NP, NULL)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    m->updateParams(m, p);
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_TIME_STATUS_NP, NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_TIME_STATUS_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_TIME_STATUS_NP, NULL)));
    m->free(m);
}

Test(MngIDsTest, GRANDMASTER_SETTINGS_NP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_GRANDMASTER_SETTINGS_NP, NULL));
    struct ptpmgmt_GRANDMASTER_SETTINGS_NP_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_GRANDMASTER_SETTINGS_NP_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_GRANDMASTER_SETTINGS_NP, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_GRANDMASTER_SETTINGS_NP,
                NULL)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    m->updateParams(m, p);
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_GRANDMASTER_SETTINGS_NP,
                NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_GRANDMASTER_SETTINGS_NP,
                &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_GRANDMASTER_SETTINGS_NP,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, PORT_DATA_SET_NP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PORT_DATA_SET_NP, NULL));
    struct ptpmgmt_PORT_DATA_SET_NP_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PORT_DATA_SET_NP_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_DATA_SET_NP, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PORT_DATA_SET_NP,
                NULL)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    m->updateParams(m, p);
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_PORT_DATA_SET_NP, NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_DATA_SET_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PORT_DATA_SET_NP,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, SUBSCRIBE_EVENTS_NP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_SUBSCRIBE_EVENTS_NP, NULL));
    struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_SUBSCRIBE_EVENTS_NP, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_SUBSCRIBE_EVENTS_NP,
                NULL)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    m->updateParams(m, p);
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_SUBSCRIBE_EVENTS_NP, NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_SUBSCRIBE_EVENTS_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_SUBSCRIBE_EVENTS_NP,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, PORT_PROPERTIES_NP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PORT_PROPERTIES_NP, NULL));
    struct ptpmgmt_PORT_PROPERTIES_NP_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PORT_PROPERTIES_NP_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_PROPERTIES_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PORT_PROPERTIES_NP,
                NULL)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    m->updateParams(m, p);
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_PORT_PROPERTIES_NP, NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_PROPERTIES_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PORT_PROPERTIES_NP,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, PORT_STATS_NP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PORT_STATS_NP, NULL));
    struct ptpmgmt_PORT_STATS_NP_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PORT_STATS_NP_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_STATS_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PORT_STATS_NP, NULL)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    m->updateParams(m, p);
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_PORT_STATS_NP, NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_STATS_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PORT_STATS_NP, NULL)));
    m->free(m);
}

Test(MngIDsTest, SYNCHRONIZATION_UNCERTAIN_NP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP,
            NULL));
    struct ptpmgmt_SYNCHRONIZATION_UNCERTAIN_NP_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_SYNCHRONIZATION_UNCERTAIN_NP_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP,
            &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP, NULL)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    m->updateParams(m, p);
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP,
                NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP,
                &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP, NULL)));
    m->free(m);
}

Test(MngIDsTest, PORT_SERVICE_STATS_NP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PORT_SERVICE_STATS_NP, NULL));
    struct ptpmgmt_PORT_SERVICE_STATS_NP_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PORT_SERVICE_STATS_NP_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_SERVICE_STATS_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PORT_SERVICE_STATS_NP,
                NULL)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    m->updateParams(m, p);
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_PORT_SERVICE_STATS_NP,
                NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_SERVICE_STATS_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PORT_SERVICE_STATS_NP,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, UNICAST_MASTER_TABLE_NP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_UNICAST_MASTER_TABLE_NP, NULL));
    struct ptpmgmt_UNICAST_MASTER_TABLE_NP_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_UNICAST_MASTER_TABLE_NP_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_UNICAST_MASTER_TABLE_NP,
                &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_UNICAST_MASTER_TABLE_NP,
                NULL)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    m->updateParams(m, p);
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_UNICAST_MASTER_TABLE_NP,
                NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_UNICAST_MASTER_TABLE_NP,
                &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_UNICAST_MASTER_TABLE_NP,
                NULL)));
    m->free(m);
}

Test(MngIDsTest, PORT_HWCLOCK_NP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_PORT_HWCLOCK_NP, NULL));
    struct ptpmgmt_PORT_HWCLOCK_NP_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_PORT_HWCLOCK_NP_t));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_HWCLOCK_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PORT_HWCLOCK_NP, NULL)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    m->updateParams(m, p);
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_PORT_HWCLOCK_NP, NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_HWCLOCK_NP, &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND, PTPMGMT_PORT_HWCLOCK_NP, NULL)));
    m->free(m);
}

Test(MngIDsTest, POWER_PROFILE_SETTINGS_NP)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(m->setAction(m, PTPMGMT_GET, PTPMGMT_POWER_PROFILE_SETTINGS_NP,
            NULL));
    struct ptpmgmt_POWER_PROFILE_SETTINGS_NP_t t;
    memset(&t, 0, sizeof(struct ptpmgmt_POWER_PROFILE_SETTINGS_NP_t));
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_POWER_PROFILE_SETTINGS_NP, &t));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_POWER_PROFILE_SETTINGS_NP, NULL)));
    ptpmgmt_pMsgParams p = m->getParams(m);
    p->implementSpecific = ptpmgmt_noImplementSpecific;
    m->updateParams(m, p);
    cr_expect(not(m->setAction(m, PTPMGMT_GET, PTPMGMT_POWER_PROFILE_SETTINGS_NP,
                NULL)));
    cr_expect(not(m->setAction(m, PTPMGMT_SET, PTPMGMT_POWER_PROFILE_SETTINGS_NP,
                &t)));
    cr_expect(not(m->setAction(m, PTPMGMT_COMMAND,
                PTPMGMT_POWER_PROFILE_SETTINGS_NP, NULL)));
    m->free(m);
}
