package Babs;
# $Id$

use Timestamp;
use OF;
use DBH qw(:all);
use Exporter;

use strict;
use constant TRUE => 1;
use constant FALSE => 0;

# Error constants
use constant E_NONE => 0;

our $VERSION = 0.1;

sub new
{
	my ($class, %prefs) = @_;

	my $this = bless {
		skip_init	=> exists $prefs{skip_init} ? $prefs{skip_init} : FALSE,
		wasp		=> $prefs{wasp},
		of		=> $prefs{of},
	}, ref($class) || $class;

	require "babs-config.inc";

	$this->udf_update() unless $this->{skip_init};

	# Propagate construction
	$this->_xml_init();

	return $this;
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

return 1;
