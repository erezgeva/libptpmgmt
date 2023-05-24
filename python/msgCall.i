/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 */

%pythoncode %{
class MessageDispatcher(object):
    def callHadler(self, msg : Message, tlv_id : int = -1,
                   tlv : BaseMngTlv = None):
        if msg is None:
            raise AttributeError('MessageDispatcher::callHadler() ' +
                                 'You must use Message object')
        if tlv is None:
            tlv_id=msg.getTlvId()
            tlv=msg.getData()
        if tlv is not None and msg.isValidId(tlv_id):
            idstr=Message.mng2str_c(tlv_id)
            callback_name=idstr + '_h'
            conv_func='conv_' + idstr
            if hasattr(self.__class__, callback_name) and\
               callable(getattr(self.__class__, callback_name)):
                data=globals()[conv_func](tlv)
                getattr(self, callback_name)(msg,data,idstr)
            elif hasattr(self.__class__, 'noTlvCallBack') and\
               callable(getattr(self.__class__, 'noTlvCallBack')):
                self.noTlvCallBack(msg, idstr)
        elif hasattr(self.__class__, 'noTlv') and\
           callable(getattr(self.__class__, 'noTlv')):
            self.noTlv(msg)
    def __init__(self):
        if type(self) is MessageDispatcher:
            raise Exception('MessageDispatcher is an abstract class and ' +
                            'cannot be instantiated directly')

class MessageBuilder(object):
    def buildTlv(self, actionField : int, tlv_id : int):
        if not self.m_msg.isValidId(tlv_id):
            return False
        if actionField == GET or Message.isEmpty(tlv_id):
            return self.m_msg.setAction(actionField, tlv_id)
        if actionField != SET and actionField != COMMAND:
            return False
        idstr=Message.mng2str_c(tlv_id)
        tlv_pkg=idstr + '_t'
        callback_name=idstr + '_b'
        if tlv_pkg in globals() and hasattr(self.__class__, callback_name) and\
           callable(getattr(self.__class__, callback_name)):
            tlv = globals()[tlv_pkg]()
            if tlv is not None and getattr(self, callback_name)(self.m_msg,tlv) and\
               self.m_msg.setAction(actionField, tlv_id, tlv):
                self.m_tlv = tlv
                return True
        return False
    def __init__(self, msg : Message):
        if type(self) is MessageBuilder:
            raise Exception('MessageDispatcher is an abstract class ' +
                            'and cannot be instantiated directly')
        if msg is None:
            raise AttributeError('MessageBuilder::MessageBuilder() ' +
                                 'You must use Message object')
        else:
            self.m_msg=msg
    def __del__(self):
        self.m_msg.clearData()
%}
