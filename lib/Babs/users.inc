# $Id$

=head2 User Routines

This sub-module provides a framework for handling users.

=over

=cut

use strict;
use warnings;

# User levels
use constant USER_LVL_COMMENTER	=> 1;
use constant USER_LVL_REGULAR	=> 2;
use constant USER_LVL_ADMIN	=> 3;

sub _user_init {
	my ($this) = @_;
	my $g = $this->{error_const_group};

	# User errors
	$g->add("E_USER_NAME");		# Invalid username
	$g->add("E_USER_PASS");		# Invalid password
	$g->add("E_USER_EMAIL");	# Invalid e-mail address
	$g->add("E_USER_TYPE");		# Invalid user type
	$g->add("E_USER_EXIST");	# Non-existant user
}

=item C<< ($err, $uerr) = $babs->user_add(\%user); >>

This routine adds the argued user to the database.
The return value is an array containing two values.
The first value is a bit vector which can contain any of the
following system-defined error constants.

=over

=item C<E_USER_NAME>

The argued username field is non-existant or invalid.

=item C<E_USER_PASS>

The argued password field is non-existant or invalid.

=item C<E_USER_EMAIL>

The argued e-mail address is non-existant or invalid.
Note that I<invalid> here can mean different things depending
upon the value of the Babs configuration option C<email_check>.

=item C<E_USER_TYPE>

The argued type field is non-existant or the user which
is creating this user account does not have permission
to create an account of this type.

=item C<E_USER_EXIST>

The argued user account already exists.

=back

The second value is itself an array reference containing
user-defined field identifiers for which the argued fields
have not met the required user-defined criteria.

On success, this method therefore returns an array of C<E_NONE>
and a reference to an empty array (denoting no offending
user-defined field values).

This method fires the C<EVT_USER_ADD> event on success.
The event argument list contains a single hash reference to all
user fields.

=cut

sub user_add {
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

	$err |= E_USER_EXIST() if
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
	$user->{email} = $dbh->prepare_str($this->str_parse(
				$user->{email}, STR_NONE()), SQL_REG);
	$user->{password} = $dbh->prepare_str($this->crypt($user->{password}), SQL_REG);

	my $sql_fields = "";
	my $sql_values = "";

	my ($field, $field_id);
	while (($field_id, $field) = each(%{ $this->{user_fields} })) {
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

=item C<< ($err, $uerr) = $babs->user_update(\%user); >>

This routine updates the argued user account in the database.
The return value is an array containing two values.
The first value is a bit vector which can contain any of
the following system-defined error constants.

=over

=item C<E_USER_NAME>

The argued username field is non-existant or invalid.

=item C<E_USER_PASS>

The argued password field is invalid.  (If the password
field is not specified, i.e., non-existant, then the
account password will not be changed.

=item C<E_USER_EMAIL>

The argued e-mail field is non-existant or invalid.
Note that the strictness of I<invalid> depends upon
the Babs configuration option C<email_check>.

=item C<E_USER_TYPE>

The argued account type is invalid or the user updating
this account in question does not have permission to
change the account type to this type.

=back

The second value returned is similar to that of
L<Babs::user_add()>.

On success, this method therefore returns an array of
C<E_NONE> and a reference to an empty array.

This method fires the C<EVT_USER_UPDATE> event on success.
The event argument list contains a single hash reference to
all user account fields.

=cut

sub user_update {
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
	$user->{email} = $dbh->prepare_str($this->str_parse(
				$user->{email}, STR_NONE()), SQL_REG);
	$user->{password} = $dbh->prepare_str($this->crypt($user->{password}),
				SQL_REG) if $user->{password};

	# Build SQL
	my $sql = "	UPDATE
				babs_users
			SET ";

	# Once again, only update the password if its been changed.
	$sql .= "		password	= '$user->{password}', " if $user->{password};

	my ($field, $field_id);
	while (($field_id, $field) = each(%{ $this->{user_fields} })) {
		$sql .= "	ud_$field_id	= '$user->{$field_id}', ";
	}

	$sql .= "
				email		= '$user->{email}',
				type		= '$user->{type}'
			WHERE
				user_id		= '$user->{user_id}' ";

	$dbh->query($sql, DB_NULL);

	return (E_NONE(), []);
}

=item C<< $user_id = $babs->user_get_id($username); >>

This method gathers the user account identification number for
the associated C<$username> or zero if no such user exists.

=cut

sub user_get_id {
	my ($this, $username) = @_;
	$username = $this->{dbh}->prepare_str($username, SQL_REG);
	return $this->{dbh}->query("	SELECT
						user_id
					FROM
						babs_users
					WHERE
						username = '$username'", DB_COL);
}

=item C<< $user = $babs->user_get($user_id); >>

This method returns a hash reference containing the user
account fields for the associated C<$user_id>.  If no such
user account exists, the hash reference will refer to an
empty hash.

=cut

sub user_get {
	my ($this, $user_id) = @_;
	return undef unless $this->user_exists($user_id);
	my %user = $this->{dbh}->query("
			SELECT
				*
			FROM
				babs_users
			WHERE
				user_id = $user_id ", DB_ROW);
	if (%user) {
		my %u = ();
		my ($k, $v);
		while (($k, $v) = each(%user)) {
			# Strip `ud_' from each user-defined field
			$u{$k =~ /^ud_/ ? $' : $k} = $v;
		}
		%user = %u;
	}
	return \%user;
}

=item C<< $removed = $babs->user_remove($user_id); >>

This routine removes the associated user from the database.

The Boolean status returned determines whether or not the
remove was successful.

On success as well, this method fires the C<EVT_USER_REMOVE>
event with an argument list

XXX: user remove event

=cut

sub user_remove {
	my ($this, $user_id) = @_;
	my $dbh = $this->{dbh};
	return FALSE() unless defined $user_id && $user_id =~ /^\d+$/;
	$dbh->query("
		DELETE FROM
			babs_stories
		WHERE
			author_id = $user_id", DB_NULL);
	$dbh->query("
		DELETE FROM
			babs_comments
		WHERE
			author_id = $user_id", DB_NULL);
	return $dbh->query("
		DELETE FROM
			babs_users
		WHERE
			user_id = $user_id", DB_NULL);
}

=item C<< $exists = $babs->user_exists($user_id); >>

This method determines whether or not the argued C<$user_id>
is associated with an existant user account.

=cut

sub user_exists {
	my ($this, $user_id) = @_;
	return FALSE() unless $user_id && $user_id =~ /^\d+$/;
	return $this->{dbh}->query("	SELECT
						user_id
					FROM
						babs_users
					WHERE
						user_id = $user_id", DB_COL);
}

=back

=cut

return TRUE();
