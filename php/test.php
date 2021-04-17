#!/usr/bin/php
<?php
/* SPDX-License-Identifier: GPL-3.0-or-later
 *
 * testing for ruby wrapper of libpmc
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 */

require("pmc.php");

const DEF_CFG_FILE = "/etc/linuxptp/ptp4l.conf";
$sk = new SockUnix();
$msg = new Message();
$buf = new Buf(1000);
$sequence = 0;

function setPriority1($newPriority1)
{
  global $sk, $msg, $buf, $sequence;
  $pr1 = new PRIORITY1_t();
  $pr1->priority1 = $newPriority1;
  $id = pmc::PRIORITY1;
  $msg->setAction(pmc::SET, $id, $pr1);
  $err = $msg->build($buf, ++$sequence);
  if($err != pmc::MNG_PARSE_ERROR_OK) {
    $txt = Message::err2str_c($err);
    echo "build error $txt\n";
  }
  if(!$sk->send($buf, $msg->getMsgLen())) {
    echo "send fail";
    return;
  }
  if(!$sk->poll(500)) {
    echo "timeout";
    return;
  }
  $cnt = $sk->rcvBuf($buf);
  if($cnt <= 0) {
    echo "rcv error $cnt";
    return;
  }
  $err = $msg->parse($buf, $cnt);
  if($err != pmc::MNG_PARSE_ERROR_OK || $msg->getTlvId() != $id ||
     $sequence != $msg->getSequence()) {
    echo "set fails";
    return;
  }
  echo "set new priority $newPriority1 success\n";
  $msg->setAction(pmc::GET, $id);
  $err = $msg->build($buf, ++$sequence);
  if($err != pmc::MNG_PARSE_ERROR_OK) {
    $txt = Message::err2str_c($err);
    echo "build error $txt\n";
  }
  if(!$sk->send($buf, $msg->getMsgLen())) {
    echo "send fail";
    return;
  }
  if(!$sk->poll(500)) {
    echo "timeout";
    return;
  }
  $cnt = $sk->rcvBuf($buf);
  if($cnt <= 0) {
    echo "rcv error $cnt";
    return;
  }
  $err = $msg->parse($buf, $cnt);
  if($err == pmc::MNG_PARSE_ERROR_MSG) {
    echo "error message";
  } else if($err != pmc::MNG_PARSE_ERROR_OK) {
    $txt = Message::err2str_c($err);
    echo "parse error $txt\n";
  } else {
    $rid = $msg->getTlvId();
    $idstr = Message::mng2str_c($rid);
    echo "Get reply for $idstr\n";
    if($rid == $id) {
      $newPr = pmc::conv_PRIORITY1($msg->getData());
      echo "priority1: " . $newPr->priority1 . "\n";
    }
  }
}

function main($cfg_file)
{
  global $sk, $msg, $buf, $sequence;
  if(!$buf->isAlloc()) {
    echo "buffer allocation failed\n";
    return;
  }
  $cfg = new ConfigFile();
  if(!$cfg->read_cfg($cfg_file)) {
    echo "fail reading configuration file";
    return;
  }
  if(!$sk->setDefSelfAddress() || !$sk->init() || !$sk->setPeerAddress($cfg)) {
    echo "fail init socket";
    return;
  }
  $prms = $msg->getParams();
  $prms->self_id->portNumber = posix_getpid();
  # Verify we can use implementSpecific_e;
  # Notice Ruby capitalize first letter;
  $prms->implementSpecific = pmc::linuxptp;
  $msg->updateParams($prms);
  $id = pmc::USER_DESCRIPTION;
  $msg->setAction(pmc::GET, $id);
  $err = $msg->build($buf, ++$sequence);
  if($err != pmc::MNG_PARSE_ERROR_OK) {
    $txt = Message::err2str_c($err);
    echo "build error $txt\n";
    return;
  }
  if(!$sk->send($buf, $msg->getMsgLen())) {
    echo "send fail";
    return;
  }
  # You can get file descriptor with $sk->getFd() and use select;
  if(!$sk->poll(500)) {
    echo "timeout";
    return;
  }
  $cnt = $sk->rcvBuf($buf);
  if($cnt <= 0) {
    echo "rcv error $cnt\n";
    return;
  }
  $err = $msg->parse($buf, $cnt);
  if($err == pmc::MNG_PARSE_ERROR_MSG) {
    echo "error message";
  } else if($err != pmc::MNG_PARSE_ERROR_OK) {
    $txt = Message::err2str_c($err);
    echo "parse error $txt\n";
  } else {
    $rid = $msg->getTlvId();
    $idstr = Message::mng2str_c($rid);
    echo "Get reply for $idstr\n";
    if($rid == $id) {
      $user = pmc::conv_USER_DESCRIPTION($msg->getData());
      echo "get user desc: " . $user->userDescription->textField . "\n";
    }
  }

  # test setting values;
  $clk_dec = new CLOCK_DESCRIPTION_t();
  $clk_dec->clockType = 0x800;
  $physicalAddress = new Binary();
  $physicalAddress->setBin(0, 0xf1);
  $physicalAddress->setBin(1, 0xf2);
  $physicalAddress->setBin(2, 0xf3);
  $physicalAddress->setBin(3, 0xf4);
  echo "physicalAddress: " . $physicalAddress->toId() . "\n";
  echo "physicalAddress: " . $physicalAddress->toHex() . "\n";
  $clk_dec->physicalAddress->setBin(0, 0xf1);
  $clk_dec->physicalAddress->setBin(1, 0xf2);
  $clk_dec->physicalAddress->setBin(2, 0xf3);
  $clk_dec->physicalAddress->setBin(3, 0xf4);
  echo "clk.physicalAddress: " . $clk_dec->physicalAddress->toId() . "\n";
  echo "clk.physicalAddress: " . $clk_dec->physicalAddress->toHex() . "\n";;
  echo "manufacturerIdentity: " .
      Binary::bufToId($clk_dec->manufacturerIdentity, 3) . "\n";
  $clk_dec->revisionData->textField = "This is a test";
  echo "revisionData: " . $clk_dec->revisionData->textField . "\n";

  # test send;
  setPriority1(147);
  setPriority1(153);
}

if(count($argv) > 1)
  $cfg_file = $argv[1];
else
  $cfg_file = DEF_CFG_FILE;
echo "Use configuration file $cfg_file\n";
main($cfg_file);
$sk->close();

/*********************************************/
// Use "Dynamically loaded extensions"
// Enable "enable_dl = On" in /etc/php/*/cli/php.ini
//
// Run ./php_ini.sh to create php.ini
//
// If libpmc and library is not installed in system, run with:
// LD_LIBRARY_PATH=.. PHPRC=. ./test.php
?>
