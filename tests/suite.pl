#!/usr/bin/perl -W

use strict;
use Thraxx;
use WASP;
use DBH qw(:all);
use DBH::MySQL;
use OF::HTML;
use Babs;

sub _ {
	if ($_[0]) {
		print "\033[1;40;32mTest succeeded\033[1;0;0m\n\n";
	} else {
		print "\033[1;40;31mTest failed!\033[1;0;0m\n\n";
		die;
	}
} 

sub test {
	print "\033[1;40;34m", @_, ":\033[1;0;0m\n";
}

my $w = WASP->new;
$w->die(1);
my $td = DBH::MySQL->new(host=>"12.226.98.118", username=>"thraxx",
		password=>"lNBDOD92Pec", database=>"thraxx",
		wasp=>$w);
my $d = DBH::MySQL->new(host=>"12.226.98.118", username=>"babs",
		password=>"", database=>"babs",
		wasp=>$w);
my $t = Thraxx->construct(wasp=>$w, dbh=>$td, skip_init=>1);

# We normally invoke Babs->new to let it consult the configuration
# and set up these objects by itself, but since this test script
# needs to be run in most any circumstance, we must make it so
# these settings can be run from anywhere unmodified.
my $b = Babs->construct(thraxx=>$t, wasp=>$t, dbh=>undef, skip_init=>1,
			isapi=>CGI->new, of=>OF::HTML->new($w), dbh=>1);

# Some of the operations tested must be done in a specific order,
# such as adding users before posting stories from them. As this
# model can leave the system in an unknown state should a procedure
# fail, cleanups will have to be performed.

# users.inc
$b->user_add();

# comments.inc

$b->comment

# crypt.inc
# event.inc
# isr.inc
# misc.inc
# of.inc
# sessions.inc
# stories.inc
# str.inc
# templates.inc
# udf.inc
# xml.inc

exit 0;
