# $Id$

our $comment;

$comment = $babs->comment_get($story_id, $comment_id);

if (ref $comment eq "HASH") {
	our ($date, $author_url, $edit_url, $delete_url, $subject,
	     $comment_url);

	my $author = $babs->dbh_selectcol("
			SELECT
				username
			FROM
				babs_users
			WHERE
				user_id = ?", $comment->{author_id});
	$date = Timestamp->new($comment->{create_date})->format($babs->{time_format});
	$author_url	= $babs->build_url("profile",
				user_id    => $comment->{author_id});
	$edit_url 	= $babs->build_url("admin/com-edit",
				story_id   => $comment->{story_id},
				comment_id => $comment->{comment_id});
	$delete_url	= $babs->build_url("admin/com-delete",
				story_id   => $comment->{story_id},
				comment_id => $comment->{comment_id});
	$comment_url	= "#";

	if ($babs->{allow_comment}) {
		if ($babs->{hier_comments}) {
			$comment_url = $babs->build_url("admin/com-add",
					story_id => $comment->{story_id},
					parent_comment_id => $comment->{comment_id});
		} else {
			$comment_url = $babs->build_url("admin/com-add",
					story_id => $comment->{story_id});
		}
	}

	$subject = $comment->{subject};
	$comment = $comment->{comment};

	return TRUE();
}

return FALSE();
