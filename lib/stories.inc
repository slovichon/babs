# stories.inc - Babs story routines
# $Id$

use constant E_STORY_HEADLINE	=> (1<<0);	# No headline
use constant E_STORY_STORY	=> (1<<1);	# No story
use constant E_STORY_EXIST	=> (1<<2);	# Non-existant story

sub story_add
{
	my ($this, $story) = @_;
	my $err = E_NONE;
	my $dbh = $this->{dbh};

	# Validate info
	$err |= E_STORY_HEADLINE	unless $story->{headline};
	$err |= E_STORY_STORY		unless $story->{story};
	$err |= E_STORY_EXIST		unless $story->{story_id} && $story->{story_id} =~ /^\d+$/;
	$err |= E_USER_EXIST		unless $this->user_exists($story->{author_id});

	# Validate user-defined fields
	my @uerr = $this->udf_validate(FLD_STORY, $story);

	return [$err, @uerr] if $err != E_NONE || @uerr;

	$story->{headline}	= $dbh->prepare_str($this->str_parse($story->{headline}, STR_NONE), SQL_REG);
	$story->{story}		= $dbh->prepare_str($this->str_parse($story->{story}), SQL_REG);

	my $sql_fields = "";
	my $sql_values = "";

	my ($field, $field_id);
	while (($field_id, $field) = each(%{ $this->{story_fields} }))
	{
		$story->{$field_id} = $dbh->prepare_str(
			$this->str_parse($story->{$field_id}, $field->{parse}), SQL_REG);

		$sql_fields .= " , ud_$field_id ";
		$sql_values .= " , '$story->{$field_id}' ";
	}

	$dbh->query("	INSERT INTO
				babs_stories
			(
				author_id,
				headline,
				story,
				create_date,
				allow_comments
				$sql_fields
			) VALUES (
				$story->{author_id},
			)", DB_NULL);
}

sub story_update
{
	my ($this, $story) = @_;
	my $err = E_NONE;
	my $dbh = $this->{dbh};

	# Validate info
	$err |= E_STORY_HEADLINE	unless $story->{headline};
	$err |= E_STORY_STORY		unless $story->{story};
	$err |= E_STORY_EXIST		unless $story->{story_id} && $story->{story_id} =~ /^\d+$/;

	# Validate user-defined fields
	my @uerr = $this->udf_validate(FLD_STORY, $story);

	return [$err, @uerr] if $err != E_NONE || @uerr;

	$story->{headline}	= $dbh->prepare_str($this->str_parse($story->{headline}, STR_NONE), SQL_REG);
	$story->{story}		= $dbh->prepare_str($this->str_parse($story->{story}), SQL_REG);

	my $sql = "	UPDATE
				babs_stories
			SET
				story		= '$story->{story}',
				mod_date	= NOW(), ";
	
	my ($field, $field_id);
	while (($field_id, $field) = each(%{ $this->{story_fields} }))
	{
		$story->{$field_id} = $dbh->prepare_str(
			$this->prepare_str($story->{$field_id}, $field->{parse}), SQL_REG);
		
		$sql .= "	ud_$field_id	= '$story->{$field_id}', ";
	}

	$sql .= "
				headline	= '$story->{headline}'
			WHERE
				story_id	= $story->{story_id} ";
	
	$dbh->query($sql, DB_NULL);

	# Calculate story id only if it will be used
	# (we're just guessing whether it will be used)
	$story->{story_id} = $dbh->last_insert_id if $this->event_has(EVT_STORY_ADD);

	$this->event_fire(EVT_STORY_ADD, $story);

	return [E_NONE, ()];
}

sub story_remove
{
	my ($this, $story_id) = @_;
	return FALSE unless $story_id && $story_id =~ /^\d+$/;
	my $dbh = $this->{dbh};
	my $recent = $this->story_is_recent($story_id);
	my $ret = $dbh->query("	DELETE FROM
					babs_stories
				WHERE
					story_id = $story_id", DB_NULL);
	$dbh->query("	DELETE FROM
				babs_comments
			WHERE
				story_id = $story_id", DB_NULL);
	$this->event_fire(EVT_STORY_REMOVE, $story_id);
}

sub story_is_recent
{
	my ($this, $story_id) = @_;
}

sub story_get
{
	my ($this, $story_id) = @_;
	return undef unless defined $story_id && $story_id =~ /^\d+$/;
	my %story = $this->{dbh}->query("	SELECT
							*
						FROM
							babs_stories
						WHERE
							story_id = $story_id", DB_ROW);
	if (%story)
	{
		my %s = ();
		my ($k, $v);
		while (($k, $v) = each(%story))
		{
			# Strip `ud_' off of user-defined fields
			$s{$k =~ /^ud_/ ? $' : $k} = $v;
		}
		%story = %s;
	}
	return %story;
}

# Note: grabs top-level comments only
# (that's all we need it for).
sub story_get_comments
{
	my ($this, $story_id) = @_;
	return [] unless defined $story_id && $story_id =~ /^\d+$/;
	my @ids = ();
	$this->{dbh}->query("	SELECT
					comment_id
				FROM
					babs_comments
				WHERE
					story_id		= $story_id
				AND	parent_comment_id	= 0", DB_ROWS);
	my $id;
	push @ids, $id while ($id) = $this->{dbh}->fetch_row();
	return @ids;
}

return 1;
