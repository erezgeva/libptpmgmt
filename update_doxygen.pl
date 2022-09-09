#!/usr/bin/perl

sub main
{
  my $cfg='doxygen.cfg';
  return unless -f $cfg;
  `doxygen -u $cfg`;
  my $yes = 'YES';
  # These options are always used, in addition to all 'GENERATE_XX'
  my %options = (
    ABBREVIATE_BRIEF => "The",
    CASE_SENSE_NAMES => $yes,
    ENABLE_PREPROCESSING => $yes,
    HAVE_DOT => $yes,
    USE_PDFLATEX => $yes,
  );
  my $reg=qr([A-Z0-9_]+);
  for((split (/\n/,`doxygen -x $cfg`))) {
    $options{"$1"} = "$2" if /^($reg)\s*=\s*(.*)/;
  }
  #for(keys %options) {
  #  print "$_=$options{$_}\n";
  #}
  my @open;
  open IN, $cfg;
  while(<IN>) {
    push @open, "$_" if /^##/;
    last if /^PROJECT_NAME\s*=/;
  }
  {
    local($^I, @ARGV) = ('', ($cfg));
    my $escape_dcmt = 1;
    my $last_empty = 0;
    while (<>) {
        if(0 < scalar @open) {
          print for(@open);
          @open=();
        }
        next if /^##/ and $escape_dcmt;
        if(/^$/) {
          next if $last_empty;
          $last_empty = 1;
        } else {
          $last_empty = 0;
        }
        #print STDOUT "$. $_";
        if(/^($reg)\s*=(.*)/) {
            my $key = "$1";
            my $val = "$2";
            $escape_dcmt = 0 if $key eq 'PROJECT_NAME';
            s/^/# / unless exists $options{"$key"} or $key =~ /^GENERATE_/ or
                $val =~ m#\\$#;
        }
        print;
    }
  }
  unlink "$cfg.bak" if -f "$cfg.bak";
}
main;
