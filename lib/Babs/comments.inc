# Babs comment routines
# $Id$

=head2 Comment Routines

These routines house a collection of functionality pertaining
to comment-handling.

=over

=cut

use strict;
use warnings;

sub _comment_init {
	my ($this) = @_;
	my $g = $this->{error_const_group};

	$g->add("E_COMMENT_COMMENT");	# No comment field
	$g->add("E_COMMENT_SUBJECT");	# No subject
	$g->add("E_COMMENT_STORY");	# No such story
	$g->add("E_COMMENT_PRIV");	# Not allowed
	$g->add("E_COMMENT_EXIST");	# Non-existant comment
	$g->add("E_COMMENT_MAX");	# Max comments reached
}

=item $babs->comment_add($comment);

This routine adds the argued comment to the database.
The return value is an array containing two values.
The first value is a bit vector which can contain any of the
following system-defined error constants.

=over

=item C<E_COMMENT_COMMENT>

The argued comment field is non-existant or invalid.

=item C<E_COMMENT_SUBJECT>

The argued subject field is non-existant or invalid.

XXX: this field should be able to be controlled by a preference
which defaults the parent comment's title or die on invalid argument.

=item C<E_COMMENT_STORY>

The argued associated story is invalid or non-existant.

=item C<E_COMMENT_PRIV>

The argued author does not have privileges to post this comment, either
temporarily or permanently.

=item C<E_COMMENT_EXIST>

The argued associated parent comment to which this comment is to
be posted is non-existant or invalid.

=item C<E_COMMENT_MAX>

The maximum number of comments has been exceeded.

XXX: there should be a total bound and maximum depth that a
particular hierarchial comment thread can reach.

=back

The second value is itself an array reference containing
user-defined field identifiers for which the argued fields
have not met the required criteria.

On success, this method therefore returns an array of C<E_NONE>
and a reference to an empty array (denoting no offending
user-defined field values).

This method adjusts the C<num_comments> field (which reflects
the number of comments) of the associated story.

This method fires the C<EVT_COMMENT_ADD> event on success.
The event argument list contains a single hash reference to all
comment fields.

=cut

