/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 *
 * @brief libptpmgmt JavaScript main module
 *
 * Load shared library through the ptpmgmt0 script.
 * And add it the Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

var loadptpmgmt = require('./loadptpmgmt');

class MessageDispatcher {
  callHadler(msg, tlv_id, tlv) {
    if(typeof msg != 'object')
      return;
    if(typeof tlv_id === 'undefined' && typeof tlv === 'undefined') {
      tlv_id = msg.getTlvId();
      tlv = msg.getData();
    }
    if(typeof tlv_id != 'number' || !msg.isValidId(tlv_id) || loadptpmgmt.Message.isEmpty(tlv_id) || typeof tlv != 'object' || tlv == null) {
      if(typeof this.noTlv === 'function')
        this.noTlv(msg);
      return;
    }
    let idStr = loadptpmgmt.Message.mng2str_c(tlv_id);
    if(typeof idStr != 'string' || idStr.length == 0)
      // Should not happen, just a precaution
      return;
    let callBack = 'this.' + idStr + '_h';
    var ret = false;
    let e = 'ret = typeof ' + callBack + " === 'function'";
    eval(e);
    if(ret) {
      var ctlv;
      e = 'ctlv = loadptpmgmt.conv_' + idStr + '(tlv)';
      eval(e);
      if(typeof ctlv != 'object' || ctlv == null) {
        if(typeof this.noTlv === 'function')
          this.noTlv(msg);
        return;
      }
      e = callBack + `(msg, ctlv, '${idStr}');`;
      eval(e);
    } else if(typeof this.noTlvCallBack === 'function')
      this.noTlvCallBack(msg, idStr);
  }
};

class MessageBuilder {
  // private properties
  #m_tlv; // ptpmgmt.BaseMngTlv
  #m_msg; // ptpmgmt.Message

  // public methods
  constructor(msg) { this.m_msg = msg; }
  getMsg() { return this.m_msg; }
  clear() {
    if(typeof this.m_msg === 'object') {
      this.m_msg.clearData();
      this.m_tlv = null;
    }
  }
  buildTlv(actionField, tlv_id) {
    if(typeof this.m_msg != 'object' || typeof tlv_id != 'number' || !this.m_msg.isValidId(tlv_id))
      return false;
    if(actionField == loadptpmgmt.GET || loadptpmgmt.Message.isEmpty(tlv_id))
      return this.m_msg.setAction(actionField, tlv_id);
    switch(actionField) {
    case loadptpmgmt.SET:
      break;
    case loadptpmgmt.COMMAND:
      break;
    default:
      return false;
    }
    let idStr = loadptpmgmt.Message.mng2str_c(tlv_id);
    if(typeof idStr != 'string' || idStr.length == 0)
      // Should not happen, just a precaution
      return false;
    let callBack = 'this.' + idStr + '_b';
    var ret = false;
    let e = 'ret = typeof ' + callBack + " === 'function'";
    eval(e);
    if(!ret)
      return false;
    var tlv;
    e = 'tlv = new loadptpmgmt.' + idStr + '_t';
    eval(e);
    if(typeof tlv != 'object' || tlv == null)
      return false;
    ret = false;
    e = 'ret = ' + callBack + '(this.m_msg, tlv)';
    eval(e);
    if(!ret)
      return false;
    if(this.m_msg.setAction(actionField, tlv_id, tlv)) {
      this.m_tlv = tlv;
      return true;
    }
    return false;
  }
};
loadptpmgmt.MessageDispatcher = MessageDispatcher;
loadptpmgmt.MessageBuilder = MessageBuilder;
module.exports = loadptpmgmt;
