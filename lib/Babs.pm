# Babs core routines
# $Id$

=head1 NAME

Babs - Babs News System API

=head1 SYNOPSIS

 use Babs;

 # Construction methods
 my $babs = Babs->new;
 my $babs = Babs->construct(thraxx=>$thraxx, wasp=>$wasp,
			    oof=>$oof, cgi=>$cgi, req=>$req);

 # Core methods
 $babs->throw($errmsg);

 # Comment-related methods
 $babs->comment_add($comment);
 $babs->comment_update($comment);
 $babs->comment_remove($story_id, $comment_id);
 $babs->comment_get($story_id, $comment_id);
 $babs->comment_get_ancestors($needle, $hay);
 $babs->comment_exists($story_id, comment_id);

 # Cryptography-related methods
 $babs->crypt;
 $babs->rand_str;
 $babs->gen_key;

 # Event-related methods
 $babs->event_fire;
 $babs->event_has;
 $babs->event_register;

 # Internal structure representation-related methods
 $babs->isr_check_field;

 # Miscellaneous methods
 $babs->slurp_file;
 $babs->valid_email;
 $babs->hasheq;
 $babs->build_url;
 $babs->build_path;
 $babs->in_array;
 $babs->gen_class;
 $babs->file_move;
 $babs->file_remove;
 $babs->mail;
 $babs->get_url;
 $babs->redirect;
 $babs->arrayeq;
 $babs->post;
 $babs->req;
 $babs->valid_referer;
 $babs->help_id;
 $babs->result_limit;
 $babs->page_limit;

 # Object-output-formatting-related methods
 $babs->oof_login_form;
 $babs->oof_close_window;
 $babs->oof_popup;
 $babs->oof_js_pms;
 $babs->oof_nav_menu;
 $babs->oof_error;

 # Session-related methods
 $babs->session_auth;
 $babs->session_require_login;
 $babs->session_is_logged_in;
 $babs->session_logout;
 $babs->session_login;

 # Story-related methods
 $babs->story_add;
 $babs->story_update;
 $babs->story_remove;
 $babs->story_set_recent;
 $babs->story_get;
 $babs->story_get_comments;
 $babs->story_exists;
 $babs->story_search;

 # String-related methods
 $babs->escape_html;
 $babs->encode_slashes;
 $babs->decode_slashes;
 $babs->str_parse;

 # Template-related methods
 $babs->template_expand;
 $babs->template_exists;
 $babs->header;
 $babs->footer;

 # User-defined fields-related methods
 $babs->udf_validate;
 $babs->udf_update;
 $babs->udf_update_db;

 # User-related methods
 $babs->user_add;
 $babs->user_update;
 $babs->user_get_id;
 $babs->user_get;
 $babs->user_remove;
 $babs->user_exists;

 # XML-related methods
 $babs->xml_getfile;
 $babs->xml_openfile;
 $babs->xml_writefile;
 $babs->xml_update;
 $babs->xml_add;
 $babs->xml_remove;
 $babs->xml_throw;
 $babs->xml_setup;

=head1 DESCRIPTION

Babs is a Web-accessible news management system written in Perl.  Babs
itself is just a large API for making news management easy, but is
distributed with a series of "example" or recommended pages that use
the API for a working application.

Babs depends on the WASP and Thraxx modules which provide Web-application
and session management framework and structure, respectively.  There are
a number of sub-modules that Babs is broken down into, for example, XML-,
user-, and story-related routines.

=head1 MODULES

Consult each module-specific documentation for information pertaining
the API functions provided by that module.  The following modules are
available.

Each module can have an optional method named C<_I<mod>_init> and
C<_I<mod>_cleanup> which respectively can perform sub-module
initialization and cleanup operations, such as constant registeration.

=over

=item comments

=item crypt

=item event

=item isr

=item misc

=item oof

=item sessions

=item stories

=item str

=item templates

=item udf

=item users

=item xml

=back

=head1 ROUTINES

=head2 Core Routines

=over

=cut

package Babs;

use WASP;
use Timestamp;
use OOF;
use Exporter;
use Thraxx;
use AutoConstantGroup;
use CGI;
use DBI;

BEGIN {
	require Apache if $ENV{MOD_PERL};
}

use strict;
use warnings;

