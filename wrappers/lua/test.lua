-- SPDX-License-Identifier: GPL-3.0-or-later
-- SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>

--[[
 - testing for lua wrapper of libptpmgmt

 - @author Erez Geva <ErezGeva2@@gmail.com>
 - @copyright © 2021 Erez Geva
 - ]]

require 'ptpmgmt'
require 'posix'
local unistd = require 'posix.unistd'

myDisp = {} -- inherit from ptpmgmt.MessageDispatcher
function myDisp:new()
  local obj = ptpmgmt.MessageDispatcher:new()
  setmetatable(self, {__index = ptpmgmt.MessageDispatcher})
  setmetatable(obj, self)
  self.__index = self
  return obj
end
function myDisp:PRIORITY1_h(msg, tlv, tlv_id)
  print("Get reply for " .. tlv_id)
  print(string.format("priority1: %d", tlv.priority1))
end
function myDisp:USER_DESCRIPTION_h(msg, tlv, tlv_id)
  print("Get reply for " .. tlv_id)
  print("get user desc: " .. tlv.userDescription.textField)
end
myBuild = {} -- Inherit from ptpmgmt.MessageBuilder
function myBuild:new(msg)
  local obj = ptpmgmt.MessageBuilder:new(msg)
  setmetatable(self, {__index = ptpmgmt.MessageBuilder})
  setmetatable(obj, self)
  self.__index = self
  return obj
end
function myBuild:PRIORITY1_b(msg, tlv)
  tlv.priority1 = self.pr
  return true
end

DEF_CFG_FILE = "/etc/linuxptp/ptp4l.conf"

sk = ptpmgmt.SockUnix()
msg = ptpmgmt.Message()
buf = ptpmgmt.Buf(1000)
opt = ptpmgmt.Options()
dispacher = myDisp:new()
builder = myBuild:new(msg)
sequence = 0

function printError(msg)
  if(ptpmgmt.Error.isError()) then
    print(ptpmgmt.Error.getError())
  else
    print(msg)
  end
  return -1
end

function nextSequence()
  -- Ensure sequence in in range of unsigned 16 bits
  sequence = sequence + 1
  if(sequence > 0xffff) then
    sequence = 1
  end
  return sequence
end

function setPriority1(newPriority1)
  local useBuild = true
  local txt
  local id = ptpmgmt.PRIORITY1
  local pr1
  if(useBuild) then
    builder.pr = newPriority1
    builder:buildTlv(ptpmgmt.SET, id)
  else
    pr1 = ptpmgmt.PRIORITY1_t()
    pr1.priority1 = newPriority1
    msg:setAction(ptpmgmt.SET, id, pr1)
  end
  local seq = nextSequence()
  local err = msg:build(buf, seq)
  if(err ~= ptpmgmt.MNG_PARSE_ERROR_OK) then
    txt = ptpmgmt.Message.err2str_c(err)
    print("build error ", txt)
  end
  if(not sk:send(buf, msg:getMsgLen())) then
    return printError("send fail")
  end
  msg:clearData()
  if(not sk:poll(500)) then
    return printError("timeout")
  end
  local cnt = sk:rcv(buf)
  if(cnt <= 0) then
    return printError("rcv cnt")
  end
  err = msg:parse(buf, cnt)
  if(err ~= ptpmgmt.MNG_PARSE_ERROR_OK or msg:getTlvId() ~= id or
     seq ~= msg:getSequence()) then
    print "set fails"
    return -1
  end
  print("set new priority " .. newPriority1 .. " success")
  msg:setAction(ptpmgmt.GET, id)
  seq = nextSequence()
  err = msg:build(buf, seq)
  if(err ~= ptpmgmt.MNG_PARSE_ERROR_OK) then
    txt = ptpmgmt.Message.err2str_c(err)
    print("build error ", txt)
  end
  if(not sk:send(buf, msg:getMsgLen())) then
    return printError("send fail")
  end
  if(not sk:poll(500)) then
    return printError("timeout")
  end
  local cnt = sk:rcv(buf)
  if(cnt <= 0) then
    return printError("rcv cnt")
  end
  err = msg:parse(buf, cnt)
  if(err == ptpmgmt.MNG_PARSE_ERROR_MSG) then
    print "error Message"
  elseif(err ~= ptpmgmt.MNG_PARSE_ERROR_OK) then
    txt = ptpmgmt.Message.err2str_c(err)
    print("parse error ", txt)
  else
    dispacher:callHadler(msg, msg:getTlvId(), msg:getData())
    return 0
  end
  return -1
end

