#!/usr/bin/perl

use strict;


opendir(DIR,".") or die "Cannot open . \n";

my @files = readdir(DIR);
closedir(DIR);
foreach(@files) {

    if(/\.([a-zA-Z]{3})$/) {

        my $o = $_;

        $o = lc($o);

        if(not($_ eq $o)) {
            system("mv", $_, $o);
        }
        #print("mv ", $_, " ", $o, "\n");
    }
}

#find( \&wanted, [ "./*.mp3" ] );
