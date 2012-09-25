#!perl.exe
use strict;
use feature qw/switch/;
use File::Find::Rule;
use Getopt::Mixed;
use File::Basename;
Getopt::Mixed::init('w:i channel-width>w width>w f:s');
my $channelWidth = -1;
my $VPRPath = "C:/Users/Dave/Documents/Visual Studio 2010/Projects/vtr/vpr/";
my $command = '"'.$VPRPath.'Debug/VPR.exe" "'.$VPRPath.'sample_arch.xml"';
my @files = ();
while(my ($option, $value, $pretty) = Getopt::Mixed::nextOption()){
   given($option){
      when (/^w$/) { $channelWidth = $value; }
      when (/^f$/) { push @files, $value; }
   }
}
my $params = '--full_stats --route_file temp.blif --net_file temp.net --place_file temp.place';
$params .= ' --route_chan_width '.$channelWidth if($channelWidth != -1);
if(scalar(@files) == 0){
   @files = File::Find::Rule->file()->name('*.blif')->in('benchmarks');
}
my $folder = "results_w$channelWidth";
mkdir $folder;
my $count = 1;
foreach my $file(@files){
   print "\nRunning $file: $count/".($#files+1)."\n"; 
   $count++;
   print "$command $file $params > $folder/".basename($file).'.txt';
   if(system("$command $file $params > $folder/".basename($file).'.txt') != 0){
      print "Error, see output file for details\n";
   }
}
