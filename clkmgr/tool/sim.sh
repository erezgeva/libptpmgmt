#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2024 Intel Corporation.
#
# Simulation using clknetsim
###############################################################################
equit()
{
 echo "$@"
 exit 1
}
s_cp()
{
  local s="$1"
  [[ -f "$s" ]] || return
  if [[ -f "$sim_tgt/$s" ]]; then
    if ! cmp -s "$s" "$sim_tgt/$s"; then
      cp "$s" "$sim_tgt/$s"
      build=true
    fi
  else
    cp "$s" "$sim_tgt/"
    build=true
  fi
}
clk_client()
{
 LD_PRELOAD=$CLKNETSIM_PATH/clknetsim.so CLKNETSIM_NODE=$c_node\
  CLKNETSIM_SOCKET=$CLKNETSIM_TMPDIR/sock LD_LIBRARY_PATH=.libs\
  $@ &> $CLKNETSIM_TMPDIR/log.$c_node &
 local lastpid=$!
 disown $lastpid
 client_pids+=" $lastpid"
}
clk_proxy()
{
 local uds_srv=$uds_base/$1
 LD_PRELOAD=$CLKNETSIM_PATH/clknetsim.so CLKNETSIM_NODE=$c_node\
  CLKNETSIM_SOCKET=$CLKNETSIM_TMPDIR/sock LD_LIBRARY_PATH=.libs\
  CLKMGR_UDS_ADDRESS=$uds_srv\
  clkmgr/proxy/clkmgr_proxy &> $CLKNETSIM_TMPDIR/log.$c_node &
 local lastpid=$!
 disown $lastpid
 client_pids+=" $lastpid"
}
c_sig()
{
 if [[ -n "$client_pids" ]]; then
  echo "Check $client_pids"
  set +e
  local p
  for p in $client_pids; do
    if pgrep $p; then
      echo "kill $p"
      kill $p
    fi
  done
  client_pids=''
  # Ensure server is killed
  if pgrep clknetsim; then
    echo 'kill clknetsim'
    killall clknetsim
  fi
 fi
}
c_ctrl()
{
  echo ''
  c_sig
}
prepare_clknetsim()
{
 # Clock simulation comes from:
 #  https://gitlab.com/chrony/clknetsim
 #  https://github.com/mlichvar/clknetsim
 [[ -f "$base/$CLKNETSIM_PATH/clknetsim.bash" ]] || equit 'clknetsim is missing'
 # Build clknetsim
 local sim_tgt=$base/$CLKNETSIM_PATH/patches
 mkdir -p $sim_tgt
 local n build=false
 # Copy patches
 cd $base/clkmgr/tool/clknetsim
 for n in *
 do s_cp "$n";done
 cd "$base/$CLKNETSIM_PATH"
 if $build || ! [[ -f .pc/applied-patches ]] ||
    ! cmp patches/series .pc/applied-patches; then
   quilt pop -af || true
   quilt push -a
   build=true
 fi
 if $build || ! [[ -x clknetsim ]] || ! [[ -f clknetsim.so ]]; then
   make
 fi
}
main()
{
 local -r base="$(realpath "$(dirname "$0")/../..")"
 local -r CLKNETSIM_PATH=../clknetsim
 local -r CLKNETSIM_TMPDIR=clkmgr/sim
 local -r uds_base=/clknetsim/unix
 prepare_clknetsim
 cd "$base"
 local a_patch
 # Probe linuxptp
 if [[ -f ../linuxptp/ptp4l.c ]]; then
   [[ -x ../linuxptp/ptp4l ]] || make -C ../linuxptp --no-print-directory
   a_patch=:../linuxptp
 elif [[ -z "$(which ptp4l)" ]]; then
   equit 'ptp4l is missing'
 fi
 # Probe chrony
 if [[ -z "$(which chronyd)" ]]; then
   [[ -f '../chrony/ntp_io_linux.c' ]] || equit 'chronyd is missing'
   if ! [[ -x ../chrony/chronyd ]]; then
     cd ../chrony
     ./configure
     make
     cd "$base"
   fi
   a_patch+=:../chrony
 fi
 # Add path to linuxptp and chrony
 [[ -z "$a_patch" ]] || local PATH=$PATH$a_patch

 # Make local and sample
 if ! [[ clkmgr/proxy/clkmgr_proxy ]]; then
   make
 fi
 if ! [[ clkmgr/sample/clkmgr_test ]]; then
   make -C clkmgr/sample --no-print-directory
 fi

 # Prepare simulation envirounment
 local client_pids
 local -i c_node=0
 export CLKNETSIM_UNIX_SUBNET=2
 rm -f $CLKNETSIM_TMPDIR/log.[0-9]* $CLKNETSIM_TMPDIR/conf.[0-9]*
 . $CLKNETSIM_PATH/clknetsim.bash

 # Test configuraton
 generate_config4 '1' '1 2 | 2 4 | 3 4 | 4 5' 0.01\
    '(sum (* 1e-9 (normal)))'\
    '(* 1e-8 (exponential))'
 echo 'node5_start = 100' >> $CLKNETSIM_TMPDIR/conf

#generate_config4 '1' '1 2' 0.01\
#  '(sum (* 1e-9 (normal)))'\
#  '(* 1e-8 (exponential))'

 local -r c_if=eth0
 local -ir run_time='60 * 5' # time limit in seconds for clknetsim server

 # Trap signals
 trap c_ctrl INT # Ctrl^C
 trap c_sig TERM # Normal Termination
 trap c_sig QUIT # Quit from keyboard
 trap c_sig HUP  # Hangup detected
 trap c_sig EXIT # on exit
 trap c_sig ERR  #

 # UDS = $uds_base/<node>:<port start from 1>

 # Start clients
 c_node='c_node + 1'
 start_client $c_node ptp4l '' '' "-i $c_if"

 c_node='c_node + 1'
 start_client $c_node ptp4l '' '' "-i $c_if"
# "uds_address $uds_base/$c_node:1"

 c_node='c_node + 1'
 start_client $c_node chronyd
# "refclock PHC /dev/ptp0 poll -6 dpoll -1"

 c_node='c_node + 1'
 clk_proxy 2:1

# TODO fix crash
#c_node='c_node + 1'
#clk_client clkmgr/sample/clkmgr_test

 # Run test with clknetsim server
 set +e
 start_server $c_node -l $run_time -n 2
 set -e
 client_pids=''
 cat $CLKNETSIM_TMPDIR/log.4
}
main "$@"
