/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 */

%pragma(php) code="abstract class MessageDispatcher {
	function callHadler(Message $msg, int $tlv_id = -1, ?BaseMngTlv $tlv = null) {
		if(is_null($tlv)) {
			$tlv_id = $msg->getTlvId();
			$tlv = $msg->getData();
		}
		if(!is_null($tlv) and $msg->isValidId($tlv_id)) {
			$idstr = Message::mng2str_c($tlv_id);
			$callback_name=$idstr . '_h';
			if(method_exists($this, $callback_name)) {
				eval(\"\$data = ptpmgmt::conv_$idstr(\$tlv);\");
				$this->$callback_name($msg,$data,$idstr);
				return;
			}
			elseif(method_exists($this, 'noTlvCallBack')) {
				$this->noTlvCallBack($msg, $idstr);
			}
		}
		elseif(method_exists($this, 'noTlv')) {
			$this->noTlv($msg);
		}
	}
}

abstract class MessageBulder {
	private Message $m_msg;
	private $m_tlv=null;
	function buildTlv(int $actionField, int $tlv_id) {
		if($actionField == ptpmgmt::GET or Message::isEmpty($tlv_id)) {
			return $this->m_msg->setAction($actionField, $tlv_id);
		}
		$idstr = Message::mng2str_c($tlv_id);
		$tlv_pkg=$idstr . '_t';
		$callback_name=$idstr . '_b';
		if(class_exists($tlv_pkg) and method_exists($this, $callback_name) and
		   eval(\"\$tlv = new $tlv_pkg();return true;\") and !is_null($tlv) and
		   $this->$callback_name($this->m_msg, $tlv) and
		   $this->m_msg->setAction($actionField, $tlv_id, $tlv)) {
			$this->m_tlv = $tlv;
			return true;
		}
		return false;
	}
	function __construct(Message $msg) {
		$this->m_msg=$msg;
	}
	function __destruct() {
		$this->m_msg->clearData();
	}
}"
