#!/usr/bin/perl
use strict;
use warnings;
use File::Copy;

do "replace.pl";
my @args = ("flex", @ARGV);
system(@args) == 0
    or die "system @args failed: $?";
my @substitutionsC = (
    ["lex\\.EvolverData\\.c", "EvolverData_lex.cpp"]
    );
replace ("lex.EvolverData.c", \@substitutionsC);
copy ("lex.EvolverData.c", "EvolverData_lex.cpp.save")
    or die "Copy failed: $!";
copy ("EvolverData_lex.h", "EvolverData_lex.h.save")
    or die "Copy failed: $!";

