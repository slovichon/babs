# Babs session routines
# $Id$

use strict;

use constant AUTH_HTTP 		=> 1;
use constant AUTH_COOKIE	=> 1;

sub _session_init
{
	my ($this) = @_;
	my $g = $this->{error_const_group};

	$g->add("E_SESSION_NOREC"); # No record for supposed logged-in user
}

sub session_require_login
{
	my ($this, $priv) = @_;
	# Default is to see whether user is logged in,
	# regardless of privileges
	$priv = USER_LVL_COMMENTER() unless $priv;
	my $user_id = $this->session_is_logged_in;
	if ($user_id)
	{
		# Check privileges
	} else {
		# Do login form show
		if ($this->{auth_type} == AUTH_HTTP())
		{
		} else {
			print 	$this->header("Log In"),
				$this->of_login_form(),
				$this->footer();
		}
		exit(2);
	}
}

sub session_is_logged_in
{
	my ($this) = @_;
	my $user_id;
	# Determine if the user is logged in according
	# to AUTH_TYPE and a valid session exists.
	return $user_id if $user_id;
	return undef;
}

sub session_logout
{
	my ($this) = @_;
	if ($this->session_is_logged_in)
	{
	} else {
		return E_SESSION_NOREC();
	}
}

sub session_login
{
	my ($this, $username, $password) = @_;
	my $thraxx = $this->{thraxx};
	my $dbh = $this->{dbh};

	# Lookup user ID
	$username = $dbh->prepare_str($username, SQL_REG);
	my $user_id = $dbh->query("	SELECT
						user_id
					FROM
						babs_users
					WHERE
						username = '$username' ", DB_COL);
	
	return E_USER_EXISTS() unless $user_id;

	my $ret = $thraxx->session_auth($user_id, $password);

	return $ret unless ref($ret) eq "HASH";

	$this->{session} = $ret;

	return E_NONE();
}

return TRUE();
