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
SIZE = 2000

$sk = Pmc::SockUnix.new
$msg = Pmc::Message.new
# Create buffer for sending
# And convert buffer to buffer pointer
$pbuf = Pmc.conv_buf("X" * SIZE)
$sequence = 0

def setPriority1(newPriority1)
  pr1 = Pmc::PRIORITY1_t.new
  pr1.priority1 = newPriority1
  id = Pmc::PRIORITY1
  $msg.setAction(Pmc::SET, id, pr1)
  err = $msg.build($pbuf, SIZE, ++$sequence)
  if err != Pmc::MNG_PARSE_ERROR_OK then
    txt = Pmc::Message.err2str_c(err)
    puts "build error " + txt
  end
  if !$sk.send($pbuf, $msg.getMsgLen()) then
    puts "send fail"
    return
  end
  if !$sk.poll(500) then
    puts "timeout"
    return
  end
  cnt = $sk.rcv($pbuf, SIZE)
  if cnt <= 0 then
    puts "rcv cnt"
    return -1
  end
  err = $msg.parse($pbuf, cnt)
  if(err != Pmc::MNG_PARSE_ERROR_OK || $msg.getTlvId() != id ||
     $sequence != $msg.getSequence()) then
    puts "set fails"
    return -1
  end
  puts "set new priority #{newPriority1} success"
  $msg.setAction(Pmc::GET, id)
  err = $msg.build($pbuf, SIZE, ++$sequence)
  if err != Pmc::MNG_PARSE_ERROR_OK then
    txt = Pmc::Message.err2str_c(err)
    puts "build error " + txt
  end
  if !$sk.send($pbuf, $msg.getMsgLen()) then
    puts "send fail"
    return
  end
  if !$sk.poll(500) then
    puts "timeout"
    return
  end
  cnt = $sk.rcv($pbuf, SIZE)
  if cnt <= 0 then
    puts "rcv cnt"
    return -1
  end
  err = $msg.parse($pbuf, cnt)
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
    end
  end
end

def main
  if ARGV.length > 0 then
    cfg_file = ARGV[0]
  else
    cfg_file = DEF_CFG_FILE
  end
  puts "Use configuration file " + cfg_file
  cfg = Pmc::ConfigFile.new
  if !cfg.read_cfg(cfg_file) then
    puts "fail reading configuration file"
    return
  end
  if !$sk.setDefSelfAddress() || !$sk.init() || !$sk.setPeerAddress(cfg) then
    puts "fail init socket"
    return
  end
  prms = $msg.getParams()
  prms.self_id.portNumber = $$
  $msg.updateParams(prms)
  id = Pmc::USER_DESCRIPTION
  $msg.setAction(Pmc::GET, id)
  err = $msg.build($pbuf, SIZE, ++$sequence)
  if err != Pmc::MNG_PARSE_ERROR_OK then
    txt = Pmc::Message.err2str_c(err)
    puts "build error " + txt
    return
  end
  if !$sk.send($pbuf, $msg.getMsgLen()) then
    puts "send fail"
    return
  end
  # You can get file descriptor with $sk.getFd() and use select
  if !$sk.poll(500) then
    puts "timeout"
    return
  end
  cnt = $sk.rcv($pbuf, SIZE)
  if cnt <= 0 then
    puts "rcv error #{cnt}"
    return
  end


  err = $msg.parse($pbuf, cnt)
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
  puts "manufacturerIdentity: " + Pmc::Binary::bufToId(clk_dec.manufacturerIdentity, 3)
  clk_dec.revisionData.textField = "This is a test";
  puts "revisionData: " + clk_dec.revisionData.textField;

  # test send
  setPriority1(147)
  setPriority1(153)
end

main
$sk.close()

######################################

# If libpmc and ruby-pmc libraries are not installed in system, run with:
#  LD_LIBRARY_PATH=.. RUBYLIB=. ./test.rb
