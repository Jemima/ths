#!/usr/bin/perl -w

use strict;

my $f = "";
{
   local($/, *FH);
   open(FH, $ARGV[0]);
   $_ = <FH>;
}

s/\**//g;
s/Partition.*//gi;
s/\[.*?\n//gs;
s/^\s*\d+\s*$//gm;
s/\n\n+/\n/gs;

my @lines = split /\n/;

#print "Input Path, Step1, Step2, Step3, Step4, Total, LUTs of size 0, LUTs of size 1,LUTs of size 2,LUTs of size 3,LUTs of size 4,LUTs of size 5,LUTs of size 6,Number of type input,Number of type output,Number of type latch,Number of type names,Number of type dual_port_ram,Number of type single_port_ram,Number of type multiply,size,chWidth,Area,Unused,Latency,Time (Inaccurate),";
#print "LUTs of size 0, LUTs of size 1,LUTs of size 2,LUTs of size 3,LUTs of size 4,LUTs of size 5,LUTs of size 6,Number of type input,Number of type output,Number of type latch,Number of type names,Number of type dual_port_ram,Number of type single_port_ram,Number of type multiply,size,chWidth,Area,Unused,Latency,Time (Inaccurate)\n";

print "Input Path, Step1, Step2, Step3, Step4, Total,Latency1, LatencyTMR\n";

my $state = 0; #0=expect time line, 1=expect VPR1, 2=expect VPR2
my $line0 = "";
my $line1 = "";
my $line2 = "";
for my $n(0..$#lines){
   my $line = $lines[$n];
   if($state == 0){
      $line0 = $line;
      $state = 1;
   } elsif($state == 1){
      if($line=~/0 LUTs of size 0/){
         $line1 = $line;
         $state = 2;
      } else {
         $line0 = $line;
         $state = 1; #Error'd line. Must be a timing line
      }
   } elsif($state == 2){
      if($line=~/0 LUTs of size 0/){
         $line2 = $line;
         $state = 0;

         $line0=~s/\t/,/g;
         
         $line1=~s/CritPathNets: [^,]+,[^,]+//gi;
         $line1=~s/(\d+) LUTs of size \d+/$1/gi;
         $line1=~s/(\d+) of type[^,]+/$1/gi;
         $line1=~s/Latency: ([\d.]+) ns/$1/gi;
         my $l1 = $1;
         $line1=~s/,[a-zA-Z]+?: /,/g;
         $line1=~s/,[^,]+?\s+([^,\s]+?),/,$1,/gi;

         $line2=~s/CritPathNets: [^,]+,[^,]+//gi;
         $line2=~s/(\d+) LUTs of size \d+/$1/gi;
         $line2=~s/(\d+) of type[^,]+/$1/gi;
         $line2=~s/Latency: ([\d.]+) ns/$1/gi;
         my $l2 = $1;
         $line2=~s/,[a-zA-Z]+?: /,/g;
         $line2=~s/,[^,]+?\s+([^,\s]+?),/,$1,/gi;

         print "$line0,$l1,$l2\n";
         #print $line0.',';
         #print $line1.','.$line2;
         #print "\n";
      } else {
         $line0 = $line;
         $state = 1; #Error'd line. Must be a timing line
      }
   }
}
