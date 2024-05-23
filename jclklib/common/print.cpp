/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file print.cpp
 * @brief Utility functions for printing
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <common/print.hpp>

#include <iostream>
#include <cstring>
#include <iomanip>

using namespace std;
using namespace JClkLibCommon;

enum LogLevel { DEBUG, INFO, ERROR };
static LogLevel currentLogLevel = INFO;

void JClkLibCommon::_PrintError(std::string msg, uint16_t line, std::string file, std::string func,
			      errno_type errnum)
{
	cerr << "*** Error: " + msg + " " + (errnum != (errno_type)-1 ? strerror(errnum) : "") + " at line " + to_string(line) +
		" in " + file + ":" + func + "\n";
	cerr.flush();
}

void JClkLibCommon::_PrintDebug(std::string msg, uint16_t line, std::string file, std::string func)
{
	if (currentLogLevel <= DEBUG) {
		cerr << "**  Debug: " + msg + " at line " + to_string(line) + " in " + file + ":" + func + "\n";
		cerr.flush();
	}
}

void JClkLibCommon::_PrintInfo(std::string msg, uint16_t line, std::string file, std::string func)
{
	if (currentLogLevel <= INFO) {
		cerr << "*   Info: " + msg + " at line " + to_string(line) + " in " + file + ":" + func + "\n";
		cerr.flush();
	}
}

#define HEX_DIGITS_PER_LINE 16
void JClkLibCommon::_DumpOctetArray(string msg, const uint8_t *arr, size_t length, uint16_t line, string file, string func)
{
	const uint8_t *index = arr;
	size_t offset;
	stringstream output;
	bool addspc = false, addcr;

	if (currentLogLevel > DEBUG) {
		return;
	}

	output << "*   Info: " << msg << " at line " << to_string(line) << " in " << file << ":" << func << "\n";
	
	while ((offset = (index - arr)) < length) {
		if (addspc)
			output << " ";
		output << "0x" << setfill('0') << setw(2) << std::hex << int(*index++);
		if (offset % HEX_DIGITS_PER_LINE != HEX_DIGITS_PER_LINE-1) {
			addspc = true;
			addcr = true;
		} else {
			addspc = false;
			addcr = false;
			output << "\n";
		}
	}
	if (addcr)
		output << "\n";
	cerr << output.str();
	cerr.flush();
}
