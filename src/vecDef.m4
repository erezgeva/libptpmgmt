dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */
dnl
dnl @file
dnl @brief Standard vector container classes
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2022 Erez Geva
dnl
dnl Create vecDef.h, used by Doxygen
dnl
/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Standard vector container classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 * This header is generated automatically, do @b NOT change,
 * or use it in your application!
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
 */

#ifndef __PTPMGMT_VEC_DEF_H
#define __PTPMGMT_VEC_DEF_H

#include "proc.h"
#include "sig.h"

__PTPMGMT_NAMESPACE_BEGIN

define(LUA_FUNCS,'back()' and 'pop_back()')dnl
define(VEC_CLASS, `/**
 * class to handle std::vector<$2> in scripts
 */
class $1
{
  private:
    std::vector<$2> elements;
  public:
    /**
     * Initiate a new empty container
     */
    $1();
    /**
     * Initiate a new container with size elements
     * @param[in] size of the new container
     * @note Elements holds default value!
     */
    $1(size_t size);
    /**
     * Initiate a new container with size elements and
     *  copy value to all elements
     * @param[in] size of the new container
     * @param[in] value for all elements in new container
     * @note This constructor is not available in PHP.
     */
    $1(size_t size, const $2 &value);
    /**
     * Copy constructor
     * @param[in] container to copy
     */
    $1(const $1 &container);
    /**
     * Get the number of elements in container
     * @return number of elements in container
     */
    size_t size() const;
    /**
     * Query if container is empty
     * @return true if size is zero
     * @note The function is named @code is_empty() @endcode in php,
     *       and @code empty? @endcode in ruby.
     */
    bool empty() const;
    /**
     * Remove all elements from container
     * @note Call std::vector<$2>::clear()
     */
    void clear();
    /**
     * Push a copy of element at the end of the container
     * @param[in] value to push
     * @note Call std::vector<$2>::push_back()
     * @note The function is named @code push_back() @endcode in Python and lua.
     */
    void push(const $2 &value);
    /**
     * Get last element and removes it from the container
     * @return last element
     * @attention initiate exception if container is empty!
     * @note Call std::vector<$2>::back() and
     *  std::vector<$2>::pop_back()
     * @note This function is not available in lua.
     *       lua have the LUA_FUNCS functions.
     */
    $2 pop();
    /**
     * Removes last element from the container
     * @attention initiate exception if container is empty!
     * @note This function available in Python and lua only.
     */
    void pop_back();
    /**
     * Get last element from container
     * @attention initiate exception if container is empty!
     * @note This function available in Python, ruby and lua.
     */
    $2 back();
    /**
     * Get first element from container
     * @attention initiate exception if container is empty!
     * @note This function available in Python, ruby and lua.
     */
    $2 front();
    /**
     * Get element from the container
     * @param[in] position of element to get
     * @return the element
     * @attention initiate exception if out of range!
     * @note This function is available in Perl, tcl and php.
     *       Ruby, Python and lua use operator[].
     */
    $2 &get(size_t position) const;
    /**
     * Replace element value in the container
     * @param[in] position of element to set
     * @param[in] value element with new value
     * @attention initiate exception if out of range!
     * @note This function is available in Perl, tcl and php.
     *       Ruby, Python and lua use operator[].
     */
    void set(size_t position, $2 &value);
    /**
     * Fetch an element from the container
     * The operator can be used for retrieving and setting!
     * @param[in] position of the element
     * @return the element
     * @attention initiate exception if out of range!
     * @note This function is available for Ruby, Python and lua.
     *       Perl, tcl and php should use @code get(), set() @endcode.
     * @note To set use @code variable[index] = new_value @endcode
     */
    $2 &operator[](size_t position);
    /**
     * Get current container capacity
     * @note This function available in Python, ruby and PHP.
     */
    size_t capacity() const;
    /**
     * Reserve more space for further elements
     * @note This function available in Python, ruby and PHP.
     */
    void reserve(size_t new_cap);
    /**
     * Get system limitation to container size
     * @note This function available in lua only.
     */
    size_t max_size() const;
};')dnl
define(VEC_CLASS_TYPE, `VEC_CLASS($1_v, $1_t)')dnl
VEC_CLASS_TYPE(FaultRecord)
VEC_CLASS_TYPE(ClockIdentity)
VEC_CLASS_TYPE(PortAddress)
VEC_CLASS_TYPE(AcceptableMaster)
VEC_CLASS_TYPE(LinuxptpUnicastMaster)
VEC_CLASS_TYPE(PtpEvent)
VEC_CLASS_TYPE(PtpSample)
VEC_CLASS_TYPE(PtpSampleExt)
define(VEC_CLASS_SIG, `VEC_CLASS(Sig$1, $2_rec_t)')dnl
VEC_CLASS_SIG(Time, SLAVE_RX_SYNC_TIMING_DATA)
VEC_CLASS_SIG(Comp, SLAVE_RX_SYNC_COMPUTED_DATA)
VEC_CLASS_SIG(Event, SLAVE_TX_EVENT_TIMESTAMPS)
VEC_CLASS_SIG(Delay, SLAVE_DELAY_TIMING_DATA_NP)

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_VEC_DEF_H */
