#!/usr/bin/perl
use File::Copy;
use strict;
use warnings;

#Removes empty lines and spaces before #version
sub cleanupFrag
{
    while (<STDIN>)
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
