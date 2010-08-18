#!/usr/bin/perl

do "replace.pl";

my $svnVersion = `svnversion`;
my $svnVersion = $svnVersion + 1;
my $version = "version = " . $svnVersion . ";";
print "Setting version $svnVersion\n";
my @substitution = (["version = \\d+;", $version]);
replace ("main.cpp", \@substitution);
