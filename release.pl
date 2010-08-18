#!/usr/bin/perl

do "replace.pl";

my $svnVersion = `svnversion`;
my $svnVersion = $svnVersion + 1;

print "Setting version $svnVersion\n";

my $version = "version = " . $svnVersion . ";";
my @substitution = (["version = \\d+;", $version]);
replace ("main.cpp", \@substitution);

$version = "PROJECT_NUMBER = " . $svnVersion;
@substitution = (["PROJECT_NUMBER\ += \\d+", $version]);
replace ("Doxyfile", \@substitution);
