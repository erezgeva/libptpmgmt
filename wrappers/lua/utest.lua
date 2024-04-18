-- SPDX-License-Identifier: GPL-3.0-or-later
-- SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>

--[[
 - Unit test with Lua wrapper of libptpmgmt

 - @author Erez Geva <ErezGeva2@@gmail.com>
 - @copyright © 2022 Erez Geva
 - ]]

require 'ptpmgmt'
luaunit = require 'luaunit'
-- See: https://luaunit.readthedocs.io/

myDisp = {} -- inherit from ptpmgmt.MessageDispatcher
function myDisp:new()
  local obj = ptpmgmt.MessageDispatcher:new()
  setmetatable(self, {__index = ptpmgmt.MessageDispatcher})
  setmetatable(obj, self)
  self.__index = self
  self.priority1 = 0
  self.func = 0
  self.id = ''
  return obj
end
function myDisp:PRIORITY1_h(msg, tlv, tlv_id)
  self.func = self.func + 0x1
  self.id = tlv_id
  self.priority1 = tlv.priority1
end
function myDisp:noTlv(msg)
  self.func = self.func + 0x2
end
function myDisp:noTlvCallBack(msg, tlv_id)
  self.func = self.func + 0x4
  self.id = tlv_id
end
myBuild = {} -- Inherit from ptpmgmt.MessageBuilder
function myBuild:new(msg)
  local obj = ptpmgmt.MessageBuilder:new(msg)
  setmetatable(self, {__index = ptpmgmt.MessageBuilder})
  setmetatable(obj, self)
  self.__index = self
  self.run = 0
  return obj
end
function myBuild:PRIORITY1_b(msg, tlv)
  self.run = 1
  tlv.priority1 = 117
  return true
end

TestMsgDisp = {}
  function TestMsgDisp:setUp()
    self.msg = ptpmgmt.Message()
    self.disp = myDisp:new()
  end

  -- Tests callHadler with empty TLV
  function TestMsgDisp:testParsedCallHadlerEmptyTLV()
    self.disp:callHadler(self.msg)
    luaunit.assertEquals(self.disp.func, 0x2,    'should call noTlv')
    luaunit.assertEquals(self.disp.id, '',       'should have empty ID')
    luaunit.assertEquals(self.disp.priority1, 0, 'should not have priority1 value')
  end

  -- Tests callHadler method with TLV
  function TestMsgDisp:testParsedCallHadlerTLV()
    tlv = ptpmgmt.PRIORITY1_t()
    tlv.priority1 = 117
    self.disp:callHadler(self.msg, ptpmgmt.PRIORITY1, tlv)
    luaunit.assertEquals(self.disp.func, 0x1,       'should call PRIORITY1_h')
    luaunit.assertEquals(self.disp.id, 'PRIORITY1', 'should have PRIORITY1 ID')
    luaunit.assertEquals(self.disp.priority1, 117,  'should have priority1 value')
  end

  -- Tests callHadler method with TLV without callback
  function TestMsgDisp:testParsedCallHadlerTLVNoCallback()
    tlv = ptpmgmt.PRIORITY2_t()
    tlv.priority2 = 117
    self.disp:callHadler(self.msg, ptpmgmt.PRIORITY2, tlv)
    luaunit.assertEquals(self.disp.func, 0x4,       'should call noTlvCallBack')
    luaunit.assertEquals(self.disp.id, 'PRIORITY2', "should have PRIORITY2 ID")
    luaunit.assertEquals(self.disp.priority1, 0,    "shouldn't have priority1 value")
  end

TestMsgBuild = {}
  function TestMsgBuild:setUp()
    self.msg = ptpmgmt.Message()
    self.build = myBuild:new(self.msg)
  end

  -- Tests build empty TLV
  function TestMsgBuild:testEmptyTLV()
    luaunit.assertTrue(self.build:buildTlv(ptpmgmt.COMMAND, ptpmgmt.ENABLE_PORT), 'should pass')
    luaunit.assertEquals(self.build.run, 0, "shouldn't call PRIORITY1 callback")
    self.build:clear()
  end

  -- Tests build TLV
  function TestMsgBuild:testTLV()
    luaunit.assertTrue(self.build:buildTlv(ptpmgmt.SET, ptpmgmt.PRIORITY1), 'should pass')
    luaunit.assertEquals(self.build.run, 1, 'should call PRIORITY1 callback')
    self.build:clear()
  end

  -- Tests build TLV that lack callback
  function TestMsgBuild:testTLVNoCallback()
    luaunit.assertFalse(self.build:buildTlv(ptpmgmt.SET, ptpmgmt.PRIORITY2), 'should no pass')
    luaunit.assertEquals(self.build.run, 0, "shouldn't call PRIORITY1 callback")
    self.build:clear()
  end

os.exit(luaunit.LuaUnit.run())
