package Babs;
# $Id$

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

sub create
{
	return __PACKAGE__->new(
		wasp	=> WASP->new(),
		isapi	=> $ENV{MOD_PERL} ? Apache::Request->new(shift) : CGI->new(),
	);
}

sub new
{
	my ($class, %prefs) = @_;

	die("No WASP object specified")	unless $prefs{wasp};
	die("No ISAPI specified")	unless $prefs{isapi};

	# Strict-preference setting
	tie %prefs, 'Babs::Prefs', %prefs;

	# This should fill up %prefs
	require "babs-config.inc";

	my $this = bless \%prefs, ref($class) || $class;
	
	# Initialize DBH
	{
		my %args = (wasp=>$this->{wasp});
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
			$args{$k} = $this->{$v} if exists $this->{$v};
		}
		$this->{dbh} = &{$this->{dbh_type}}(*{$this->{dbh_type}}{PACKAGE},
			%args);
	}

	# Initialize OF
	{
		# Load OF prefs
		my %of_prefs = ();
		$this->{of} = &{$this->{of_type}}(*{$this->{of_type}}{PACKAGE},
			$this->{wasp}, \%of_prefs);
	}

	# Propagate construction
	$this->_xml_init();
	$this->_udf_init();

	return $this;
}

sub throw
{
	my ($this) = shift;
	my $msg = join '', @_;
	$this->{wasp}->throw("Babs error: $msg");
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
