/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 */

%header %{static char MessageDispatcherCode[] =
"class Ptpmgmt::MessageDispatcher\n"
"  def callHadler(msg, tlv_id = -1, tlv = nil)\n"
"    if tlv == nil then\n"
"      tlv = msg.getData()\n"
"      tlv_id = msg.getTlvId()\n"
"    end\n"
"    if tlv != nil and msg.isValidId(tlv_id) then\n"
"      idstr = Ptpmgmt::Message.mng2str_c(tlv_id)\n"
"      callback_name=idstr + '_h'\n"
"      if self.class.method_defined?(callback_name) then\n"
"        data = eval('Ptpmgmt::conv_' + idstr + '(tlv)')\n"
"        self.instance_eval(callback_name + '(msg, data, idstr)')\n"
"      elsif self.class.method_defined?(:noTlvCallBack) then\n"
"        self.noTlvCallBack(msg, idstr)\n"
"      elsif self.class.method_defined?(:noTlv) then\n"
"        self.noTlv(msg)\n"
"      end\n"
"    end\n"
"  end\n"
"end\n"
"class Ptpmgmt::MessageBulder\n"
"  def buildTlv(actionField, tlv_id)\n"
"    if actionField == Ptpmgmt::GET or Ptpmgmt::Message.isEmpty(tlv_id) then\n"
"      return @m_msg.setAction(actionField, tlv_id)\n"
"    end\n"
"    idstr = Ptpmgmt::Message.mng2str_c(tlv_id)\n"
"    tlv_pkg=idstr + '_t'\n"
"    callback_name=idstr + '_b'\n"
"    begin\n"
"      klass = Module.const_get('Ptpmgmt::' + tlv_pkg)\n"
"    rescue\n"
"      return false\n"
"    end\n"
"    if klass.is_a?(Class) and self.class.method_defined?(callback_name) then\n"
"      tlv = eval('Ptpmgmt::' + tlv_pkg + '.new')\n"
"      if tlv != nil and self.instance_eval(callback_name + '(@m_msg, tlv)') and\n"
"        @m_msg.setAction(actionField, tlv_id, tlv) then\n"
"        @m_tlv = tlv\n"
"        return true\n"
"      end\n"
"    end\n"
"    return false\n"
"  end\n"
"  def initialize(msg)\n"
"    @m_msg=msg\n"
"    ObjectSpace.define_finalizer(self, proc { msg.clearData() })\n"
"  end\n"
"end";%}
%init %{rb_eval_string(MessageDispatcherCode);%}
