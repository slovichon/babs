# $Id$

our $story;

$story = $babs->story_get($story_id);

if (ref $story eq "HASH") {
	our ($date, $profile_url, $edit_url, $delete_url,
	     $author, $comment_url, $headline, $overview);

	$date = Timestamp->new($story->{create_date})->format($babs->{time_format});
	$profile_url 	= $babs->build_url("profile", user_id => $story->{author_id});
	$edit_url 	= $babs->build_url("admin/story-edit",   story_id => $story->{story_id});
	$delete_url 	= $babs->build_url("admin/story-delete", story_id => $story->{story_id});
	$author = $babs->{dbh}->query("
			SELECT
				username
			FROM
				babs_users
			WHERE
				user_id = " . $story->{author_id}, DB_COL);
	if ($babs->{allow_comments}) {
		$comment_url = $babs->build_url("admin/com-add",
				story_id => $story->{story_id},
				parent_comment_id => 0);
	} else {
		$comment_url = "#";
	}
	$headline	= $story->{headline};
	$overview	= $story->{overview};
	$story		= $story->{story};
	return TRUE();
}

return FALSE();
