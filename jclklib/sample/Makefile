# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# Makefile for jclklib test application
#
# @author Song Yoong Siang <yoong.siang.song@intel.com>
# @copyright © 2024 Intel Corporation.
# @license BSD-3-Clause
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
