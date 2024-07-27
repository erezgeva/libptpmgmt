#!/usr/bin/perl
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# extra formatter, run after astyle
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
###############################################################################
use File::Basename;
use File::Touch;
use Cwd 'realpath';

sub main
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
        "debian/changelog debian/copyright */*/test.* " .
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
            if($file =~ /\.cpp$/ or $file =~ /\.h$/) {
                if(/[^a-zA-Z0-9{}()<>©~"'?:@&;%!.,*#_^+=| \[\]\$\/\\-]/) {
                    err 'for wrong char';
                } elsif(/\\[^xntr0"'\\]/) {
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
chdir dirname(realpath($0)) . "/..";
main;
