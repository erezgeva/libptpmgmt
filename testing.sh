#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# testing script
#
# probe libraries according to Debian locations
###############################################################################
main()
{
 local file i opt n cmds use_asan
 # Default values
 local -r def_ifName=enp0s31f6
 local -r def_cfgFile=/etc/linuxptp/ptp4l.conf
 local -r def_linuxptpLoc=../linuxptp
 # environment variables
 local -A envVals
 envVals['ifName']='PTP_IF_NAME'
 envVals['cfgFile']='PTP_CFG_FILE'
 envVals['linuxptpLoc']='PTP_PATH'
 ##############################################################################
 while getopts 'i:c:l:as' opt; do
   case $opt in
     i) local -r ifName="$OPTARG" ;;
     c) local -r cfgFile="$OPTARG" ;;
     l) local -r linuxptpLoc="$OPTARG" ;;
     a) use_asan=true ;;
     s) local -r probeSystem=true ;; # Use system libraries
   esac
 done
 for n in ifName cfgFile linuxptpLoc; do
   local -n var=$n
   if [[ -z "$var" ]]; then
     local -n env="${envVals["$n"]}"
     if [[ -n "$env" ]]; then
       eval "local -r $n='$env'"
     else
       eval "local -r $n=\"\$def_$n\""
     fi
   fi
 done
 ##############################################################################
 if ! [[ -f "$cfgFile" ]]; then
   echo "Linux PTP configuration file is missing"
   exit -1
 fi
 ##############################################################################
 # Root does not need sudo, we assume proper root :-)
 if [[ $(id -u) -ne 0 ]]; then
   set +e
   sudo id &> /dev/null
   local -ir ret=$?
   set -e
   if [[ $ret -ne 0 ]]; then
     echo "Script need to run pmc tool with sudo"
     echo "But sudo is not available!!"
     exit -1
   fi
   local -r sudo='sudo '
 fi
 ##############################################################################
 local -r uds_address_full="$(grep uds_address "$cfgFile")"
 local -r udsreg='[[:space:]]*uds_address[[:space:]]+'
 if [[ $uds_address_full =~ ^$udsreg(.*[^[:space:]]+) ]]; then
   local -r udsFile="${BASH_REMATCH[1]}"
 else
   # Use default value
   local -r udsFile='/var/run/ptp4l'
 fi
 if [[ -S "$udsFile" ]]; then
   $sudo chmod ga+wr "$udsFile"
   # We change UDS
   local -r setUds=true
 fi
 ##############################################################################
 local -r uds="$linuxptpLoc/uds.c"
 if [[ -f "$uds" ]]; then
   if [[ -z "$setUds" ]]; then
     # Add all users for testing (so we can test without using root :-)
     local -r reg='^#define UDS_FILEMODE'
     [[ -z "$(grep "$reg.*GRP)" "$uds")" ]] ||\
       sed -i "/$reg/ s#GRP).*#GRP|S_IROTH|S_IWOTH)#" "$uds"
   fi
   make --no-print-directory -j -C "$linuxptpLoc"
   local -r pmctool="$sudo\"$linuxptpLoc/pmc\""
   local -r phcctrltool="$sudo$linuxptpLoc/phc_ctl"
   local -r ptpLocCfrm=true
 else
   [[ -n "$setUds" ]] || local -r useSudo="$sudo"
   local -r pmctool="$sudo/usr/sbin/pmc"
   local -r phcctrltool="$sudo/usr/sbin/phc_ctl"
 fi
 ##############################################################################
 # script languages source
 local -r mach=$(uname -m)
 local -r fmach="/$mach*"
 local ldPathPerl ldPathRuby ldPathPython3 ldPathPhp ldPathTcl\
       ldPathJson needCmpl ldPathLua1 ldPathLua2 ldPathLua3
 # Lua 5.4 need lua-posix version 35
 local -r luaVersions='1 2 3'
 local -r pyVersions='3'
 getFirstFile "/usr/lib$fmach/libptpmgmt.so*"
 if [[ -f "$file" ]] && [[ -n "$probeSystem" ]]; then
   probeLibs
 else
   local -r ldPath='LD_PRELOAD=./libptpmgmt.so'
   needCmpl=y
   # We need all!
   ldPathPerl="$ldPath PERL5LIB=perl"
   ldPathRuby="$ldPath RUBYLIB=ruby"
   for i in $luaVersions; do
     eval "ldPathLua$i='$ldPath LUA_CPATH=\"lua/5.$i/?.so;;\"'"
   done
   for i in $pyVersions; do
     eval "ldPathPython$i=\"$ldPath PYTHONPATH=python:python/$i\""
   done
   ldPathPhp="$ldPath PHPRC=php"
   ldPathTcl="$ldPath TCLLIBPATH=tcl"
   ldPathJson="$ldPath LD_LIBRARY_PATH=. PERL5LIB=perl"
 fi
 local -r runOptions="-u -f $cfgFile"
 ##############################################################################
 local -r instPmcLib=/usr/sbin/pmc-ptpmgmt
 if [[ -x $instPmcLib ]] && [[ -n "$probeSystem" ]]; then
   local -r pmclibtool=$instPmcLib
 else
   local -r pmclibtool=./tools/pmc
   needCmpl=y
 fi
 local -r instPhcCtlLib=/usr/sbin/phc_ctl-ptpmgmt
 if [[ -x $instPhcCtlLib ]] && [[ -n "$probeSystem" ]]; then
   local -r phcctrllibtool=$instPhcCtlLib
 else
   local -r phcctrllibtool=./tools/phc_ctl
 fi
 ##############################################################################
 if [[ -n "$needCmpl" ]]; then
   if ! [[ -f defs.mk ]]; then
     echo "You must configure before you can compile!"
     return
   fi
   printf " * build libptpmgmt\n"
   if [[ -n "$use_asan" ]]; then
     ASAN_OPTIONS='verbosity=1:strict_string_checks=1'
     ASAN_OPTIONS+=':detect_stack_use_after_return=1'
     ASAN_OPTIONS+=':check_initialization_order=1:strict_init_order=1'
     ASAN_OPTIONS+=':detect_invalid_pointer_pairs=2'
     export ASAN_OPTIONS
     time make -j USE_ASAN=1
   else
     use_asan=false
     time make -j
   fi
 else
   use_asan=false
 fi
 if ! $use_asan && [[ -n "$(which valgrind)" ]]; then
   local use_valgrind=true
 else
   local use_valgrind=false
 fi
 if [[ -z "$(pgrep ptp4l)" ]]; then
   printf "\n * Run ptp daemon"
   if [[ -n "$ptpLocCfrm" ]]; then
     printf ":\n   cd \"$(realpath $linuxptpLoc)\" %s\n\n"\
            "&& make && sudo ./ptp4l -f $cfgFile -i $ifName"
   fi
   printf "\n\n"
   return
 fi
 compare_pmc
 ! $use_asan || return
 test_scripts
 test_json
 test_phc_ctl
 printf "\n #####  Test End  #####\n"
}
###############################################################################
compare_pmc()
{ # compare linuxptp-pmc with libptpmgmt-pmc dump
 # all TLVs that are supported by linuxptp ptp4l
 local -r tlvs='ANNOUNCE_RECEIPT_TIMEOUT CLOCK_ACCURACY CLOCK_DESCRIPTION
   CURRENT_DATA_SET DEFAULT_DATA_SET DELAY_MECHANISM DOMAIN
   LOG_ANNOUNCE_INTERVAL LOG_MIN_PDELAY_REQ_INTERVAL LOG_SYNC_INTERVAL
   PARENT_DATA_SET PRIORITY1 PRIORITY2 SLAVE_ONLY TIMESCALE_PROPERTIES
   TIME_PROPERTIES_DATA_SET TRACEABILITY_PROPERTIES USER_DESCRIPTION
   VERSION_NUMBER PORT_DATA_SET
   TIME_STATUS_NP GRANDMASTER_SETTINGS_NP PORT_DATA_SET_NP PORT_PROPERTIES_NP
   PORT_STATS_NP SUBSCRIBE_EVENTS_NP SYNCHRONIZATION_UNCERTAIN_NP MASTER_ONLY
   PORT_SERVICE_STATS_NP UNICAST_MASTER_TABLE_NP PORT_HWCLOCK_NP'
 local -r setmsg="set PRIORITY2 137"
 local -r verify="get PRIORITY2"
 for n in $tlvs; do cmds+=" \"get $n\"";done

 if $use_asan; then
   printf "\n * Run libptpmgmt pmc with AddressSanitizer\n"
   eval "$useSudo$pmclibtool $runOptions $cmds"
   return
 fi

 printf "\n * Create output with linuxptp pmc\n"
 local pmcOut=$(eval "$pmctool $runOptions \"$setmsg\"")
 pmcOut+="\n"
 pmcOut+=$(eval "$pmctool $runOptions \"$verify\"")
 pmcOut+="\n"
 pmcOut+=$(time eval "$pmctool $runOptions $cmds" | grep -v ^sending:)

 # real  0m0.113s
 # user  0m0.009s
 # sys   0m0.002s

 printf "\n * Create output with libptpmgmt\n"
 local libptpOut=$(eval "$useSudo$pmclibtool $runOptions \"$setmsg\"")
 libptpOut+="\n"
 libptpOut+=$(eval "$useSudo$pmclibtool $runOptions \"$verify\"")
 libptpOut+="\n"
 libptpOut+=$(time eval "$useSudo$pmclibtool $runOptions $cmds" |\
   grep -v ^sending:)

 # real  0m0.003s
 # user  0m0.001s
 # sys   0m0.004s

 printf "\n * We expect 'protocolAddress', %s\n%s\n\n"\
          "'timeSource' and 'portState' difference"\
          " * Statistics may apprear"
 diff <(printf "$pmcOut") <(printf "$libptpOut") | grep '^[0-9-]' -v

 if $use_valgrind; then
   printf "\n * Valgrid test"
   $useSudo valgrind --read-inline-info=yes $pmclibtool $runOptions $cmds |&\
     sed -n '/ERROR SUMMARY/ {s/.*ERROR SUMMARY//;p}'
 fi
}
###############################################################################
test_scripts()
{ # Test script languages wrappers
 local perlOut
 # Expected output of testing scripts
 local -r scriptOut=\
"Use configuration file $cfgFile
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
maskEvent(NOTIFY_TIME_SYNC)=2, getEvent(NOTIFY_TIME_SYNC)=have
maskEvent(NOTIFY_PORT_STATE)=1, getEvent(NOTIFY_PORT_STATE)=not
Events size 1, seq[0]=1, ts[0]=4.500000000
"
 enter perl
 printf "\n * We except real 'user desc' on '>'\n"
 diff <(printf "$scriptOut") <(printf "$perlOut\n") | grep '^[0-9-]' -v
 enter ruby
 enter lua
 enter python
 enter php
 enter tcl
}
do_perl()
{
 perlOut="$(time eval "$ldPathPerl $useSudo perl/test.pl $runOptions")"
}
test_json()
{ # Use perl
 printf "\n =====  Test JSON  ===== \n\n"
 eval "$ldPathJson $useSudo ./testJson.pl | jsonlint"
 if $use_valgrind; then
   printf "\n * Valgrid test of testJson.pl"
   eval "$ldPathJson $useSudo valgrind --read-inline-info=yes\
     ./testJson.pl" |& sed -n '/ERROR SUMMARY/ {s/.*ERROR SUMMARY//;p}'
 fi
}
do_ruby()
{
 time eval "$ldPathRuby $useSudo ruby/test.rb $runOptions" |\
   diff - <(printf "$perlOut\n")
}
do_lua()
{
 for i in $luaVersions; do
   printf "\n lua 5.$i ---- \n"
   local -n need="ldPathLua$i"
   time eval "$need $useSudo lua5.$i lua/test.lua $runOptions" |\
     diff - <(printf "$perlOut\n")
 done
}
do_python()
{
 for i in $pyVersions; do
   if [[ -z "$(which python$i)" ]]; then
     continue
   fi
   local -n need="ldPathPython$i"
   if [[ -n "$need" ]]; then
     [[ -f python/$i/_ptpmgmt.so ]] || continue
     eval "$need py${i}compile python/ptpmgmt.py"
   fi
   printf "\n $(readlink $(command -v python$i)) ---- \n"
   time eval "$need $useSudo python$i python/test.py $runOptions" |\
     diff - <(printf "$perlOut\n")
   if [[ -n "$need" ]]; then
     eval "py${i}clean python"
   fi
 done
}
parse_phc_out()
{
 parseOut=''
 readarray <<< "$@"
 for n in "${MAPFILE[@]}"; do
   if [[ $n =~ ^$matchPhc:[[:space:]](.*[^$'\n']) ]]; then
     local s1="${BASH_REMATCH[1]}"
     if [[ $s1 =~ (.*[[:digit:]]+\.[[:digit:]])[[:digit:]]+([[:space:]].*) ]]
       then parseOut+="${BASH_REMATCH[1]}${BASH_REMATCH[2]}\n"
     else
       parseOut+="$s1\n"
     fi
   fi
 done
}
test_phc_ctl()
{ # Use python3
 printf "\n =====  Test phc_ctl  ===== \n\n"
 if [[ -z "$(which python3)" ]]; then
   echo "python3 is not installed on system!!!"
   return
 fi
 if [[ -n "$ldPathPython3" ]]; then
   if ! [[ -f python/3/_ptpmgmt.so ]]; then
     echo "Fail to find python3 library!!!"
     return
   fi
   eval "$ldPathPython3 py3compile python/ptpmgmt.py"
 fi
 local -r matchPhc='phc_ctl[^\[]*\[[[:digit:]]+\.[[:digit:]]+\]'
 local -r runPhc="$ifName freq 20000000 set 0 wait 4 adj 4 get"
 local parseOut runOut
 printf " * Create output with linuxptp phc_ctrl, wait 4 seconds ...\n"
 runOut=$(eval "$phcctrltool $runPhc" 2>&1)
 parse_phc_out "$runOut"
 if [[ -z "$parseOut" ]]; then
   echo "No output !!!"
   return
 fi
 local phcOut="$parseOut"
 printf " * Create output with libptpmgmt, wait 4 seconds ...\n"
 runOut=$(eval "$sudo $ldPathPython3 $phcctrllibtool $runPhc")
 parse_phc_out "$runOut"
 if [[ -z "$parseOut" ]]; then
   echo "No output !!!"
   return
 fi
 runOut=$(diff <(printf "$phcOut") <(printf "$parseOut"))
 if [[ -n "$runOut" ]]; then
   echo "$runOut" | grep '^[0-9-]' -v
 fi
 if $use_valgrind; then
   printf "\n * Valgrid test of phc_ctl"
   eval "$sudo $ldPathPython3 PYTHONMALLOC=malloc"\
     " valgrind --read-inline-info=yes"\
     " $phcctrllibtool $ifName freq 500000000 set 0 wait 0.1 adj 4 get" |&\
     sed -n '/ERROR SUMMARY/ {s/.*ERROR SUMMARY//;p}'
 fi
 [[ -z "$ldPathPython3" ]] || py3clean python
}
do_php()
{
 [[ -z "$ldPathPhp" ]] || php/php_ini.sh php/
 time eval "$ldPathPhp $useSudo php/test.php $runOptions" |\
   diff - <(printf "$perlOut\n")
}
do_tcl()
{
 if [[ -n "$ldPathTcl" ]]; then
   if ! [[ -f tcl/ptpmgmt.so ]]; then
     echo "Fail to find tcl library!!!"
     return
   fi
   tcl/pkgIndex_tcl.sh tcl
 fi
 time eval "$ldPathTcl $useSudo tcl/test.tcl $runOptions" |\
   diff - <(printf "$perlOut\n")
 sed -i 's/^load .*/package require ptpmgmt/' tcl/test.tcl
}
enter()
{
 printf "\n =====  Run $1  ===== \n"
 do_$1
}
getFirstFile()
{
 local f
 for f in $@; do
   if [[ -f "$f" ]]; then
     file="$f"
     return
   fi
 done
 file=''
}
probeLibs()
{
 local -i jsonCount=0
 getFirstFile "/usr/lib$fmach/libptpmgmt_fastjson.so.*"
 if [[ -f "$file" ]]; then
   jsonCount='jsonCount + 1'
 fi
 getFirstFile "/usr/lib$fmach/libptpmgmt_jsonc.so.0.*"
 if [[ -f "$file" ]]; then
   jsonCount='jsonCount + 1'
 fi
 # One from JSON plugin is sufficient
 if [[ $jsonCount -eq 0 ]];  then
   needCmpl=y
   ldPathJson="LD_LIBRARY_PATH=."
 fi
 getFirstFile "/usr/lib$fmach/perl*/*/auto/PtpMgmtLib/PtpMgmtLib.so"
 if ! [[ -f "$file" ]]; then
   needCmpl=y
   ldPathPerl="PERL5LIB=perl"
   ldPathJson+=" PERL5LIB=perl"
 fi
 file="$(ruby -rrbconfig -e \
   'puts RbConfig::CONFIG["vendorarchdir"]')/ptpmgmt.so"
 if ! [[ -f "$file" ]]; then
   needCmpl=y
   ldPathRuby="RUBYLIB=ruby"
 fi
 for i in $luaVersions; do
   getFirstFile "/usr/lib$fmach/liblua5.$i-ptpmgmt.so*"
   if ! [[ -f "$file" ]]; then
     # Lua comes in a single package for all versions,
     # so a single probing flag is sufficient.
     needCmpl=y
     eval "ldPathLua$i='LUA_CPATH=\"lua/5.$i/?.so;;\"'"
   fi
 done
 for i in $pyVersions; do
   getFirstFile "/usr/lib/python$i*/dist-packages/_ptpmgmt.*$mach*.so"
   if ! [[ -f "$file" ]]; then
     eval "ldPathPython$i=\"PYTHONPATH=python:python/$i\""
   fi
 done
 [[ -z "$needPython3" ]] || needCmpl=y
 if ! [[ -f "$(php-config --extension-dir)/ptpmgmt.so" ]]; then
   needCmpl=y
   ldPathPhp="PHPRC=php"
 fi
 getFirstFile "/usr/lib/tcltk/*/ptpmgmt/ptpmgmt.so"
 if ! [[ -f "$file" ]]; then
   needCmpl=y
   ldPathTcl="TCLLIBPATH=tcl"
 fi
}
###############################################################################
main "$@"

manual()
{
 # Test network layer sockets on real target
net=enp7s0;pmc -i $net -f /etc/linuxptp/ptp4l.$net.conf "get CLOCK_DESCRIPTION"
 # Using UDS
net=enp7s0;pmc -u -f /etc/linuxptp/ptp4l.$net.conf "get CLOCK_DESCRIPTION"
}
