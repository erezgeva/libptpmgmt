#!/usr/bin/perl
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2021 Erez Geva
#
# extra formatter, run after astyle
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
###############################################################################

use File::Touch;

sub main
{
    for(glob "*.cpp *.h *.sh Makefile debian/rules debian/changelog " .
             "debian/copyright scripts/* */test.* testJson.pl sample/*.cpp " .
             "sample/*.h")
    {
        next if -l or not -f;
        my $file = $_;
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
            if($file =~ /\.cpp$/ or $file =~ /\.h$/) {
                if(/[^a-zA-Z0-9{}()<>~"'?:@&;%!.,*#_^+=| \[\]\$\/\\-]/) {
                    print STDERR "Check: $file:$.: for wrong char: $_\n";
                } elsif(/\\[^ntr"'\\]/) {
                    print STDERR "Check: $file:$.: wrong escape char: $_\n";
                }
                if(/\*INDENT-ON\*/i || /\*INDENT-OFF\*/i || /\*NOPAD\*/i) {
                    print STDERR "Check: $file:$.: for using astyle skip: $_\n";
                }
                if(/[^"_]NULL\b/) {
                    print STDERR "Check: $file:$.: use C++ nullptr: $_\n";
                }
                if(/.{85}/) {
                    print STDERR "Check: $file:$.: line is too long: $_\n";
                }
            } else {
                if(/[^a-zA-Z0-9{}()<>~"'?:@&;%!.,*#_^+=| \[\]\$\/\\\t`-]/) {
                    print STDERR "Check: $file:$.: for wrong char: $_\n";
                } elsif(/\\[^ntrds"()<>.+\d\$'\\ ]/) {
                    print STDERR "Check: $file:$.: wrong escape char: $_\n";
                } elsif(/.\t/) {
                    print STDERR "Check: $file:$.: Tabs are allowed only in begining: $_\n";
                }
                if(/.{85}/) {
                    print STDERR "Check: $file:$.: line is too long: $_\n";
                }
            }
            #######################################
            # proper comments in headers
            if($file =~ /\.h$/) {
                if(m#^//# or m#[^:]//#) { # Ignore protocol '://' sign
                    print STDERR "Check: $file:$.: use C comments only: $_\n";
                }
            }
            print "$_\n";
        }
        $touch_obj->touch($file);
    }
}
main;
