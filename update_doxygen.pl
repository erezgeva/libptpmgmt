#!/usr/bin/perl

sub main
{
  my $cfg='doxygen.cfg';
  return unless -f $cfg;
  my $yes = 'YES';
  # These options are always used
  my %options = (
    CASE_SENSE_NAMES => $yes, # Default is system dependent.
    HAVE_DOT => $yes, # Default value change in 1.9.3
  );
  my $key_reg=qr([A-Z0-9_]+); # regular expression to catch a key
  # Unmark all options, so the double comments retain thier place
  # Also catch any option that default value is changed on new version
  {
    local($^I, @ARGV) = ('', ($cfg));
    my $cont_comm = 0; # Continue strip comment a multiple lines option
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
  unlink "$cfg.bak" if -f "$cfg.bak";
}
main;
