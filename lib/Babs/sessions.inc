#!/usr/bin/perl
# $Id$
# Babs session routines

use strict;

use constant AUTH_HTTP   => 1;
use constant AUTH_COOKIE => 2;
use constant AUTH_URL    => 3; # Tack on params sto all URLs.

sub _session_init {
	my ($this) = @_;
	my $g = $this->{error_const_group};

	$g->add("E_SESSION_NOREC"); # No record for supposed logged-in user
}

sub session_auth {
	my ($this, $username, $password) = @_;
}

sub session_require_login {
	my ($this, $priv) = @_;
	my $dbh = $this->{dbh};
	# Default is to see whether user is logged in,
	# regardless of privileges
	$priv = USER_LVL_COMMENTER() unless $priv;
	my $user_id = $this->session_is_logged_in;
	if ($user_id) {
		# Check privileges
		my $user = $this->user_get($user_id);
		return $user_id if $user_id && $user->{type} > $priv;
	}

	# Do login form show
	if ($this->{auth_type} == AUTH_HTTP()) {
		my $tried = 0;
		if ($this->{req}) {
			my $auth = $this->{req}->header_in("Authorization");
			if ($auth =~ /Basic (.*)/) {
				$tried = 1;
				require MIME::Base64;
				my $enc_pw = $1;
				my $auth;
				eval {
					$auth = MIME::Base64::decode_base64($enc_pw);
				};
				if (!$@ && $auth =~ /(.*?):(.*)/) {
					my $user = $1;
					my $pass = $2;
					$user_id = $dbh->selectrow_array(<<SQL, {}, $user);
						SELECT
							user_id
						FROM
							babs_users
						WHERE
							username = ?
SQL
					unless ($user_id && $this->{thraxx}->session_auth($user_id, $pass)) {
						return $user_id;
					}
				}
			}
		} else {
#			XXX: CGI.pm auth
		}

		$this->header("Error");
#		$this->header($tried ? "Error" : "Log In"));
		print	$this->{oof}->p("The credentials you have specified are ".
					"invalid. Please try again.");
		$this->footer();
	} else {
		$this->header("Log In");
		print $this->of_login_form();
		$this->footer();
	}

	exit(2);
}

sub session_is_logged_in {
	my ($this) = @_;
	my $user_id;

	# Determine if the user is logged in according
	# to AUTH_TYPE and a valid session exists.
	if ($this->{auth_type} == AUTH_HTTP()) {
		my ($ret, $username, $password);
		if ($this->{req}) {
			($ret, $password) = $this->{req}->get_basic_auth_pw;
			return undef unless $ret == 0;
			$username = $this->{req}->connection->user;
		} else {
			$username = $this->{cgi}->remote_user;
		}
	} elsif ($this->{auth_type} == AUTH_COOKIE()) {
		my ($session_id, $session_key);
		$this->{session_id} = $session_id;
#	} elsif ($this->{auth_type} == AUTH_HIDDEN()) {
#		# Same as AUTH_COOKIE
#	} else {
#		$this->throw("Invalid auth_type Babs configuration directive value");
	}
	return $user_id ? $user_id : undef;
}

sub session_logout {
	my ($this) = @_;
	if ($this->session_is_logged_in && $this->{session_id}) {
		$this->{thraxx}->session_remove($this->{session_id});
	} else {
		return E_SESSION_NOREC();
	}
}

sub session_login {
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
