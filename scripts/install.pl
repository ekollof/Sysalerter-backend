#!/usr/bin/perl
#
# This script prepares the system to be monitored by sysalert and generates a configuration file.

use strict;
use POSIX;
use English;

my $SYSALERTDIR;
my $METHOD;
my $HOME=getenv("HOME");

sub ask {
        my $answer;

        my ($question, $default) = @_;

        if (!defined($default)) {
                print $question." : ";
        } else {
                print $question." : [$default] ";
        }
        chomp($answer = <>);
        if ($answer eq "") {
                $answer = $default;
        }

        return $answer
}

sub testmail
{
	my ($server, $address) = @_;
	return 0;	
}

# TODO:
#
# - ask for confguration details (initial tresholds, which scripts to run when 
# calamity occurs, where to store log and pid files)
# - display summary of config options and provide opportunity to the user 
# to change values
# - generate config file from template
# - create config directory
# - install config file in config location
# - if home dir install, create $HOME/bin and copy binary there
# - if system-wide, ask if the service should be started on boot (rc script or
# SMF manifest import)


# main
print "CONFIGURATION\n";
print "-------------\n\n";
my $choice = ask("System wide or home dir installation? (system/home)", "system");
if ($choice eq "system") {
	$METHOD="system";
	$SYSALERTDIR="/etc/sysalerter";
} else {
	print "Installing in $HOME\n";
	$SYSALERTDIR="$HOME/.sysalerter";
}

my $mailserver = ask("Which mail server should I use? (ip number/hostname)", "localhost");
my $mailuser = ask("Which mail adres should I send notifications to? (mail address)", "root\@localhost");
print "Testing mail address: ";
if (testmail($mailserver, $mailuser)) {
	print "Mail address seems to work. Please check for a test mail in your inbox\n";
} else {
	print "There was a problem sending mail to $mailuser via $mailserver. Please check your configuration.\n";
	print "You can always change the mailserver address by editing the corresponding values in ".$SYSALERTDIR."/config.\n";
}


my $interval = ask("Please specify check interval (in seconds)", 60);
my $pidfile = ask("Where should the pid file of sysalerter be stored?", $METHOD eq "system" ? "/var/run/sysalert.pid" : "$HOME/sysalert.pid");
my $logsfile = ask("Where should sysalerter log everything?", $METHOD eq "system" ? "/var/log/sysalert.log" : "$HOME/sysalert.log");
my $database = ask("Where should the database be stored?", $METHOD eq "system" ? "/var/tmp" : "$HOME/sysalert.db");


print "\nLOAD AVERAGE\n";
print "-------------\n";
my $loadtreshold = ask("Specify load average treshold", 2.00);
my $loadscript = ask("What command or script should I run when it is exceeded?", $SYSALERTDIR."/load.sh");

print "\nDISK\n";
print "----\n";
my $disktreshold = ask("Specify minimum free disk space (in percent)", 20);
my $diskscript = ask("What command or script should I run when it is exceeded?", $SYSALERTDIR."/disk.sh");

print "\nCPU\n";
print "---\n";
my $cpuwhich = ask("What cpu stat should I monitor? (kernel/user)", "user");
my $cputreshold = ask("How much cpu should be minimally free (in percent)", 10);
my $cpuscript = ask("What command or script should I run when it is exceeded?", $SYSALERTDIR."/cpu.sh");

#print "\nSWAP\n";
#print "----\n";
#my $swaptreshold = ask("How much swap should be minimally free (in percent)", 20);
#my $loadscript = ask("What command or script should I run when it is exceeded?", $SYSALERTDIR."/swap.sh");