use constant TRUE => 1;
use constant FALSE => 0;

# Error constants
use constant E_NONE => 0;

our $VERSION = 0.1;

=item my $babs = Babs->new;

Create a new Babs object. Babs requires a fair amount of configuration
(due to database communication, for example), and using this approach
will make Babs search for its various configuration settings.  No
arguments are required, because the configuration information will be
(attempted to be) gathered.

=cut

sub new {
	my $class = shift;
	my %prefs = (
		wasp => WASP->new(),
		cgi  => CGI->new(),
	);

	$prefs{req} = Apache->request(shift) if $ENV{MOD_PERL};

	# Fill up %prefs
	my ($path) = ($INC{'Babs.pm'} =~ m!(.*/)!);
	eval slurp_file($prefs{wasp}, "$path/babs-config.inc");

	# Initialize DBI
	$prefs{dbh} = DBI->connect("dbi:$prefs{dbh_type}:$prefs{dbh_database}",
			$prefs{dbh_username}, $prefs{dbh_password});

	# Initialize OOF
	{
		# Load OOF prefs
		my %oof_prefs = ();
		$prefs{oof} = OOF->new(wasp=>$prefs{wasp}, filter=>$prefs{oof_type},
				prefs=>\%oof_prefs);
	}

	# Initialize Thraxx
	$prefs{thraxx} = Thraxx->new(wasp=>$prefs{wasp}, dbh=>$prefs{dbh});

	return construct($class, %prefs);
}

=item my $babs = Babs->construct(%prefs);

This I<lower-level> constructor creates a new Babs object but depends
on all configuration information to be previously set up and all such
parameters passed as arguments. The named arguments are:

=over

=item cgi

A CGI instance.  See L<CGI>.

=item wasp

A WASP instance.  See L<WASP>.

=item dbh

A DBI instance (database handle).  See L<DBI>.

=item oof

An OOF instance.  See L<OOF>.

=item req (optional)

A request instance. This is used for speed advantages when
Babs is running under C<mod_perl>.  The example parameter being
referred to here is that which is provided in C<@ARGV> in a page
running under C<mod_perl>. See L<Apache>.

=back

=cut

sub construct {
	my ($class, %prefs) = @_;

	# Error-check our environment
	die("No WASP object specified")		unless $prefs{wasp};
	$prefs{wasp}->throw("No CGI specified")	unless $prefs{cgi};
	$prefs{wasp}->throw("No DBH specified")	unless $prefs{dbh};
	$prefs{wasp}->throw("No OOF specified")	unless $prefs{oof};

	# Set other properties
	$prefs{error_const_group} = AutoConstantGroup->new;
	$prefs{session_id} = undef;

	unless (tied %prefs) {
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

=item $babs->throw($errmsg);

This method provides generic error-handling behavior. All arguments
are concatenated together to form a string being the associated error
message.

The exact behavior of this method can be affected by changing WASP
settings, as this method depends on C<WASP::throw()> (see L<WASP>).
Note that Babs has preferences for controlling the default behavior
for error-handling.

XXX: make sure babs provides these options.

=cut

sub throw {
	my ($this) = shift;
	my $msg = "Babs Error: " . join '', @_;

	if ($this->{mail_errors}) {
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
require "Babs/oof.inc";
require "Babs/sessions.inc";
require "Babs/stories.inc";
require "Babs/str.inc";
require "Babs/templates.inc";
require "Babs/udf.inc";
require "Babs/users.inc";
require "Babs/xml.inc";

sub DESTROY {
	my ($this) = @_;

	# Propagate destruction
	$this->_xml_cleanup();
}

package Babs::Prefs;

sub TIEHASH {
	my ($class, %prefs) = @_;
	return bless \%prefs, $class;
}

sub EXISTS {
	my ($this, $k) = @_;
	return exists $this->{$k};
}

sub FETCH {
	my ($this, $k) = @_;
	unless (exists $this->{$k}) {
		$this->{wasp}->throw("Requested Babs directive not set; directive: $k");
	}
	return $this->{$k};
}

sub STORE {
	my ($this, $k, $v) = @_;
	# Perhaps we should do some checking here...
	$this->{$k} = $v;
}

=back

=head1 AUTHOR

Jared Yanovich E<lt>jaredy@closeedge.netE<gt>

=cut

1;
