#!/usr/bin/perl

use strict;

my ($id_file, $rc_file);

$id_file = "resource.h";
$rc_file = "Script1.rc";

my ($id_h, $rc_h);
my ($num_id, $num_rc);
$num_id = 0;
$num_rc = 0;

my %h;

open($id_h,"<", $id_file) || die "Canot open $id_file: $!\n";
open($rc_h,"<", $rc_file) || die "Canot open $rc_file: $!\n";

while(<$id_h>) {
    if(/^#define (IDS_.+?)\s+(\d+)/) {
        my $key = $1;
        my $val = $2;
        chomp $val;

        #print $key, "->", $val, "\n";

        $h{$key} = [$val];
        $num_id = $num_id + 1;
    }
}

while(<$rc_h>) {
    # see "man perlre" for explanation of "match a double-quoted string" capturing
    if(/^\s+(IDS_.+?)\s+\"((?:[^"\\]++|\\.)*+)\"/) {
        my $key = $1;
        my $val = $2;

        #print $key, "->", $val, "\n";
        $num_rc = $num_rc + 1;

        if(exists $h{$key}) {
            push @{$h{$key}}, $val;
        } else {
            print "Unmatched key: $key\n";
        }
    }
}

my ($key, $val);
while(($key, $val) = each %h) {
    print $key, "->", @{$val}, "\n";
}

my $header_file = "strings.res.h";
my $src_file = "strings.res.cpp";

my ($header_h, $src_h);

open($header_h,">", $header_file) || die "Canot open file: $!\n";
open($src_h,">", $src_file) || die "Canot open file: $!\n";

print $header_h "#ifndef STRINGS_H\n#define STRINGS_H\n\nextern const char* StringIds[];\n\nenum eStringIds {\n\tIDS_undefined_string_sebi = 0,\n";
print $src_h "const char* StringIds[] = { \n\t\"undefined_string_sebi\",\n";

my $i = 1;
while(($key, $val) = each %h) {
    print $key, "->", @{$val}, "\n";
    print $header_h "\t", $key, "\t = ", $i, ",\n";
    print $src_h "\t\"", ${$val}[1], "\",\n";
    $i = $i + 1;
}

print $header_h "};\n\n#endif // STRINGS_H\n";
print $src_h "};\n";

close($header_h);
close($src_h);

print "num id: $num_id\n";
print "num rc: $num_rc\n";
