#!/usr/bin/perl
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# update_changelog:
#   Copy changelog from debian to other distributions changelog
# update_doxygen:
#   Update doxygen configuration file
# update_goarch:
#   Create converting scripts from configure/GNU target architecture
#    to GOARCH using Debian dpkg-architecture tool.
#   See: https://pkg.go.dev/internal/goarch
#        https://gist.github.com/asukakenji/f15ba7e588ac42795f421b48b8aede63
#        https://wiki.debian.org/SupportedArchitectures
# format:
#   extra formatter, run after astyle
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
###############################################################################
use File::Basename;
use File::Touch;
use Cwd 'realpath';

###############################################################################
# update_changelog.pl

sub update_changelog()
{
  my @keep;
  my $mail;
  my $ver;
  my $key = qr/[A-Z][a-z][a-z]/;
  my $debfile='debian/changelog';
  my $rpmfile='rpm/libptpmgmt.spec';
  my $arcfile='archlinux/changelog';
  my $first;
  exit unless -f $debfile and -f $rpmfile and -f $arcfile;
  `./tools/new_version.sh -u`;
  open IN, $rpmfile or die "Fail open $rpmfile: $!";
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
    if(/^libptpmgmt \(([^)]+)/) {
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

###############################################################################
# update_doxygen.pl

sub update_doxygen_file
{
  my $cfg= shift;
  return unless -f $cfg;
  my $yes = 'YES';
  my $key_reg=qr([A-Z0-9_]+); # regular expression to catch a key
  # These options are always used
  my %options = (
    CASE_SENSE_NAMES => $yes, # Default is system dependent.
  );
  # Store current used options
  my %cur_options = %options;
  for((split (/\n/,`doxygen -x $cfg`))) {
    $cur_options{$1} = $2 if /^($key_reg)\s*=\s*(.*)/;
  }
  # Unmark all options, so the double comments retain thier place
  # Also catch any option that default value is changed on new version
  {
    local($^I, @ARGV) = ('', ($cfg));
    my $cont_comm = 0; # Continue strip comments on multiple lines option
    while (<>) {
      if(/^#([A-Z])/) {
        $cont_comm = 0;
        s/^#//;
        $cont_comm = 1 if m#\\$#;
      } elsif(/^#/ and $cont_comm) {
        s/^#//;
        $cont_comm = 0 unless m#\\$#;
      } else {
        $cont_comm = 0;
      }
      print;
    }
  }
  # Store current used options on new version
  for((split (/\n/,`doxygen -x $cfg`))) {
    $options{$1} = $2 if /^($key_reg)\s*=\s*(.*)/;
  }
  # DEBUG: See options
  #for(keys %options) {
  #  print "$_=$options{$_}\n";
  #}
  # Update doxygen to new version
  `doxygen -u $cfg`;
  # Move double comments before first option, to begin of file
  my $first_option;
  my @first_2comments;
  open IN, $cfg;
  while(<IN>) {
    push @first_2comments, $_ if /^##/;
    if(/^($key_reg)\s*=/) {
        $first_option = $1;
        last;
    }
  }
  {
    local($^I, @ARGV) = ('', ($cfg));
    my $escape_dcmt = 1; # Remove double comments before first option
    my $last_empty = 0; # Prevent sequance of empty lines
    my $cont_comm = 0; # Continue comment a multiple lines option
    while (<>) {
      # print double comments at the begin of file
      if(0 < scalar @first_2comments) {
        print for(@first_2comments);
        @first_2comments=();
      }
      next if /^##/ and $escape_dcmt;
      if(/^$/) {
        $cont_comm = 0;
        next if $last_empty;
        $last_empty = 1;
      } else {
        $last_empty = 0;
      }
      #print STDOUT "$. $_"; # DEBUG: print file
      if(/^($key_reg)\s*=(.*)/) {
        $cont_comm = 0;
        my $key = $1;
        my $val = $2;
        $escape_dcmt = 0 if $key eq $first_option;
        unless(exists $options{$key}) {
          s/^/#/;
          $cont_comm = 1 if $val =~ m#\\$#;
        }
      } elsif(/^ / and $cont_comm) {
        s/^/#/;
        $cont_comm = 0 unless m#\\$#;
      } else {
        $cont_comm = 0;
      }
      print;
    }
  }
  # New options added due to default value changed on new version!
  for(keys %options) {
    print "New option: $_\n" unless exists $cur_options{$_};
  }
  unlink "$cfg.bak" if -f "$cfg.bak";
}
sub update_doxygen
{
 update_doxygen_file 'tools/doxygen.cfg.in';
 update_doxygen_file 'tools/doxygen.clkmgr.cfg.in';
}

###############################################################################
# update_goarch.pl

my $dpkg_arc;
sub gnu
{
  my $deb_arch = shift;
  local $_ = `$dpkg_arc -qDEB_TARGET_GNU_CPU -a$deb_arch 2> /dev/null`;
  s/\n//;
  $_;
}
sub case
{
  my ($goarch, $gnu) = @_;
  return <<"EOF";
    $gnu) GOARCH='$goarch';;
EOF
}
sub update_goarch
{
  $dpkg_arc=`which dpkg-architecture 2> /dev/null`;
  $dpkg_arc =~ s/\n//;
  # Make sure we have the tool
  return unless $dpkg_arc ne '' and -x $dpkg_arc;
  my $fname = 'tools/goarch.sh';
  open OUT, "> $fname";
  # Header
  print OUT <<'EOF';
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Convert GNU target to GOARCH
###############################################################################
main()
{
  [[ -n "$1" ]] || return
  local GOARCH
  case "$1" in
EOF
  print OUT case('386', gnu('i386')); # GO arch and Debian different
  my $same, $s;
  for(qw(amd64 arm arm64 ppc64 mips mips64 riscv64 s390 s390x sparc sparc64)) {
    my $gnu = gnu($_);
    if($gnu eq $_) {
      $same="$same$s$_";
      $s = '|';
    } else {
      print OUT case($_, $gnu);
    }

  }
  # Closing
  # For unkown architecture, return the GNU architecture, they might be equal.
  print OUT "    $same)";
  print OUT <<'EOF';
 GOARCH="$1";;
    *)
      if [[ -n "`which dpkg-architecture 2> /dev/null`" ]]; then
        GOARCH="`dpkg-architecture -qDEB_TARGET_GNU_CPU -a$1 2> /dev/null`"
      else
        GOARCH="$1"
      fi
      ;;
  esac
  printf "$GOARCH"
}
main "$@"
EOF
  close OUT;
  chmod 0755, $fname;
}

