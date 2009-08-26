#!/usr/bin/perl
use File::Copy;
print 'replacing _(';
open (my $in, "<", "foam.vcproj")
    or die ("Could not open foam.vcproj: $!\n");
open (my $out, ">", "foam.tmp")
    or die ("Could not open foam.tmp: $!\n");

while (<$in>)
{
    s/_\(/\$\(/;
    print $out $_;
}

close ($in)
    or die "$in: $!";
close ($out)
    or die "$out: $!";
move("foam.tmp", "foam.vcproj");
