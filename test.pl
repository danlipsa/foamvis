#!/usr/bin/perl

use strict;
use warnings;

my $location = "/home/dlipsa/Documents/swansea-phd/foam/";
my @tests = 
    ("ctrctn/dump_0.1520_0.2400_8.0000_0001.dmp",
     "periodicshear/shear_65_0001_2.dmp",
     "coarse3d/summary_30_0001_sph_free.dmp",
     "shear_160/shear_160_0000_1.dmp",
     "coarse_62/summary_62_0377_sph_free.dmp",
     "straight_5x2q_2per/straightq_2per_10_0001.dmp",
     "straight_6x2q_2/straightq_2_12_0001.dmp",
     "sphereall/sphere_0.711204_144_0001.dmp",
     "cststress_vfm/cststress_16_0001.dmp");

sub main ()
{
    my $foam;
    if ($ENV{'OSTYPE'} eq "darwin10.0")
    {
	$foam = "./foam.app/Contents/MacOS/foam";
    }
    else
    {
	$foam = "./foam";
    }

    foreach (@tests)
    {
	my $test = $_;
	my @args = ($foam, $location . $test);
	system(@args) == 0
	    or die "system @args failed: $?"
    }
}

main ();


