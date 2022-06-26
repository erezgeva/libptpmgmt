/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 */

%perlcode %{

############# Class : PtpMgmtLib::MessageDispatcher ##############

package PtpMgmtLib::MessageDispatcher;
use vars qw(@ISA %OWNER %ITERATORS %BLESSEDMEMBERS);
@ISA = qw( PtpMgmtLib );
%OWNER = ();
%ITERATORS = ();
sub callHadler {
    my ($self, $msg, $tlv_id, $btlv) = @_;
    die "you must use Message object with $self->callHadler"
        unless defined $msg and ref $msg eq 'PtpMgmtLib::Message';
    if(ref $btlv eq 'PtpMgmtLib::BaseMngTlv') {
        die "You must provide TLV ID" unless defined $tlv_id;
    } else {
        $tlv_id = $msg->getTlvId();
        $btlv = $msg->getData();
    }
    if (ref $btlv eq 'PtpMgmtLib::BaseMngTlv' and $msg->isValidId($tlv_id)) {
        my $idstr = PtpMgmtLib::Message::mng2str_c($tlv_id);
        my $tlv;
        my $callback_name="${idstr}_h";
        if ($self->can($callback_name) and
            eval "\$tlv = PtpMgmtLib::conv_$idstr(\$btlv)" and defined $tlv) {
            $self->$callback_name($msg, $tlv, $idstr);
            return;
        }
    } elsif($self->can(noTlv)) {
        $self->noTlv($msg);
    }
}
sub new {
    my $pkg = shift;
    die __PACKAGE__ . " is an abstract class and cannot be instantiated directly"
        if $pkg eq __PACKAGE__;
    my $self = bless {}, $pkg;
    $self;
}

sub DESTROY {
    return unless $_[0]->isa('HASH');
    my $self = tied(%{$_[0]});
    return unless defined $self;
    delete $ITERATORS{$self};
    if (exists $OWNER{$self}) {
        delete $OWNER{$self};
    }
}

sub DISOWN {
    my $self = shift;
    my $ptr = tied(%$self);
    delete $OWNER{$ptr};
}

sub ACQUIRE {
    my $self = shift;
    my $ptr = tied(%$self);
    $OWNER{$ptr} = 1;
}


############# Class : PtpMgmtLib::MessageBulder ##############

package PtpMgmtLib::MessageBulder;
use vars qw(@ISA %OWNER %ITERATORS %BLESSEDMEMBERS);
@ISA = qw( PtpMgmtLib );
%OWNER = ();
%ITERATORS = ();
sub buildTlv {
    my ($self, $actionField, $tlv_id) = @_;
    $m_msg = $self->{m_msg};
    if($actionField == $PtpMgmtLib::GET or PtpMgmtLib::Message::isEmpty($tlv_id)) {
        return $m_msg->setAction($actionField, $tlv_id);
    }
    my $idstr = PtpMgmtLib::Message::mng2str_c($tlv_id);
    my $tlv_pkg="PtpMgmtLib::${idstr}_t";
    my $cnt;
    eval "\$cnt = scalar  %{ ${tlv_pkg}:: }";
    my $callback_name="${idstr}_b";
    if ($cnt > 0 and $self->can($callback_name)) {
         my $tlv;
         eval "\$tlv = ${tlv_pkg}\->new";
         if (ref $tlv eq $tlv_pkg and
             $self->$callback_name($m_msg, $tlv) and
             $m_msg->setAction($actionField, $tlv_id, $tlv)) {
             $self->{m_tlv} = $tlv;
             return 1; # true
         }
    }
    0; # false
}
sub new {
    my ($pkg, $msg) = @_;
    die __PACKAGE__ . " is an abstract calss and cannot be instantiated directly"
        if $pkg eq __PACKAGE__;
    die "you must initialize $pkg with Message object"
        if ref $msg ne 'PtpMgmtLib::Message';
    my $self = bless { m_msg => $msg }, $pkg;
    $self;
}

sub DESTROY {
    local($., $@, $!, $^E, $?);
    return unless $_[0]->isa('HASH');
    my $self = tied(%{$_[0]});
    return unless defined $self;
    delete $ITERATORS{$self};
    $self->{m_msg}->clearData();
    if (exists $OWNER{$self}) {
        delete $OWNER{$self};
    }
}

sub DISOWN {
    my $self = shift;
    my $ptr = tied(%$self);
    delete $OWNER{$ptr};
}

sub ACQUIRE {
    my $self = shift;
    my $ptr = tied(%$self);
    $OWNER{$ptr} = 1;
}
%}
