#!/usr/bin/perl
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# testing for Perl wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
###############################################################################

use PtpMgmtLib;

{
package myDisp;
use base 'PtpMgmtLib::MessageDispatcher';
sub PRIORITY1_h
{
  my ($self, $msg, $tlv, $tlv_id) = @_;
  print "Get reply for $tlv_id\n" .
        "priority1: " . $tlv->swig_priority1_get() . "\n";
}
sub USER_DESCRIPTION_h
{
  my ($self, $msg, $tlv, $tlv_id) = @_;
  print "Get reply for $tlv_id\n";
  print "get user desc: " .
        $tlv->swig_userDescription_get()->swig_textField_get() . "\n";
}
} # package myDisp

{
package myBuild;
use base 'PtpMgmtLib::MessageBuilder';
sub PRIORITY1_b
{
  my ($self, $msg, $tlv) = @_;
  $tlv->swig_priority1_set($self->{pr});
  1;
}
} # package myBuild

use constant DEF_CFG_FILE => '/etc/linuxptp/ptp4l.conf';

my $sk = PtpMgmtLib::SockUnix->new;
die "Fail socket" unless defined $sk;
my $msg = PtpMgmtLib::Message->new;
my $buf = PtpMgmtLib::Buf->new(1000);
my $opt = PtpMgmtLib::Options->new;
my $dispacher = myDisp->new();
my $builder = myBuild->new($msg);
my $sequence = 0;

sub printError
{
  if(PtpMgmtLib::Error::isError()) {
    print PtpMgmtLib::Error::getError() . "\n";
  } else {
    print(shift . "\n");
  }
  return -1
}

sub nextSequence
{
  # Ensure sequence in in range of unsigned 16 bits
  $sequence = 1 if ++$sequence > 0xffff;
  $sequence;
}

sub setPriority1
{
  my $useBuild = 1;
  my $newPriority1 = shift;
  my $id = $PtpMgmtLib::PRIORITY1;
  my $pr1;
  if ($useBuild) {
    $builder->{pr} = $newPriority1;
    $builder->buildTlv($PtpMgmtLib::SET, $id);
  } else {
    $pr1 = PtpMgmtLib::PRIORITY1_t->new;
    $pr1->swig_priority1_set($newPriority1);
    $msg->setAction($PtpMgmtLib::SET, $id, $pr1);
  }
  my $seq = nextSequence;
  my $err = $msg->build($buf, $seq);
  my $txt = PtpMgmtLib::Message::err2str_c($err);
  die "build error $txt\n" if $err != $PtpMgmtLib::MNG_PARSE_ERROR_OK;
  die "send" unless $sk->send($buf, $msg->getMsgLen());
  $msg->clearData();

  return printError("timeout") unless $sk->poll(500);

  my $cnt = $sk->rcv($buf);
  return printError("rcv $cnt") if $cnt <= 0;

  $err = $msg->parse($buf, $cnt);
  if($err != $PtpMgmtLib::MNG_PARSE_ERROR_OK || $msg->getTlvId() != $id ||
     $seq != $msg->getSequence()) {
    print "set fails\n";
    return -1;
  }
  print "set new priority $newPriority1 success\n";

  $msg->setAction($PtpMgmtLib::GET, $id);
  $seq = nextSequence;
  $err = $msg->build($buf, $seq);
  $txt = PtpMgmtLib::Message::err2str_c($err);
  die "build error $txt\n" if $err != $PtpMgmtLib::MNG_PARSE_ERROR_OK;

  die "send" unless $sk->send($buf, $msg->getMsgLen());

  return printError("timeout") unless $sk->poll(500);

  my $cnt = $sk->rcv($buf);
  return printError("rcv $cnt") if $cnt <= 0;
  $err = $msg->parse($buf, $cnt);
  if($err == $PtpMgmtLib::MNG_PARSE_ERROR_MSG) {
    print "error message\n";
  } elsif($err != $PtpMgmtLib::MNG_PARSE_ERROR_OK) {
    $txt = PtpMgmtLib::Message::err2str_c($err);
    print "Parse error $txt\n";
  } else {
    $dispacher->callHadler($msg, $msg->getTlvId(), $msg->getData());
    return 0;
  }
  -1;
}

