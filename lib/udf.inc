# udf.inc - Babs user-defined fields routines
# $Id$
# Notes:
#	- User-defined fields exist in the database
#	with the `ud_' prefix to provide namespace
#	avoidance with our own fields and/or database
#	keywords.

use Storable;

use constant FLD_USER		=> 1;
use constant FLD_STORY		=> 2;
use constant FLD_COMMENT	=> 3;

sub _udf_init
{
	my $this = shift;
	$this->udf_update unless $this->{skip_init};
}

sub udf_validate
{
	my ($this, $type, $data) = @_;
	my @errs = ();
	my %flds = ();
	my %map = (
		FLD_USER	=> "user_fields",
		FLD_STORY	=> "story_fields",
		FLD_COMMENT	=> "comment_fields",
	);

	$this->{wasp}->throw("Unrecognized field type; type: $type") unless exists $map{$type};

	# We will process all items and generate
	# as much error information as possible
	# in hopes that the user will not multiply
	# submit data, correcting errors only one
	# at a time.
	my ($field_id, $field);
	while (($field_id, $field) = each %{ $this->{$map{$type}} })
	{
		if ($field->{validation})
		{
			unless ($data->{$field_id} =~ $field->{validation})
			{
				if ($field->{required})
				{
					push @errs, $field_id;
				} else {
					$data->{$field_id} = $field->{default_value};
				}
			}
		}
	}

	return @errs;
}

# Checks to see if the user-defined fields fingerprints
# have changed and updates the structures accordingly.
sub udf_update
{
	my $this = shift;
	my $change = FALSE;
	my %old_types;
	my %changed = ();

	@old_types{qw(user_fields story_fields comment_fields)} =
		$this->{dbh}->query("	SELECT
						user_fields_sig,
						story_fields_sig,
						comment_fields_sig
					FROM
						babs_config", DB_ROW);
	my ($field_id, $field_sig, $old_fields);
	while (($field_id, $field_sig) = each %old_types)
	{
		$old_fields = Storable::thaw($field_sig);
		$old_fields = {} unless ref $old_fields eq "HASH";
		if (hasheq($old_fields, $this->{$field_id}))
		{
			$change = TRUE;
			$changed{$field_id} = $old_fields;
		}
	}

	$this->udf_update_db(\%changed) if $change;
}

# An update has occured, write the new structure out
# to the database.
sub udf_update_db
{
	my ($this, $changed) = @_;
	my $dbh = $this->{dbh};

	# Mapping between field types and database table names
	my %map = (
		user_fields	=> "babs_users",
		story_fields	=> "babs_stories",
		comment_fields	=> "babs_comments",
	);

	my ($id, $fields, @removed, %diff, $field_id,
		$field, $new_field_id, $new_field, %sigs);

	while (($id, $old_fields) = each %$changed)
	{
		# Removed fields
		@removed = ();

		# "Difference" fields (i.e., new fields)
		%diff = %{ $this->{$id} };

		while (($field_id, $field) = each %$old_fields)
		{
			if (exists $diff{$field_id} && hasheq($diff{$field_id}, $field))
			{
				# Field exists in both; remove
				delete $diff{$field_id};
			} else {
				# There's a difference (new field); save for later
				push @removed, $field_id;
			}
		}

		# Removed dropped fields
		foreach (@removed)
		{
			$dbh->query("	ALTER TABLE
						$map{$id}
					DROP
						ud_$_", DB_NULL);
		}

		# Add new fields
		while (($new_field_id, $new_field) = each %diff)
		{
			$dbh->query("	ALTER TABLE
						$map{$id}
					ADD
						ud_$new_field_id $new_field->{database_type}", DB_NULL);
		}

		# Create signature on current/"next" fields
		$sigs{$id} = $dbh->prepare_str(Storable::freeze($this->{$id}), SQL_REG);
	}

	# Update signatures
	if ($dbh->query("	SELECT
					COUNT(*)
				FROM
					babs_config", DB_COL))
	{
		# Already exists; update
		$dbh->query("	UPDATE
					babs_config
				SET
					user_fields_sig		= '$sigs{user_fields}',
					story_fields_sig	= '$sigs{story_fields}',
					comment_fields_sig	= '$sigs{comment_fields}'", DB_NULL);
	} else {
		# Doesn't exist; insert
		$dbh->query("	INSERT INTO
					babs_config
				(
					user_fields_sig,
					story_fields_sig,
					comment_fields_sig
				) VALUES (
					'$sigs{user_fields}',
					'$sigs{story_fields}',
					'$sigs{comment_fields}'
				)", DB_NULL);
	}
}

return 1;
