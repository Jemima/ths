#!/usr/bin/perl -w
use strict;

if (!-e$ARGV[0]){
   print $ARGV[0] . " not found";
   exit;
}
{
  local $/=undef;
  #open FILE, $ARGV[0];
  $_ = <>;
  #close FILE;
}
print "$ARGV[0],";


/(\d+)\s+ LUTs\s+ of\s+ size\s+ 0\s+
(\d+)\s+ LUTs\s+ of\s+ size\s+ 1\s+
(\d+)\s+ LUTs\s+ of\s+ size\s+ 2\s+
(\d+)\s+ LUTs\s+ of\s+ size\s+ 3\s+
(\d+)\s+ LUTs\s+ of\s+ size\s+ 4\s+
(\d+)\s+ LUTs\s+ of\s+ size\s+ 5\s+
(\d+)\s+ LUTs\s+ of\s+ size\s+ 6\s+
\s+
(\d+)\s+ of\s+ type\s+ input\s+
(\d+)\s+ of\s+ type\s+ output\s+
(\d+)\s+ of\s+ type\s+ latch\s+
(\d+)\s+ of\s+ type\s+ names\s+
(\d+)\s+ of\s+ type\s+ dual_port_ram\s+
(\d+)\s+ of\s+ type\s+ single_port_ram\s+
(\d+)\s+ of\s+ type\s+ multiply/isx;
print "$1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,";


/The\s+ circuit\s+ will\s+ be\s+ mapped\s+ into\s+ a\s+ (\d+)\s+ x\s+ (\d+)\s+ array\s+ of\s+ clbs/isx;
print "$1,";


/\s+ T\s+ Cost\s+ Av\.\s+ BB\s+ Cost\s+ Av.\s+ TD\s+ Cost\s+ Av\s+ Tot\s+ Del\s+ P\s+ to\s+ P\s+ Del\s+ d_max\s+ Ac\s+ Rate\s+ Std\s+ Dev\s+ R\s+ limit\s+ Exp\s+ Tot\.\s+ Moves\s+ Alpha\s+
[\s+-]+\s+
(\s+?\S+){6}\s+?(\S+)/sx;
print "$2,";


/Placement Estimated Crit Path Delay: (.+)/m;
print "$1,";

/Best routing used a channel width factor of (\d+)/;
print "$1,";

/Av.\s+ wire\s+ segments\s+ per\s+ net:\s+ ([0-9.]+)\s+
\s+ Maximum\s+ segments\s+ used\s+ by\s+ a\s+ net:\s+ (\d+)/sx;
 print "$1, $2,";
 

/Total\s+ Logic\s+ Block\s+ Area\s+ \(Warning,\s+ need\s+ to\s+ add\s+ pitch\s+ of\s+ routing\s+ to\s+ blocks\s+ with\s+ height\s+ >\s+ 3\):\s+ ([0-9.e+-]+)\s+ \s+
Total\s+ Used\s+ Logic\s+ Block\s+ Area:\s+ ([0-9.e+-]+)\s+ \s+
\s+
Routing\s+ area\s+ \(in\s+ minimum\s+ width\s+ transistor\s+ areas\):\s+
Total\s+ Routing\s+ Area:\s+ ([0-9.e+-]+)\s+Per\s+ logic\s+ tile:\s+ ([0-9.e+-]+)\s+
\s+/xs;
print "$1,$2,$3,$4";


/Segment\s+ usage\s+ by\s+ type\s+ \(index\):\s+
Segment\s+ type\s+Fractional\s+ utilization\s+
------------\s+----------------------\s+
\s+ 0\s+([0-9.e+-]+)/xs;
print ",$1";

/Segment\s+ usage\s+ by\s+ length:\s+
Segment\s+ length\s+Fractional\s+ utilization\s+
--------------\s+----------------------\s+
\s+ 4\s+([0-9.e+-]+)/xs;
print ",$1";

/Critical\s+ Path:\s+ ([0-9.e+-]+)\s+ \(s\)/xs;
print ",$1";

/Routing\s+ took\s+ ([0-9.e+-]+)\s+ seconds\s+
The\s+ entire\s+ flow\s+ of\s+ VPR\s+ took\s+ ([0-9.e+-]+)\s+ seconds/xs;

print "$1,$2\n";
