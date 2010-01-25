#!/usr/bin/perl
use strict;
use warnings;

my $location = "/home/dlipsa/Documents/phd-swansea/WIVC";

sub main ($)
{
    my $dataset = shift;
    unlink <movie/*.jpg>;
    my @args;
    if ($dataset eq "coarse3d")
    {
	@args = ("./foam", $location . "/COARSE3D",
		    "summary_30_0*.dmp");
    }
    else
    {
	@args = ("./foam", $location . "/PERIODICSHEAR",
		    "shear_65_0*_2.dmp");
    }
    system(@args) == 0
	or die "system @args failed: $?"
}

if ($#ARGV != 0)
{
    print $#ARGV, "\n";
    print "$0 <coarse3d | periodicshear>\n";
    exit(13);
}

main ($ARGV[0]);
