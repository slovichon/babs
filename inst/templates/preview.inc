# $Id$

my $story = $babs->story_get($story_id);

if (ref $story eq "HASH") {
	our ($date, $words, $comments, $author, $profile_url,
	     $story_url, $headline, $overview);

	$date 	= Timestamp->new($story->{"create_date"})->format($babs->{time_format});
	$words	= ($babs->str_parse($story->{story}, Babs::STR_NONE()) =~ /\w+/g);
	$comments = $babs->{dbh}->query("
			SELECT
				COUNT(*)
			FROM
				babs_comments
			WHERE
				story_id = " . $story->{story_id}, DB_COL);
	$author = $babs->{dbh}->query("
			SELECT
				username
			FROM
				babs_users
			WHERE
				user_id = " . $story->{author_id}, DB_COL);
	$profile_url	= $babs->build_url("profile", user_id => $story->{author_id});
	$story_url	= $babs->build_url("view", story_id => $story->{story_id});
	$headline	= $story->{headline};
	$overview	= $story->{overview};

	return TRUE();
}

return FALSE();
