#!/usr/bin/perl

do "replace.pl";
my @args = ("flex", @ARGV);
system(@args) == 0
    or die "system @args failed: $?";
my @substitutionsC = (
    ["lex\\.EvolverData\\.c", "EvolverData_lex.cpp"]
    );
replace ("lex.EvolverData.c", \@substitutionsC);

