#!/bin/bash
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
# testing script
###############################################################################
main()
{
 local -r ifName=enp0s25
 local -r cfgFile=/etc/linuxptp/ptp4l.conf
 local -r linuxptpLoc=~/TSN/build/linuxptp
 #local -r useLdPath=t # testing scripts from sys or local
 #local -r useLocal=t # lib pmc from sys or local
 #local -r useLinuxptp=t
 ##############################################################################
 local -r uds=$linuxptpLoc/uds.c
 local -r config=$linuxptpLoc/config.c
 if [ -d "$linuxptpLoc" -a -f "$uds" -a -f "$config" ]; then
    # Add all users for testing (so we can test without using root :-)
    local -r reg='^#define UDS_FILEMODE'
    if [ -n "$(grep "$reg.*GRP)" $uds)" ];then
       sed -i "/$reg/ s#GRP)#GRP|S_IROTH|S_IWOTH)#" $uds
    fi
    # Remove the deprecated message
    local -r reg2='^\s\sfprintf(stderr, "option %s is deprecated,'
    if [ -n "$(grep "$reg2" $config)" ];then
       sed -i "/$reg2/,+1d" $config
    fi
    make --no-print-directory -j -C $linuxptpLoc
 fi
 ##############################################################################
 echo " * build libpmc"
 time make -j
 cat << EOF

 * Starting
 * Run ptp daemon: make;sudo $linuxptpLoc/ptp4l -f $cfgFile -i $ifName

EOF
 [ -z "$(pgrep ptp4l)" ] && return

 ##############################################################################
 case "$useLocal" in
    [aA]) # always
        local -r pmclibtool="./pmc"
         ;;
    [nN]) # None
        local -r pmclibtool="/usr/sbin/pmc.lib"
         ;;
    *) # tentive
        if [ -x /usr/sbin/pmc.lib ]; then
            local -r pmclibtool="/usr/sbin/pmc.lib"
        else
            local -r pmclibtool="./pmc"
        fi
        ;;
 esac
 case "$useLinuxptp" in
    [aA]) # always
        local -r pmctool="$linuxptpLoc"
         ;;
    [nN]) # None
        local -r pmctool=/usr/sbin
         ;;
    *) # tentive
        if [ -x "$linuxptpLoc/pmc" ]; then
            local -r pmctool="$linuxptpLoc"
        else
            local -r pmctool=/usr/sbin
        fi
        ;;
 esac
 ##############################################################################
 # compare linuxptp-pmc with libpmc-pmc dump
 local -r t1=$(mktemp org.XXXXXXXXXX) t2=$(mktemp new.XXXXXXXXXX)
 local n
 local -a cmds
 # all TLVs that are supported by linuxptp ptp4l
 local -r tlvs='ANNOUNCE_RECEIPT_TIMEOUT CLOCK_ACCURACY CLOCK_DESCRIPTION
    CURRENT_DATA_SET DEFAULT_DATA_SET DELAY_MECHANISM DOMAIN
    LOG_ANNOUNCE_INTERVAL LOG_MIN_PDELAY_REQ_INTERVAL LOG_SYNC_INTERVAL
    PARENT_DATA_SET PRIORITY1 PRIORITY2 SLAVE_ONLY TIMESCALE_PROPERTIES
    TIME_PROPERTIES_DATA_SET TRACEABILITY_PROPERTIES USER_DESCRIPTION
    VERSION_NUMBER PORT_DATA_SET
    TIME_STATUS_NP GRANDMASTER_SETTINGS_NP PORT_DATA_SET_NP PORT_PROPERTIES_NP
    PORT_STATS_NP SUBSCRIBE_EVENTS_NP SYNCHRONIZATION_UNCERTAIN_NP'
 local -r setmsg="set PRIORITY2 137"
 local -r verify="get PRIORITY2"
 for n in $tlvs; do cmds+=("get $n"); done

 echo " * Make $t1 using linuxptp pmc"
 time sudo $pmctool/pmc -u -f $cfgFile "${cmds[@]}" | grep -v ^sending: > $t1
 sudo $pmctool/pmc -u -f $cfgFile "$setmsg" >> $t1
 sudo $pmctool/pmc -u -f $cfgFile "$verify" >> $t1

