#!/usr/bin/perl -W
# $Id$

use WASP qw(:std);
use Babs;
use strict;
use warnings;

my $babs = Babs->new(shift);
my $cgi = $babs->{cgi};

my $template = $cgi->param("template") || "";
$template = "story" unless $babs->valid_template("story", $template);

my $story_id = $cgi->param("story_id") || 0;
$story_id = 0 unless $story_id && $story_id =~ /^\d+$/;

if (my $story = $babs->template_expand($template, {story_id => $story_id})) {
	print $story;
} else {
	$babs->error_page("The requested story could not be found.");
}

EXIT_SUCCESS;
