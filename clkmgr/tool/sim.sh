#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2024 Intel Corporation.
#
# Simulation using clknetsim
###############################################################################

clk_client()
{
 c_node='c_node + 1'
 rm -f $CLKNETSIM_TMPDIR/log.$c_node $CLKNETSIM_TMPDIR/conf.$c_node
 LD_PRELOAD=$CLKNETSIM_PATH/clknetsim.so CLKNETSIM_NODE=$c_node\
   CLKNETSIM_SOCKET=$CLKNETSIM_TMPDIR/sock LD_LIBRARY_PATH=.libs\
   $@ "${c_args[@]}" &> $CLKNETSIM_TMPDIR/log.$c_node &
 local lastpid=$!
 disown $lastpid
 client_pids+=" $lastpid"
}

main()
{
 cd "$(realpath "$(dirname "$0")/../..")"
 local -r CLKNETSIM_PATH=../clknetsim
 local -r CLKNETSIM_TMPDIR=clkmgr/sim
 if ! [[ -f "$CLKNETSIM_PATH/clknetsim.bash" ]]; then
   echo "clknetsim is missing"
   return
 fi
 local a_patch
 if [[ -f ../linuxptp/ptp4l ]]; then
   a_patch=:../linuxptp
 elif [[ -z "$(which ptp4l)" ]]; then
   echo "ptp4l is missing"
   return
 fi
 if [[ -n "$a_patch" ]]; then
   local PATH=$PATH$a_patch
 fi
 local client_pids
 local -a c_args
 local -i c_node=0
 . $CLKNETSIM_PATH/clknetsim.bash

 export CLKNETSIM_UNIX_SUBNET=2

 # Test configuraton
 generate_config4 '1' '1 2 3' 0.01\
   '(sum (* 1e-9 (normal)))'\
   '(* 1e-8 (exponential))'
 echo 'node3_start = 100' >> $CLKNETSIM_TMPDIR/conf
 local -r c_if=eth0

 # Start clients
 c_node='c_node + 1'
 start_client $c_node ptp4l '' '' "-i $c_if"

 c_node='c_node + 1'
 start_client $c_node ptp4l '' '' "-i $c_if"

#c_args=('GET TIME_STATUS_NP' 'GET TIME_PROPERTIES_DATA_SET' 'GET PORT_DATA_SET')
#clk_client pmc
#clk_client ptp-tools/pmc "-i $c_if"
 # TODO First step: disable connect_chrony() in clkmgr/proxy/main.cpp
 clk_client clkmgr/proxy/clkmgr_proxy

 # Run test with clknetsim server
 set +e
 start_server $c_node -l 110
 set -e
 cat $CLKNETSIM_TMPDIR/log.3
}
main "$@"
