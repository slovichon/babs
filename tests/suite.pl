#!/usr/bin/perl -W

use Babs;
use strict;
use warnings;

sub _ {
	if ($_[0]) {
		print "\033[1;32mTest succeeded\033[1;0;0m\n\n";
	} else {
		print "\033[1;31mTest failed!\033[1;0;0m\n\n";
		die;
	}
} 

sub test {
	print "\033[1;34m", @_, ":\033[1;0;0m\n";
}

my $w = WASP->new;
$w->die(1);
my $td = DBH::MySQL->new(host=>"12.226.98.118", username=>"thraxx",
		password=>"lNBDOD92Pec", database=>"thraxx",
		wasp=>$w);
my $d = DBH::MySQL->new(host=>"12.226.98.118", username=>"babs",
		password=>"", database=>"babs",
		wasp=>$w);
my $t = Thraxx->construct(wasp=>$w, dbh=>$td, skip_init=>1);

# We normally invoke Babs->new to let it consult the configuration
# and set up these objects by itself, but since this test script
# needs to be run in most any circumstance, we must make it so
# these settings can be run from anywhere unmodified.
my $b = Babs->construct(thraxx=>$t, wasp=>$t, dbh=>undef, skip_init=>1,
			isapi=>CGI->new, of=>OF::HTML->new($w), dbh=>1);

# Some of the operations tested must be done in a specific order,
# such as adding users before posting stories from them. As this
# model can leave the system in an unknown state should a procedure
# fail, cleanups will have to be performed.

# users.inc
my %user = (
	
);
test "user_add";
_ $b->user_add();

$b->user_update;
$b->user_get_id;
$b->user_get;
$b->user_exists;

# story.inc
$b->story_add;
$b->story_update;
$b->story_remove;
$b->story_set_recent;
$b->story_get;
$b->story_get;
$b->story_get_comments;
$b->story_exists;
$b->story_search;

# comments.inc
$b->comment_add();
$b->comment_update;
$b->comment_remove;
$b->comment_get;
$b->comment_get_ancestors;
$b->comment_exists;

# crypt.inc
$b->crypt("");
# These are impossible to test
$b->rand_str;
$b->gen_key;

# dbh.inc
$b->dbh_selectcol;
$b->dbh_quote;

# events.inc
$b->event_fire;
$b->event_has;
$b->event_register;

# isr.inc
$b->isr_check_field;

# misc.inc
$b->slurp_file;
$b->valid_email;
$b->hasheq;
$b->build_url;
$b->build_path;
$b->in_array;
$b->gen_class;
$b->file_move;
$b->file_remove;
$b->mail;
$b->get_url;
$b->redirect;
$b->arrayeq;
$b->post;
$b->req;
$b->valid_referer;
$b->help_id;
$b->result_limit;
$b->page_limit;

# oof.inc
$b->oof_login_form;
$b->oof_close_window;
$b->oof_popup;
$b->oof_js_pms;
$b->oof_nav_menu;
$b->oof_error;

# session.inc
$b->session_auth;
$b->session_require_login;
$b->session_is_logged_in;
$b->session_logout;
$b->session_login;

# str.inc
$b->escape_http;
$b->unescape_http;
$b->escape_html;
$b->escape_html;
$b->unescape_html;
_ $b->escape_slashes(q!a"b" c\\" d' \\\\ef'!) eq qq!a\\"b\\" c\\\\\\"d\\' \\\\\\\\ef\\'!;
_ $b->unescape_slashes("") eq "";
$b->str_parse;

# templates.inc
$b->template_expand;
$b->template_exists;
$b->header;
$b->footer;

# udf.inc
$b->udf_validate;
$b->udf_update;
$b->udf_update_db;

# xml.inc
$b->xml_getfile;
$b->xml_openfile;
$b->xml_writefile;
$b->xml_update;
$b->xml_add;
$b->xml_remove;
$b->xml_throw;
$b->xml_setup;

$b->user_remove($user{id});

exit 0;