sub main
{
  die "buffer allocation failed" unless $buf->isAlloc();
  die  "fail parsing command line"
    if $opt->parse_options([$0, @ARGV]) != $PtpMgmtLib::Options::OPT_DONE;
  my $cfg_file = $opt->val('f');
  $cfg_file = DEF_CFG_FILE unless -f $cfg_file;
  die "Config file $uds_address does not exist" unless -f $cfg_file;
  print "Use configuration file $cfg_file\n";

  my $cfg = PtpMgmtLib::ConfigFile->new;
  die "ConfigFile" unless $cfg->read_cfg($cfg_file);

  die "SockUnix" unless $sk->setDefSelfAddress() &&
    $sk->init() && $sk->setPeerAddress($cfg);

  die "useConfig" unless $msg->useConfig($cfg);
  my $prms = $msg->getParams();
  my $self_id = $prms->swig_self_id_get();
  $self_id->swig_portNumber_set($$ & 0xffff); # getpid()
  $prms->swig_self_id_set($self_id);
  $prms->swig_domainNumber_set($cfg->domainNumber());
  $msg->updateParams($prms);
  $msg->useConfig($cfg);
  my $id = $PtpMgmtLib::USER_DESCRIPTION;
  $msg->setAction($PtpMgmtLib::GET, $id);
  my $seq = nextSequence;
  my $err = $msg->build($buf, $seq);
  my $txt = PtpMgmtLib::Message::err2str_c($err);
  die "build error $txt\n" if $err != $PtpMgmtLib::MNG_PARSE_ERROR_OK;

  die "send" unless $sk->send($buf, $msg->getMsgLen());

  # You can get file descriptor with sk->fileno() and use Perl select
  return printError("timeout") unless $sk->poll(500);

  my $cnt = $sk->rcv($buf);
  return printError("rcv $cnt") if $cnt <= 0;

  $err = $msg->parse($buf, $cnt);

  if($err == $PtpMgmtLib::MNG_PARSE_ERROR_MSG) {
    print "error message\n";
  } elsif($err != $PtpMgmtLib::MNG_PARSE_ERROR_OK) {
    $txt = PtpMgmtLib::Message::err2str_c($err);
    print "Parse error $txt\n";
  } else {
    $dispacher->callHadler($msg);
  }

  # test setting values
  my $clk_dec = PtpMgmtLib::CLOCK_DESCRIPTION_t->new;
  $clk_dec->swig_clockType_set(0x800);
  my $physicalAddress = PtpMgmtLib::Binary->new;
  $physicalAddress->setBin(0, 0xf1);
  $physicalAddress->setBin(1, 0xf2);
  $physicalAddress->setBin(2, 0xf3);
  $physicalAddress->setBin(3, 0xf4);
  print("physicalAddress: " . $physicalAddress->toId() . "\n");
  print("physicalAddress: " . $physicalAddress->toHex() . "\n");
  $clk_dec->swig_physicalAddress_set($physicalAddress);
  my $clk_physicalAddress = $clk_dec->swig_physicalAddress_get();
  print("clk.physicalAddress: " . $clk_physicalAddress->toId() . "\n");
  print("clk.physicalAddress: " . $clk_physicalAddress->toHex() . "\n");
  my $manufacturerIdentity = $clk_dec->swig_manufacturerIdentity_get();
  print("manufacturerIdentity: " .
    PtpMgmtLib::Binary::bufToId($manufacturerIdentity, 3) . "\n");
  $clk_dec->swig_revisionData_get()->swig_textField_set("This is a test");
  print("revisionData: " .
    $clk_dec->swig_revisionData_get()->swig_textField_get() . "\n");

  setPriority1(147);
  setPriority1(153);

  my $event = PtpMgmtLib::SUBSCRIBE_EVENTS_NP_t->new;
  $event->setEvent($PtpMgmtLib::NOTIFY_TIME_SYNC);
  print("getEvent(NOTIFY_TIME_SYNC)=" .
        ($event->getEvent($PtpMgmtLib::NOTIFY_TIME_SYNC) ? 'have' : 'not') . "\n" .
        "getEvent(NOTIFY_PORT_STATE)=" .
        ($event->getEvent($PtpMgmtLib::NOTIFY_PORT_STATE) ? 'have' : 'not') . "\n");

  # test SigEvent that represent std::vector<SLAVE_TX_EVENT_TIMESTAMPS_rec_t>
  # See std_vectors.md for more information
  my $evnts = PtpMgmtLib::SigEvent->new;
  my $e = PtpMgmtLib::SLAVE_TX_EVENT_TIMESTAMPS_rec_t->new;
  $e->swig_sequenceId_set(1);
  my $t = PtpMgmtLib::Timestamp_t->new;
  $t->fromFloat(4.5);
  $e->swig_eventEgressTimestamp_set($t);
  $evnts->push($e);
  print("Events size " . $evnts->size() .
        ", seq[0]=" . $evnts->get(0)->swig_sequenceId_get() .
        ", ts[0]=" . $evnts->get(0)->swig_eventEgressTimestamp_get()->string() .
        "\n");
  0;
}
main;
$sk->close();

# LD_PRELOAD=../../libptpmgmt.so PERL5LIB=. ./test.pl
