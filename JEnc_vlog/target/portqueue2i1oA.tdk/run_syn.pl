#! /opt/xtensa/XtDevTools/install/tools/RD-2011.1-linux/XtensaTools/Tools/bin/perl
####################################################################################
#          Copyright (c) 2006-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.         # 
#   These coded instructions, statements, and computer programs are the            #
#   copyrighted works and confidential proprietary information of Tensilica Inc.   #
#   They may not be modified, copied, reproduced, distributed, or disclosed to     #
#   third parties in any manner, medium, or form, in whole or in part, without     #
#   the prior written consent of Tensilica Inc.                                    #
####################################################################################

BEGIN {@INC = map ({s'/usr/xtensa/tools-6.1'/opt/xtensa/XtDevTools/install/tools/RD-2011.1-linux/XtensaTools/Tools'; $_} @INC); }

use FileHandle;
use Getopt::Long;
use Cwd;
use File::Path;
use File::Copy;
use File::Basename;
use strict 'vars';

my $help = 0;
my $rc = 0;
my $topo = 0;
my $xdaDir = "/home/hbokhari/XtensaStuff/RD-2011.1-linux/LX4_uclib_1/Hardware/xda";
my $pwd = cwd();
my $ret;
my $status;

my $usage = <<USAGE;
$0 [options]

This script is used to launch off a synthesis run on the user-defined TIE file.
The output synthesis directory will be generated within the directory that this
script was invoked from.

-xda <dir>	Path to desired xda directory, which includes the edited
		CadSetup.file and Xttop.ioc files.
		Defaults to /home/hbokhari/XtensaStuff/RD-2011.1-linux/LX4_uclib_1/Hardware/xda.

-topo		Use Design Compiler topographical synthesis mode using TCL
		scripts in XG mode (dc_shell-topo). If not specified, TCL
		scripts in XG mode will be used instead (dc_shell-xg-t).

-rc		Use RTL Compiler to perform synthesis. If not specified, Design
		Compiler (dc_shell-xg-t) in XG mode will be used instead.
USAGE

$ret = GetOptions(
	 "help" => \$help,
         "xda=s" => \$xdaDir,
	 "topo" => \$topo,
	 "rc" => \$rc);

if ($help || ! $ret) {
  print "$usage";
  exit;
}

$xdaDir = "$pwd/" . $xdaDir if ($xdaDir !~ /^\//);
die "Error: $xdaDir does not exist. Please retry with an existing xda directory. Exiting.\n" unless -d $xdaDir;
die "Error: only one of -topo or -rc can be specified. Exiting.\n" if (($topo + $rc) > 1);

my $error = <<ERROR_MESSAGE;

WARNING: Your TIE extensions modify the characteristics of the Xtensa 
AR register file. This will result in an increase in the area of the 
AR register file. This increase is not reflected in the synthesis result 
of the tdk flow. Please build an Xtensa configuration with your TIE 
extensions by using the processor generator. Then run synthesis on it to 
get an accurate measure of the area and timing of your Xtensa processor.

ERROR_MESSAGE

my $RegfileArea = 0;
my ($AreaFile) = <portqueue2i1oA.area>;
open(AREAFILE,"<$AreaFile") or die (
"ERROR: Cannot open area estimation file \"$AreaFile\". 
Please make sure that the TIE compiler run completed successfully.\n");

while (<AREAFILE>) {
  if ($_ =~ /^\s+(\S+)\s+TIE_AR_Regfile \[additional area\]/) {
    $RegfileArea = $1;
    last;
  }
}
close(AREAFILE);
if ($RegfileArea > 0) {
  print STDERR "$error";
}

if ($topo) {
  $status = system("$xdaDir/scripts/dc/tdk_syn.pl -xda $xdaDir -tdk $pwd -topo");
} elsif ($rc) {
  $status = system("$xdaDir/scripts/dc/tdk_syn.pl -xda $xdaDir -tdk $pwd -rc");
} else {
  $status = system("$xdaDir/scripts/dc/tdk_syn.pl -xda $xdaDir -tdk $pwd");
}

if ($status != 0) {
  print "Error: TDK synthesis flow did not complete cleanly. Check log files and try again.\n";
  exit;
} else {
  print "TDK synthesis flow completed successfully.\n";
}
