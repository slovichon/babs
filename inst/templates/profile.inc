# $Id$

my $user = $babs->user_get($user_id);

if (ref $user eq "HASH") {
	our ($type, $email, $stories, $last_story_headline, $username);

	$type = $babs->{levels}[$user->{type}];
	$email = $babs->{of}->email($user->{email});
	$stories = $babs->{dbh}->query("
			SELECT
				COUNT(*)
			FROM
				babs_stories
			WHERE
				author_id = " . $user->{user_id}, DB_COL);
	my $last_story = $babs->{dbh}->query("
				SELECT
					story_id,
					create_date,
					headline
				FROM
					babs_stories
				WHERE
					author_id = " . $user->{user_id} . "
				ORDER BY
					create_date DESC
				LIMIT
					1 ", DB_ROW);
	
	if (ref $last_story eq "HASH") {
		my $last_story_date = Timestamp->new($last_story->{create_date})->format(
					$babs->{time_format});
		my $last_story_url = $babs->build_path("view",
					story_id => $last_story->{story_id});
		$last_story_headline = $babs->{of}->link("&quot;$last_story_headline&quot; " .
					"posted on $last_story_date");
	} else {
		$last_story_headline = "Never";
	}

	$username = $user->{username};

	return TRUE();
}

return FALSE();
