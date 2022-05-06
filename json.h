/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief convert a management or a signaling message to JSON
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_JSON_H
#define __PTPMGMT_JSON_H

#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include "msg.h"

#ifndef SWIG
namespace ptpmgmt
{
#endif

/**
 * Convert Message to JSON string
 * @param[in] message received from PTP entity
 * @param[in] indent base indent for the JSON string
 * @return JSON string
 */
std::string msg2json(const Message &message, int indent = 0);

#ifndef SWIG
}; /* namespace ptpmgmt */
#endif

#endif /* __PTPMGMT_JSON_H */
