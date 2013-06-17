#!/usr/bin/perl
use File::Copy;
use strict;
use warnings;

# Removes empty lines and spaces before the first nonempty line
# Adds a "do not modify" warning
sub cleanupFrag
{
    my $firstLine = 1;
  LINE: 
    while (<STDIN>)
    {
	my $line = $_;
	if ($line =~ /^$/)
	{
	    next LINE;
	}
	if ($firstLine)
	{
	    $line =~ s/ *(.*)/$1/;
	    $firstLine = 0;
            $line = $line . "\n" . 
                "// WARNING: Do not modify!\n" . 
                "//<file> automatically generated from <file>.in\n" . 
                "//=============================================\n";
	}
	print $line;
    }
}

cleanupFrag ();
