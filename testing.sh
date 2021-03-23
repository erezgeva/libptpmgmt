#!/bin/bash
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
# testing script
###############################################################################
cmd()
{
    echo $*
    $*
}
main()
{
 local -r ifName=enp0s25
 local -r cfgFile=/etc/linuxptp/ptp4l.conf
 local -r linuxptpLoc=~/TSN/build/linuxptp
 # Add all users for testing (so we can test without using root :-)
 local -r uds=$linuxptpLoc/uds.c
 local -r reg='^#define UDS_FILEMODE'
 if [ -n "$(grep "$reg.*GRP)" $uds)" ];then
    sed -i "/$reg/ s#GRP)#GRP|S_IROTH|S_IWOTH)#" $uds
 fi
 # Remove the deprecated message
 local -r config=$linuxptpLoc/config.c
 local -r reg2='^\s\sfprintf(stderr, "option %s is deprecated,'
 if [ -n "$(grep "$reg2" $config)" ];then
    sed -i "/$reg2/,+1d" $config
 fi
 make --no-print-directory -j -C $linuxptpLoc
 echo " * build libpmc"
 time make -j
 cat << EOF

 * Starting
 * Run ptp daemon: make;sudo $linuxptpLoc/ptp4l -f $cfgFile -i $ifName

EOF
 [ -z "$(pgrep ptp4l)" ] && return
 ##############################################################################
 # compare linuxptp-pmc with libpmc-pmc dump
 local -r t1=$(mktemp org.XXXXXXXXXX) t2=$(mktemp new.XXXXXXXXXX)
 local -r t3=$(mktemp script.XXXXXXXXXX)
 local n
 local -a cmds
 local -r tlvs='ANNOUNCE_RECEIPT_TIMEOUT CLOCK_ACCURACY CLOCK_DESCRIPTION
    CURRENT_DATA_SET DEFAULT_DATA_SET DELAY_MECHANISM DOMAIN
    LOG_ANNOUNCE_INTERVAL LOG_MIN_PDELAY_REQ_INTERVAL LOG_SYNC_INTERVAL
    PARENT_DATA_SET PRIORITY1 PRIORITY2 SLAVE_ONLY TIMESCALE_PROPERTIES
    TIME_PROPERTIES_DATA_SET TRACEABILITY_PROPERTIES USER_DESCRIPTION
    VERSION_NUMBER PORT_DATA_SET
    TIME_STATUS_NP GRANDMASTER_SETTINGS_NP PORT_DATA_SET_NP PORT_PROPERTIES_NP
    PORT_STATS_NP SUBSCRIBE_EVENTS_NP SYNCHRONIZATION_UNCERTAIN_NP'
 for n in $tlvs; do cmds+=("get $n"); done

 echo " * Make $t1 using linuxptp pmc"
 time sudo $linuxptpLoc/pmc -u -f $cfgFile "${cmds[@]}" |\
    grep -v ^sending: > $t1

# real  0m0.113s
# user  0m0.009s
# sys   0m0.002s

 printf "\n * Make $t2 using libpmc\n"
 time sudo ./pmc -u -f $cfgFile "${cmds[@]}" | grep -v ^sending: > $t2

# real  0m0.019s
# user  0m0.004s
# sys   0m0.011s

 echo ""
 cmd diff $t1 $t2

 ##############################################################################
 # Test script languages wrappers
 printf "\n =====  Run Perl  ===== \n"
 cd perl
 LD_LIBRARY_PATH=.. ./test.pl | tee ../$t3
 cd ..
 cd lua
 printf "\n =====  Run lua  ===== \n"
 local i
 for i in 1 2 3; do
    echo " lua 5.$i ---- "
    ln -sf 5.$i/pmc.so
    LD_LIBRARY_PATH=.. lua5.$i test.lua | diff - ../$t3
 done
 cd ..
 cd python
 printf "\n =====  Run python  ===== \n"
 for i in 2 3; do
    if [ -f $i/_pmc.so ]; then
        echo " $(readlink $(which python$i)) ---- "
        ln -sf $i/_pmc.so
        rm -rf pmc.pyc __pycache__
        LD_LIBRARY_PATH=.. python$i test.py | diff - ../$t3
    fi
 done
 cd ..
 rm $t1 $t2 $t3
}
###############################################################################
main

manual()
{
# Test network layer sockets on real target
# network layer
net=enp7s0 ; pmc -i $net -f /etc/linuxptp/ptp4l.$net.conf "get CLOCK_DESCRIPTION"
# Using UDS
net=enp7s0 ; pmc -u -f /etc/linuxptp/ptp4l.$net.conf "get CLOCK_DESCRIPTION"
}
