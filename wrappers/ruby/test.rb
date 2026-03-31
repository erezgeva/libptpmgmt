#!/usr/bin/ruby
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# testing for ruby wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
###############################################################################

require 'ptpmgmt'
include Ptpmgmt

class MyDisp < MessageDispatcher
  def PRIORITY1_h(msg, tlv, tlv_id)
    puts "Get reply for " + tlv_id
    puts "priority1: #{tlv.priority1}"
  end
  def USER_DESCRIPTION_h(msg, tlv, tlv_id)
    puts "Get reply for " + tlv_id
    puts "get user desc: " + tlv.userDescription.textField
  end
end
class MyBuild < MessageBuilder
  def pr(pr)
    @pr = pr
  end
  def PRIORITY1_b(msg, tlv)
    tlv.priority1 = @pr
    return true
  end
end

DEF_CFG_FILE = "/etc/linuxptp/ptp4l.conf"

$sk = SockUnix.new
$msg = Message.new
$buf = Buf.new(1000)
$opt = Options.new
$dispacher = MyDisp.new
$builder = MyBuild.new($msg)
$sequence = 0

def printError(msg)
  if Error.isError() then
    puts Error.getError()
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
  id = PRIORITY1
  if useBuild then
    $builder.pr(newPriority1)
    $builder.buildTlv(SET, id)
  else
    pr1 = PRIORITY1_t.new
    pr1.priority1 = newPriority1
    $msg.setAction(SET, id, pr1)
  end
  seq = nextSequence()
  err = $msg.build($buf, seq)
  if err != MNG_PARSE_ERROR_OK then
    txt = Message.err2str_c(err)
    puts "build error " + txt
  end
  if !$sk.send($buf, $msg.getMsgLen()) then
    return printError("send fail")
  end
  $msg.clearData()
  if !$sk.poll(500) then
    return printError("timeout")
  end
  cnt = $sk.rcv($buf)
  if cnt <= 0 then
    return printError("rcv cnt")
  end
  err = $msg.parse($buf, cnt)
  if err != MNG_PARSE_ERROR_OK || $msg.getTlvId() != id ||
     seq != $msg.getSequence() then
    puts "set fails"
    return -1
  end
  puts "set new priority #{newPriority1} success"
  $msg.setAction(GET, id)
  seq = nextSequence()
  err = $msg.build($buf, seq)
  if err != MNG_PARSE_ERROR_OK then
    txt = Message.err2str_c(err)
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
  if err == MNG_PARSE_ERROR_MSG then
    puts "error message"
  elsif err != MNG_PARSE_ERROR_OK then
    txt = Message.err2str_c(err)
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
  if $opt.parse_options([$0] + ARGV) != Options::OPT_DONE then
    puts "fail parsing command line"
    return -1
  end
  cfg_file = $opt.val('f')
  if cfg_file == "" then
    cfg_file = DEF_CFG_FILE
  end
  puts "Use configuration file " + cfg_file
  cfg = ConfigFile.new
  if !cfg.read_cfg(cfg_file) then
    puts "fail reading configuration file"
    return -1
  end
  if !$sk.setDefSelfAddress() || !$sk.init() || !$sk.setPeerAddress(cfg) then
    return printError("fail init socket")
  end
  prms = $msg.getParams()
  if cfg.haveSpp() then
    if !$msg.useAuth(cfg) then
      return printError("fail using Authentication")
    end
    prms.sendAuth = true
  end
  prms.self_id.portNumber = $$ & 0xffff # getpid()
  # Verify we can use implementSpecific_e
  # Notice Ruby capitalize first letter
  prms.implementSpecific = Linuxptp
  prms.domainNumber = cfg.domainNumber()
  $msg.updateParams(prms)
  $msg.useConfig(cfg)
  id = USER_DESCRIPTION
  $msg.setAction(GET, id)
  seq = nextSequence()
  err = $msg.build($buf, seq)
  if err != MNG_PARSE_ERROR_OK then
    txt = Message.err2str_c(err)
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
  if err == MNG_PARSE_ERROR_MSG then
    puts "error message"
  elsif err != MNG_PARSE_ERROR_OK then
    txt = Message.err2str_c(err)
    puts "parse error " + txt
  else
    $dispacher.callHadler($msg)
  end

  # test setting values
  clk_dec = CLOCK_DESCRIPTION_t.new
  clk_dec.clockType = 0x800
  physicalAddress = Binary.new
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
    Binary::bufToId(clk_dec.manufacturerIdentity, 3)
  clk_dec.revisionData.textField = "This is a test"
  puts "revisionData: " + clk_dec.revisionData.textField

  # test send
  setPriority1(147)
  setPriority1(153)

  event = SUBSCRIBE_EVENTS_NP_t.new
  event.setEvent(Ptpmgmt.NOTIFY_TIME_SYNC())
  puts "getEvent(NOTIFY_TIME_SYNC)=" +
       (event.getEvent(Ptpmgmt.NOTIFY_TIME_SYNC()) ? 'have' : 'not')
  puts "getEvent(NOTIFY_PORT_STATE)=" +
       (event.getEvent(Ptpmgmt.NOTIFY_PORT_STATE()) ? 'have' : 'not')

  # test SigEvent that represent std::vector<SLAVE_TX_EVENT_TIMESTAMPS_rec_t>
  # See std_vectors.md for more information
  evnts = SigEvent.new
  e = SLAVE_TX_EVENT_TIMESTAMPS_rec_t.new
  e.sequenceId = 1
  e.eventEgressTimestamp.fromFloat(4.5)
  evnts << e
  puts "Events size #{evnts.size()}, seq[0]=#{evnts[0].sequenceId}, ts[0]=" +
        evnts[0].eventEgressTimestamp.string()
  return 0
end

main
$sk.close()

# LD_PRELOAD=../../.libs/libptpmgmt.so RUBYLIB=. ./test.rb
