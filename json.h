/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief convert management messages to json
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PMC_JSON_H
#define __PMC_JSON_H

#include <string>
#include <cstdint>
#include "msg.h"

/**
 * Convert Message to Json string
 * @param[in] message received from PTP entity
 * @return Json string
 */
std::string msg2json(Message &message);

#endif /*__PMC_JSON_H*/
