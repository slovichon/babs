# Babs story routines
# $Id$

use DBH qw(:all);
use strict;

sub _story_init {
	my ($this) = @_;
	my $g = $this->{error_const_group};
	
	$g->add("E_STORY_HEADLINE");	# No headline
	$g->add("E_STORY_STORY");	# No story
	$g->add("E_STORY_EXIST");	# Non-existant story
}

sub story_add {
	my ($this, $story) = @_;
	my $err = E_NONE();
	my $dbh = $this->{dbh};

	# Validate info
	$err |= E_STORY_HEADLINE()	unless $story->{headline};
	$err |= E_STORY_STORY()		unless $story->{story};
	$err |= E_STORY_EXIST()		unless $story->{story_id} && $story->{story_id} =~ /^\d+$/;
	$err |= E_USER_EXIST()		unless $this->user_exists($story->{author_id});

	# Validate user-defined fields
	my $uerr = $this->udf_validate(FLD_STORY(), $story);

	return ($err, $uerr) if $err != E_NONE() || @$uerr;

	# Prepare data
	$story->{headline} = $dbh->prepare_str($this->str_parse(
				$story->{headline}, STR_NONE()), SQL_REG);
	$story->{story} = $dbh->prepare_str($this->str_parse($story->{story}), SQL_REG);

	# Build SQL
	my $sql_fields = "";
	my $sql_values = "";

	my ($field, $field_id);
	while (($field_id, $field) = each(%{ $this->{story_fields} })) {
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

	$story->{is_recent} = TRUE();

	# Calculate story id only if it will be used
	# (we're just guessing whether it will be used)
	if ($this->event_has(EVT_STORY_ADD())) {
		$story->{story_id} = $dbh->last_insert_id;
		$this->event_fire(EVT_STORY_ADD(), $story);
	}
	
	return (E_NONE(), []);
}

sub story_update {
	my ($this, $story) = @_;
	my $err = E_NONE();
	my $dbh = $this->{dbh};

	# Validate info
	$err |= E_STORY_HEADLINE()	unless $story->{headline};
	$err |= E_STORY_STORY()		unless $story->{story};
	$err |= E_STORY_EXIST()		unless $story->{story_id} && $story->{story_id} =~ /^\d+$/;

	# Validate user-defined fields
	my $uerr = $this->udf_validate(FLD_STORY(), $story);

	return ($err, $uerr) if $err != E_NONE() || @$uerr;

	# Prepare data
	$story->{headline} = $dbh->prepare_str($this->str_parse(
				$story->{headline}, STR_NONE()), SQL_REG);
	$story->{story} = $dbh->prepare_str($this->str_parse($story->{story}), SQL_REG);

	# Build SQL
	my $sql = "	UPDATE
				babs_stories
			SET
				story		= '$story->{story}',
				mod_date	= NOW(), ";
	
	my ($field, $field_id);
	while (($field_id, $field) = each(%{ $this->{story_fields} })) {
		$sql .= "	ud_$field_id	= '$story->{$field_id}', ";
	}

	$sql .= "
				headline	= '$story->{headline}'
			WHERE
				story_id	= $story->{story_id} ";
	
	$dbh->query($sql, DB_NULL);

	$this->event_fire(EVT_STORY_UPDATE(), $story);

	return (E_NONE(), []);
}

sub story_remove {
	my ($this, $story_id) = @_;
	return FALSE() unless $story_id && $story_id =~ /^\d+$/;
	my $dbh = $this->{dbh};

	if ($this->event_has(EVT_STORY_REMOVE())) {
		# Only gather the story if it will be used
		my $story = $this->story_get($story_id);
		$this->event_fire(EVT_STORY_REMOVE(), $story);
	}
	
	my $rows = $dbh->query("	DELETE FROM
						babs_stories
					WHERE
						story_id = $story_id", DB_NULL);
	$dbh->query("	DELETE FROM
				babs_comments
			WHERE
				story_id = $story_id", DB_NULL);
	return $rows ? E_NONE() : E_STORY_EXIST();
}

# This sets the field `is_recent' in a story to the
# corresponding value so it can easily be reused
# without worry of other components continually requesting
# recentness.
sub story_set_recent {
	my ($this, $story, $type) = @_;
	return if defined $story->{is_recent};
	$story->{is_recent} = FALSE();
	return unless $story->{story_id} && $story->{story_id} =~ /^\d+$/;

	# Check type
	$type = "default_result_limit" unless defined $type;
	return FALSE() unless grep {$type eq $_} qw(default_result_limit max_result_limit);

	# Ask for the most recent stories
	$this->{dbh}->query("	SELECT
					story_id	
				FROM
					babs_stories
				ORDER BY
					create_date DESC
				LIMIT
					$this->{$type}", DB_COL);
	my $id;
	while (($id) = $this->{dbh}->fetch_row) {
		if ($id == $story->{story_id}) {
			# Requested story is a recent one
			$story->{is_recent} = TRUE();
			return;
		}
	}
	return;
}

sub story_get {
	my ($this, $story_id) = @_;
	return undef unless $this->story_exists($story_id);
	my %story = $this->{dbh}->query("	SELECT
							*
						FROM
							babs_stories
						WHERE
							story_id = $story_id", DB_ROW);
	if (%story) {
		my %s = ();
		my ($k, $v);
		while (($k, $v) = each(%story)) {
			# Strip `ud_' off of user-defined fields
			$s{$k =~ /^ud_/ ? $' : $k} = $v;
		}
		%story = %s;
	}
	return \%story;
}

# sub story_get {
#	my ($this, $story_id) = @_;
#	return 	$this->story_search
#		(
#			criteria => [
#				[
#					{field=>"story_id", value=>$story_id, op=>"="}
#				]
#			]
#		);
# }

# Note: grabs top-level comments only
# (that's all we need it for).
sub story_get_comments {
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
	return \@ids;
}

sub story_exists {
	my ($this, $story_id) = @_;
	return FALSE() unless $story_id && $story_id =~ /^\d+$/;
	return $this->{dbh}->query("	SELECT
						story_id
					FROM
						babs_stories
					WHERE
						story_id = $story_id", DB_COL);
}

sub story_search {
	my ($this, %prefs) = @_;
	# Determine offset
	my $offset = $prefs{offset} || 0;
	$offset = 0 unless $offset =~ /^\d+$/;
	# Determine sort field
	my $sort_field = $prefs{sort_field};
	$sort_field = "create_date" unless
		$this->isr_contains("babs_stories", $sort_field);
	# Determine sort order
	my $sort_order = $prefs{sort_order};
	$sort_order = "ASC" unless
		Babs::in_array($sort_order, [qw(ASC DESC)]);
	# Determine output template
#	my $template = $prefs{template};
#	return [] unless $this->template_exists($template);
	# Determine result limit (check bounds)
	my $limit = $prefs{limit} || 0;
	$limit = 0 unless $limit =~ /^\d+$/;
	$limit = 0 if $limit > $this->{XXX_result_limit};
	if ($limit) {
		# Try user preference
		# Try default preference
	}
	# Build database query
	my $where = "";
	if (ref $prefs{criteria} eq "ARRAY" && @{ $prefs{criteria} }) {
		$where .= " WHERE ";
		my ($ands, $criteria, $val);
		# foreach `and' group in the `or' groups
		foreach $ands (@{ $prefs{criteria} }) {
			$where .= " ( ";
			foreach $criteria (@$ands) {
				if (defined($val = $this->isr_check_field(
					"babs_stories", $criteria->{field},
					$criteria->{op}, $criteria->{value}))) {
					$where .= sprintf " %s %s %s AND ",
							$criteria->{field},
							$criteria->{op}, $val;
				}
			}
			$where =~ s/ AND $/) OR /;
		}
		$where =~ s/ OR $//;
	}
	# Query database
	my $dbh = $this->{dbh};
	$dbh->query("	SELECT
				*
			FROM
				babs_stories
			$where
			ORDER BY
				$sort_field $sort_order
			LIMIT
				$limit,
				$offset", DB_ROWS);
	my @stories = ();
	my %story;
	while (%story = $dbh->fetch_row) {
		push @stories, {%story};
	}
	return \@stories;
}

return TRUE();
