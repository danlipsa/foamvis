#!/usr/bin/perl
use File::Copy;
use strict;
use warnings;

# Replaces all occurences of strings in file `inName`. The strings to
# be replaced and their substitutions are passed as a list of pairs:
# (string, replacement)
sub replace
{
    my ($inName, $pairs) = @_;
    my $outName = "temp.txt";
    print "processing $inName ...\n";
    open (my $in, "<", $inName)
	or die ("Could not open $inName: $!\n");
    open (my $out, ">", $outName)
	or die ("Could not open $outName: $!\n");
    while (<$in>)
    {
	my $line = $_;
	foreach (@{$pairs})
	{
	    my ($src, $dest) = @{$_};
	    $line =~ s/$src/$dest/;
	}
	print $out $line;
    }
    close ($in)
	or die "$in: $!";
    close ($out)
	or die "$out: $!";
    move($outName, $inName);
}

# last expression needs to return a value
1;
