#!/usr/bin/perl -W

use strict;
use Thraxx;
use WASP;
use DBH qw(:all);
use DBH::MySQL;
use OF::HTML;
use Babs;

sub _ {
	if ($_[0]) {
		print "\033[1;40;32mTest succeeded\033[1;0;0m\n\n";
	} else {
		print "\033[1;40;31mTest failed!\033[1;0;0m\n\n";
		die;
	}
} 

sub test {
	print "\033[1;40;34m", @_, ":\033[1;0;0m\n";
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

# comments.inc
comment_add {
comment_update {
comment_remove {
comment_get {
comment_get_ancestors {
comment_exists {

# crypt.inc
crypt {
rand_str {
gen_key {

# dbh.inc
dbh_selectcol {
dbh_quote {

# events.inc
event_fire {
event_has {
event_register {

# isr.inc
isr_check_field {

# misc.inc
sub slurp_file {
sub valid_email {
sub hasheq {
sub build_url {
sub build_path {
sub in_array {
sub gen_class {
sub file_move {
sub file_remove {
sub _mail {
$args{subject}	=~ s/\r\n|[\r\n]//g;
sub mail {
sub get_url {
sub redirect {
sub arrayeq {
sub post {
sub req {
sub valid_referer {
sub help_id {
sub result_limit {
sub page_limit {

# oof.inc
sub oof_login_form {
sub of_close_window {
sub of_popup {
sub of_js_pms {
sub of_nav_menu {
sub of_error {

# session.inc
sub session_auth {
sub session_require_login {
sub session_is_logged_in {
sub session_logout {
sub session_login {

# story.inc
sub story_add {
sub story_update {
sub story_remove {
sub story_set_recent {
sub story_get {
sub story_get {
sub story_get_comments {
sub story_exists {
sub story_search {

# str.inc
sub escape_http {
sub unescape_http {
sub escape_html {
sub escape_html {
sub unescape_html {
sub escape_slashes {
sub unescape_slashes {
sub str_parse {

# templates.inc
sub template_expand {
sub template_exists {
sub header {
sub footer {
sub _udf_init {
sub udf_validate {
# not multiply submit data, correcting errors only one
sub udf_update {
sub udf_update_db {
sub _user_init {
sub user_add {
sub user_update {
sub user_get_id {
sub user_get {
sub user_remove {
sub user_exists {
sub _xml_init {
sub _xml_cleanup {
sub xml_getfile {
sub xml_openfile {
sub xml_writefile {
sub xml_update {
sub xml_add {
sub xml_remove {
sub xml_throw {
sub xml_setup {

exit 0;
