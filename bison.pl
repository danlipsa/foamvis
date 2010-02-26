#!/usr/bin/perl
use File::Copy;

my @args = ("bison", @ARGV);
system(@args) == 0
    or die "system @args failed: $?";

my $inName = "EvolverData.tab.c";
my $outName = "temp.cpp";
print "replacing .tab.h with _yacc.h in $inName\n";
open (my $in, "<", $inName)
    or die ("Could not open $inName: $!\n");
open (my $out, ">", $outName)
    or die ("Could not open $outName: $!\n");

while (<$in>)
{
    # Linux and Mac OS X
    s/EvolverData\.tab\.h/EvolverData_yacc.h/;
    # Windows XP
    s/y\.tab\.h/EvolverData_yacc.h/;
    print $out $_;
}

close ($in)
    or die "$in: $!";
close ($out)
    or die "$out: $!";
move($outName, $inName);
