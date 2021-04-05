--SPDX-License-Identifier: GPL-3.0-or-later

--[[
 - testing for lua wrapper of libpmc

 - @author Erez Geva <ErezGeva2@@gmail.com>
 - @copyright 2021 Erez Geva
 - ]]

require 'pmc'
require 'posix'
local unistd = require 'posix.unistd'

DEF_CFG_FILE = "/etc/linuxptp/ptp4l.conf"
SIZE = 2000

sk = pmc.SockUnix()
msg = pmc.Message()
buf = pmc.Buf(SIZE)
sequence = 0

function setPriority1(newPriority1)
    local txt
    local pr1 = pmc.PRIORITY1_t()
    pr1.priority1 = newPriority1
    local id = pmc.PRIORITY1
    msg:setAction(pmc.SET, id, pr1)
    sequence = sequence + 1
    local err = msg:build(buf(), SIZE, sequence)
    if(err ~= pmc.MNG_PARSE_ERROR_OK) then
        txt = pmc.Message.err2str_c(err)
        print("build error ", txt)
    end
    if(not sk:send(buf(), msg:getMsgLen())) then
        print "send fail"
        return
    end
    if(not sk:poll(500)) then
        print "timeout"
        return
    end
    local cnt = sk:rcv(buf(), SIZE)
    if(cnt <= 0) then
        print "rcv cnt"
        return -1
    end
    err = msg:parse(buf(), cnt)
    if(err ~= pmc.MNG_PARSE_ERROR_OK or msg:getTlvId() ~= id or
       sequence ~= msg:getSequence()) then
        print "set fails"
        return -1
    end
    print("set new priority " .. newPriority1 .. " success")
    msg:setAction(pmc.GET, id)
    sequence = sequence + 1
    err = msg:build(buf(), SIZE, sequence)
    if(err ~= pmc.MNG_PARSE_ERROR_OK) then
        txt = pmc.Message.err2str_c(err)
        print("build error ", txt)
    end
    if(not sk:send(buf(), msg:getMsgLen())) then
        print "send fail"
        return
    end
    if(not sk:poll(500)) then
        print "timeout"
        return
    end
    local cnt = sk:rcv(buf(), SIZE)
    if(cnt <= 0) then
        print "rcv cnt"
        return -1
    end
    err = msg:parse(buf(), cnt)
    if(err == pmc.MNG_PARSE_ERROR_MSG) then
        print "error Message"
    elseif(err ~= pmc.MNG_PARSE_ERROR_OK) then
        txt = pmc.Message.err2str_c(err)
        print("parse error ", txt)
    else
        local rid = msg:getTlvId()
        local idstr = pmc.Message.mng2str_c(rid)
        print("Get reply for " .. idstr)
        if (rid == id) then
            local newPr = pmc.conv_PRIORITY1(msg:getData())
            print(string.format("priority1: %d", newPr.priority1))
        end
    end
end

function main()
    if(not buf:isAlloc()) then
        print "buffer allocation failed"
        return
    end
    local txt
    local cfg_file = arg[1]
    if(cfg_file == nil or cfg_file == '') then
       cfg_file = DEF_CFG_FILE
    end
    print("Use configuration file " .. cfg_file)
    local cfg = pmc.ConfigFile()
    if(not cfg:read_cfg(cfg_file)) then
        print "fail reading configuration file"
        return
    end
    if(not sk:setDefSelfAddress() or not sk:init() or not
            sk:setPeerAddress(cfg)) then
        print "fail init socket"
        return
    end
    local prms = msg:getParams()
    prms.self_id.portNumber = unistd.getpid()
    msg:updateParams(prms)
    local id = pmc.USER_DESCRIPTION
    msg:setAction(pmc.GET, id)
    sequence = sequence + 1
    local err = msg:build(buf(), SIZE, sequence)
    if(err ~= pmc.MNG_PARSE_ERROR_OK) then
        txt = pmc.Message.err2str_c(err)
        print("build error ", txt)
        return
    end
    if(not sk:send(buf(), msg:getMsgLen())) then
        print "send fail"
        return
    end
    -- You can get file descriptor with sk:getFd() and use Lua socket.select()
    if(not sk:poll(500)) then
        print "timeout"
        return
    end
    local cnt = sk:rcv(buf(), SIZE)
    if(cnt <= 0) then
        print("rcv error", cnt)
        return
    end
    err = msg:parse(buf(), cnt)
    if(err == pmc.MNG_PARSE_ERROR_MSG) then
        print "error Message"
    elseif(err ~= pmc.MNG_PARSE_ERROR_OK) then
        txt = pmc.Message.err2str_c(err)
        print("parse error ", txt)
    else
        local rid = msg:getTlvId()
        local idstr = pmc.Message.mng2str_c(rid)
        print("Get reply for " .. idstr)
        if (rid == id) then
            local user = pmc.conv_USER_DESCRIPTION(msg:getData())
            print("get user desc: " .. user.userDescription.textField)
        end
    end

    -- test setting values
    local clk_dec = pmc.CLOCK_DESCRIPTION_t()
    clk_dec.clockType = 0x800
    local physicalAddress = pmc.Binary()
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
        pmc.Binary.bufToId(clk_dec.manufacturerIdentity, 3))
    clk_dec.revisionData.textField = "This is a test"
    print("revisionData: " .. clk_dec.revisionData.textField)

    setPriority1(147)
    setPriority1(153)
end

main()
sk:close()

--[[
# If libpmc library is not installed in system, run with:
ln -sf 5.1/pmc.so && LD_LIBRARY_PATH=.. lua5.1 test.lua
ln -sf 5.2/pmc.so && LD_LIBRARY_PATH=.. lua5.2 test.lua
ln -sf 5.3/pmc.so && LD_LIBRARY_PATH=.. lua5.3 test.lua

]]
