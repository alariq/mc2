#!/usr/bin/perl

use strict;


opendir(DIR,".") or die "Cannot open . \n";

my @files = readdir(DIR);
closedir(DIR);
foreach(@files) {

    if(/\.(cpp|CPP|h|H|hpp|HPP|hxx|HXX)$/) {

        my $o = $_;

        $o = lc($o);

        system("mv", $_, $o);
        #print("mv ", $_, " ", $o, "\n");
    }
}

#find( \&wanted, [ "./*.mp3" ] );
