/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 */

%go_import("reflect")

%insert(go_wrapper) %{
type MessageDispatcher interface {
  CallHadler(msg Message, a ...interface{})
}
type MessageDispatcherNoTlvCallBack interface {
  NoTlvCallBack(msg Message, tlv_id string)
}
type MessageDispatcherNoTlv interface {
  NoTlv(msg Message)
}
func MessageDispatcherCallHadler(self MessageDispatcher, msg Message,
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

type MessageBuilder interface {
  BuildTlv(msg Message, actionField ActionField_e, tlv_id Mng_vals_e) bool
}
func MessageBuilderBuildTlv(self MessageBuilder, msg Message,
                            actionField ActionField_e,
                            tlv_id Mng_vals_e) bool {
  if !msg.IsValidId(tlv_id) {
    return false;
  }
  if actionField == GET || MessageIsEmpty(tlv_id) {
    return msg.SetAction(actionField, tlv_id)
  }
  if actionField != SET && actionField != COMMAND {
    return false;
  }
  idstr := MessageMng2str_c(tlv_id)
  callback_name := idstr + "_b"
  mthd := reflect.ValueOf(self).MethodByName(callback_name)
  if mthd.IsValid() {
    inputs := make([]reflect.Value, 1)
    inputs[0] = reflect.ValueOf(msg)
    ret := mthd.Call(inputs)
    if val, ok := ret[0].Interface().(BaseMngTlv); ok {
      return msg.SetAction(actionField, tlv_id, val)
    }
  }
  return false
}
%}
