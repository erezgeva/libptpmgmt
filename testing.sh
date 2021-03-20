#!/bin/bash
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# @author Erez Geva <ErezGeva2@gmail.com>
# @copyright 2021 Erez Geva
#
# testing script
###############################################################################
main()
{
 local -r if=enp0s25
 local -r linuxptp_loc=~/TSN/build/linuxptp
 # Add all users for testing (so we can test without using root :-)
 local -r uds=$linuxptp_loc/uds.c
 local -r reg='^#define UDS_FILEMODE'
 if [ -n "$(grep "$reg.*GRP)" $uds)" ];then
    sed -i "/$reg/ s#GRP)#GRP|S_IROTH|S_IWOTH)#" $uds
 fi
 # Remove the deprecated message
 local -r config=$linuxptp_loc/config.c
 local -r reg2='^\s\sfprintf(stderr, "option %s is deprecated,'
 if [ -n "$(grep "$reg2" $config)" ];then
    sed -i "/$reg2/,+1d" $config
 fi
 make --no-print-directory -j -C $linuxptp_loc
 make -j
 printf "\n * Starting\n"
 echo " * Run ptp daemon: make;sudo ./ptp4l -f /etc/linuxptp/ptp4l.conf -i $if"
 [ -z "$(pgrep ptp4l)" ] && return
 ##############################################################################
 # compare linuxptp-pmc with libpmc-pmc dump
 local -r tlvs='ANNOUNCE_RECEIPT_TIMEOUT CLOCK_ACCURACY CLOCK_DESCRIPTION
    CURRENT_DATA_SET DEFAULT_DATA_SET DELAY_MECHANISM DOMAIN
    LOG_ANNOUNCE_INTERVAL LOG_MIN_PDELAY_REQ_INTERVAL LOG_SYNC_INTERVAL
    PARENT_DATA_SET PRIORITY1 PRIORITY2 SLAVE_ONLY TIMESCALE_PROPERTIES
    TIME_PROPERTIES_DATA_SET TRACEABILITY_PROPERTIES USER_DESCRIPTION
    VERSION_NUMBER PORT_DATA_SET
    TIME_STATUS_NP GRANDMASTER_SETTINGS_NP PORT_DATA_SET_NP PORT_PROPERTIES_NP
    PORT_STATS_NP SUBSCRIBE_EVENTS_NP SYNCHRONIZATION_UNCERTAIN_NP'

 if [ ! -f org.txt ]; then
    echo " * Make org.txt using linuxptp pmc"
    time (for n in $tlvs; do\
    sudo $linuxptp_loc/pmc -u -f /etc/linuxptp/ptp4l.conf "get $n";\
    done) > org.txt
 fi

# real    0m3.189s
# user    0m0.167s
# sys     0m0.138s

 echo " * Make new.txt using libpmc"
 time (for n in $tlvs; do\
 ./pmc -u -f /etc/linuxptp/ptp4l.conf "get $n";done) > new.txt

# real    0m0.085s * 37.517647059
# user    0m0.040s * 4.175
# sys     0m0.033s * 4.18

 local -r cmd="pmc -i $if -f /etc/linuxptp/ptp4l.conf \"get CLOCK_DESCRIPTION\""
 echo " * Test PTP transport layer with linuxptp pmc"
 echo "$linuxptp_loc/$cmd"
 eval "sudo $linuxptp_loc/$cmd"
 echo " * Test PTP transport layer with libpmc"
 echo "./$cmd"
 eval "sudo ./$cmd"
}
###############################################################################
main

manual()
{
# Raw
pmc -i enp7s0 -f /etc/linuxptp/ptp4l.enp7s0.conf "get CLOCK_DESCRIPTION"
# UDS
pmc -u -i enp7s0 -f /etc/linuxptp/ptp4l.enp7s0.conf "get CLOCK_DESCRIPTION"

# udp
pmc -i enp0s25 -f /etc/linuxptp/ptp4l.enp0s25.conf "get CLOCK_DESCRIPTION"
# UDS
pmc -u -i enp0s25 -f /etc/linuxptp/ptp4l.enp0s25.conf "get CLOCK_DESCRIPTION"
}
