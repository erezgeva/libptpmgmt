/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 */

%luacode %{
ptpmgmt.MessageDispatcher = {}
function ptpmgmt.MessageDispatcher:callHadler(msg, tlv_id, tlv)
    if(type(msg) ~= 'userdata' or getmetatable(msg)['.type'] ~= 'Message') then
        error('MessageDispatcher::callHadler() msg must be a Message object', 2)
    end
    if(tlv == nil) then
        tlv = msg:getData()
        tlv_id = msg:getTlvId()
    elseif(type(tlv_id) ~= 'number') then
        error('MessageDispatcher::callHadler() tlv_id must be a number', 2)
    elseif(type(tlv) ~= 'userdata') then
        error('MessageDispatcher::callHadler() tlv must be an object', 2)
    end
    if(tlv ~= nil and msg:isValidId(tlv_id) and
       not ptpmgmt.Message.isEmpty(tlv_id)) then
        local tlv_id_str = ptpmgmt.Message.mng2str_c(tlv_id)
        local callback = getmetatable(self)[tlv_id_str .. '_h']
        if(type(callback) == 'function') then
            local data
            local tlv_type = getmetatable(tlv)['.type']
            if(tlv_type == 'BaseMngTlv') then
                data = ptpmgmt['conv_' .. tlv_id_str](tlv)
            elseif(tlv_type == tlv_id_str .. '_t') then
                data = tlv
            else
                error('MessageDispatcher::callHadler() tlv must be ' ..
                      'a BaseMngTlv object', 2)
            end
            if(data ~= nil) then
                callback(self, msg, data, tlv_id_str)
            end
        elseif(type(getmetatable(self)['noTlvCallBack']) == 'function') then
            self:noTlvCallBack(msg, tlv_id_str)
        end
    else
        if(type(getmetatable(self)['noTlv']) == 'function') then
            self:noTlv(msg)
        end
    end
end
function ptpmgmt.MessageDispatcher:new()
    local obj = {}
    setmetatable(obj, self)
    self.__index = self
    return obj
end
function ptpmgmt.MessageDispatcher:clone(inhObj)
    if(type(inhObj) ~= 'table') then
        error('MessageDispatcher:clone() inhObj is not an object', 2)
    end
    local obj = self:new()
    setmetatable(inhObj, {__index = self})
    setmetatable(obj, inhObj)
    inhObj.__index = inhObj
    return obj
end
ptpmgmt.MessageBuilder = { m_msg = 0, m_tlv = 0 }
function ptpmgmt.MessageBuilder:buildTlv(actionField, tlv_id)
    if(type(actionField) ~= 'number') then
        error('MessageBuilder::buildTlv() actionField must be a number', 2)
    elseif(type(tlv_id) ~= 'number') then
        error('MessageBuilder::buildTlv() tlv_id must be a number', 2)
    end
    local msg = self.m_msg
    if(not msg:isValidId(tlv_id)) then
        return false
    end
    if(actionField == ptpmgmt.GET or ptpmgmt.Message.isEmpty(tlv_id)) then
        return msg:setAction(actionField, tlv_id)
    end
    if(actionField ~= ptpmgmt.SET and actionField ~= ptpmgmt.COMMAND) then
        return false
    end
    local tlv_id_str = ptpmgmt.Message.mng2str_c(tlv_id)
    local tlv_type = tlv_id_str .. '_t'
    local callback = getmetatable(self)[tlv_id_str .. '_b']
    if(type(callback) == 'function' and type(ptpmgmt[tlv_type]) == 'table') then
        local tlv = ptpmgmt[tlv_type]()
        if(tlv ~= nil and callback(self, msg, tlv) and
           msg:setAction(actionField, tlv_id, tlv)) then
            self.m_tlv = tlv
            return true
        end
    end
    return false
end
function ptpmgmt.MessageBuilder:getMsg()
    return self.m_msg
end
function ptpmgmt.MessageBuilder:clear()
    self.m_msg:clearData()
    self.m_tlv = 0
end
function ptpmgmt.MessageBuilder:new(msg)
    if(type(msg) ~= 'userdata' or getmetatable(msg)['.type'] ~= 'Message') then
        error('MessageBuilder::new() msg must be a Message object', 2)
    end
    local obj = { m_msg = msg }
    setmetatable(obj, self)
    self.__index = self
    return obj
end
function ptpmgmt.MessageBuilder:clone(inhObj, msg)
    if(type(inhObj) ~= 'table') then
        error('MessageBuilder:clone() inhObj is not an object', 2)
    end
    if(type(msg) ~= 'userdata' or getmetatable(msg)['.type'] ~= 'Message') then
        error('MessageBuilder:clone() msg must be a Message object', 2)
    end
    local obj = self:new(msg)
    setmetatable(inhObj, {__index = self})
    setmetatable(obj, inhObj)
    inhObj.__index = inhObj
    return obj
end
%}
