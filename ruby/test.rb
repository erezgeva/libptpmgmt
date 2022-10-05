#!/usr/bin/ruby
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2021 Erez Geva
#
# testing for ruby wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
###############################################################################

require 'ptpmgmt'

class MyDisp < Ptpmgmt::MessageDispatcher
  def PRIORITY1_h(msg, tlv, tlv_id)
    puts "Get reply for " + tlv_id
    puts "priority1: #{tlv.priority1}"
  end
  def USER_DESCRIPTION_h(msg, tlv, tlv_id)
    puts "Get reply for " + tlv_id
    puts "get user desc: " + tlv.userDescription.textField
  end
end
class MyBuild < Ptpmgmt::MessageBuilder
  def pr(pr)
    @pr = pr
  end
  def PRIORITY1_b(msg, tlv)
    tlv.priority1 = @pr
    return true
  end
end

DEF_CFG_FILE = "/etc/linuxptp/ptp4l.conf"

$sk = Ptpmgmt::SockUnix.new
$msg = Ptpmgmt::Message.new
$buf = Ptpmgmt::Buf.new(1000)
$opt = Ptpmgmt::Options.new
$dispacher = MyDisp.new
$builder = MyBuild.new($msg)
$sequence = 0

def printError(msg)
  if Ptpmgmt::Error.isError() then
    puts Ptpmgmt::Error.getError()
  else
    puts msg
  end
  return -1
end

def nextSequence()
  # Ensure sequence in in range of unsigned 16 bits
  if ++$sequence > 0xffff then
    $sequence = 1
  end
  return $sequence
end

def setPriority1(newPriority1)
  useBuild = true
  id = Ptpmgmt::PRIORITY1
  if useBuild then
    $builder.pr(newPriority1)
    $builder.buildTlv(Ptpmgmt::SET, id)
  else
    pr1 = Ptpmgmt::PRIORITY1_t.new
    pr1.priority1 = newPriority1
    $msg.setAction(Ptpmgmt::SET, id, pr1)
  end
  seq = nextSequence()
  err = $msg.build($buf, seq)
  if err != Ptpmgmt::MNG_PARSE_ERROR_OK then
    txt = Ptpmgmt::Message.err2str_c(err)
    puts "build error " + txt
  end
  if !$sk.send($buf, $msg.getMsgLen()) then
    return printError("send fail")
  end
  if !useBuild then
    $msg.clearData()
  end
  if !$sk.poll(500) then
    return printError("timeout")
  end
  cnt = $sk.rcv($buf)
  if cnt <= 0 then
    return printError("rcv cnt")
  end
  err = $msg.parse($buf, cnt)
  if err != Ptpmgmt::MNG_PARSE_ERROR_OK || $msg.getTlvId() != id ||
     seq != $msg.getSequence() then
    puts "set fails"
    return -1
  end
  puts "set new priority #{newPriority1} success"
  $msg.setAction(Ptpmgmt::GET, id)
  seq = nextSequence()
  err = $msg.build($buf, seq)
  if err != Ptpmgmt::MNG_PARSE_ERROR_OK then
    txt = Ptpmgmt::Message.err2str_c(err)
    puts "build error " + txt
  end
  if !$sk.send($buf, $msg.getMsgLen()) then
    return printError("send fail")
  end
  if !$sk.poll(500) then
    return printError("timeout")
  end
  cnt = $sk.rcv($buf)
  if cnt <= 0 then
    return printError("rcv cnt")
  end
  err = $msg.parse($buf, cnt)
  if err == Ptpmgmt::MNG_PARSE_ERROR_MSG then
    puts "error message"
  elsif err != Ptpmgmt::MNG_PARSE_ERROR_OK then
    txt = Ptpmgmt::Message.err2str_c(err)
    puts "parse error " + txt
  else
    $dispacher.callHadler($msg, $msg.getTlvId(), $msg.getData())
    return 0
  end
  return -1
end