###############################################################################
# format.pl

sub do_format
{
  my ($file, $exit);
  sub err
  {
    print STDERR "Check: $file:$.: " . shift . ": $_\n";
    $exit = -1;
  }
  my @list = @ARGV;
  push @list, glob
    "*/*.sh */*/*.sh Makefile */Makefile w*/*/Makefile debian/rules " .
    "*/utest/Makefile debian/changelog debian/copyright */*/test.* " .
    "*/*.pl */*/*.pl */*/*test/*.go */*.i";
  for(@list)
  {
    next if -l or not -f;
    $file = $_;
    # As script removes empty lines only
    # Use touch to prevent compilation
    my $touch_obj = File::Touch->new(reference => $file);
    local @ARGV = ($file);
    local $^I = '';
    # skip empty lines at start of
    my $not_first_empty_lines = 0;
    # combine empty lines into single empty line
    # and skip empty lines at end of file
    my $had_empty_lines = 0;
    while(<>) {
      #######################################
      # Handle empty lines
      s/\s*$//;
      if(/^$/) {
        $had_empty_lines = 1 if $not_first_empty_lines;
        next;
      }
      print "\n" if $had_empty_lines;
      $not_first_empty_lines = 1;
      $had_empty_lines = 0;
      #######################################
      # Verify we use proper characters!
      if($file =~ /\.cpp$/ or $file =~ /\.h$/ or $file =~ /\.hpp$/) {
        if(/[^a-zA-Z0-9{}()<>©~"'?:@&;%!.,*#_^+=| \[\]\$\/\\-]/) {
          err 'for wrong char';
        } elsif(/\\[^xntrbf0"'\\]/) {
          err 'wrong escape char';
        }
        if(/\*INDENT-ON\*/i || /\*INDENT-OFF\*/i || /\*NOPAD\*/i) {
          err 'for using astyle skip';
        }
        if(/[^"_]NULL\b/) {
          err 'use C++ nullptr';
        }
        #######################################
        # proper comments in headers
        if($file =~ /\.h$/) {
          if(m#^//# or m#[^:]//#) { # Ignore protocol '://' sign
            err 'use C comments only';
          }
        }
      } elsif($file =~ /\.sh$/) {
        if(/[^a-zA-Z0-9{}()<>©~"'?:@&;%!.,*#_^+=| \[\]\$\/\\`-]/) {
          err 'for wrong char';
        } elsif(/\\[^0-9nes".()\[\]\$\/\\]/) {
          err 'wrong escape char';
        }
      } else {
        if(/[^a-zA-Z0-9{}()<>©~"'?:@&;%!.,*#_^+=| \[\]\$\/\\\t`-]/) {
          err 'for wrong char';
        } elsif(/[^\t]\t/) {
          err 'Tabs are allowed only in begining';
        }
      }
      if(/.{85}/) {
        err 'line is too long';
      }
      print "$_\n";
    }
    $touch_obj->touch($file);
  }
  exit $exit if $exit;
}

###############################################################################
chdir dirname(realpath($0)) . "/..";
my $n = basename($0);
if($n eq 'update_changelog.pl') {
  update_changelog;
} elsif($n eq 'update_doxygen.pl') {
  update_doxygen;
} elsif($n eq 'update_goarch.pl') {
  update_goarch;
} elsif($n eq 'format.pl') {
  do_format;
}
