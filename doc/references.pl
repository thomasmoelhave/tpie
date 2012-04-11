#!perl
# -*- mode: perl; tab-width: 4; indent-tabs-mode: t; -*-
# vim:set sw=4 ts=4 sts=4 noet:
# Copyright 2012, The TPIE development team
# 
# This file is part of TPIE.
# 
# TPIE is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
# 
# TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with TPIE.  If not, see <http://www.gnu.org/licenses/>

# The purpose of this file is to produce basicconcepts.dox from basicconcepts.dox.in and tpie.bib

use warnings;
use strict;

if (!@ARGV) {
    print "Parameters: <inputfile> <outputfile>\n";
    exit;
}

# read an entire file
sub slurp {
    my ($filename) = @_;
    my $contents;
    {
		my $fp;
		open $fp, '<', $filename;
		local $/ = undef;
		$contents = <$fp>;
		close $fp;
    }
    return $contents;
}

my $bibfile = slurp 'tpie.bib';
my $input = slurp $ARGV[0];

my $ofp;
open $ofp, '>', $ARGV[1] or die "Couldn't open $ARGV[1] for writing";

my @referencestrings = $bibfile =~ m/^@(?:article|book|incollection|inproceedings|manual|mastersthesis|misc|phdthesis|techreport|unpublished)\{(.*?)^}/gism;

print "bibfile is ".length($bibfile)." bytes and has ".scalar(@referencestrings)." references\ninput is ".length($input)." bytes\n";

my %referenceorder = ();
my %references;
my $refstring;
for $refstring (@referencestrings) {
    my ($handle) = ($refstring =~ m/ *([^ ,]*)/);
	my $pos = index $input, $handle;
    if (-1 == $pos) {
		next;
    }
	$referenceorder{$pos} = $handle;

    my %attrs = map {
		if ($refstring =~ /$_\s+=\s+\{(.*?)\}, *$/msi) {
			my $s = $1;
			$s =~ s/[{}\\]//g;
			$s =~ s/\s+/ /g;
			($_, $s);
		} else {
			warn "No $_ in $handle";
			($_, "No $_");
		}
    } qw(author title);

    my $link = $handle;
    $link =~ s/://g;
    $attrs{'link'} = $link;

    $references{$handle} = \%attrs;

    print "$handle is $attrs{title}\n";
}

my $n = 1;
for my $pos (sort {$a <=> $b} keys %referenceorder) {
	my $handle = $referenceorder{$pos};
	$references{$handle}->{'num'} = $n;
	++$n;
}

for $refstring (keys %references) {
    my $ref = $references{$refstring};
    $input =~ s/\b$refstring\b/<a href="#$ref->{link}">$ref->{num}<\/a>/g;
}

print $ofp $input;

for my $refstring (sort {$references{$a}->{num} <=> $references{$b}->{num}} keys %references) {
    my $ref = $references{$refstring};
    print $ofp '<a href="#back">^</a> ';
    print $ofp "<span id=\"$ref->{link}\">$ref->{num}: $ref->{author}, $ref->{title}<\/span>\n\n";
}

print $ofp "*/\n";

close $ofp;
