/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

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
    my ($self, $m_msg, $b, $c, $d) = @_;
    if (defined $m_msg) {
        my $callHadler;
        die "you must use Message object with $self->callHadler"
            if ref $m_msg ne 'PtpMgmtLib::Message';
        $self->{m_msg} = $m_msg;
        if(ref $c eq 'PtpMgmtLib::BaseMngTlv') {
            die "You must provide TLV ID" unless defined $b;
            $self->{m_tlv_id} = $b;
            $self->{m_tlv} = $c;
            $callHadler = $d;
        } else {
            $self->{m_tlv_id} = $m_msg->getTlvId();
            $self->{m_tlv} = $m_msg->getData();
            $callHadler = $b;
        }
        # Default is true
        return if defined $callHadler and not $callHadler;
    } else {
        $m_msg = $self->{m_msg};
        # We need to read last value
        $self->{m_tlv_id} = $m_msg->getTlvId();
        $self->{m_tlv} = $m_msg->getData();
    }
    if(ref $m_msg ne 'PtpMgmtLib::Message') {
        warn "call $self->callHadler without setting Message object";
        return;
    }
    my $m_tlv = $self->{m_tlv};
    if (ref $m_tlv eq 'PtpMgmtLib::BaseMngTlv') {
        my $idstr = PtpMgmtLib::Message::mng2str_c($self->{m_tlv_id});
        my $tlv;
        my $callback_name="${idstr}_h";
        if ($self->can($callback_name) and
            eval "\$tlv = PtpMgmtLib::conv_$idstr(\$m_tlv)" and defined $tlv) {
            $self->$callback_name($m_msg, $tlv, $idstr);
            return;
        }
    }
    $self->noTlv($m_msg) if $self->can (noTlv);
}
sub new {
    my ($pkg, $m_msg, $callHadler) = @_;
    die "you must initialize $pkg with Message object"
        if defined $m_msg and ref $m_msg ne 'PtpMgmtLib::Message';
    my $self = bless {}, $pkg;
    $self->callHadler($m_msg, $callHadler);
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
    delete $self->{m_tlv};
    if($actionField == $PtpMgmtLib::GET or PtpMgmtLib::Message::isEmpty($tlv_id)) {
        return $m_msg->setAction($actionField, $tlv_id)
    }
    my $idstr = PtpMgmtLib::Message::mng2str_c($tlv_id);
    my $tlv_pkg="PtpMgmtLib::${idstr}_t";
    my $cnt;
    eval "\$cnt = scalar  %{ ${tlv_pkg}:: }";
    my $callback_name="${idstr}_b";
    if ($cnt > 0 and $self->can($callback_name)) {
         my $tlv;
         eval "\$tlv = ${tlv_pkg}\->new;";
         if (ref $tlv eq $tlv_pkg) {
             $self->{m_tlv} = $tlv;
             $self->{m_tlv_id} = $tlv_id;
             $self->{m_tlv_pkg} = $tlv_pkg;
             return $m_msg->setAction($actionField, $tlv_id, $tlv)
                if $self->$callback_name($m_msg, $tlv);
         }
    }
    0; # Fail
}
# Return
#   TLV object
#   TLV ID
#   TLV Perl package name
sub getTlv {
    my $self = shift;
    ($self->{m_tlv}, $self->{m_tlv_id}, $self->{m_tlv_pkg});
}
sub new {
    my ($pkg, $m_msg) = @_;
    die "you must initialize $pkg with Message object"
        if ref $m_msg ne 'PtpMgmtLib::Message';
    my $self = bless { m_msg => $m_msg }, $pkg;
    $self;
}

sub DESTROY {
    local($., $@, $!, $^E, $?);
    return unless $_[0]->isa('HASH');
    my $self = tied(%{$_[0]});
    return unless defined $self;
    delete $self->{m_tlv}; # Delete created TLV, if exist
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
%}
