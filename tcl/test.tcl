#!/usr/bin/tclsh

# SPDX-License-Identifier: GPL-3.0-or-later
#
# testing for tcl wrapper of libpmc
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
# @copyright GNU General Public License 3.0 or later
#

package require pmc

set DEF_CFG_FILE "/etc/linuxptp/ptp4l.conf"
SockUnix sk
Message msg
Buf buf
set sequence 0

proc nextSequence {} {
  global sequence
  incr sequence
  # Ensure sequence in in range of unsigned 16 bits
  if {$sequence > 0xffff} {
    $sequence = 1
  }
  return $sequence
}

proc setPriority1 {newPriority1} {
  global GET SET PRIORITY1 MNG_PARSE_ERROR_OK MNG_PARSE_ERROR_MSG
  set pr1 [PRIORITY1_t]
  $pr1 configure -priority1 $newPriority1
  set id $PRIORITY1
  msg setAction $SET $id $pr1
  set seq [ nextSequence ]
  set err [ msg build buf $seq ]
  if {$err != $MNG_PARSE_ERROR_OK} {
    set txt [ Message_err2str_c $err ]
    puts "build error $txt"
    return -1
  }
  if { ! [ sk send buf [ msg getMsgLen ] ] } {
    puts "send fail"
    return -1
  }
  if { ! [ sk poll 500 ] } {
    puts "timeout"
    return -1
  }
  set cnt [ sk rcvBuf buf ]
  if {$cnt <= 0} {
    puts "rcv error $cnt"
    return -1
  }
  set err [ msg parse buf $cnt ]
  if {$err != $MNG_PARSE_ERROR_OK || [ msg getTlvId ] != $id || \
      $seq != [ msg getSequence ] } {
    puts "set fails"
    return -1
  }
  puts "set new priority $newPriority1 success"
  msg setAction $GET $id
  set seq [ nextSequence ]
  set err [ msg build buf $seq ]
  if {$err != $MNG_PARSE_ERROR_OK} {
    set txt [ Message_err2str_c $err ]
    puts "build error $txt"
    return -1
  }
  if { ! [ sk send buf [ msg getMsgLen ] ] } {
    puts "send fail"
    return -1
  }
  if { ! [ sk poll 500 ] } {
    puts "timeout"
    return -1
  }
  set cnt [ sk rcvBuf buf ]
  if {$cnt <= 0} {
    puts "rcv error $cnt"
    return -1
  }
  set err [ msg parse buf $cnt ]
  if {$err == $MNG_PARSE_ERROR_MSG} {
    puts "error message"
  } elseif {$err != $MNG_PARSE_ERROR_OK} {
    set txt [ Message_err2str_c $err ]
    puts "parse error $txt"
  } else {
    set rid [ msg getTlvId ]
    set idstr [ Message_mng2str_c $rid ]
    puts "Get reply for $idstr"
    if {$rid == $id} {
      set newPr [ conv_PRIORITY1 [ msg getData ] ]
      puts "priority1: [ $newPr cget -priority1 ]"
      return 0
    }
  }
  return -1
}

proc main {cfg_file} {
  global GET MNG_PARSE_ERROR_OK MNG_PARSE_ERROR_MSG linuxptp USER_DESCRIPTION
  if { ! [ buf alloc 1000 ] } {
    puts "buffer allocation failed"
    return -1
  }
  set cfg [ConfigFile]
  if { ! [ $cfg read_cfg $cfg_file ] } {
    puts "fail reading configuration file"
    return -1
  }
  if { ! [ sk setDefSelfAddress ] || ! [ sk init ]  || \
       ! [ sk setPeerAddress $cfg ] } {
    puts "fail init socket"
    return -1
  }
  set prms [ msg getParams ]
  [ $prms cget -self_id] configure -portNumber [ expr [ pid ] & 0xffff ]
  # Verify we can use implementSpecific_e
  $prms configure -implementSpecific $linuxptp
  $prms configure -domainNumber [ $cfg domainNumber ]
  msg updateParams $prms
  msg useConfig $cfg
  set id $USER_DESCRIPTION
  msg setAction $GET $id
  set seq [ nextSequence ]
  set err [ msg build buf $seq ]
  if {$err != $MNG_PARSE_ERROR_OK} {
    set txt [ Message_err2str_c $err ]
    puts "build error $txt"
    return -1
  }
  if { ! [ sk send buf [ msg getMsgLen ] ] } {
    puts "send fail"
    return -1
  }
  # You can get file descriptor with sk->getFd and use select
  if { ! [ sk poll 500 ] } {
    puts "timeout"
    return -1
  }
  set cnt [ sk rcvBuf buf ]
  if {$cnt <= 0} {
    puts "rcv error $cnt"
    return -1
  }
  set err [ msg parse buf $cnt ]
  if {$err == $MNG_PARSE_ERROR_MSG} {
    puts "error message"
  } elseif {$err != $MNG_PARSE_ERROR_OK} {
    set txt [ Message_err2str_c $err ]
    puts "parse error $txt"
  } else {
    set rid [ msg getTlvId ]
    set idstr [ Message_mng2str_c $rid ]
    puts "Get reply for $idstr"
    if {$rid == $id} {
      set user [ conv_USER_DESCRIPTION [ msg getData ] ]
      puts "get user desc: [ [ $user cget -userDescription ] cget -textField ]"
    }
  }

  # test setting values
  set clk_dec [CLOCK_DESCRIPTION_t]
  $clk_dec configure -clockType 0x800
  set physicalAddress [Binary]
  $physicalAddress setBin 0 0xf1
  $physicalAddress setBin 1 0xf2
  $physicalAddress setBin 2 0xf3
  $physicalAddress setBin 3 0xf4
  puts "physicalAddress: [ $physicalAddress toId ]"
  puts "physicalAddress: [ $physicalAddress toHex ]"
  [ $clk_dec cget -physicalAddress ] setBin 0 0xf1
  [ $clk_dec cget -physicalAddress ] setBin 1 0xf2
  [ $clk_dec cget -physicalAddress ] setBin 2 0xf3
  [ $clk_dec cget -physicalAddress ] setBin 3 0xf4
  puts "clk.physicalAddress: [ [ $clk_dec cget -physicalAddress ] toId ]"
  puts "clk.physicalAddress: [ [ $clk_dec cget -physicalAddress ] toHex ]"
  set val [ Binary_bufToId [ $clk_dec cget -manufacturerIdentity ] 3 ]
  puts "manufacturerIdentity: $val"
  [ $clk_dec cget -revisionData ] configure -textField "This is a test"
  puts "revisionData: [ [ $clk_dec cget -revisionData ] cget -textField ]"

  # test send
  setPriority1 147
  setPriority1 153
  return 0
}

if {$::argc > 0} {
  set cfg_file [lindex $::argv 0]
} else {
  set cfg_file $DEF_CFG_FILE
}

puts "Use configuration file $cfg_file"
main $cfg_file
sk close

######################################

# If libpmc library is not installed in system, run with:
#  LD_LIBRARY_PATH=.. ./test.tcl
