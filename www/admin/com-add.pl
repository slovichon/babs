#!/usr/bin/perl -W
# $Id$

use Babs;
use strict;
use warnings;

my $babs = Babs->new(shift);
my $story_id   = $babs->req("story_id");
my $comment_id = $babs->req("comment_id");
my $user_id = $babs->require_login(Babs::PERM_COMMENT_ADD, $story_id, $comment_id);

my $oof = $babs->{oof};
my $printform = 1;
my $errmsg = "";

if ($babs->req("submitted")) {
	# "Add Comment" form submitted; try to add comment

	if ($errmsg) {
		# There was an error; redisplay "Add Comment" form
		$printform = 0;
	} else {
		# No error; try to add the comment
	}
}

if ($printform) {
	# Simple request; display "Add Comment" form
	print	$babs->header("Adding Comment"),
		$oof->header("Adding Comment"),
		$oof->form(
			$oof->table(
				{class=>"babsTable"},
				[{class=>"babsDesc", value=>"Title:"},
				 {class=>$babs->gen_class, value=>
				  $oof->input(type=>"text", name=>"title", value=>$fields{title})}],
				[{class=>"babsDesc", value=>":"},
				 {class=>$babs->gen_class, value=>
				  $oof->input(type=>"text", name=>"title", value=>$fields{title})}],
			)
		),
		$babs->footer();
}
