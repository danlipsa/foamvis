#!/usr/bin/perl

do "replace.pl";
my $majorVersion = "0.1";
my $svnVersion = `svnversion`;
my $svnVersion = $svnVersion + 1;

print "Setting version $majorVersion.$svnVersion\n";

my $version = "version = \"" . $majorVersion . "." . $svnVersion . "\";";
my @substitution = (["version = \"[\d.]+\";", $version]);
replace ("main.cpp", \@substitution);

$version = "PROJECT_NUMBER = " . $majorVersion . "." . $svnVersion;
@substitution = (["PROJECT_NUMBER\ += [\d.]+", $version]);
replace ("Doxyfile", \@substitution);
