/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 */

%{
/* The class full code.
   As Lua native clases do not use destructor, we use a C++ class.
   The class store a reference to a Message object.
   Once MessageBuilder is deleted,
    this object will clear the send TLV from the Message object,
    preventing using a deleted TLV by mistake :-)
   Beware, Lua garbage collection can not be controlled,
    the send TLV may be deleted before calling clearData(),
    this class only gurantee, they will be called
    on the same garbage collection cycle.
   In case of using threads, it is better not to rely on this protection,
    and explicitly call clearData().
 */
namespace ptpmgmt {
class MessageBuilderBase
{
  private:
    Message &m_msg;
  public:
    /* used by buildTlv() to fetch reference to the Message object */
    Message &getMsg() { return m_msg; }
    /* Used by MessageBuilder constructor to store a Message object reference */
    MessageBuilderBase(Message &msg) : m_msg(msg) {}
    /* Clear the send TLV from the Message object */
    ~MessageBuilderBase() { m_msg.clearData(); }
};
};
%}
/* API to wrap */
class MessageBuilderBase
{
  public:
    Message &getMsg();
    MessageBuilderBase(Message &msg);
    /* No need for explicit destructor wrapper,
       Lua will delete the object once MessageBuilder object is. */
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
    elseif(type(tlv) ~= 'userdata') then
        error('MessageDispatcher::callHadler() tlv must be an object', 2)
    end
    if(tlv ~= nil and msg:isValidId(tlv_id) and
       not ptpmgmt.Message.isEmpty(tlv_id)) then
        local idstr = ptpmgmt.Message.mng2str_c(tlv_id)
        local callback_name = idstr .. '_h'
        if(type(getmetatable(self)[callback_name]) == "function") then
            local data
            if(getmetatable(tlv)['.type'] == 'BaseMngTlv') then
                data = ptpmgmt['conv_' .. idstr](tlv)
            elseif(getmetatable(tlv)['.type'] == idstr .. '_t') then
                data = tlv
            else
                error('MessageDispatcher::callHadler() tlv must be a BaseMngTlv object', 2)
            end
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
    if(not msg:isValidId(tlv_id)) then
        return false;
    end
    if(actionField == ptpmgmt.GET or ptpmgmt.Message.isEmpty(tlv_id)) then
        return msg:setAction(actionField, tlv_id)
    end
    if(actionField ~= ptpmgmt.SET and actionField ~= ptpmgmt.COMMAND) then
        return false;
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
