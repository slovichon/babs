package Babs;

$VERSION = "0.1";
@ISA = qw(Exporter);
@EXPORT = qw(	BABS_STR_NONE BABS_STR_HTML BABS_STR_URL
		BABS_STR_ALL

		BABS_PATH_ABS BABS_PATH_REL BABS_PATH_SYS

		BABS_E_NONE);

# WASP libraries
use WASP;
use WASP::Timestamp;

# Other stuff
use Exporter;

use strict;
use constant TRUE => 1;
use constant FALSE => 0;

require "babs-config.inc";
require "comments.inc";
require "crypt.inc";
require "isr.inc";
require "sessions.inc";
require "stories.inc";
require "templates.inc";
require "userfields.inc";
require "users.inc";
require "xml.inc";

sub new
{
	my ($class,%prefs) = @_;

	my $this = bless {
			skip_init => exists $prefs{skip_init} ? $prefs{skip_init} : FALSE,
			}, ref($class) || $class;


	$this->userfields_update() unless $this->skip_init;

	return bless \%prefs, ref($class) || $class;
}

# properties
sub skip_init { my $this = shift; return $this->{skip_init}; }
sub of




















return 1;
