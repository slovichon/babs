# Babs user routines
# $Id$

use strict;

# User levels
use constant USER_LVL_COMMENTER	=> 1;
use constant USER_LVL_REGULAR	=> 2;
use constant USER_LVL_ADMIN	=> 3;

sub _user_init
{
	my ($this) = @_;
	my $g = $this->{error_const_group};

	# User errors
	$g->add("E_USER_NAME");		# Invalid username
	$g->add("E_USER_NAME_USE");	# Username already in use
	$g->add("E_USER_PASS");		# Invalid password
	$g->add("E_USER_EMAIL");	# Invalid e-mail address
	$g->add("E_USER_TYPE");		# Invalid user type
	$g->add("E_USER_EXIST");	# Non-existant user
}

sub user_add
{
	my ($this, $user) = @_;
	my $err = E_NONE();
	my $dbh = $this->{dbh};

	# Validate info
	$err |= E_USER_NAME()	unless $user->{username} && $user->{username} =~ /^[a-zA-Z0-9_-]{4,30}$/;
	$err |= E_USER_PASS()	unless $user->{password} && length($user->{password}) < 6;
	$err |= E_USER_EMAIL()	unless $this->valid_email($user->{email});
	$err |= E_USER_TYPE()	unless $user->{type} && $user->{type} =~ /^\d+$/;

	# Validate unique username
	my $t = $dbh->prepare_str($user->{username}, SQL_REG);

	$err |= E_USER_NAME_USE() if
		$dbh->query("	SELECT
					username
				FROM
					babs_users
				WHERE
					username = '$t'", DB_COL);

	# Validate user-defined fields
	my $uerr = $this->udf_validate(FLD_USER(), $user);
	
	return ($err, $uerr) if $err != E_NONE() || @$uerr;

	# Prepare data
	$user->{email}		= $dbh->prepare_str($this->str_parse(
					$user->{email}, STR_NONE()), SQL_REG);
	$user->{password}	= $dbh->prepare_str($this->crypt($user->{password}), SQL_REG);

	my $sql_fields = "";
	my $sql_values = "";

	my ($field, $field_id);
	while (($field_id, $field) = each(%{ $this->{user_fields} }))
	{
		$sql_fields .= ", ud_$field_id";
		$sql_values .= ", '$user->{$field_id}'";
	}

	$dbh->query("	INSERT INTO
				babs_users
			(
				username,
				password,
				email,
				type
				$sql_fields
			) VALUES (
				'$user->{username}',
				'$user->{password}',
				'$user->{email}',
				'$user->{type}'
				$sql_values
			)", DB_NULL);
	return (E_NONE(), []);
}

sub user_update
{
	my ($this, $user) = @_;
	my $err = E_NONE();
	my $dbh = $this->{dbh};

=comment
Notes:
	- If no new password is specified, don't change it.

	- We obviously cannot print the concurrent password
	out as the default field value.

	- Usernames cannot be changed.
=cut
	# Validate info
	$err |= E_USER_PASS()	if $user->{password} && !(length($user->{password} < 6));
	$err |= E_USER_EMAIL()	unless $this->valid_email($user->{email});
	$err |= E_USER_TYPE()	unless $user->{type} && $user->{type} =~ /^\d+$/;

	# Validate user-defined fields
	my $uerr = $this->udf_validate(FLD_USER(), $user);

	return ($err, $uerr) if $err != E_NONE() || @$uerr;

	# Prepare data
	$user->{email}		= $dbh->prepare_str($this->str_parse(
					$user->{email}, STR_NONE()), SQL_REG);
	$user->{password}	= $dbh->prepare_str($this->crypt($user->{password}),
					SQL_REG) if $user->{password};

	# Build SQL
	my $sql = "	UPDATE
				babs_users
			SET ";

	# Once again, only update the password if its been changed.
	$sql .= "		password	= '$user->{password}', " if $user->{password};

	my ($field, $field_id);
	while (($field_id, $field) = each(%{ $this->{user_fields} }))
	{
		$sql .= "	ud_$field_id	= '$user->{$field_id}', ";
	}

	$sql .= "
				email		= '$user->{email}',
				type		= '$user->{type}'
			WHERE
				user_id		= '$user->{user_id}' ";

	$dbh->query($sql, DB_NULL);

	return (E_NONE, []);
}

sub user_get_id
{
	my ($this, $username) = @_;
	$username = $this->{dbh}->prepare_str($username, SQL_REG);
	return $this->{dbh}->query("	SELECT
						user_id
					FROM
						babs_users
					WHERE
						username = '$username'", DB_COL);
}

sub user_get
{
	my ($this, $user_id) = @_;
	return undef unless $this->user_exists($user_id);
	my %user = $this->{dbh}->query("	SELECT
							*
						FROM
							babs_users
						WHERE
							user_id = $user_id", DB_ROW);
	if (%user)
	{
		my %u = ();
		my ($k, $v);
		while (($k, $v) = each(%user))
		{
			# Strip `ud_' from each user-defined field
			$u{$k =~ /^ud_/ ? $' : $k} = $v;
		}
		%user = %u;
	}
	return \%user;
}

sub user_remove
{
	my ($this, $user_id) = @_;
	my $dbh = $this->{dbh};
	return FALSE() unless defined $user_id && $user_id =~ /^\d+$/;
	$dbh->query("	DELETE FROM
				babs_stories
			WHERE
				author_id = $user_id", DB_NULL);
	$dbh->query("	DELETE FROM
				babs_comments
			WHERE
				author_id = $user_id", DB_NULL);
	return $dbh->query("	DELETE FROM
					babs_users
				WHERE
					user_id = $user_id", DB_NULL);
}

sub user_exists
{
	my ($this, $user_id) = @_;
	return FALSE() unless $user_id && $user_id =~ /^\d+$/;
	return $this->{dbh}->query("	SELECT
						user_id
					FROM
						babs_users
					WHERE
						user_id = $user_id", DB_COL);
}

return TRUE();
