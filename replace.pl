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
sub replaceBlocks
{
    my ($inName, $beginComment, $endComment) = @_;
    my $beginBlock = "begin";
    my $endBlock = "end";
    my $outName = "temp.txt";
    # 0 outsite a block, 1 inside a block (between begin and end)
    my $state = 0;
    my $match = qr/^\s*$beginComment\s$beginBlock\s(\w+\.\w+)\s*$endComment$/;
    my $otherMatch = qr/^\s*$beginComment\s$endBlock\s$endComment$/;
    print "processing $inName ...\n";
    open (my $in, "<", $inName)
	or die ("Could not open $inName: $!\n");
    open (my $out, ">", $outName)
	or die ("Could not open $outName: $!\n");    
    while (<$in>)
    {
	my $line = $_;	
	if ($line =~ $match)
	{
	    my $temp = $match;
	    $match = $otherMatch;
	    $otherMatch = $temp;
	    if ($state == 0)
	    {
		print $out $line;
		my $chunkName = $1;
		open (my $chunk, "<", $chunkName)
		    or die ("Could not open $chunkName: $!\n");
		while (<$chunk>)
		{
		    print $out $_;
		}
		print $out "\n";
		close ($chunk);
	    }
	    $state = ! $state;
	}
	if ($state == 0)
	{
	    print $out $line;
	}
    }
    close ($in)
	or die "$in: $!";
    close ($out)
	or die "$out: $!";
    move($outName, $inName);
}

sub replaceBlocksHtml
{
    my ($inName) = @_;
    replaceBlocks ($inName, "<!--", "-->");
}

sub ignoreAfter
{
    my ($inName, $str) = @_;
    my $outName = "temp.txt";
    print "processing $inName ...\n";
    open (my $in, "<", $inName)
	or die ("Could not open $inName: $!\n");
    open (my $out, ">", $outName)
	or die ("Could not open $outName: $!\n");
    my $match = qr/^(.*)<hr>.*$/;
  READ: {
      while (<$in>)
      {
	  my $line = $_;
	  if ($line =~ $match)
	  {
	      print $out "$1\n";
	      last READ;
	  }	
	  print $out $line;
      }
    }
    close ($in)
	or die "$in: $!";
    close ($out)
	or die "$out: $!";
    move($outName, $inName);
}

# last expression needs to return a value
1;
