#!/usr/bin/perl -W
# $Id$

# The search and display functionality here is designed to
# be as flexible as possible, but no more.  Parameters include:
#
#	template=$template_name	Name of template to search/display.
#	param=$val, param=...	Any number of parameters specific to
#				the search type.
#	sort=$sort_field	Field to sort on.
#	order=$sort_order	Ascending or descending.
#	nresults=$nresults	Number of results to fetch and display.
#	offset=$page_offset	Page offset.
#
# Try to associate search results with the user's session.  There is
# a good chance the user will peruse a few results at least.  Saving
# them is at least better than requesting them on each page display.

use Babs;
use strict;
use warnings;

my $babs  = Babs->new(shift);
my $cgi   = $babs->{cgi};
my $valid = 1;

my ($template, @params, $sort, $order, $nresults, $offset);

$template = $cgi->param("template");

if ($babs->template_exists($template)) {
	
} else {
}

EXIT_SUCCESS;
