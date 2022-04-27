%@* SPDX-License-Identifier: LGPL-3.0-or-later
 %- SPDX-FileCopyrightText: Copyright 2021 Erez Geva *%@
%_
%@** @file
 * @brief List of management IDs and conversions functions
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * This header is generated automatically, do @b NOT change,
 *  or use it in your application!
 *
 *%@
%_
%#ifndef __PTPMGMT_MNG_IDS_H
%#define __PTPMGMT_MNG_IDS_H
%_
%#include "msg.h"
%_
%#ifndef SWIG
namespace ptpmgmt
{
%#endif
%_
%@**
 * @brief Management TLVs ID
 * @details
 *%- The Management TLVs the application may use on message.
 * @note this enumerator is defined in @"msg.h@" header.
 *%- Use this include in your application
 *%- @code{.cpp} #include "msg.h" @endcode
 * @attention Do @b NOT @p use this header in your application.
 *%@
enum mng_vals_e {
#define A(n, v, sc, a, sz, f) %- %-  n,
#include "ids.h"
};
%_
%#ifndef SWIG
%_
}; %@* namespace ptpmgmt *%@
%_
%@* For Doxygen only *%@
%_
using namespace ptpmgmt;
%_
#define caseUF(n) \
%@**%^\
 * Convert data to n##_t structure%^\
 * @param[in] data pointer to the Message dataField%^\
 * @return pointer to n##_t%^\
 * @note This function is available in scripts only!%^\
 *%- C++ code can simply cast.%^\
 * @note Use Message:getData() to retrieve the data from the Message%^\
 *%@%^\
n##_t%^*conv_##n(BaseMngTlv *data);
#define A(n, v, sc, a, sz, f) case##f(n)
#include "ids.h"
%_
#define S(n) \
%@**%^\
 * Convert data to n##_t structure%^\
 * @param[in] tlv pointer to a TLV from a signaling message%^\
 * @return pointer to n##_t%^\
 * @note This function is available in scripts only!%^\
 *%- C++ code can simply cast.%^\
 * @note Use Message:getSigMngTlv() to retrieve the tlv%^\
 *%@%^\
n##_t%^\
*conv_##n(BaseSigTlv *tlv);
S(ORGANIZATION_EXTENSION)
S(PATH_TRACE)
S(ALTERNATE_TIME_OFFSET_INDICATOR)
S(ENHANCED_ACCURACY_METRICS)
S(L1_SYNC)
S(PORT_COMMUNICATION_AVAILABILITY)
S(PROTOCOL_ADDRESS)
S(SLAVE_RX_SYNC_TIMING_DATA)
S(SLAVE_RX_SYNC_COMPUTED_DATA)
S(SLAVE_TX_EVENT_TIMESTAMPS)
S(CUMULATIVE_RATE_RATIO)
S(SLAVE_DELAY_TIMING_DATA_NP)
%_
#define vecClass(n, t) \
%@**%^\
 * class to handle std::vector<t> in scripts%^\
 * @note This class is available in scripts only!%^\
 *%- C++ code can simply use%^\
 *%- the std::vector<t> directly.%^\
 *%@%^\
class n%^\
{%^\
 %-public:%^\
 %-%- n();%^\
 %-%- %@**%^\
 %- %- * Initiate a new vector with size%^\
 %- %- * @param[in] size of new vector%^\
 %- %- * @note The vector holds empty values!%^\
 %- %- * @note This class is available in scripts only!%^\
 %- %- *%@%^\
 %-%- n(size_t size);%^\
 %-%- %@**%^\
 %- %- * Initiate a new vector with size and%^\
 %- %- *%- initiate all records with a value%^\
 %- %- * @param[in] size of new vector%^\
 %- %- * @param[in] value for all records in new vector%^\
 %- %- * @note The vector holds copies of the value!%^\
 %- %- * @note This class is available in scripts only!%^\
 %- %- *%@%^\
 %-%- n(size_t size, const t &value);%^\
 %-%- %@**%^\
 %- %- * Copy constructor%^\
 %- %- * @param[in] vector to copy%^\
 %- %- * @note This class is available in scripts only!%^\
 %- %- *%@%^\
 %-%- n(const n &vector);%^\
 %-%- %@**%^\
 %- %- * Get number of records in vector%^\
 %- %- * @return number of records in vector%^\
 %- %- * @note Call std::vector<t>::size()%^\
 %- %- * @note This class is available in scripts only!%^\
 %- %- *%@%^\
 %-%- size_t size() const;%^\
 %-%- %@**%^\
 %- %- * Query if vector does not hold any records%^\
 %- %- * @return true if size is zero%^\
 %- %- * @note Call std::vector<t>::empty()%^\
 %- %- * @note This class is available in scripts only!%^\
 %- %- *%@%^\
 %-%- bool empty() const;%^\
 %-%- %@**%^\
 %- %- * Remove all records from vector%^\
 %- %- * @note Call std::vector<t>::clear()%^\
 %- %- * @note This class is available in scripts only!%^\
 %- %- *%@%^\
 %-%- void clear();%^\
 %-%- %@**%^\
 %- %- * Push a copy of value record into the vector%^\
 %- %- *%- and place it at the end of the vector%^\
 %- %- * @param[in] value record to push%^\
 %- %- * @note Call std::vector<t>::push_back()%^\
 %- %- * @note The vector holds a copy of the value!%^\
 %- %- * @note This class is available in scripts only!%^\
 %- %- *%@%^\
 %-%- void push(const t &value);%^\
 %-%- %@**%^\
 %- %- * Get last record and removes it from the vector%^\
 %- %- * @return last record%^\
 %- %- * @attention initiate exception if vector is empty!%^\
 %- %- * @note Call std::vector<t>::back() and%^\
 %- %- *%- std::vector<t>::pop_back()%^\
 %- %- * @note This class is available in scripts only!%^\
 %- %- *%@%^\
 %-%- t pop();%^\
 %-%- %@**%^\
 %- %- * Get a record from the vector%^\
 %- %- * @param[in] position of record to get%^\
 %- %- * @return the record%^\
 %- %- * @attention initiate exception if position is out of vector range!%^\
 %- %- * @note Call std::vector<t>::at()%^\
 %- %- * @note This class is available in scripts only!%^\
 %- %- *%@%^\
 %-%- t &get(size_t position) const;%^\
 %-%- %@**%^\
 %- %- * Replace a record in the vector with a copy of a value%^\
 %- %- * @param[in] position of record to set%^\
 %- %- * @param[in] value record to copy%^\
 %- %- * @attention initiate exception if position is out of vector range!%^\
 %- %- * @note Call std::vector<t>::at()%^\
 %- %- * @note This class is available in scripts only!%^\
 %- %- *%@%^\
 %-%- void set(size_t position, t &value);%^\
};
#define vecClassType(n) vecClass(n##_v, n##_t)
vecClassType(FaultRecord)
vecClassType(ClockIdentity)
vecClassType(PortAddress)
vecClassType(AcceptableMaster)
#define vecClassSig(n, t) vecClass(Sig##n, t##_rec_t)
vecClassSig(Time, SLAVE_RX_SYNC_TIMING_DATA)
vecClassSig(Comp, SLAVE_RX_SYNC_COMPUTED_DATA)
vecClassSig(Event, SLAVE_TX_EVENT_TIMESTAMPS)
vecClassSig(Delay, SLAVE_DELAY_TIMING_DATA_NP)
%#endif %@* SWIG *%@
%_
%#endif %@* __PTPMGMT_MNG_IDS_H *%@