sub comment_add {
	my ($this, $comment) = @_;
	my $dbh = $this->{dbh};
	my $err = E_NONE();

	# Validate story
	return (E_STORY_EXIST(), []) unless $this->story_exists($comment->{story_id});

	my $story = $this->story_get($comment->{story_id});

	# Validate info
	$err |= E_COMMENT_COMMENT()	unless $comment->{comment};
	$err |= E_COMMENT_SUBJECT()	unless $comment->{subject};
	$err |= E_COMMENT_PRIV()	unless $this->{allow_comments};
	$err |= E_COMMENT_MAX()		if $this->{max_comments} &&
						$story->{num_comments} >= $this->{max_comments};
	$err |= E_USER_EXIST()		unless $this->user_exists($comment->{author_id});
	$err |= E_COMMENT_EXIST()	if $comment->{parent_comment_id} &&
						!$this->comment_exists($comment->{story_id},
							$comment->{parent_comment_id});

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
	while (($field_id, $field) = each( %{ $this->{comment_fields} } )) {
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

=item $babs->comment_update($comment);

This routine updates the argued comment to the database.
The return value is an array containing two values.
The first value is a bit vector which can contain any of
the following system-defined error constants.

=over

=item C<E_COMMENT_COMMENT>

The argued comment field is non-existant or invalid.

=item C<E_COMMENT_SUBJECT>

The argued subject field is non-existant or invalid.

=item C<E_COMMENT_STORY>

The argued associated story is non-existant or invalid.

=item C<E_COMMENT_EXIST>

The argued comment to which this update is supposed to happen
is non-existant or invalid.

=back

The second value returned is similar to that of C<Babs::comment_add()>.

On success, this method therefore returns an array of C<E_NONE> and
a reference to an empty array.

This method fires the C<EVT_COMMENT_UPDATE> event on success.
The event argument list contains a single hash reference to all
comment fields.

=cut

sub comment_update {
	my ($this, $comment) = @_;
	my $dbh = $this->{dbh};
	my $err = E_NONE();

	# Validate info
	$err |= E_COMMENT_COMMENT()	unless $comment->{comment};
	$err |= E_COMMENT_SUBJECT()	unless $comment->{subject};
	$err |= E_COMMENT_STORY()	unless $this->story_exists($comment->{story_id});
	$err |= E_COMMENT_EXIST()	unless $this->comment_exists($comment->{story_id},
						$comment->{comment_id});

	my $uerr = $this->udf_validate(FLD_COMMENT(), $comment);

	return ($err, $uerr) if $err != E_NONE() || @$uerr;

	# Prepare data
	$comment->{subject} = $dbh->prepare_str($this->str_parse(
				$comment->{subject}, STR_NONE()), SQL_REG);
	$comment->{comment} = $dbh->prepare_str($this->str_parse($comment->{comment}), SQL_REG);

	my $sql = "	UPDATE
				babs_comments
			SET
				mod_date	= NOW(), ";

	my ($field_id, $field);
	while (($field_id, $field) = each %{ $this->{comment_fields} }) {
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

=item $babs->comment_remove($story_id, $comment_id);

This method removes the argued comment and all of its children from
the database. A list of the comment identifiers, if any, is returned.

This method adjusts the C<num_comments> field of the associated
story.

This method fires the C<EVT_COMMENT_REMOVE> event.

=cut

sub comment_remove {
	my ($this, $story_id, $comment_id) = @_;
	my $dbh = $this->{dbh};

	return () unless $this->story_exists($story_id);
	return () unless $this->comment_exists($story_id, $comment_id);

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
	while (($i_id, $i_parent) = $dbh->fetch_row) {
		if (exists $all{$i_parent}) {
			push @{ $all{$i_parent} }, $i_id;
		} else {
			$all{$i_parent} = [$i_id];
		}
	}

	# Select child comments of target
	my ($i_children, $i_child);
	my @targets = ();
	while (($i_parent, $i_children) = each %all) {
		foreach $i_child (@$i_children) {
			if (in_array($comment_id, $this->comment_get_ancestors($i_child, \%all))) {
				push @targets, $i_child;
			}
		}
	}

	foreach $i_id (@targets) {
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

=item $babs->comment_get($story_id, $comment_id);

This method gathers a comment. The value returned is a hash reference
containing as key/value pairs the various comment fields, including
user-defined fields as regular key/value pairs, just as any other
field. If the requested story could not be gathered, an C<undef>
value is returned instead.

=cut

sub comment_get
{
	my ($this, $story_id, $comment_id) = @_;
	return undef unless $this->comment_exists($story_id, $comment_id);
	my %comment = $this->{dbh}->query("	SELECT
							*
						FROM
							babs_comments
						WHERE
							story_id   = $story_id
						AND	comment_id = $comment_id", DB_ROW);
	if (%comment) {
		my %c = ();
		my ($k, $v);
		while (($k, $v) = each %comment) {
			# Strip `ud_' from each user-defined field
			$c{$k =~ /^ud_/ ? $' : $k} = $v;
		}
		%comment = %c;
	}
	return \%comment;
}

=item $babs->comment_get_ancesters($needle, $hay)

This method returns an array reference containing comment identifiers
for every ancestor of the argued comment.

Arguments:

=over

=item $needle

This argument should specify the comment identifier for which ancestors
will be searched.

=item $hay

This argument should be a hash reference containing comment identifiers
as keys and array references of their lists of children directly
underneath each as values.

=back

=cut

sub comment_get_ancestors {
	my ($this, $needle, $hay) = @_;
	my @ancestors = ($needle);
	my $resume = TRUE();
	my ($parent, $children, $child);

STARTOVER:
	while ($resume) {
		$resume = FALSE();
		while (($parent, $children) = each %$hay) {
			foreach $child (@$children) {
				if ($child == $needle) {
					push @ancestors, $parent;
					# Find grandparent
					$needle = $parent;
					$resume = TRUE();
					next STARTOVER;
				}
			}
		}
	}

	return \@ancestors;
}

=item $babs->comment_exists($story_id, $comment_id);

This method determines whether the argued comment identifier
refers to an existant comment.

=cut

sub comment_exists {
	my ($this, $story_id, $comment_id) = @_;
	return FALSE() unless $story_id   && $story_id   =~ /^\d+$/ &&
			      $comment_id && $comment_id =~ /^\d+$/;
	return $this->{dbh}->query("	SELECT
						comment_id
					FROM
						babs_comments
					WHERE
						story_id   = $story_id
					AND	comment_id = $comment_id", DB_COL);
}

=back

=cut

return TRUE();
