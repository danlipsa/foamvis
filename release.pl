#!/usr/bin/perl
use strict;
use warnings;
use File::Copy;
use POSIX qw/strftime/;

do "replace.pl";
my $majorVersion = "0.7";
my $svnVersion = `svnversion`;
$svnVersion = $svnVersion + 1;
my $currentDate = strftime ('%F %T', localtime);

print "Setting version $majorVersion.$svnVersion, date $currentDate\n";

my $version = "version = \"" . $majorVersion . "." . $svnVersion . "\";";
my $date = "date = \"" . $currentDate . "\";";
my @substitution = (["version = \"[\\d.]+\";", $version],
		    ["date = \"[^\"]+\";", $date]);
replace ("Options.cpp", \@substitution);

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
