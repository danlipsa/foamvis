#!/usr/bin/perl
use strict;
use warnings;

my $location = "/home/dlipsa/Documents/swansea-phd/foam/";
my @tests = 
    ("321_foam/sphere321_0.400000_1.100000_37_0001.dmp",
     "coarse100/coarse_01_0100_0407.dmp",
     "coarse3d/summary_30_0001_sph_free.dmp",
     "coarse_62/summary_62_0377_sph_free.dmp",
     "cststress_vfm/cststress_16_0001.dmp",
     "ctrctn/dump_0.1520_0.2400_8.0000_0001.dmp",
     "ellipse_in_flow/ell0.700000_344v_5.000000_1.dmp",
     "periodicshear/shear_65_0001_2.dmp",
     "sphereall/sphere_0.711204_144_0001.dmp",
     "shear_160/shear_160_0000_1.dmp",
     "straight_5x2q_2per/straightq_2per_10_0001.dmp",
     "straight_6x2q_2/straightq_2_12_0001.dmp",
     "wetfoam/dump_050.dmp",

     "all_outside.fe",
     "dry_cross_4.fe",
     "dump_0.1520_0.2400_8.0000_0600.dmp",
     "dump_0.1520_0.2400_8.0000_0039.dmp",
     "flowerc_10_001c.dmp",
     "flower.fe",
     "hex.fe",
     "hex_0016_9.643703.dmp",
     "outside.fe",
     "outside_first.fe",
     "outside_original.dmp",
     "per_1_400v_0.0050_0.0005.dmp",
     "pull2_bi_204_00099.dmp",
     "saus_4h_703v_0.130000_0.065000_0.680678_0197.dmp",
     "tor2d.fe",
     "twointor.fe",
     "wet_surface_vert.fe.dmp",
     "wetfoam_100_0002.dmp",
     "dump_0.1480_0.2400_11.0000_0729.dmp",
     "tmp_in.dmp"
    );


sub main ()
{
    my $foam;
    if ($ENV{'OSTYPE'} eq "darwin10.0")
    {
	$foam = "./foam_debug.app/Contents/MacOS/foam_debug";
    }
    else
    {
	$foam = "./foam_debug";
    }

    foreach (@tests)
    {
	my $test = $_;
	my @args = ($foam, $location . $test);
	system(@args) == 0
	    or warn "system @args failed: $?"
    }
}

main ();


