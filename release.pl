#!/usr/bin/perl
use strict;
use warnings;
use File::Copy;

do "replace.pl";
my $majorVersion = "0.4";
my $svnVersion = `svnversion`;
$svnVersion = $svnVersion + 1;

print "Setting version $majorVersion.$svnVersion\n";

my $version = "version = \"" . $majorVersion . "." . $svnVersion . "\";";
my @substitution = (["version = \"[\\d.]+\";", $version]);
replace ("main.cpp", \@substitution);

$version = "PROJECT_NUMBER = " . $majorVersion . "." . $svnVersion;
@substitution = (["PROJECT_NUMBER\ += [\\d.]+", $version]);
replace ("Doxyfile", \@substitution);

my @bisonFlexFiles = ("EvolverData_lex.cpp", "EvolverData_lex.h",
		      "EvolverData_yacc.cpp", "EvolverData_yacc.h");
foreach (@bisonFlexFiles)
{
    my $file = $_;
    copy("${file}.save", $file)
	or die "Copy failed: $!";
}
