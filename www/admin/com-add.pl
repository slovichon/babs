#!/usr/bin/perl -W
# $Id$

use WASP qw(:std);
use Babs;
use strict;
use warnings;

my $babs = Babs->new(shift);
my $story_id   = $babs->req("story_id");
my $comment_id = $babs->req("comment_id");
my $user_id = $babs->require_login(Babs::PERM_COMMENT_ADD, $story_id, $comment_id);
my $parent_comment_id = 0;

my $oof = $babs->{oof};
my $printform = 1;		# Show "Add Comment" form by default
my $err = Babs::E_NONE();
my $uerr = [];
my $comment = {};

if ($babs->req("submitted")) {
	# "Add Comment" form submitted; try to add comment
	$comment = {
		story_id		=> $story_id,
		parent_comment_id 	=> $parent_comment_id,
		author_id		=> $user_id,
		subject			=> $babs->req("subject"),
		comment			=> $babs->req($comment),
	};
	# Add user-defined fields
	my ($key, $val);
	while (($key, $val) = each $babs->{comment_fields}) {
		$comment{$key} = $babs->req($key);
	}
	($err, $uerr) = $babs->comment_add($comment);

	if ($err || @$uerr) {
		# There was an error; redisplay "Add Comment" form
		$printform = 0;
	} else {
		# No error; try to add the comment
		print	$babs->header("Added Comment"),
			$oof->header("Added Comment"),
			$oof->p("You have successfully posted a comment."),
			$babs->of_actions(),
			$babs->footer();
	}
}

if ($printform) {
	# Simple request; display "Add Comment" form
	print	$babs->header("Adding Comment"),
		$oof->header("Adding Comment"),
		# XXX: user preference
		$oof->p("You are adding a comment to the following story"),
		$babs->template_expand("story", $story_id);

	# Print comment thread
	if ($babs->{hier_comments}) {
		my $current_level = $parent_comment_id;
		my $comments = "";
		while ($current_level != 0) {
			$comments = $babs->template_expand("comment", $story_id, $current_level) . $comments;
			# Get parent's parent
			$current_level = $babs->comment_get($story_id, $current_level)->{parent_comment_id};
		}
		print $comments;
	}

	if ($err != Babs::E_NONE() || @$uerr) {
		my $p = "You have entered invalid input.";

		# Standard errors
		my %errors = (
			Babs::E_COMMENT_COMMENT()	=> "Please enter a comment.",
			Babs::E_COMMENT_SUBJECT()	=> "Please enter a subject.",
			Babs::E_COMMENT_STORY()		=> "The story being commented on does not exist.",
			Babs::E_COMMENT_PRIV()		=> "You are not allowed to post comments at this time.",
			Babs::E_COMMENT_EXIST()		=> "The comment being commented on does not exist.",
			Babs::E_COMMENT_MAX()		=> "The maximum number of comments has been reached.",
		);

		my ($errval, $msg);
		while (($errval, $msg) = each %errors) {
			$p .= " " . $msg if $err & $errval;
		}

		# User-defined errors
		my $field_id;
		foreach $field_id (@$uerr) {
			$p .= " " . $babs->{comment_fields}{$field_id}{error_message};
		}

		print $oof->p($p);
	}

	my $comment_label = "Comment:";

	print	$oof->form_start(),
			$oof->table_start(class=>"babsTable"),
				# Header
				$oof->table_row({class=>"babsHeader", colspan=>2, value=>"Adding Comment"}),
				# Standard fields
				$oof->table_row({class=>"babsDesc", value=>"Subject:"},
				 {class=>$babs->gen_class, value=>
				  $oof->input(type=>"text", name=>"subject",
				  	      value=>$babs->escape_html($comment->{subject})}),

				$oof->table_row({class=>"babsDesc", value=>$comment_label},
				 {class=>$babs->gen_class, value=>
				  $oof->input(type=>"textarea", name=>"comment", value=>$comment->{comment})});
				  
				# User-defined fields
				
				# Footer
	print			$oof->table_row(),
			$oof->table_end(),
		$oof->form_end(),
		$babs->footer();
}

EXIT_SUCCESS;
