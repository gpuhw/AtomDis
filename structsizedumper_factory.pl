#!/usr/bin/perl

print << "EO_B";
#include <stdio.h>
#include "atombios_types.h"
#include "atombios.h"

int main() {

EO_B

while (<>) {
  if (/^\s*typedef\s+(struct|union)\s+_(\w+)/) {
    print "  printf (\"$2 = %04x\\n\", sizeof($2));\n";
  }
}

print << "EO_E";

  return 0;
}
EO_E

