use strict;

open FILE, "../Bibtex/thesis.bib";
my $line = 0;
while(<FILE>){
   $line++;
   my $n = tr/{//;
   my $m = tr/}//;
   print "$line: " . ($n-$m);
   print "\n";
}
