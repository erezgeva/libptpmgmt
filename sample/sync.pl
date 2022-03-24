#!/usr/bin/perl
# SPDX-License-Identifier: GPL-3.0-or-later

# Sample for probing synchronization state
#
# This example sample the PTP daemons once.
# For proper probing, the synchronization state should be sample periodicly!
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#

use PmcLib;
use feature 'switch'; # 'given'
no warnings 'experimental::smartmatch';

# CLIENT_SYNC_THRESHOULD should be set acording to the used system
#  and the precision, the user require from the system
use constant {
    DEF_CFG_FILE => '/etc/linuxptp/ptp4l.conf',
    BUF_SIZE => 1000,
    CLIENT_SYNC_THRESHOULD => 100, # 100 nanoseconds
};
my ($sequence, $sk, $msg, $buf) = (0);
my ($peerMeanPathDelay, $portState, $gmIdentity, $sourceOffset);

sub init
{
    $sk = PmcLib::SockUnix->new;
    die "Fail socket" unless defined $sk;

    $msg = PmcLib::Message->new;
    $buf = PmcLib::Buf->new(BUF_SIZE);
    die "buffer allocation failed" unless $buf->isAlloc();
    my $cfg_file = $ARGV[0];
    $cfg_file = DEF_CFG_FILE unless -f $cfg_file;
    die "Config file $uds_address does not exist" unless -f $cfg_file;

    my $cfg = PmcLib::ConfigFile->new;
    die "ConfigFile" unless $cfg->read_cfg($cfg_file);

    die "SockUnix" unless $sk->setDefSelfAddress() &&
                          $sk->init() &&
                          $sk->setPeerAddress($cfg);

    die "useConfig" unless $msg->useConfig($cfg);
    my $prms = $msg->getParams();
    my $self_id = $prms->swig_self_id_get();
    $self_id->swig_portNumber_set($$ & 0xffff); # getpid()
    $prms->swig_self_id_set($self_id);
    $prms->swig_boundaryHops_set(1);
    $msg->updateParams($prms);
}

sub sendId
{
    my $id = shift;
    $msg->setAction($PmcLib::GET, $id);
    my $err = $msg->build($buf, ++$sequence);
    my $txt = PmcLib::Message::err2str_c($err);
    die "build error $txt\n" if $err != $PmcLib::MNG_PARSE_ERROR_OK;
    die "send" unless $sk->send($buf, $msg->getMsgLen());
}

sub rcv
{
    my $data;
    return if $PmcLib::MNG_PARSE_ERROR_OK != $msg->parse($buf, shift);
    given($msg->getTlvId()) {
        when($PmcLib::PORT_DATA_SET) {
            $data = PmcLib::conv_PORT_DATA_SET($msg->getData());
            $peerMeanPathDelay = $data->swig_peerMeanPathDelay_get()->getIntervalInt();
            $portState = PmcLib::Message::portState2str_c($data->swig_portState_get());
        }
        when($PmcLib::PARENT_DATA_SET) {
            $data = PmcLib::conv_PARENT_DATA_SET($msg->getData());
            $gmIdentity = $data->swig_grandmasterIdentity_get()->string();
        }
        when($PmcLib::CURRENT_DATA_SET) {
            $data = PmcLib::conv_CURRENT_DATA_SET($msg->getData());
            $sourceOffset = $data->swig_offsetFromMaster_get()->getIntervalInt();
        }
    }
}

sub probe
{
    sendId(eval('$PmcLib::'.$_)) for qw(PORT_DATA_SET PARENT_DATA_SET CURRENT_DATA_SET);
    # Proper receive should check the port ID, to ensure the information match the desired port.
    for (1..3) {
        unless($sk->poll(500)) {
            print "timeout";
            return;
        }
        my $cnt = $sk->rcv($buf);
        if($cnt <= 0) {
            print "rcv $cnt\n";
            return;
        }
        rcv $cnt;
    }
    given($portState)
    {
        when('SOURCE') {
            print "Port is source clock\n";
        }
        when('CLIENT') {
            # Proper sync should probe few times, before conclude
            if (abs($sourceOffset) <= CLIENT_SYNC_THRESHOULD) {
                print "Port is sync with $sourceOffset offset from $gmIdentity source\n";
            } else {
                print "Port is NOT sync yet, current $sourceOffset offset from $gmIdentity source\n";
            }
        }
        default {
            print "Port is probing ...\n";
            return;
        }
    }
    print "Distance to peer $peerMeanPathDelay nanoseconds\n";
}

init;

probe;

$sk->close();
