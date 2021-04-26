#!/usr/bin/perl
# SPDX-License-Identifier: GPL-3.0-or-later

# copy changelog from debian to rpm specificaton file
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
# @copyright GNU General Public License 3.0 or later

sub main()
{
  my @keep;
  my $mail;
  my $ver;
  my $key = qr/[A-Z][a-z][a-z]/;
  my $debfile='debian/changelog';
  my $rpmfile='rpm/libpmc.spec';
  my $arcfile='archlinux/changelog';
  my $first;
  exit unless -f $debfile and -f $rpmfile;
  open IN, $rpmfile or die "Fail open $debfile: $!";
  my @org = <IN>;
  close IN;
  open OUT, ">$rpmfile" or die "Fail write file $rpmfile: $!";
  for(@org) {
    print OUT;
    last if /^%changelog/;
  }
  open OUT2, ">$arcfile" or die "Fail write file $arcfile: $!";
  open IN, $debfile or die "Fail open $debfile: $!";
  while(<IN>) {
    chomp;s/\s*$//;
    next if /^$/;
    if(/^libpmc \(([^)]+)/) {
      $ver=$1;
      next;
    }
    if(/^ -- [^<]+<([^>]+)>  ($key), (\d\d) ($key) (\d\d\d\d) /) {
      if(defined $first) {
        print OUT "\n";
        print OUT2 "\n";
      }
      print OUT "* $2 $4 $3 $5 $1 $ver-1\n";
      print OUT2 "* $2 $4 $3 $5 $1 $ver-1\n";
      print OUT "$_\n" for @keep;
      print OUT2 "$_\n" for @keep;
      $first=1;
      @keep=();
      next;
    }
    s/^  //;s/^\*/-/;
    push @keep, $_;
  }
  close IN;
  close OUT;
  close OUT2;
}
main;
