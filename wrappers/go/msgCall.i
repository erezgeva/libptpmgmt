/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 */

/* TODO is working with SWIG director more simply?
        Is it possible to have the callback with the TLV type? */

%include "allocTlv.i"

%go_import("reflect")

%insert(go_wrapper) %{
type MessageDispatcherIf interface {
  /* Call MessageDispatcherCallHadler() with proper struct using the interface
   * 2 variants of calling:
   * - CallHadler(msg Message)
   * - CallHadler(msg Message, tlv_id Mng_vals_e, tlv BaseMngTlv)
   */
  CallHadler(msg Message, a ...interface{})
}
type MessageDispatcherNoTlvCallBack interface {
  /* Optional callback called in case a tlv do not have a callback */
  NoTlvCallBack(msg Message, tlv_id string)
}
type MessageDispatcherNoTlv interface {
  /* Optional callback called in case there is not TLV */
  NoTlv(msg Message)
}
func MessageDispatcherCallHadler(self MessageDispatcherIf, msg Message,
                                 a interface{}) {
  var tlv_id Mng_vals_e
  var tlv BaseMngTlv
  val := reflect.ValueOf(a)
  switch val.Len() {
    case 2:
      var ok bool
      if tlv_id, ok = val.Index(0).Interface().(Mng_vals_e); !ok {
        return
      }
      if tlv, ok = val.Index(1).Interface().(BaseMngTlv); !ok {
        return
      }
    case 0:
      tlv_id = msg.GetTlvId()
      tlv = msg.GetData()
    default:
      return
  }
  if tlv.Swigcptr() != 0 && msg.IsValidId(tlv_id) {
    idstr := MessageMng2str_c(tlv_id)
    callback_name := idstr + "_h"
    mthd := reflect.ValueOf(self).MethodByName(callback_name)
    if mthd.IsValid() {
      inputs := make([]reflect.Value, 3)
      inputs[0] = reflect.ValueOf(msg)
      inputs[1] = reflect.ValueOf(tlv)
      inputs[2] = reflect.ValueOf(idstr)
      mthd.Call(inputs)
      return
    }
    if NoTlvCallBack, ok := self.(MessageDispatcherNoTlvCallBack); ok {
      NoTlvCallBack.NoTlvCallBack(msg, idstr)
    }
  } else {
    if NoTlv, ok := self.(MessageDispatcherNoTlv); ok {
      NoTlv.NoTlv(msg)
    }
  }
}

/**
 * @note Do not forget to call msg.ClearData() and free the allocated tlv
 *       with ptpmgmt.FreeTlv(tlv) after sending the message.
 *       You may use 'defer ptpmgmt.MessageBuilderFree(msg, tlv)'
 *        if BuildTlv return true and the sending is done
 *        in the same function context :-)
 */
type MessageBuilderIf interface {
  /** Call MessageBuilderBuildTlv() with proper struct using the interface */
  BuildTlv(actionField ActionField_e, tlv_id Mng_vals_e) bool
}
func MessageBuilderBuildTlv(self MessageBuilderIf, msg Message,
                            actionField ActionField_e,
                            tlv_id Mng_vals_e) bool {
  if !msg.IsValidId(tlv_id) {
    return false
  }
  if actionField == GET || MessageIsEmpty(tlv_id) {
    return msg.SetAction(actionField, tlv_id)
  }
  if actionField != SET && actionField != COMMAND {
    return false
  }
  tlv := AllocTlv(tlv_id)
  if tlv != nil {
    idstr := MessageMng2str_c(tlv_id)
    callback_name := idstr + "_b"
    mthd := reflect.ValueOf(self).MethodByName(callback_name)
    if mthd.IsValid() {
      inputs := make([]reflect.Value, 2)
      inputs[0] = reflect.ValueOf(msg)
      inputs[1] = reflect.ValueOf(tlv)
      ret := mthd.Call(inputs)
      if val, ok := ret[0].Interface().(bool); ok && val &&
        msg.SetAction(actionField, tlv_id, tlv) {
          return true;
      }
    }
    /* Resource comes from C++, we should release unless we use it */
    FreeTlv(tlv)
  }
  return false
}
func MessageBuilderFree(msg Message, tlv BaseMngTlv) {
  msg.ClearData()
  FreeTlv(tlv)
}
%}
