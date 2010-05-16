#!/usr/bin/perl
use File::Copy;

sub replace
{
    my ($name, $substitutions) = @_;
    my $outName = "temp.txt";
    print "processing $name ...\n";
    open (my $in, "<", $name)
	or die ("Could not open $name: $!\n");
    open (my $out, ">", $outName)
	or die ("Could not open $outName: $!\n");
    while (<$in>)
    {
	my $line = $_;
	foreach (@{$substitutions})
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
    move($outName, $name);
}

