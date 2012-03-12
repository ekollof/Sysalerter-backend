#!/usr/bin/perl
# 
# File:   createdbhdr.pl
# Author: ekollof
#
# Created on June 13, 2008, 3:46 PM
#
# Creates a header file from the SQLITE file that will be compiled into
# the code for initializing a database if alerter has never ran.

use strict;

my @header;
my $line;	

open(TEMPLATE, "SQLITE") || die;
while(<TEMPLATE>) {
	chomp;
	push(@header, $_);
}
close(TEMPLATE);

open(HEADER, ">dbinit.h") || die;
print HEADER <<EOM;
/* This is automatically generated. Do not edit. */
#ifndef DBINIT_H
#define DBINIT_H

char createdb[] = \"\"
EOM

foreach (@header) {
	print HEADER "\"$_\""."\n";
}
print HEADER "\"\";\n\n#endif\n\n";
close(HEADER);
