#!/usr/bin/perl
# SPDX-License-Identifier: GPL-3.0-or-later

# testing for Perl wrapper of libpmc
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva

BEGIN { push @INC, '.' }

use PmcLib;

use constant DEF_CFG_FILE => '/etc/linuxptp/ptp4l.conf';
use constant SIZE => 2000;

sub main
{
    my $sk = shift;
    my $cfg_file = $ARGV[0];
    $cfg_file = DEF_CFG_FILE unless -f $cfg_file;
    die "Config file $uds_address does not exist" unless -f $cfg_file;
    print "Use configuration file $cfg_file\n";

    my $cfg = PmcLib::configFile->new;
    die "configFile" unless $cfg->read_cfg($cfg_file);

    die "sockUnix" unless $sk->setDefSelfAddress() &&
                          $sk->init() &&
                          $sk->setPeerAddress($cfg);

    my $msg = PmcLib::message->new;
    die "useConfig" unless $msg->useConfig($cfg);
    my $prms = $msg->getParams();
    my $self_id = $prms->swig_self_id_get();
    $self_id->swig_portNumber_set($$); # getpid()
    $prms->swig_self_id_set($self_id);
    $prms->swig_boundaryHops_set(1);
    $msg->updateParams($prms);
    my $id = $PmcLib::USER_DESCRIPTION;
    $msg->setAction($PmcLib::GET, $PmcLib::USER_DESCRIPTION);
    my $strbuf = 'X' x SIZE; # Keep this variale as long as you use buf!
    my $buf = PmcLib::conv_buf($strbuf);
    my $sequance = 1;
    my $err = $msg->build($buf, SIZE, $sequance);
    my $t = PmcLib::message::err2str_c($err);
    die "build error $t\n" unless $err == $PmcLib::MNG_PARSE_ERROR_OK;

    die "send" unless $sk->send($buf, $msg->getMsgLen());

    # You can get file descriptor with sk->getFd() and use Perl select
    unless($sk->poll(500)) {
        print "timeout";
        return;
    }

    my $cnt = $sk->rcv($buf, SIZE);
    if($cnt <= 0) {
        print "rcv $cnt\n";
        return;
    }

    $err = $msg->parse($buf, $cnt);

    if($err == $PmcLib::MNG_PARSE_ERROR_MSG) {
        print "error message\n";
    } elsif($err != $PmcLib::MNG_PARSE_ERROR_OK) {
        my $t = PmcLib::message::err2str_c($err);
        print "Parse error $t\n";
    } else {
        my $rid = $msg->getTlvId();
        my $idstr = PmcLib::message::mng2str_c($rid);
        print "Get reply for $idstr\n";
        if ($rid == $id) {
            my $user = PmcLib::conv_USER_DESCRIPTION($msg->getData());
            print "get user desc: " .
                $user->swig_userDescription_get()->swig_textField_get() . "\n";
        }
    }

    # test setting values
    my $clk_dec = PmcLib::CLOCK_DESCRIPTION_t->new;
    $clk_dec->swig_clockType_set(0x800);
    my $physicalAddress = PmcLib::binary->new;
    $physicalAddress->set(0, 0xf1);
    $physicalAddress->set(1, 0xf2);
    $physicalAddress->set(2, 0xf3);
    $physicalAddress->set(3, 0xf4);
    print("physicalAddress: " . $physicalAddress->toId() . "\n");
    print("physicalAddress: " . $physicalAddress->toHex() . "\n");
    $clk_dec->swig_physicalAddress_set($physicalAddress);
    my $clk_physicalAddress = $clk_dec->swig_physicalAddress_get();
    print("clk.physicalAddress: " . $clk_physicalAddress->toId() . "\n");
    print("clk.physicalAddress: " . $clk_physicalAddress->toHex() . "\n");
    my $manufacturerIdentity = $clk_dec->swig_manufacturerIdentity_get();
    print("manufacturerIdentity: " .
            PmcLib::binary::bufToId($manufacturerIdentity, 3) . "\n");
    $clk_dec->swig_revisionData_get()->swig_textField_set("This is a test");
    print("revisionData: " .
            $clk_dec->swig_revisionData_get()->swig_textField_get() . "\n");
}
my $sk = PmcLib::sockUnix->new;
die "Fail socket" unless defined $sk;
main $sk;
$sk->close();

# If libpmc library is not installed in system,
#  run with: LD_LIBRARY_PATH=.. ./test.pl
