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
 while getopts 'l:as' opt; do
   case $opt in
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
 if ! [[ -f defs.mk ]]; then
   echo "You must configure before you can compile!"
   return
 fi
 if ! [[ -f $linuxptpLoc/makefile ]]; then
   echo "You must provide linuxptp source folder!"
   return
 fi
 make -C $linuxptpLoc
 make ptp4l_sim
 local tmp=`mktemp -d`
 local -r fcfg=$tmp/ptp4l.conf
 local -r uds=$tmp/ptp4l
 cat << EOF > $fcfg
[global]
clientOnly 1
use_syslog 0
uds_address $uds
userDescription testing with ptp4l
EOF
 # The interface is ignored in the ptp4l_sim.so wrapper.
 # But ptp4l and phc_ctl except an interface.
 local -r opts="-f $fcfg -i eth0"
 LD_PRELOAD=./objs/ptp4l_sim.so $linuxptpLoc/ptp4l $opts &
 #pid=$!
 sleep 1
 echo "#####  Test Start  #####"
 tools/testing.sh $opts -p $pass_args -l $linuxptpLoc
 sleep 1
 kill %1
 wait
}
main "$@"
