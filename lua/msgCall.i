/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 */

%{
namespace ptpmgmt {
class MessageBuilderBase
{
  private:
    Message &m_msg;
  public:
    Message &getMsg() { return m_msg; }
    MessageBuilderBase(Message &msg) : m_msg(msg) {}
    ~MessageBuilderBase() { m_msg.clearData(); }
};
};
%}
class MessageBuilderBase
{
  public:
    Message &getMsg();
    MessageBuilderBase(Message &msg);
};

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
    elseif(type(tlv) ~= 'userdata' or getmetatable(tlv)['.type'] ~= 'BaseMngTlv') then
        error('MessageDispatcher::callHadler() tlv must be a BaseMngTlv object', 2)
    end
    if(tlv ~= nil and msg:isValidId(tlv_id)) then
        local idstr = ptpmgmt.Message.mng2str_c(tlv_id)
        local callback_name = idstr .. '_h'
        if(type(getmetatable(self)[callback_name]) == "function") then
            local data = ptpmgmt['conv_' .. idstr](tlv)
            if(data ~= nil) then
                getmetatable(self)[callback_name](self, msg, data, idstr)
            end
        elseif(type(getmetatable(self)['noTlvCallBack']) == "function") then
            self:noTlvCallBack(msg, idstr)
        end
    else
        if(type(getmetatable(self)['noTlv']) == "function") then
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

ptpmgmt.MessageBuilder = { m_buildBase = 0, m_tlv = 0 }
function ptpmgmt.MessageBuilder:buildTlv(actionField, tlv_id)
    if(type(actionField) ~= 'number') then
        error('MessageBuilder::buildTlv() actionField must be a number', 2)
    elseif(type(tlv_id) ~= 'number') then
        error('MessageBuilder::buildTlv() tlv_id must be a number', 2)
    end
    local msg = self.m_buildBase:getMsg()
    if(actionField == ptpmgmt.GET or ptpmgmt.Message.isEmpty(tlv_id)) then
        return msg:setAction(actionField, tlv_id)
    end
    local idstr = ptpmgmt.Message.mng2str_c(tlv_id)
    local tlv_pkg = idstr .. '_t'
    local callback_name = idstr .. '_b'
    if(type(getmetatable(self)[callback_name]) == "function" and
       type(ptpmgmt[tlv_pkg]) == "table") then
        local tlv = ptpmgmt[tlv_pkg]()
        if(tlv ~= nil and
           getmetatable(self)[callback_name](self, msg, tlv) and
           msg:setAction(actionField, tlv_id, tlv)) then
            self.m_tlv = tlv
            return true
        end
    end
    return false
end
function ptpmgmt.MessageBuilder:new(msg)
    local obj = { m_buildBase = ptpmgmt.MessageBuilderBase(msg) }
    setmetatable(obj, self)
    self.__index = self
    return obj
end
%}
