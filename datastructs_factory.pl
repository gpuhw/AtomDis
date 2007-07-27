#!/usr/bin/perl

#
# Copyright 2007 Matthias Hopf <mhopf@suse.de>
#
# AtomBIOS disassembler + data structure dumper
#
# datastructs_factory.pl:
# Datastruct dumper generator (using atombios.h).
#
# License: to be determined
#


while (<>) {
    $l++;
    if (/^\s*typedef\s+(struct|union)\s+_(\w+)/) {
	$t = $1;
	$s = $2;
	print "int ${s}_dumper (uint8_t *data, int indent) {\n";
	print "  $s *d = ($s *) data;\n";
	print "  int i;\n";
	print "  if (d) {\n";
	while (<>) {
	    $l++;
	    chomp;
	    s|\s*//.*||g;
	    next if /^\s*\{\s*$/ || /^\s*$/;
	    last if /\}/;
	    if (/^\s*(\w+)\s+(\w+)(\s*(:\d+))?(\s*\[(\d+)\])?\s*;/) {
		$e = $1;
		$n = $2;
		$b = $4;
		$a = $6;
		if ($a == 0) {
		    $loop_dst = "d->$n";
		    $loop_beg = "    printf (\"%s$e $n$b%s";
		    $loop_arg = ", IND(indent), FILL(".length($e.$n.$b).")";
		    $loop_end = "";
		} else {
		    $loop_dst = "d->${n}[i]";
		    $loop_beg = "    for (i = 0; i < $a; i++) {\n      printf (\"%s$e $n$b%s [%d]";
		    $loop_arg = ", IND(indent), FILL(".(length($e.$n.$b)+4)."+(i>9)), i";
		    $loop_end = " }";
		}
		if ($e eq "UCHAR") {
		    print "$loop_beg = %02x\\n\"$loop_arg, $loop_dst);$loop_end\n";
		} elsif ($e eq "USHORT") {
		    print "$loop_beg = %04x\\n\"$loop_arg, $loop_dst);$loop_end\n";
		} elsif ($e eq "ULONG") {
		    print "$loop_beg = %08x\\n\"$loop_arg, $loop_dst);$loop_end\n";
		} else {
		    if ($b ne "") {
			print "    printf (\"%s  <unparsable> line $l: $_\\n%sskipping...\\n\", IND(indent), IND(indent));\n";
			last;
		    } else {
			print "$loop_beg :\\n\"$loop_arg); ${e}_dumper ((uint8_t*) &$loop_dst, indent+1);$loop_end\n";
		    }
		}
	    } else {	
		print "    printf (\"%s  <unparsable> line $l: $_\\n%sskipping...\\n\", IND(indent), IND(indent));\n";
		last;
	    }
	}
	print "  }\n";
	print "  return sizeof ($s);\n}\n";
    }
}
