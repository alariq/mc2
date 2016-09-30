#!/usr/bin/perl

use strict;
use File::Copy;



my $dir = $ARGV[0];
my $backup_pref = "orig_";
my $backup_dir = "bak";

unless(-d $backup_dir) {
    mkdir $backup_dir;
}

opendir(my $dh, $dir) || die "Can't open $dir: $!";
while (readdir $dh) {
    if(/.+\.(h|H|CPP|cpp|hpp|HPP)$/) {
        print "$dir/$_\n";

        my $out = $_;
        my $in = $backup_pref.$_;
        rename ($_, $in ) || die "Can't rename $_ : $!";
        move($in, $backup_dir) || die "Can't move file $in: $!";
        rename($backup_dir."/".$in, $backup_dir."/".$_) || die "Can't rename $backup_dir."/".$in : $!";

        open (my $inH, $backup_dir."/".$_) || die "Can't open $in : $!";
        open (my $outH,">", $out) || die "Can't open $out : $!";

        while(<$inH>) {
            if(/(.*?)\#include\s*(\<|\")\s*(\w+)(\\|\/\w+)*\.(\w{1,3})\s*(\>|\")(.*)/) {
                #chomp $_;
                print "orig: ", $_ , " -> ";
                #print "quotes: [$2] dir: [$3] file: [$4] ext: [$5]\n";
                my $prefix = $1;
                my $q = $2;
                my $eq = ">";
                $eq = "\"" if $q eq "\"";
                my $d = lc $3;
                my $f = lc substr($4, 1);
                my $e = lc $5;
                my $rest = $7;
                my $str = "";
                if($f) {
                    $str = $prefix."#include"."$q".$d."/".$f.".".$e.$eq.$rest."\n";
                } else {
                    $str = $prefix."#include".$q.$d.".".$e.$eq.$rest."\n";
                }
                print $str;
                print $outH $str;
            } else {
                print $outH $_;
            }

        }
        close($inH);
        close($outH);
        #unlink $in;
    }
}
closedir $dh;
