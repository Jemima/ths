
use strict;

if($#ARGV != 1){
   print "Calling format is graph_plif.pl infile outfile\n";
   exit 1;
}

open FILE, $ARGV[0];
my $hold = "";
my $modelName = "";
my $state = 0; #Bitmask. LSB->MSB InModel
while($_ = <FILE>){
   if(/\\/){
      $hold = $hold." ".$_;
      next;
   }
   $_ = $hold." ".$_;
   if(/.model(.*)/){
      if($state & 1 == 1){ #in a model, so end it and start a new one
         
      }
   }
}