function main()
  if(not buf:isAlloc()) then
    print "buffer allocation failed"
    return -1
  end
  table.insert(arg, 1, "myname")
  local ret = opt:parse_options(arg)
  if(ret ~= ptpmgmt.Options.OPT_DONE) then
    print "fail parsing command line"
    return -1
  end
  local cfg_file = opt:val('f')
  if(cfg_file == '') then
    cfg_file = DEF_CFG_FILE
  end
  print("Use configuration file " .. cfg_file)
  local cfg = ptpmgmt.ConfigFile()
  if(not cfg:read_cfg(cfg_file)) then
    return printError("fail reading configuration file")
  end
  if(not sk:setDefSelfAddress() or not sk:init() or
     not sk:setPeerAddress(cfg)) then
    return printError("fail init socket")
  end
  local prms = msg:getParams()
  if(cfg:haveSpp()) then
    if(not msg:useAuth(cfg)) then
      return printError("fail using Authentication");
    end
    prms.sendAuth = true
  end
  -- When using Lua 5.3, you can use "and" bitwise operator.
  -- Lua 5.1 does not support bitwise operators.
  local pid = unistd.getpid()
  while pid > 0xffff do
    pid = pid - 0xffff
  end
  prms.self_id.portNumber = pid
  prms.domainNumber = cfg:domainNumber()
  msg:updateParams(prms)
  msg:useConfig(cfg)
  local id = ptpmgmt.USER_DESCRIPTION
  msg:setAction(ptpmgmt.GET, id)
  local seq = nextSequence()
  local err = msg:build(buf, seq)
  local txt
  if(err ~= ptpmgmt.MNG_PARSE_ERROR_OK) then
    txt = ptpmgmt.Message.err2str_c(err)
    print("build error ", txt)
    return -1
  end
  if(not sk:send(buf(), msg:getMsgLen())) then
    return printError("send fail")
  end
  -- You can get file descriptor with sk:fileno() and use Lua socket.select()
  if(not sk:poll(500)) then
    return printError("timeout")
  end
  local cnt = sk:rcv(buf)
  if(cnt <= 0) then
    return printError("rcv cnt")
  end
  err = msg:parse(buf, cnt)
  if(err == ptpmgmt.MNG_PARSE_ERROR_MSG) then
    print "error Message"
  elseif(err ~= ptpmgmt.MNG_PARSE_ERROR_OK) then
    txt = ptpmgmt.Message.err2str_c(err)
    print("parse error ", txt)
  else
    dispacher:callHadler(msg)
  end

  -- test setting values
  local clk_dec = ptpmgmt.CLOCK_DESCRIPTION_t()
  clk_dec.clockType = 0x800
  local physicalAddress = ptpmgmt.Binary()
  physicalAddress:setBin(0, 0xf1)
  physicalAddress:setBin(1, 0xf2)
  physicalAddress:setBin(2, 0xf3)
  physicalAddress:setBin(3, 0xf4)
  print("physicalAddress: " .. physicalAddress:toId())
  print("physicalAddress: " .. physicalAddress:toHex())
  clk_dec.physicalAddress:setBin(0, 0xf1)
  clk_dec.physicalAddress:setBin(1, 0xf2)
  clk_dec.physicalAddress:setBin(2, 0xf3)
  clk_dec.physicalAddress:setBin(3, 0xf4)
  print("clk.physicalAddress: " .. clk_dec.physicalAddress:toId())
  print("clk.physicalAddress: " .. clk_dec.physicalAddress:toHex())
  print("manufacturerIdentity: " ..
    ptpmgmt.Binary.bufToId(clk_dec.manufacturerIdentity, 3))
  clk_dec.revisionData.textField = "This is a test"
  print("revisionData: " .. clk_dec.revisionData.textField)

  setPriority1(147)
  setPriority1(153)

  local event = ptpmgmt.SUBSCRIBE_EVENTS_NP_t()
  event:setEvent(ptpmgmt.NOTIFY_TIME_SYNC)
  local txt
  if(event:getEvent(ptpmgmt.NOTIFY_TIME_SYNC)) then
    txt = 'have'
  else
    txt = 'not'
  end
  print(string.format("getEvent(NOTIFY_TIME_SYNC)=%s", txt))
  if(event:getEvent(ptpmgmt.NOTIFY_PORT_STATE)) then
    txt = 'have'
  else
    txt = 'not'
  end
  print(string.format("getEvent(NOTIFY_PORT_STATE)=%s", txt))

  -- test SigEvent that represent std::vector<SLAVE_TX_EVENT_TIMESTAMPS_rec_t>
  -- See std_vectors.md for more information
  evnts = ptpmgmt.SigEvent()
  e = ptpmgmt.SLAVE_TX_EVENT_TIMESTAMPS_rec_t()
  e.sequenceId = 1
  e.eventEgressTimestamp:fromFloat(4.5)
  evnts:push_back(e)
  print(string.format("Events size %d, seq[0]=%d, ts[0]=%s", evnts:size(),
      evnts[0].sequenceId, evnts[0].eventEgressTimestamp:string()))
  return 0
end

main()
sk:close()

-- LD_PRELOAD=../../.libs/libptpmgmt.so LUA_CPATH="./5.3/?.so;;" lua5.3 test.lua
