# Babs core routines
# $Id$
package Babs;

use Timestamp;
use OF;
use DBH qw(:all);
use Exporter;

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
	delete $INC{"babs-config.inc"} if exists $INC{"babs-config.inc"};
	require "babs-config.inc";

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

	return construct($class, %prefs);
}

sub construct
{
	my ($class, %prefs) = @_;

	# Error-check our environment
	die("No WASP object specified")	unless $prefs{wasp};
	$prefs{wasp}->throw("No ISAPI specified")	unless $prefs{isapi};
	$prefs{wasp}->throw("No DBH specified")		unless $prefs{dbh};
	$prefs{wasp}->throw("No OF specified")		unless $prefs{of};

	# Strict-preference setting
	tie %prefs, 'Babs::Prefs', %prefs;

	# This should fill up %prefs
	delete $INC{"babs-config.inc"} if exists $INC{"babs-config.inc"};
	require "babs-config.inc";

	my $this = bless \%prefs, ref($class) || $class;

	# Propagate construction
	$this->_xml_init();
	$this->_udf_init();

	# Property definition
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

require "comments.inc";
require "crypt.inc";
require "event.inc";
require "isr.inc";
require "sessions.inc";
require "stories.inc";
require "templates.inc";
require "udf.inc";
require "users.inc";
require "xml.inc";

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
