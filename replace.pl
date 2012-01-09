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

# Replaces all occurences of blocks in file `inName`. A block is marked with
# BEGIN_COMMENT begin <file> END_COMMENT
# ...
# BEGIN_COMMENT end END_COMMENT
# Blocks cannot be nested. All lines between the markers will be replaced with 
# the contet of the <file>. Markers are left in place.
sub replaceBlock
{
    my ($inName, $beginComment, $endComment) = @_;
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
