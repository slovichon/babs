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
#	show=$num		Number of results to fetch and display.
#	offset=$page_offset	Page offset.
#
# Try to associate search results with the user's session.  There is
# a good chance the user will peruse a few results at least.  Saving
# them is at least better than requesting them on each page display.
#
# Later on, a system can be added that translates the exact parameters
# from user-friendly controls, e.g., "search by user" to (param=>username,
# op=>"=", val=>$fieldval).

use WASP qw(:std);
use Babs;
use strict;
use warnings;

sub num_in_array {
	my ($needle, $hay) = @_;
	foreach my $i (@$hay) {
		return TRUE if $needle == $i;
	}
	return FALSE;
}

my $babs  = Babs->new(shift);
my $oof   = $babs->{oof};

my ($template, @params, @op, @val, $sort, $order, $show, $offset);

$template = $babs->req("template");

if ($babs->template_exists($template)) {
	@param  = $babs->req("param")	|| ();
	@op	= $babs->req("op")	|| ();
	@val	= $babs->req("val")	|| ();
	$sort   = $babs->req("sort")	|| "";
	$order  = $babs->req("order")	|| "";
	$show   = $babs->req("show")	|| 0;
	$offset = $babs->req("offset")	|| 0;

	# Only use valid parameters.
	# @params

	# XXX:  validate sort field is existent and logical for template type
	unless ($babs->isr_field_valid($template, $sort)) {
		$sort = "";
	}

	unless ($order eq "asc" || $order eq "desc") {
		$order = "asc";
	}

	unless ($show =~ /^\d+$/ && num_in_array($show, $babs->{allowable_num_search_results})) {
		# XXX:  check user prefs, then systemwide defaults.
		$show = $babs->get_default_search_results();
	}

	# This also needs to be check within the range
	# of the number of results.
	unless ($offset =~ /^\d+$/) {
		$offset = 0;
	}

	# 
} else {
	# Display a search form.
	print	$babs->header("Search"),
		$oof->form(
			{method=>"get"},
			$oof->table(
				{},
				[{class=>"babsDesc", value=>"Search:"},
					{class=>$babs->gen_class(),
					# The default param value used here must be usable
					# by all options in the following select list.
					# XXX:  fuzzy match via LIKE/=/REGEXP
					 value=>$oof->input(type=>"hidden", name=>"param", value=>"story") .
					 	$oof->input(type=>"hidden", name=>"op", value=>"LIKE") .
					 	$oof->input(type=>"text", name=>"val")}],
				[{class=>"babsDesc", value=>"Summary:"},
					{class=>$babs->gen_class(),
					 value=>$oof->input(type=>"select", name=>"template",
					 			options=>{
									"Quick" => "quick"
								})}],
				[{class=>"babsDesc", value=>"Sort:"},
					{class=>$babs->gen_class(),
					 value=>$oof->input(type=>"select", name=>"order",
					 			options=>{
									"Ascending" => "asc",
									"Descending" => "desc"
								})}],
				[{class=>"babsDesc", value=>"Number of results:"},
					{class=>$oof->input(type=>"select", name=>"",
								value=>$babs->get_default_search_results(),
								sort=>$babs->{allowable_num_search_results},
								options=>{
									{map { ($_, $_) } $babs->{allowable_num_search_results}}
								})}],
				[{class=>"babsFooter", colspan=>2,
				  value=>$oof->input(type=>"submit", value=>"Search") .
					 $oof->input(type=>"reset",  value=>"Reset")}],
			)
		),
		$babs->footer();
}

EXIT_SUCCESS;
