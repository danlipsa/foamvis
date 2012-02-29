#!/usr/bin/perl
use File::Copy;
use strict;
use warnings;

#Removes empty lines and spaces before the first nonempty line
sub cleanupFrag
{
    my $first = 1;
  LINE: 
    while (<STDIN>)
    {
	my $line = $_;
	if ($line =~ /^$/)
	{
	    next LINE;
	}
	if ($first)
	{
	    $line =~ s/ *(.*)/$1/;
	    $first = 0;
	}
	print $line;
    }
}

cleanupFrag ();
