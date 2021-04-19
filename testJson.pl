#!/usr/bin/perl
# SPDX-License-Identifier: GPL-3.0-or-later

# testing Json convertor
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva

BEGIN { push @INC, './perl' }

use PmcLib;

use constant DEF_CFG_FILE => '/etc/linuxptp/ptp4l.conf';

my $sk = PmcLib::SockUnix->new;
die "Fail socket" unless defined $sk;
my $msg;
my $buf;
my $sequence = 0;

sub runId
{
    my $id = shift;
    $msg->setAction($PmcLib::GET, $id);
    my $err = $msg->build($buf, ++$sequence);
    my $txt = PmcLib::Message::err2str_c($err);
    die "build error $txt\n" if $err != $PmcLib::MNG_PARSE_ERROR_OK;
    die "send" unless $sk->send($buf, $msg->getMsgLen());

    # You can get file descriptor with sk->getFd() and use Perl select
    unless($sk->poll(500)) {
        print "timeout";
        return;
    }

    my $cnt = $sk->rcv($buf);
    if($cnt <= 0) {
        print "rcv $cnt\n";
        return;
    }

    $msg->parse($buf, $cnt);

    my $j = PmcLib::msg2json($msg);
    print "$j,\n";
}

sub creatJsonTest
{
    $msg = PmcLib::Message->new;
    $buf = PmcLib::Buf->new(1000);
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
    $self_id->swig_portNumber_set($$); # getpid()
    $prms->swig_self_id_set($self_id);
    $prms->swig_boundaryHops_set(1);
    $msg->updateParams($prms);

    print "[\n";
    runId(eval('$PmcLib::'.$_)) for qw(ANNOUNCE_RECEIPT_TIMEOUT CLOCK_ACCURACY),
        qw(CLOCK_DESCRIPTION CURRENT_DATA_SET DEFAULT_DATA_SET DELAY_MECHANISM),
        qw(DOMAIN LOG_ANNOUNCE_INTERVAL LOG_MIN_PDELAY_REQ_INTERVAL),
        qw(LOG_SYNC_INTERVAL PARENT_DATA_SET PRIORITY1 PRIORITY2 SLAVE_ONLY),
        qw(TIMESCALE_PROPERTIES TIME_PROPERTIES_DATA_SET TRACEABILITY_PROPERTIES),
        qw(USER_DESCRIPTION VERSION_NUMBER PORT_DATA_SET TIME_STATUS_NP),
        qw(GRANDMASTER_SETTINGS_NP PORT_DATA_SET_NP PORT_PROPERTIES_NP),
        qw(PORT_STATS_NP SUBSCRIBE_EVENTS_NP SYNCHRONIZATION_UNCERTAIN_NP),
        qw(PATH_TRACE_LIST); # last one is unsupported by linuxptp
    print "{}]\n";
}
sub toJsonTest
{
    my $json2msg = PmcLib::Json2msg->new;
    my $json = <<EOF;
{
  "sequenceId" : 12,
  "sdoId" : 0,
  "domainNumber" : 0,
  "versionPTP" : 2,
  "minorVersionPTP" : 1,
  "unicastFlag" : false,
  "PTPProfileSpecific" : 0,
  "messageType" : "Management",
  "sourcePortIdentity" :
  {
    "clockIdentity" : "c47d46.fffe.20acae",
    "portNumber" : 0
  },
  "targetPortIdentity" :
  {
    "clockIdentity" : "000000.0000.000000",
    "portNumber" : 20299
  },
  "actionField" : "SET",
  "tlvType" : "MANAGEMENT",
EOF
    my $close = "}";
    my $jsonPr1 = <<EOF;
  "managementId" : "PRIORITY1",
  "dataField" :
  {
    "priority1" : 153
  }
EOF
    $json2msg->fromJson($json . $jsonPr1 . $close);
    my $jsonTbl = <<EOF;
  "managementId" : "UNICAST_MASTER_TABLE",
  "dataField" :
  {
    "logQueryInterval" : 1,
    "actualTableSize" : 3,
    "PortAddress" :
    [
      {
        "networkProtocol" : "IEEE_802_3",
        "addressField" : "c4:7d:46:20:ac:ae"
      },
      {
        "networkProtocol" : "IEEE_802_3",
        "addressField" : "c4:7d:46:20:ac:ae"
      },
      {
        "networkProtocol" : "IEEE_802_3",
        "addressField" : "c4:7d:46:20:ac:ae"
      }
    ]
  }
EOF
    $json2msg->fromJson($json . $jsonTbl . $close);
}
creatJsonTest;
toJsonTest;
$sk->close();
# dpkg --remove --force-all pmc libpmc libpmc-dev libpmc-perl
# p='pmc libpmc libpmc-dev libpmc-perl' && apt install $p && apt-mark auto $p
# jsonlint-py3 1.txt
# LD_LIBRARY_PATH=. ./testJson.pl | jsonlint-py3
