#!/usr/bin/perl
use File::Copy;
my $inName = "EvolverData_yacc.cpp";
    my $outName = "temp.cpp";
print "replacing $inName with $outName";
open (my $in, "<", $inName)
    or die ("Could not open $inName: $!\n");
open (my $out, ">", $outName)
    or die ("Could not open $outName: $!\n");

while (<$in>)
{
    s/y\.tab\.h/EvolverData_yacc.h/;
    print $out $_;
}

close ($in)
    or die "$in: $!";
close ($out)
    or die "$out: $!";
move($outName, $inName);
