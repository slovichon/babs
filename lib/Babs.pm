# Babs core routines
# $Id$
package Babs;

use Timestamp;
use OF;
use DBH qw(:all);
use Exporter;
use Thraxx;
use AutoConstantGroup;

BEGIN {
	if ($ENV{MOD_PERL})
	{
		require Apache;
		require Apache::Request;
	} else {
		require CGI;
	}
}

use strict;
use constant TRUE => 1;
use constant FALSE => 0;

# Error constants
use constant E_NONE => 0;

our $VERSION = 0.1;

sub new
{
	my $class = shift;
	my %prefs = (
		wasp  => WASP->new(),
		isapi => $ENV{MOD_PERL} ? Apache::Request->new(shift) : CGI->new(),
	);

	# Fill up %prefs
	my ($path) = ($INC{'Babs.pm'} =~ m!(.*/)!);
	eval slurp_file($prefs{wasp}, "$path/babs-config.inc");

	# Initialize DBH
	{
		my %args = (wasp => $prefs{wasp});
		my %map = (
			host		=> "dbh_host",
			username	=> "dbh_username",
			password	=> "dbh_password",
			database	=> "dbh_database",
		);
		# These settings are optional; only pass them
		# to DBH::new() if they are specified.
		my ($k, $v);
		while (($k, $v) = each %map)
		{
			$args{$k} = $prefs{$v} if exists $prefs{$v};
		}
		$prefs{dbh} = &{$prefs{dbh_type}}(
				*{$prefs{dbh_type}}{PACKAGE}, %args);
	}

	# Initialize OF
	{
		# Load OF prefs
		my %of_prefs = ();
		$prefs{of} = &{$prefs{of_type}}(*{$prefs{of_type}}{PACKAGE},
			$prefs{wasp}, \%of_prefs);
	}

	# Initialize Thraxx
	{
		$prefs{thraxx} = Thraxx->new(wasp=>$prefs{wasp},
					dbh=>$prefs{dbh});
	}

	return construct($class, %prefs);
}

sub construct
{
	my ($class, %prefs) = @_;

	# Error-check our environment
	die("No WASP object specified")			unless $prefs{wasp};
	$prefs{wasp}->throw("No ISAPI specified")	unless $prefs{isapi};
	$prefs{wasp}->throw("No DBH specified")		unless $prefs{dbh};
	$prefs{wasp}->throw("No OF specified")		unless $prefs{of};

	# Set other properties
	$prefs{error_const_group} = AutoConstantGroup->new;
	$prefs{session_id} = undef;

	unless (tied %prefs)
	{
		# Strict-preference setting
		tie %prefs, 'Babs::Prefs', %prefs unless tied %prefs;

		# Fill up %prefs
		my ($path) = ($INC{'Babs.pm'} =~ m!(.*/)!);
		eval slurp_file($prefs{wasp}, "$path/babs-config.inc");
	}

	my $this = bless \%prefs, ref($class) || $class;

	# Propagate construction
	$this->_comment_init();
	$this->_stories_init();
	$this->_udf_init();
	$this->_users_init();
	$this->_xml_init();

	# Property initialization
	$this->{gen_class} = 0;

	return $this;
}

sub throw
{
	my ($this) = shift;
	my $msg = "Babs Error: " . join '', @_;

	if ($this->{mail_errors})
	{
		$this->_mail(to=>$this->{admin_email}, from=>$this->{admin_email},
				subject=>"$this->{site_name} Babs Error Report", body=>$msg);
	}

	# Try to display prettily
	# $this->template_get();
	# $this->template_get();

	$this->{wasp}->throw($msg);
}

require "Babs/comments.inc";
require "Babs/crypt.inc";
require "Babs/event.inc";
require "Babs/isr.inc";
require "Babs/misc.inc";
require "Babs/of.inc";
require "Babs/sessions.inc";
require "Babs/stories.inc";
require "Babs/str.inc";
require "Babs/templates.inc";
require "Babs/udf.inc";
require "Babs/users.inc";
require "Babs/xml.inc";

sub DESTROY
{
	my ($this) = @_;

	# Propagate destruction
	$this->_xml_cleanup();
}

package Babs::Prefs;

sub TIEHASH
{
	my ($class, %prefs) = @_;
	return bless \%prefs, $class;
}

sub EXISTS
{
	my ($this, $k) = @_;
	return exists $this->{$k};
}

sub FETCH
{
	my ($this, $k) = @_;
	unless (exists $this->{$k})
	{
		$this->{wasp}->throw("Requested Babs directive not set; directive: $k");
	}
	return $this->{$k};
}

sub STORE
{
	my ($this, $k, $v) = @_;
	# Perhaps we should do some checking here...
	$this->{$k} = $v;
}

return 1;