# real  0m0.113s
# user  0m0.009s
# sys   0m0.002s

 printf "\n * Make $t2 using libpmc\n"
 time sudo $pmclibtool -u -f $cfgFile "${cmds[@]}" | grep -v ^sending: > $t2
 sudo $pmclibtool -u -f $cfgFile "$setmsg" >> $t2
 sudo $pmclibtool -u -f $cfgFile "$verify" >> $t2

# real  0m0.019s
# user  0m0.004s
# sys   0m0.011s

 printf "\n * We excpect 'protocolAddress' and 'timeSource' in diff"
 printf "\n * Statistics may aprear  '[tx/rx]_[PTP massage type]'\n\n"
 cmd diff $t1 $t2 | grep '^[0-9-]' -v
 rm $t1 $t2

 ##############################################################################
 # Test script languages wrappers
 # Expected output of testing scripts
 local -r ldPathBase='LD_LIBRARY_PATH=..'
 local -r t3=$(mktemp script.XXXXXXXXXX)
 local ldPathPerl ldPathLua1 ldPathLua2 ldPathLua3 ldPathPython2 ldPathPython3
 case "$useLdPath" in
    [aA]) # always
        ldPathPerl=$ldPathBase
        ldPathLua1=$ldPathBase
        ldPathLua2=$ldPathBase
        ldPathLua3=$ldPathBase
        ldPathPython2=$ldPathBase
        ldPathPython3=$ldPathBase
         ;;
    [nN]) # None
         ;;
    *) # tentive
        probeLibs
        ;;
 esac
 local -r scriptOut=\
'Use configuration file /etc/linuxptp/ptp4l.conf
Get reply for USER_DESCRIPTION
get user desc:
physicalAddress: f1:f2:f3:f4
physicalAddress: f1f2f3f4
clk.physicalAddress: f1:f2:f3:f4
clk.physicalAddress: f1f2f3f4
manufacturerIdentity: 00:00:00
revisionData: This is a test
set new priority 147 success
Get reply for PRIORITY1
priority1: 147
set new priority 153 success
Get reply for PRIORITY1
priority1: 153
'
 printf "\n =====  Run Perl  ===== \n * We except real 'user desc' on '>'\n"
 cd perl
 eval "$ldPathPerl ./test.pl" > ../$t3
 cd ..
 diff <(printf "$scriptOut") $t3 | grep '^[0-9-]' -v
 cd lua
 printf "\n =====  Run lua  ===== \n"
 local i
 for i in 1 2 3; do
    echo " lua 5.$i ---- "
    local -n ldPathLua=ldPathLua$i
    if [ -n "$ldPathLua" ]; then
        ln -sf 5.$i/pmc.so
    else
        rm -f pmc.so
    fi
    eval "$ldPathLua lua5.$i test.lua" | diff - ../$t3
 done
 cd ..
 cd python
 printf "\n =====  Run python  ===== \n"
 for i in 2 3; do
    getFirstFile "$i/*.so"
    if [ -f "$file" ]; then
        echo " $(readlink $(which python$i)) ---- "
        local -n ldPathPython=ldPathPython$i
        rm -f *.so
        [ -n "$ldPathPython" ] && ln -sf $file
        rm -rf pmc.pyc __pycache__
        eval "$ldPathPython python$i test.py" | diff - ../$t3
    fi
 done
 cd ..
 rm $t3
}
###############################################################################
cmd()
{
    echo $*
    $*
}
getFirstFile()
{
    local f
    for f in $@; do
        if [ -f "$f" ]; then
            file="$f"
            return
        fi
    done
    file=''
}
probeLibs()
{
    local file
    local -r mach=$(uname -m)
    local -r fmach="/$mach*"
    getFirstFile "/usr/lib$fmach/perl*/*/auto/PmcLib/PmcLib.so"
    if [ ! -f "$file" ]; then
        ldPathPerl=$ldPathBase
    fi
    for i in 1 2 3; do
        getFirstFile "/usr/lib$fmach/lua/5.$i/pmc.so"
        if [ ! -f "$file" ]; then
            local -n ld=ldPathLua$i
            ld=$ldPathBase
        fi
        getFirstFile "/usr/lib/python$i*/dist-packages/_pmc.*$mach*.so"
        if [ ! -f "$file" ]; then
            local -n ld=ldPathPython$i
            ld=$ldPathBase
        fi
    done
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
