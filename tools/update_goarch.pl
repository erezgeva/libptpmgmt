#!/usr/bin/perl
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# Create converting scripts from configure/GNU target architecture
#  to GOARCH using Debian dpkg-architecture tool.
#
# See: https://pkg.go.dev/internal/goarch
#      https://gist.github.com/asukakenji/f15ba7e588ac42795f421b48b8aede63
#      https://wiki.debian.org/SupportedArchitectures
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
###############################################################################
use File::Basename;
use Cwd 'realpath';
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
sub main
{
  $dpkg_arc=`which dpkg-architecture 2> /dev/null`;
  $dpkg_arc =~ s/\n//;
  # Make sure we have the tool
  return unless $dpkg_arc ne '' and -x $dpkg_arc;
  my $fname = 'goarch.sh';
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
# We create the tool in the same folder :-)
chdir dirname(realpath($0));
main;
