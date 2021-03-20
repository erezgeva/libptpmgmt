#!/usr/bin/perl
# SPDX-License-Identifier: GPL-3.0-or-later

# testing for Perl wrapper of libpmc
#
# @author Erez Geva <ErezGeva2@gmail.com>
# @copyright 2021 Erez Geva

BEGIN { push @INC, '.' }

use PmcLib;

sub main
{
    my $sk = shift;
    my $cfg_file = $ARGV[0];
    $cfg_file = '/etc/linuxptp/ptp4l.conf' unless -f $uds_address;
    print "cfg_file=$cfg_file\n";

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

    my $buf = PmcLib::buffer->new;
    die unless $buf->alloc(2000);
    my $err = $msg->build($buf->getBuf(), $buf->getSize(), 1);

    die "build error $msg->err2str_c($err)\n" unless
            $err == $PmcLib::MNG_PARSE_ERROR_OK;

    die "send" unless $sk->send($buf->getBuf(), $msg->getMsgLen());

    unless($sk->poll(500)) {
        print "timeout";
        return;
    }

    my $cnt = $sk->rcv($buf->getBuf(), $buf->getSize());
    if($cnt <= 0) {
        print "rcv $cnt\n";
        return;
    }

    $err = $msg->parse($buf->getBuf(), $cnt);

    if($err == $PmcLib::MNG_PARSE_ERROR_MSG) {
        print "error message\n";
    } elsif($err != $PmcLib::MNG_PARSE_ERROR_OK) {
        print "Parse error $msg->err2str_c($err)\n";
    } else {
        my $rid = $msg->getTlvId();
        if ($rid == $id) {
            my $user = PmcLib::conv_USER_DESCRIPTION($msg->getData());
            print "get user desc: " .
                $user->swig_userDescription_get()->swig_textField_get() .
                "\n";
        } else {
            print "Get reply for $msg->mng2str_c($rid)\n";
        }
    }
    # test setting value
    my $clk_dec = PmcLib::CLOCK_DESCRIPTION_t->new;
    $clk_dec->swig_clockType_set(0x800);
}
my $sk = PmcLib::sockUnix->new;
main $sk;
$sk->close();

# If libpmc library is not installed in system,
#  run with: LD_LIBRARY_PATH=.. ./test.pl
