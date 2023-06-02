#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Run ptp4l with a dummy clock and call the testing
###############################################################################
main()
{
 cd "$(dirname "$(realpath "$0")")/.."
 # Default values
 local -r def_linuxptpLoc=../linuxptp
 # environment variables
 ##############################################################################
 local pass_args
 while getopts 'cl:as' opt; do
   case $opt in
     c) local do_config=true ;;
     l) local -r linuxptpLoc="$OPTARG" ;;
     a) pass_args+=" -a" ;; # Use AddressSanitizer
     s) pass_args+=" -s" ;; # Use system libraries
   esac
 done
 if [[ -z "$linuxptpLoc" ]]; then
   local -n env='PTP_PATH'
   if [[ -n "$env" ]]; then
     local -r linuxptpLoc="$env"
   else
     local -r linuxptpLoc="$def_linuxptpLoc"
   fi
 fi
 ##############################################################################
 if ! [[ -f $linuxptpLoc/makefile ]]; then
   echo "You must provide linuxptp source folder!"
   return
 fi
 if [[ -n "$do_config" ]]; then
   make config
 elif ! [[ -f defs.mk ]]; then
   echo "You must configure before you can compile!"
   return
 fi
 make -C $linuxptpLoc
 make ptp4l_sim
 if [[ $(id -u) -eq 0 ]]; then
   local -r cfg=/tmp/ptp4l.conf
 else
   local tmp=`mktemp -d`
   local -r cfg=$tmp/ptp4l.conf
   local -r uds=$tmp/ptp4l
 fi
 cat << EOF > $cfg
[global]
clientOnly 1
use_syslog 0
userDescription testing with ptp4l
EOF
 if [[ -n "$uds" ]]; then
   printf "uds_address $uds\nuds_ro_address $uds.ro\n" >> $cfg
 fi
 # The interface is ignored in the ptp4l_sim.so wrapper.
 # But ptp4l and phc_ctl except an interface.
 local -r opts="-f $cfg -i eth0"
 LD_PRELOAD=./objs/ptp4l_sim.so $linuxptpLoc/ptp4l $opts &
 #pid=$!
 sleep 1
 tools/testing.sh $opts -p $pass_args -l $linuxptpLoc
 sleep 1
 kill %1
 wait
}
main "$@"
