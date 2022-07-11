%@* SPDX-License-Identifier: LGPL-3.0-or-later
 %- SPDX-FileCopyrightText: Copyright 2022 Erez Geva *%@
%_
%@** @file
 * @brief standard vector container classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 *
 * This header is generated automatically, do @b NOT change,
 *  or use it in your application!
 *
 * This header is used by Doxygen only!
 *
 * @note See std_vectors.md for more information
 *
 * @note The classes are available in scripts only!
 *
 * Any C++ function or structure that use std::vector<>
 * will be translated to the follow classes in scripts.
 *
 *%@
%_
%#ifndef __PTPMGMT_VEC_DEF_H
%#define __PTPMGMT_VEC_DEF_H
%_
%#include "proc.h"
%#include "sig.h"
%_
namespace ptpmgmt
{
%_
#define vecClass(n, T) \
%@**%^\
 * class to handle std::vector<T> in scripts%^\
 *%@%^\
class n%^\
{%^\
 %-private:%^\
 %-%- std::vector<T> elements;%^\
 %-public:%^\
 %-%- %@**%^\
 %- %- * Initiate a new empty container%^\
 %- %- *%@%^\
 %-%- n();%^\
 %-%- %@**%^\
 %- %- * Initiate a new container with size elements%^\
 %- %- * @param[in] size of the new container%^\
 %- %- * @note Elements holds default value!%^\
 %- %- *%@%^\
 %-%- n(size_t size);%^\
 %-%- %@**%^\
 %- %- * Initiate a new container with size elements and%^\
 %- %- *%- copy value to all elements%^\
 %- %- * @param[in] size of the new container%^\
 %- %- * @param[in] value for all elements in new container%^\
 %- %- * @note This constructor is not available in PHP.%^\
 %- %- *%@%^\
 %-%- n(size_t size, const T &value);%^\
 %-%- %@**%^\
 %- %- * Copy constructor%^\
 %- %- * @param[in] container to copy%^\
 %- %- *%@%^\
 %-%- n(const n &container);%^\
 %-%- %@**%^\
 %- %- * Get the number of elements in container%^\
 %- %- * @return number of elements in container%^\
 %- %- *%@%^\
 %-%- size_t size() const;%^\
 %-%- %@**%^\
 %- %- * Query if container is empty%^\
 %- %- * @return true if size is zero%^\
 %- %- * @note The function is named @code is_empty() @endcode in php,%^\
 %- %- * %- %- %- and @code empty? @endcode in ruby.%^\
 %- %- *%@%^\
 %-%- bool empty() const;%^\
 %-%- %@**%^\
 %- %- * Remove all elements from container%^\
 %- %- * @note Call std::vector<T>::clear()%^\
 %- %- *%@%^\
 %-%- void clear();%^\
 %-%- %@**%^\
 %- %- * Push a copy of element at the end of the container%^\
 %- %- * @param[in] value to push%^\
 %- %- * @note Call std::vector<T>::push_back()%^\
 %- %- * @note The function is named @code push_back() @endcode in Python and lua.%^\
 %- %- *%@%^\
 %-%- void push(const T &value);%^\
 %-%- %@**%^\
 %- %- * Get last element and removes it from the container%^\
 %- %- * @return last element%^\
 %- %- * @attention initiate exception if container is empty!%^\
 %- %- * @note Call std::vector<T>::back() and%^\
 %- %- *%- std::vector<T>::pop_back()%^\
 %- %- * @note This function is not available in lua.%^\
 %- %- * %- %- %- lua have the 'back()' and 'pop_back()' functions.%^\
 %- %- *%@%^\
 %-%- T pop();%^\
 %-%- %@**%^\
 %- %- * Removes last element from the container%^\
 %- %- * @attention initiate exception if container is empty!%^\
 %- %- * @note This function available in Python and lua only.%^\
 %- %- *%@%^\
 %-%- void pop_back();%^\
 %-%- %@**%^\
 %- %- * Get last element from container%^\
 %- %- * @attention initiate exception if container is empty!%^\
 %- %- * @note This function available in Python, ruby and lua.%^\
 %- %- *%@%^\
 %-%- T back();%^\
 %-%- %@**%^\
 %- %- * Get first element from container%^\
 %- %- * @attention initiate exception if container is empty!%^\
 %- %- * @note This function available in Python, ruby and lua.%^\
 %- %- *%@%^\
 %-%- T front();%^\
 %-%- %@**%^\
 %- %- * Get element from the container%^\
 %- %- * @param[in] position of element to get%^\
 %- %- * @return the element%^\
 %- %- * @attention initiate exception if out of range!%^\
 %- %- * @note This function is available in Perl, tcl and php.%^\
 %- %- * %- %- %- Ruby, Python and lua use operator[].%^\
 %- %- *%@%^\
 %-%- T &get(size_t position) const;%^\
 %-%- %@**%^\
 %- %- * Replace element value in the container%^\
 %- %- * @param[in] position of element to set%^\
 %- %- * @param[in] value element with new value%^\
 %- %- * @attention initiate exception if out of range!%^\
 %- %- * @note This function is available in Perl, tcl and php.%^\
 %- %- * %- %- %- Ruby, Python and lua use operator[].%^\
 %- %- *%@%^\
 %-%- void set(size_t position, T &value);%^\
 %-%- %@**%^\
 %- %- * Fetch an element from the container%^\
 %- %- * The operator can be used for retrieving and setting!%^\
 %- %- * @param[in] position of the element%^\
 %- %- * @return the element%^\
 %- %- * @attention initiate exception if out of range!%^\
 %- %- * @note This function is available for Ruby, Python and lua.%^\
 %- %- * %- %- %- Perl, tcl and php should use @code get(), set() @endcode.%^\
 %- %- * @note To set use @code variable[index] = new_value @endcode%^\
 %- %- *%@%^\
 %-%- T &operator[](size_t position);%^\
 %-%- %@**%^\
 %- %- * Get current container capacity%^\
 %- %- * @note This function available in Python, ruby and PHP.%^\
 %- %- *%@%^\
 %-%- size_t capacity() const;%^\
 %-%- %@**%^\
 %- %- * Reserve more space for further elements%^\
 %- %- * @note This function available in Python, ruby and PHP.%^\
 %- %- *%@%^\
 %-%- void reserve( size_t new_cap );%^\
 %-%- %@**%^\
 %- %- * Get system limitation to container size%^\
 %- %- * @note This function available in lua only.%^\
 %- %- *%@%^\
 %-%- size_t max_size() const;%^\
};
#define vecClassType(n) vecClass(n##_v, n##_t)
vecClassType(FaultRecord)
vecClassType(ClockIdentity)
vecClassType(PortAddress)
vecClassType(AcceptableMaster)
vecClassType(LinuxptpUnicastMaster)
vecClassType(PtpEvent)
vecClassType(PtpSample)
vecClassType(PtpSampleExt)
#define vecClassSig(n, T) vecClass(Sig##n, T##_rec_t)
vecClassSig(Time, SLAVE_RX_SYNC_TIMING_DATA)
vecClassSig(Comp, SLAVE_RX_SYNC_COMPUTED_DATA)
vecClassSig(Event, SLAVE_TX_EVENT_TIMESTAMPS)
vecClassSig(Delay, SLAVE_DELAY_TIMING_DATA_NP)
%_
}; %@* namespace ptpmgmt *%@
%_
%#endif %@* __PTPMGMT_VEC_DEF_H *%@





max_size

