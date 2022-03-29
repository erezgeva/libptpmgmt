#!/usr/bin/ruby

# SPDX-License-Identifier: GPL-3.0-or-later
#
# testing for ruby wrapper of libpmc
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#

require 'pmc'

DEF_CFG_FILE = "/etc/linuxptp/ptp4l.conf"

$sk = Pmc::SockUnix.new
$msg = Pmc::Message.new
$buf = Pmc::Buf.new(1000)
$sequence = 0

def nextSequence()
  # Ensure sequence in in range of unsigned 16 bits
  if ++$sequence > 0xffff then
    $sequence = 1
  end
  return $sequence
end

def setPriority1(newPriority1)
  pr1 = Pmc::PRIORITY1_t.new
  pr1.priority1 = newPriority1
  id = Pmc::PRIORITY1
  $msg.setAction(Pmc::SET, id, pr1)
  seq = nextSequence()
  err = $msg.build($buf, seq)
  if err != Pmc::MNG_PARSE_ERROR_OK then
    txt = Pmc::Message.err2str_c(err)
    puts "build error " + txt
  end
  if !$sk.send($buf, $msg.getMsgLen()) then
    puts "send fail"
    return -1
  end
  if !$sk.poll(500) then
    puts "timeout"
    return -1
  end
  cnt = $sk.rcv($buf)
  if cnt <= 0 then
    puts "rcv cnt"
    return -1
  end
  err = $msg.parse($buf, cnt)
  if(err != Pmc::MNG_PARSE_ERROR_OK || $msg.getTlvId() != id ||
     seq != $msg.getSequence()) then
    puts "set fails"
    return -1
  end
  puts "set new priority #{newPriority1} success"
  $msg.setAction(Pmc::GET, id)
  seq = nextSequence()
  err = $msg.build($buf, seq)
  if err != Pmc::MNG_PARSE_ERROR_OK then
    txt = Pmc::Message.err2str_c(err)
    puts "build error " + txt
  end
  if !$sk.send($buf, $msg.getMsgLen()) then
    puts "send fail"
    return -1
  end
  if !$sk.poll(500) then
    puts "timeout"
    return -1
  end
  cnt = $sk.rcv($buf)
  if cnt <= 0 then
    puts "rcv cnt"
    return -1
  end
  err = $msg.parse($buf, cnt)
  if err == Pmc::MNG_PARSE_ERROR_MSG then
    puts "error message"
  elsif err != Pmc::MNG_PARSE_ERROR_OK then
    txt = Pmc::Message.err2str_c(err)
    puts "parse error " + txt
  else
    rid = $msg.getTlvId()
    idstr = Pmc::Message.mng2str_c(rid)
    puts "Get reply for " + idstr
    if rid == id then
      newPr = Pmc.conv_PRIORITY1($msg.getData())
      puts "priority1: #{newPr.priority1}"
      return 0
    end
  end
  return -1
end

def main
  if !$buf.isAlloc() then
    puts "buffer allocation failed"
    return -1
  end
  if ARGV.length > 0 then
    cfg_file = ARGV[0]
  else
    cfg_file = DEF_CFG_FILE
  end
  puts "Use configuration file " + cfg_file
  cfg = Pmc::ConfigFile.new
  if !cfg.read_cfg(cfg_file) then
    puts "fail reading configuration file"
    return -1
  end
  if !$sk.setDefSelfAddress() || !$sk.init() || !$sk.setPeerAddress(cfg) then
    puts "fail init socket"
    return -1
  end
  prms = $msg.getParams()
  prms.self_id.portNumber = $$ & 0xffff # getpid()
  # Verify we can use implementSpecific_e
  # Notice Ruby capitalize first letter
  prms.implementSpecific = Pmc::Linuxptp
  prms.domainNumber = cfg.domainNumber()
  $msg.updateParams(prms)
  $msg.useConfig(cfg)
  id = Pmc::USER_DESCRIPTION
  $msg.setAction(Pmc::GET, id)
  seq = nextSequence()
  err = $msg.build($buf, seq)
  if err != Pmc::MNG_PARSE_ERROR_OK then
    txt = Pmc::Message.err2str_c(err)
    puts "build error " + txt
    return -1
  end
  if !$sk.send($buf, $msg.getMsgLen()) then
    puts "send fail"
    return -1
  end
  # You can get file descriptor with $sk.getFd() and use select
  if !$sk.poll(500) then
    puts "timeout"
    return -1
  end
  cnt = $sk.rcv($buf)
  if cnt <= 0 then
    puts "rcv error #{cnt}"
    return -1
  end

  err = $msg.parse($buf, cnt)
  if err == Pmc::MNG_PARSE_ERROR_MSG then
    puts "error message"
  elsif err != Pmc::MNG_PARSE_ERROR_OK then
    txt = Pmc::Message.err2str_c(err)
    puts "parse error " + txt
  else
    rid = $msg.getTlvId()
    idstr = Pmc::Message.mng2str_c(rid)
    puts "Get reply for " + idstr
    if rid == id then
      user = Pmc.conv_USER_DESCRIPTION($msg.getData())
      puts "get user desc: " + user.userDescription.textField
    end
  end

  # test setting values
  clk_dec = Pmc::CLOCK_DESCRIPTION_t.new
  clk_dec.clockType = 0x800
  physicalAddress = Pmc::Binary.new
  physicalAddress.setBin(0, 0xf1)
  physicalAddress.setBin(1, 0xf2)
  physicalAddress.setBin(2, 0xf3)
  physicalAddress.setBin(3, 0xf4)
  puts "physicalAddress: " + physicalAddress.toId()
  puts "physicalAddress: " + physicalAddress.toHex()
  clk_dec.physicalAddress.setBin(0, 0xf1)
  clk_dec.physicalAddress.setBin(1, 0xf2)
  clk_dec.physicalAddress.setBin(2, 0xf3)
  clk_dec.physicalAddress.setBin(3, 0xf4)
  puts "clk.physicalAddress: " + clk_dec.physicalAddress.toId()
  puts "clk.physicalAddress: " + clk_dec.physicalAddress.toHex()
  puts "manufacturerIdentity: " +
    Pmc::Binary::bufToId(clk_dec.manufacturerIdentity, 3)
  clk_dec.revisionData.textField = "This is a test"
  puts "revisionData: " + clk_dec.revisionData.textField

  # test send
  setPriority1(147)
  setPriority1(153)

  event = Pmc::SUBSCRIBE_EVENTS_NP_t.new
  event.setEvent(Pmc::NOTIFY_TIME_SYNC())
  puts "maskEvent(NOTIFY_TIME_SYNC)=" +
       "#{Pmc::SUBSCRIBE_EVENTS_NP_t.maskEvent(Pmc::NOTIFY_TIME_SYNC())}" +
       ", getEvent(NOTIFY_TIME_SYNC)=" +
       (event.getEvent(Pmc::NOTIFY_TIME_SYNC()) ? 'have' : 'not')
  puts "maskEvent(NOTIFY_PORT_STATE)=" +
       "#{Pmc::SUBSCRIBE_EVENTS_NP_t.maskEvent(Pmc::NOTIFY_PORT_STATE())}" +
       ", getEvent(NOTIFY_PORT_STATE)=" +
       (event.getEvent(Pmc::NOTIFY_PORT_STATE()) ? 'have' : 'not')

  return 0
end

main
$sk.close()

######################################

# If libpmc and ruby-pmc libraries are not installed in system, run with:
#  LD_LIBRARY_PATH=.. RUBYLIB=. ./test.rb
