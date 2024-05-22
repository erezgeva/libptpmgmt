# SPDX-License-Identifier: LGPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
#
# Makefile for jclklib test application
#
# @author Song Yoong Siang <yoong.siang.song@intel.com>
# @copyright © 2024 Intel Corporation. All rights reserved.
# @license LGPL-3.0-or-later
#
###############################################################################

# Variables
CXX = g++
CXXFLAGS = -Wall
INCLUDES = -I$(shell pwd)/..
LDFLAGS = -L$(shell pwd)/../client
LDLIBS = -ljclk
TARGET = jclk_test

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(TARGET).o
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Compiling
$(TARGET).o: $(TARGET).cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Cleaning
clean:
	rm -f $(TARGET) $(TARGET).o

.PHONY: all clean
