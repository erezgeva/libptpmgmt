#!/usr/bin/perl
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Unit test with Perl wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

use PtpMgmtLib;

{
package myDisp;
use base 'PtpMgmtLib::MessageDispatcher';
use fields qw(priority1 func id);
sub new {
    my $self = shift->SUPER::new(@_);
    $self->{priority1} = 0;
    $self->{func} = 0;
    $self->{id} = '';
    return $self;
}
sub PRIORITY1_h
{
    my ($self, $msg, $tlv, $tlv_id) = @_;
    $self->{func} |= 0x1;
    $self->{id} = $tlv_id;
    $self->{priority1} = $tlv->swig_priority1_get();
}
sub noTlv
{
    my ($self, $msg) = @_;
    $self->{func} |= 0x2;
}
sub noTlvCallBack
{
    my ($self, $msg, $tlv_id) = @_;
    $self->{func} |= 0x4;
    $self->{id} = $tlv_id;
}
} # package myDisp

{
package myBuild;
use base 'PtpMgmtLib::MessageBuilder';
use fields qw(run);
sub PRIORITY1_b
{
    my ($self, $msg, $tlv) = @_;
    $self->{run} = 1;
    $tlv->swig_priority1_set(117);
    1;
}
} # package myBuild

{
package Testing;

use base qw(Test::Class);
use Test::More;

# setup for all tests
sub setup_test : Test(setup) {
    my $self = shift;
    $self->{msg} = PtpMgmtLib::Message->new;
    $self->{disp} = myDisp->new();
    $self->{build} = myBuild->new($self->{msg});
    my $disp = $self->{disp};
    $self->{build}->{run} = 0;
}

# Tests callHadler with empty TLV
sub test_parsedCallHadlerEmptyTLV : Test(3) {
    my $self = shift;
    my $msg = $self->{msg};
    my $disp = $self->{disp};
    $disp->callHadler($msg);
    is($disp->{func}, 0x2,    'test_parsedCallHadlerEmptyTLV call noTlv');
    is($disp->{id}, '',       'test_parsedCallHadlerEmptyTLV no ID');
    is($disp->{priority1}, 0, 'test_parsedCallHadlerEmptyTLV no priority1');
}

# Tests callHadler method with TLV
sub test_parsedCallHadlerTLV : Test(3) {
    my $self = shift;
    my $msg = $self->{msg};
    my $disp = $self->{disp};
    my $tlv = PtpMgmtLib::PRIORITY1_t->new;
    $tlv->swig_priority1_set(117);
    $disp->callHadler($msg, $PtpMgmtLib::PRIORITY1, $tlv);
    is($disp->{func}, 0x1,       'test_parsedCallHadlerTLV call PRIORITY1_h');
    is($disp->{id}, 'PRIORITY1', 'test_parsedCallHadlerTLV with PRIORITY1 ID');
    is($disp->{priority1}, 117,  'test_parsedCallHadlerTLV with priority1 value');
}

# Tests callHadler method with TLV without callback
sub test_parsedCallHadlerTLVNoCallback : Test(3) {
    my $self = shift;
    my $msg = $self->{msg};
    my $disp = $self->{disp};
    my $tlv = PtpMgmtLib::PRIORITY2_t->new;
    $tlv->swig_priority2_set(117);
    $disp->callHadler($msg, $PtpMgmtLib::PRIORITY2, $tlv);
    is($disp->{func}, 0x4,
       'test_parsedCallHadlerTLVNoCallback call noTlvCallBack');
    is($disp->{id}, 'PRIORITY2',
       'test_parsedCallHadlerTLVNoCallback with PRIORITY2 ID');
    is($disp->{priority1}, 0,
       'test_parsedCallHadlerTLVNoCallback no priority1');
}

# Tests build empty TLV
sub test_buildEmptyTLV : Test(2) {
    my $self = shift;
    my $msg = $self->{msg};
    my $build = $self->{build};
    ok($build->buildTlv($PtpMgmtLib::COMMAND, $PtpMgmtLib::ENABLE_PORT),
       'test_buildEmptyTLV pass');
    is($build->{run}, 0, 'test_buildEmptyTLV do not need PRIORITY1 callback');
    $build->clear();
}

# Tests build TLV
sub test_buildTLV : Test(2) {
    my $self = shift;
    my $msg = $self->{msg};
    my $build = $self->{build};
    ok($build->buildTlv($PtpMgmtLib::SET, $PtpMgmtLib::PRIORITY1),
       'test_buildTLV pass');
    is($build->{run}, 1, 'test_buildTLV call PRIORITY1 callback');
    $build->clear();
}

# Tests build TLV that lack callback
sub test_buildTLVNoCallback : Test(2) {
    my $self = shift;
    my $msg = $self->{msg};
    my $build = $self->{build};
    ok(!$build->buildTlv($PtpMgmtLib::SET, $PtpMgmtLib::PRIORITY2),
       'test_buildTLVNoCallback no callback');
    is($build->{run}, 0, 'test_buildTLVNoCallback do not have callback');
    $build->clear();
}
} # package Testing

my $tests = Testing->new;
Test::Class->runtests($tests);
