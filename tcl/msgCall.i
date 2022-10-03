/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 */

%header %{static char MessageDispatcherCode[] =
"::oo::class create ptpmgmt::MessageDispatcher {\n"
"  method callHadler { msg { tlv_id -1 } { tlv {} } } {\n"
"    if { $tlv_id < 0 } {\n"
"      set tlv_id [ $msg getTlvId ]\n"
"      set tlv [ $msg getData ]\n"
"    }\n"
"    set objClass [ info object class [self object] ]\n"
"    set objMthods \" [ info class methods $objClass -private ] \"\n"
"    if { [ $msg isValidId $tlv_id ] } {\n"
"      set idstr [ ptpmgmt::Message_mng2str_c $tlv_id ]\n"
"      set callback_name \"${idstr}_h\"\n"
"      if { [ regexp \" $callback_name \" $objMthods ] } {\n"
"        set conv \"ptpmgmt::conv_$idstr\"\n"
"        set tlv2 [ $conv $tlv ]\n"
"        my $callback_name $msg $tlv2 $idstr\n"
"      } else {\n"
"        if { [ regexp { noTlvCallBack } $objMthods ] } {\n"
"           my noTlvCallBack $msg $idstr\n"
"        }\n"
"      }\n"
"    } else {\n"
"      if { [ regexp { noTlv } $objMthods ] } {\n"
"        my noTlv $msg\n"
"      }\n"
"    }\n"
"  }\n"
"  constructor {} {}\n"
"}\n"
"::oo::class create ptpmgmt::MessageBuilder {\n"
"  variable m_msg\n"
"  variable m_tlv\n"
"  method buildTlv { actionField  tlv_id } {\n"
"    variable m_msg\n"
"    variable m_tlv\n"
"    if { $actionField == $ptpmgmt::GET || [ ptpmgmt::Message_isEmpty $tlv_id ] } {\n"
"      return [ $m_msg setAction $actionField $tlv_id ]\n"
"    }\n"
"    set objClass [ info object class [self object] ]\n"
"    set objMthods \" [ info class methods $objClass -private ] \"\n"
"    set idstr [ ptpmgmt::Message_mng2str_c $tlv_id ]\n"
"    set tlv_pkg \"ptpmgmt::${idstr}_t\"\n"
"    set callback_name \"${idstr}_b\"\n"
"    if { [ regexp \" $callback_name \" $objMthods ] } {\n"
"      set tlv [ $tlv_pkg ]\n"
"      if { [ my $callback_name $m_msg $tlv ] &&\\\n"
"           [ $m_msg setAction $actionField $tlv_id $tlv ] } {\n"
"        set m_tlv $tlv\n"
"        return true\n"
"      }\n"
"    }\n"
"    return false\n"
"  }\n"
"  constructor {msg} {\n"
"    variable m_msg\n"
"    set m_msg $msg\n"
"  }\n"
"  destructor {\n"
"    variable m_msg\n"
"    $m_msg clearData\n"
"  }\n"
"}\n";%}
%init %{Tcl_Eval(interp, MessageDispatcherCode);%}
