#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation.
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2025 Intel Corporation.
#
# Simulation using clknetsim
###############################################################################
equit()
{
 echo "$@"
 exit 1
}
c_mk()
{
   make -C "$1" --no-print-directory
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
 [[ -f "$CLKNETSIM_PATH/clknetsim.bash" ]] || equit 'clknetsim is missing'
 # Build clknetsim
 if ! [[ -x "$CLKNETSIM_PATH/clknetsim" ]] ||\
    ! [[ -f "$CLKNETSIM_PATH/clknetsim.so" ]]; then
   c_mk "$CLKNETSIM_PATH"
 fi
}
probe_linuxptp()
{
 if [[ -z "$(which ptp4l)" ]]; then
   [[ -f ../linuxptp/ptp4l.c ]] || equit 'ptp4l is missing'
   [[ -x ../linuxptp/ptp4l ]] || c_mk ../linuxptp
   a_path+=:../linuxptp
 fi
}
probe_chrony()
{
 # Chrony comes from:
 #  https://chrony-project.org
 #  https://gitlab.com/chrony/chrony
 if [[ -z "$(which chronyd)" ]]; then
   [[ -f ../chrony/ntp_io_linux.c ]] || equit 'chronyd is missing'
   if ! [[ -x ../chrony/chronyd ]]; then
     cd ../chrony
     ./configure
     make
     cd "$base"
   fi
   a_path+=:../chrony
 fi
}
make_clkmgr()
{
 if [[ -z "$(which clkmgr_proxy)" ]]; then
   # Make local
   if ! [[ -x clkmgr/proxy/clkmgr_proxy ]]; then
     make CXXFLAGS="$CXXFLAGS -U_FORTIFY_SOURCE" --no-print-directory
   fi
   a_path+=:clkmgr/proxy
 fi
 if ! [[ -x clkmgr/sample/clkmgr_test ]]; then
   # Make sample
   if ! [[ -f libclkmgr.la ]]; then
     make CXXFLAGS="$CXXFLAGS -U_FORTIFY_SOURCE" --no-print-directory libclkmgr.la
   fi
   c_mk clkmgr/sample
 fi
 a_path+=:clkmgr/sample
}
main()
{
 local -ir test_mode=${1:-0}
 local -r base="$(realpath "$(dirname "$0")/../..")"
 # clknetsim location
 local -r CLKNETSIM_PATH=../clknetsim
 # clknetsim folder for simulation logs and configuration files
 local -r CLKNETSIM_TMPDIR=clkmgr/sim
 cd "$base"
 prepare_clknetsim
 local a_path
 probe_linuxptp
 probe_chrony
 make_clkmgr
 # Add path to linuxptp, chrony, clkmgr_proxy and clkmgr_test
 local PATH=$PATH$a_path

 # Prepare simulation envirounment
 local client_pids
 local -i c_node=0
 local -r c_if=eth0
 local -ir run_time='250' # time limit in seconds for clknetsim server
 local -i ptp4l_node chronyd_node
 export CLKNETSIM_UNIX_SUBNET=2
 rm -f $CLKNETSIM_TMPDIR/log.[0-9]* $CLKNETSIM_TMPDIR/conf.[0-9]*

 # includes the clknetsim script
 . $CLKNETSIM_PATH/clknetsim.bash

 # Test configuraton:
 #  Node 1: ptp4l GrandMaster
 #  Node 2: ptp4l follower
 #  Node 3: chronyd
 #  Node 4: Clock Manager Proxy
 #  Node 5: Clock Manager Client
 #  subnet: Node 1 <-> Node 2
 #          Node 2 <-> Node 4
 #          Node 3 <-> Node 4
 #          Node 4 <-> Node 5
 #  Initial offset: 0.01 second
 #  Frequency expr: sum of scaled random numbers from standard normal distribution
 #  Delay expr: scaled random number from exponential distribution
 generate_config4 '1 4 5' '1 2 | 2 4 | 3 4 | 4 5' 0.01\
   '(sum (* 1e-9 (normal)))' '(* 1e-8 (exponential))'

 # Start Clock Manager Proxy, which is Node 4 at 20th second
 echo 'node4_start = 20' >> $CLKNETSIM_TMPDIR/conf

 # Start Clock Manager Client, which is Node 5 at 30th second
 echo 'node5_start = 30' >> $CLKNETSIM_TMPDIR/conf

 # Simulate a time jump of 0.1s at the 100th second
 echo 'node1_step = (* 0.1 (equal 0.1 (sum 1.0) 100))' >> "$CLKNETSIM_TMPDIR/conf"

 # Simulate network down between 150s and 200s for ptp4l GrandMaster
 local -r node1_delay2="$(printf '%s %s'\
   '(+ (* 1e-8 (exponential))'\
   '(* -1 (equal 0.1 (min time 200) time) (equal 0.1 (max time 150) time)))')"
 echo "node1_delay2 = $node1_delay2" >> "$CLKNETSIM_TMPDIR/conf"

 # Trap signals
 trap c_ctrl INT # Ctrl^C
 trap c_sig TERM # Normal Termination
 trap c_sig QUIT # Quit from keyboard
 trap c_sig HUP  # Hangup detected
 trap c_sig EXIT # on exit
 trap c_sig ERR  #

 # Start ptp4l GrandMaster as Node 1
 c_node='c_node + 1'
 start_client $c_node ptp4l '' '' "-i $c_if"

 # Start ptp4l follower as Node 2
 c_node='c_node + 1'
 start_client $c_node ptp4l '' '' "-i $c_if"
 ptp4l_node=$c_node

 # Start chronyd as Node 3 with PHC0 as reference clock and 2^-3 poll interval
 c_node='c_node + 1'
 start_client $c_node chronyd 'refclock PHC /dev/ptp0 poll -3 dpoll -1'
 chronyd_node=$c_node

 c_node='c_node + 1'
 export LD_LIBRARY_PATH=.libs

 # test_mode: Controls the simulation mode for clknetsim.
 # Usage: Pass as the first argument to the script, e.g. ./sim.sh 1
 # If no argument is provided, defaults to 0 (normal mode).
 # Possible values:
 #   0 - ptp4l single domain
 #   1 - chronyd
 case $test_mode in
  0)
    start_client $c_node clkmgr_proxy "
      {
        \"timeBases\": [{
          \"timeBaseName\": \"Global Clock\",
          \"ptp4l\": {
            \"interfaceName\": \"$c_if\",
            \"udsAddr\": \"/clknetsim/unix/${ptp4l_node}:1\",
            \"domainNumber\": 0,
            \"transportSpecific\": 0
          }
        }]
      }" '' ' -l 1'
    ;;
  1)
    start_client $c_node clkmgr_proxy "
      {
        \"timeBases\": [{
          \"timeBaseName\": \"Global Clock\",
          \"chrony\": { \"udsAddr\": \"/clknetsim/unix/${chronyd_node}:1\" }
        }]
      }" '' ' -l 1'
    ;;
  *)
   equit "Error: Unsupported test_mode: '$test_mode'."
   ;;
 esac

 # Start Clock Manager client as Node 5 with
 #  - 10 nanoseconds chrony offset threshold
 #  - wait 1 second for event changes
 #  - 0 second idle time (never sleep)
 echo "Starting Clock Manager simulation (mode: $test_mode)"
 c_node='c_node + 1'
 start_client $c_node clkmgr '' '_test' '-m 10 -t 1 -i 0'

 # Run test with clknetsim server
 set +e
 start_server $c_node -l $run_time -n $CLKNETSIM_UNIX_SUBNET
 set -e
 client_pids=''
 echo -e "\nlog1:"
 cat $CLKNETSIM_TMPDIR/log.1
 echo -e "\nlog2:"
 cat $CLKNETSIM_TMPDIR/log.2
 echo -e "\nlog3:"
 cat $CLKNETSIM_TMPDIR/log.3
 echo -e "\nlog4:"
 cat $CLKNETSIM_TMPDIR/log.4
 echo -e "\nlog5:"
 grep -v -E '^\s*$|Waiting|No event|sleep for' $CLKNETSIM_TMPDIR/log.5
}
main "$@"
