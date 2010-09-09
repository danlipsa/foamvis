#!/usr/bin/perl
use strict;
use warnings;
use File::Copy;

do "replace.pl";
my @args = ("bison", @ARGV);
system(@args) == 0
    or die "system @args failed: $?";
my @substitutionsC = (
    ["EvolverData\\.tab\\.h", "EvolverData_yacc.h"],
    ["EvolverData\\.tab\\.c", "EvolverData_yacc.cpp"],
    ["y\\.tab\\.h", "EvolverData_yacc.h"]);
replace ("EvolverData.tab.c", \@substitutionsC);

my @substitutionsH = (
    ["EvolverData\\.tab\\.h", "EvolverData_yacc.h"]
    );
replace ("EvolverData.tab.h", \@substitutionsH);
copy ("EvolverData.tab.c", "EvolverData_yacc.cpp.save")
    or die "Copy failed: $!";;
copy ("EvolverData.tab.h", "EvolverData_yacc.h.save")
    or die "Copy failed: $!";;
