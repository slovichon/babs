# Babs comment routines
# $Id$

use DBH qw(:all);
use strict;

sub _comment_init
{
	my ($this) = @_;
	my $g = $this->{error_const_group};

	$g->add("E_COMMENT_COM");	# No comment field
	$g->add("E_COMMENT_SUBJECT");	# No subject
	$g->add("E_COMMENT_STORY");	# No such story
	$g->add("E_COMMENT_PRIV");	# Not allowed
	$g->add("E_COMMENT_EXIST");	# Non-existant comment
	$g->add("E_COMMENT_MAX");	# Max comments reached
}

sub comment_add
{
	my ($this, $comment) = @_;
	my $dbh = $this->{dbh};
	my $err = E_NONE();

	# Validate story
	return (E_STORY_EXIST(), []) unless $this->story_exists($comment->{story_id});

	my $story = $this->story_get($comment->{story_id});

	# Validate info
	$err |= E_COMMENT_COM()		unless $comment->{comment};
	$err |= E_COMMENT_SUBJECT()	unless $comment->{subject};
	$err |= E_COMMENT_PRIV()	unless $this->{allow_comments};
	$err |= E_COMMENT_MAX()		if $this->{max_comments} &&
						$story->{num_comments} >= $this->{max_comments};
	$err |= E_USER_EXIST()		unless $this->user_exists($comment->{author_id});
	$err |= E_COMMENT_EXIST()	if $comment->{parent_comment_id} &&
						!$this->comment_exists($comment->{parent_comment_id});

	my $uerr = $this->udf_validate(FLD_COMMENT(), $comment);

	return ($err, $uerr) if $err != E_NONE() || @$uerr;

	# Prepare data
	$comment->{comment} = $dbh->prepare_str($this->str_parse($comment->{comment}), SQL_REG);
	$comment->{subject} = $dbh->prepare_str($this->str_parse($comment->{subject}, STR_NONE()), SQL_REG);
	$comment->{parent_comment_id} = 0 unless $comment->{parent_comment_id};

	# Generate unique comment id
	$comment->{comment_id} = 1 + $dbh->query("	SELECT
								comment_id
							FROM
								babs_comments
							WHERE
								story_id = $comment->{story_id}
							ORDER BY
								comment_id DESC
							LIMIT 1 ", DB_NULL);

	# Build SQL
	my $sql_fields = "";
	my $sql_values = "";

	my ($field_id, $field);
	while (($field_id, $field) = each( %{ $this->{comment_fields} } ))
	{
		$comment->{$field_id} = $dbh->prepare_str($this->str_parse(
						$comment->{$field_id}, $field->{parse}), SQL_REG);

		$sql_fields .= " ,ud_$field_id ";
		$sql_values .= " ,'{$comment->{$field_id}}' ";
	}

	$dbh->query("	INSERT INTO
				babs_comments
			(
				story_id,
				comment_id,
				author_id,
				subject,
				comment,
				create_date,
				parent_comment_id
				$sql_fields
			) VALUES (
				$comment->{story_id},
				$comment->{comment_id},
				$comment->{author_id},
				'$comment->{subject}',
				'$comment->{comment}',
				NOW(),
				$comment->{parent_comment_id},
			)", DB_NULL);

	$dbh->query("	UPDATE
				babs_stories
			SET
				num_comments = num_comments + 1
			WHERE
				story_id = $comment->{story_id}", DB_NULL);

	$this->event_fire(EVT_COMMENT_ADD(), $comment);

	return (E_NONE(), []);
}

sub comment_update
{
	my ($this, $comment) = @_;
	my $dbh = $this->{dbh};
	my $err = E_NONE();

	# Validate info
	$err |= E_COMMENT_COM()		unless $comment->{comment};
	$err |= E_COMMENT_SUBJECT()	unless $comment->{subject};
	$err |= E_COMMENT_STORY()	unless $this->story_exists($comment->{story_id});
	$err |= E_COMMENT_EXIST()	unless $this->comment_exists($comment->{comment_id});

	my $uerr = $this->udf_validate(FLD_COMMENT(), $comment);

	return ($err, $uerr) if $err != E_NONE() || @$uerr;

	# Prepare data
	$comment->{subject} = $dbh->prepare_str($this->str_parse($comment->{subject}, STR_NONE()), SQL_REG);
	$comment->{comment} = $dbh->prepare_str($this->str_parse($comment->{comment}), SQL_REG);

	my $sql = "	UPDATE
				babs_comments
			SET
				mod_date	= NOW(), ";

	my ($field_id, $field);
	while (($field_id, $field) = each %{ $this->{comment_fields} })
	{
		$comment->{$field_id} = $dbh->prepare_str($this->str_parse(
						$comment->{$field_id}, $field->{parse}), SQL_REG);

		$sql .= "	ud_$field_id	= '$comment->{$field_id}', ";
	}

	$sql .= "
				subject		= '$comment->{subject}',
				comment		= '$comment->{comment}'
			WHERE
				story_id	= $comment->{story_id}
			AND	comment_id	= $comment->{comment_id} ";
	$dbh->query($sql, DB_NULL);
	$this->event_fire(EVT_COMMENT_UPDATE(), $comment);
	return (E_NONE(), []);
}

sub comment_remove
{
	my ($this, $story_id, $comment_id) = @_;
	my $dbh = $this->{dbh};

	return FALSE() unless $this->story_exists($story_id);
	return FALSE() unless $this->comment_exists($comment_id);

	# We must remove all "child" comments of the comment
	# in question. To do this, we compile a list and find
	# every comment with an "ancestor" of the target and
	# remove them.
	$dbh->query("	SELECT
				comment_id,
				parent_comment_id
			FROM
				babs_comments
			WHERE
				story_id = $story_id", DB_ROWS);

	my %all = ();

	# Create a list of parent => child comments
	my ($i_id, $i_parent);
	while (($i_id, $i_parent) = $dbh->fetch_row)
	{
		if (exists $all{$i_parent})
		{
			push @{ $all{$i_parent} }, $i_id;
		} else {
			$all{$i_parent} = [$i_id];
		}
	}

	# Select child comments of target
	my ($i_children, $i_child);
	my @targets = ();
	while (($i_parent, $i_children) = each %all)
	{
		foreach $i_child (@$i_children)
		{
			if (in_array($comment_id, $this->comment_get_ancestors($i_child, \%all)))
			{
				push @targets, $i_child;
			}
		}
	}

	foreach $i_id (@targets)
	{
		$this->event_fire(EVT_COMMENT_REMOVE(), $i_id);
	}

	# Removal
	$dbh->query("	DELETE FROM
				babs_comments
			WHERE
				story_id = $story_id
			AND	comment_id IN (" . join(",", @targets) . ")", DB_NULL);

	$dbh->query("	UPDATE
				babs_stories
			SET
				num_comments = num_comments - " . @targets . "
			WHERE
				story_id = $story_id", DB_NULL);

	return @targets;
}

sub comment_get
{
	my ($this, $story_id, $comment_id) = @_;
	return undef unless $this->comment_exists($comment_id);
	my %comment = $this->{dbh}->query("	SELECT
							*
						FROM
							babs_comments
						WHERE
							story_id   = $story_id
						AND	comment_id = $comment_id", DB_ROW);
	if (%comment)
	{
		my %c = ();
		my ($k, $v);
		while (($k, $v) = each %comment)
		{
			# Strip `ud_' from each user-defined field
			$c{$k =~ /^ud_/ ? $' : $k} = $v;
		}
		%comment = %c;
	}
	return \%comment;
}

sub comment_get_ancestors
{
	my ($this, $needle, $hay) = @_;
	my @ancestors = ($needle);
	my $resume = TRUE();
	my ($parent, $children, $child);

	STARTOVER: while ($resume)
	{
		$resume = FALSE();
		while (($parent, $children) = each(%$hay))
		{
			foreach $child (@$children)
			{
				if ($child == $needle)
				{
					push @ancestors, $parent;
					# Find parent's parent
					$needle = $parent;
					$resume = TRUE();
					next STARTOVER;
				}
			}
		}
	}

	return \@ancestors;
}

sub comment_exists
{
	my ($this, $comment_id) = @_;
	return FALSE() unless $comment_id && $comment_id =~ /^\d+$/;
	return $this->{dbh}->query("	SELECT
						comment_id
					FROM
						babs_comments
					WHERE
						comment_id = $comment_id", DB_COL);
}

return TRUE();
