#!/usr/bin/perl
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com>
#
# Unit test for signaling with Perl wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2026 Erez Geva
#
###############################################################################

use PtpMgmtLib;
use UtestLib;

{
package mySigCb;
use base 'PtpMgmtLib::MessageSigTlvCallback';
sub new {
    my $self = shift->SUPER::new(@_);
    $self->{mask} = 0;
    return $self;
}
sub callback
{
  my ($self, $msg, $tlvType, $sigTlv) = @_;
  return 1 if $tlvType != $PtpMgmtLib::MANAGEMENT;
  # First TLV
  if (PtpMgmtLib::get_MngTlvId($sigTlv) == $PtpMgmtLib::PRIORITY2) {
    my $tlv = PtpMgmtLib::get_BaseMngTlv($sigTlv);
    my $pr2 = PtpMgmtLib::conv_PRIORITY2($tlv);
    $self->{mask} += 1;
    return $pr2->swig_priority2_get() != 119; # return false on success!
  }
  # Second TLV
  if (PtpMgmtLib::get_MngTlvId($sigTlv) == $PtpMgmtLib::DOMAIN) {
    my $tlv = PtpMgmtLib::get_BaseMngTlv($sigTlv);
    my $domain = PtpMgmtLib::conv_DOMAIN($tlv);
    $self->{mask} += 10;
    return $domain->swig_domainNumber_get() != 7; # return false on success!
  }
  return 1; # return true on failure!
}
}

{
package Testing;
use base qw(Test::Class);
use Test::More;

sub test_traverseSig : Test(15) {
  my $msg = PtpMgmtLib::Message->new;
  my $buf = UtestLib::get2MngTlvsSig();
  my $prms = $msg->getParams();
  $prms->swig_rcvSignaling_set(1); # true
  $prms->swig_filterSignaling_set(0); # false
  ok($msg->updateParams($prms), 'updateParams');
  is($msg->parse($buf->swig_buf_get(), $buf->swig_size_get()),
    $PtpMgmtLib::MNG_PARSE_ERROR_SIG, 'parse');
  is($msg->getSigTlvsCount(), 2, 'getSigTlvsCount');
  is($msg->getSigTlvType(0), $PtpMgmtLib::MANAGEMENT, 'getSigTlvType');
  is($msg->getSigMngTlvType(0), $PtpMgmtLib::PRIORITY2, 'getSigMngTlvType');
  is($msg->getSigTlvType(1), $PtpMgmtLib::MANAGEMENT, 'getSigTlvType');
  is($msg->getSigMngTlvType(1), $PtpMgmtLib::DOMAIN, 'getSigMngTlvType');
  my $sigTlv = $msg->getSigTlv(0);
  is(PtpMgmtLib::get_MngTlvId($sigTlv), $PtpMgmtLib::PRIORITY2, 'get_MngTlvId');
  my $mngTlv1_1 = $msg->getSigMngTlv(0);
  my $mngTlv1_2 = PtpMgmtLib::get_BaseMngTlv($sigTlv);
  my $pr2_1 = PtpMgmtLib::conv_PRIORITY2($mngTlv1_1);
  my $pr2_2 = PtpMgmtLib::conv_PRIORITY2($mngTlv1_2);
  is($pr2_1->swig_priority2_get(), 119, 'pr2_1->priority2');
  is($pr2_2->swig_priority2_get(), 119, 'pr2_2->priority2');
  my $sigTlv = $msg->getSigTlv(1);
  is(PtpMgmtLib::get_MngTlvId($sigTlv), $PtpMgmtLib::DOMAIN, 'get_MngTlvId');
  my $mngTlv1_1 = $msg->getSigMngTlv(1);
  my $mngTlv1_2 = PtpMgmtLib::get_BaseMngTlv($sigTlv);
  my $domain1 = PtpMgmtLib::conv_DOMAIN($mngTlv1_1);
  my $domain2 = PtpMgmtLib::conv_DOMAIN($mngTlv1_2);
  is($domain1->swig_domainNumber_get(), 7, 'domain1->domainNumber');
  is($domain2->swig_domainNumber_get(), 7, 'domain2->domainNumber');
  my $cb = mySigCb->new();
  ok(not($msg->traversSigTlvsCl($cb)), 'traversSigTlvsCl');
  is($cb->{mask}, 11, 'mySigCb mask');
}

} # package Testing
my $tests = Testing->new;
Test::Class->runtests($tests);
