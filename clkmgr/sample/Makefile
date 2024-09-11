# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# Makefile for clkmgr test application
#
# @author Song Yoong Siang <yoong.siang.song@intel.com>
# @copyright © 2024 Intel Corporation.
# @license BSD-3-Clause
#
###############################################################################

# Variables
CC = gcc
CFLAGS = -Wall
CXX = g++
CXXFLAGS = -Wall
INCLUDES = -I$(shell pwd)/.. -I$(shell pwd)/../pub
LDFLAGS = -L$(shell pwd)/../client
LDLIBS = -lclkmgr
TARGET = clkmgr_test
CTARGET = clkmgr_c_test

# Default target
all: $(TARGET) $(CTARGET)

# Linking
$(TARGET): $(TARGET).o
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(CTARGET): $(CTARGET).o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Compiling
$(TARGET).o: $(TARGET).cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(CTARGET).o: $(CTARGET).c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Cleaning
clean:
	rm -f $(TARGET) $(TARGET).o $(CTARGET) $(CTARGET).o

.PHONY: all clean
