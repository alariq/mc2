#!/usr/bin/perl

use strict;


opendir(DIR,".") or die "Cannot open . \n";

my @files = readdir(DIR);
closedir(DIR);
foreach(@files) {

    # all files with 1 to 3 letters extension: *.h, *.cpp, *.pl
    if(/\.([a-zA-Z]{1,3})$/ || -d ) {

        my $o = $_;

        $o = lc($o);

        if(not($_ eq $o)) {
            system("mv", $_, $o);
        }
        #print("mv ", $_, " ", $o, "\n");
    }
}

#find( \&wanted, [ "./*.mp3" ] );
