###############################################################
# Driverlib.pm - A package of helper functions for Perl drivers
# 
# Copyright (c) 2005 David R. O'Hallaron, All rights reserved.
###############################################################

package Driverlib;

use Socket;

# Autogenerated header file with project-specific constants
use lib ".";
use Driverhdrs;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
	     driver_post
	     );

use strict;

#####
# Public functions
#

#
# driver_post - This is the routine that a driver calls when 
#    it needs to transmit an autoresult string to the result server.
#
sub driver_post ($$) {
    my $userid = shift;       # User id for this submission
    my $result = shift;       # Autoresult string
    my $autograded = shift;   # Set if called by an autograder

    # Echo the autoresult string to stdout if the driver was called
    # by an autograder
    if ($autograded) {
        print "\n";
        print "AUTORESULT_STRING=$result\n";
        return;
    }	

    # If the driver was called with a specific userid, then submit
    # the autoresult string to the result server over the Internet.
    if ($userid) {
        my $status = submitr($Driverhdrs::SERVER_NAME, 
                             $Driverhdrs::SERVER_PORT, 
                             $Driverhdrs::COURSE_NAME, 
                             $userid, 
                             $Driverhdrs::PROJ, 
                             $result);
        
        # Print the status of the transfer
        if (!($status =~ /OK/)) {
            print "$status\n";
            print "Did not send autoresult string to the result server.\n";
            exit(1);
        }
        print "Success: Sent autoresult string for $userid to the result server.\n";
    }	
}


#####
# Private functions
#

#
# submitr - Sends an autoresult string to the result server
#
sub submitr ($$$$$$) {
    my $hostname = shift;
    my $port = shift;
    my $course = shift;
    my $userid = shift;
    my $proj = shift;
    my $result = shift;

    my $internet_addr;
    my $enc_result;
    my $paddr;
    my $line;
    my $http_version;
    my $errcode;
    my $errmsg;

    # Establish the connection to the server
    socket(SERVER, PF_INET, SOCK_STREAM, getprotobyname('tcp'));
    $internet_addr = inet_aton($hostname)
        or die "Could not convert $hostname to an internet address: $!\n";
    $paddr = sockaddr_in($port, $internet_addr);
    connect(SERVER, $paddr)
        or die "Could not connect to $hostname:$port:$!\n";

    select((select(SERVER), $| = 1)[0]); # enable command buffering

    # Send HTTP request to server
    $enc_result = url_encode($result);
    print SERVER  "GET /$course/submitr.pl/?userid=$userid&proj=$proj&result=$enc_result&submit=submit HTTP/1.0\r\n\r\n";

    # Get first HTTP response line
    $line = <SERVER>;
    chomp($line);
    ($http_version, $errcode, $errmsg) = split(/\s+/, $line);
    if ($errcode != 200) {
        return "Error: HTTP request failed with error $errcode: $errmsg";
    }

    # Read the remaining HTTP response header lines
    while ($line = <SERVER>) {
        if ($line =~ /^\r\n/) {
            last;
        }
    }

    # Read and return the response from the result server
    $line = <SERVER>;
    chomp($line);

    close SERVER;
    return $line;
    
}

#
# url_encode - Encode text string so it can be included in URI of GET request
#
sub url_encode ($) {
    my $value = shift;

    $value =~s/([^a-zA-Z0-9_\-.])/uc sprintf("%%%02x",ord($1))/eg;
    return $value;
}

# Always end a module with a 1 so that it returns TRUE
1;