def main
  if !$buf.isAlloc() then
    puts "buffer allocation failed"
    return -1
  end
  if $opt.parse_options([$0] + ARGV) != Ptpmgmt::Options::OPT_DONE then
    puts "fail parsing command line"
    return -1
  end
  cfg_file = $opt.val('f')
  if cfg_file == "" then
    cfg_file = DEF_CFG_FILE
  end
  puts "Use configuration file " + cfg_file
  cfg = Ptpmgmt::ConfigFile.new
  if !cfg.read_cfg(cfg_file) then
    puts "fail reading configuration file"
    return -1
  end
  if !$sk.setDefSelfAddress() || !$sk.init() || !$sk.setPeerAddress(cfg) then
    return printError("fail init socket")
  end
  prms = $msg.getParams()
  prms.self_id.portNumber = $$ & 0xffff # getpid()
  # Verify we can use implementSpecific_e
  # Notice Ruby capitalize first letter
  prms.implementSpecific = Ptpmgmt::Linuxptp
  prms.domainNumber = cfg.domainNumber()
  $msg.updateParams(prms)
  $msg.useConfig(cfg)
  id = Ptpmgmt::USER_DESCRIPTION
  $msg.setAction(Ptpmgmt::GET, id)
  seq = nextSequence()
  err = $msg.build($buf, seq)
  if err != Ptpmgmt::MNG_PARSE_ERROR_OK then
    txt = Ptpmgmt::Message.err2str_c(err)
    puts "build error " + txt
    return -1
  end
  if !$sk.send($buf, $msg.getMsgLen()) then
    return printError("send fail")
  end
  # You can get file descriptor with $sk.fileno() and use select
  if !$sk.poll(500) then
    return printError("timeout")
  end
  cnt = $sk.rcv($buf)
  if cnt <= 0 then
    return printError("rcv error #{cnt}")
  end

  err = $msg.parse($buf, cnt)
  if err == Ptpmgmt::MNG_PARSE_ERROR_MSG then
    puts "error message"
  elsif err != Ptpmgmt::MNG_PARSE_ERROR_OK then
    txt = Ptpmgmt::Message.err2str_c(err)
    puts "parse error " + txt
  else
    $dispacher.callHadler($msg)
  end

  # test setting values
  clk_dec = Ptpmgmt::CLOCK_DESCRIPTION_t.new
  clk_dec.clockType = 0x800
  physicalAddress = Ptpmgmt::Binary.new
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
    Ptpmgmt::Binary::bufToId(clk_dec.manufacturerIdentity, 3)
  clk_dec.revisionData.textField = "This is a test"
  puts "revisionData: " + clk_dec.revisionData.textField

  # test send
  setPriority1(147)
  setPriority1(153)

  event = Ptpmgmt::SUBSCRIBE_EVENTS_NP_t.new
  event.setEvent(Ptpmgmt::NOTIFY_TIME_SYNC())
  puts "maskEvent(NOTIFY_TIME_SYNC)=" +
       "#{Ptpmgmt::SUBSCRIBE_EVENTS_NP_t.maskEvent(Ptpmgmt::NOTIFY_TIME_SYNC())}" +
       ", getEvent(NOTIFY_TIME_SYNC)=" +
       (event.getEvent(Ptpmgmt::NOTIFY_TIME_SYNC()) ? 'have' : 'not')
  puts "maskEvent(NOTIFY_PORT_STATE)=" +
       "#{Ptpmgmt::SUBSCRIBE_EVENTS_NP_t.maskEvent(Ptpmgmt::NOTIFY_PORT_STATE())}" +
       ", getEvent(NOTIFY_PORT_STATE)=" +
       (event.getEvent(Ptpmgmt::NOTIFY_PORT_STATE()) ? 'have' : 'not')

  # test SigEvent that represent std::vector<SLAVE_TX_EVENT_TIMESTAMPS_rec_t>
  # See std_vectors.md for more information
  evnts = Ptpmgmt::SigEvent.new
  e = Ptpmgmt::SLAVE_TX_EVENT_TIMESTAMPS_rec_t.new
  e.sequenceId = 1
  e.eventEgressTimestamp.fromFloat(4.5)
  evnts << e
  puts "Events size #{evnts.size()}, seq[0]=#{evnts[0].sequenceId}, ts[0]=" +
        evnts[0].eventEgressTimestamp.string()
  return 0
end

main
$sk.close()

# LD_PRELOAD=../libptpmgmt.so RUBYLIB=. ./test.rb
