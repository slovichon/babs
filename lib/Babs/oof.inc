# Babs output-formatting routines
# $Id$

use POSIX ();
use strict;
use warnings;

# Should this be a template?
sub oof_login_form {
	my ($this) = @_;
	my $oof = $this->{oof};

	# Failed log-in attempt
	my $redirect = $this->post("babs_redirect");

	# If there wasn't a failed login, then it was the referring page
	$redirect = $this->{cgi}->referer unless $redirect;

	# Only redirect if it's not the login page
	$redirect = $redirect && $redirect eq $this->build_url("login") ?
		$this->{cgi}->escapeHTML($redirect) : "";

	my $oof_join = $this->{allow_join} ?
		$oof->link("Not a member?", $this->build_url("admin/user-add")) .
		$oof->br : "";

	return	$oof->form(
			{action => $this->build_url("login-post")},
			$oof->table(
				{class => "babsTable"},
				$oof->table_row({class=>"babsHeader", colspan=>2, value=>"Log In"}),
				$oof->table_row({class=>"babsDesc", value=>"Username:"},
					{class=>"babsData1", value=>$oof->input(type=>"text", name=>"babs_username")}),
				$oof->table_row({class=>"babsDesc", value=>"Password:"},
					{class=>"babsData2", value=>$oof->input(type=>"password", name=>"babs_password")}),
				$oof->table_row({class=>"babsFooter", colspan=>2, value=>
					$oof->input(type=>"hidden", name=>"babs_redirect", value=>$redirect) .
					$oof->input(type=>"submit", name=>"babs_submitted", class=>"babsButton", value=>"Log In") .
					$oof->input(type=>"reset", class=>"babsButton", value=>"Clear")}),
				$oof->table_row({class=>"babsFooter", colspan=>2, value=>
					$oof_join .
					$oof->link("Forgot your password?", $this->build_url("lost-pass")) .
					$oof->br .
					$oof->link("Forgot your username?", $this->build_url("lost-user"))})
			)
		);
}

sub oof_close_window {
	my ($this, $title) = @_;

	my $data = $this->encode_slashes($this->{oof}->link(value=>$title,
			href=>"#", onclick=>"window.close()"));

	return	qq{<script type="text/javascript">} .
		"document.write('$data')" .
		"</script>";
}

sub oof_popup {
	my ($this, $value, $href) = @_;
	$href = $this->encode_slashes($href);
	my $name = "babs" . $this->random_str(8, RAND_VIS_ALNUM());
	return $this->{oof}->link(value=>$value, href=>$href,
		onclick=>"window.open('$href', '$name', 'width=500,height=300'); return false;");
}

# Prevent multiple submissions
sub oof_js_pms {
	return <<EOJS;
			if (this.value == 'Processing...') {
				window.alert('Please allow sufficient time for processing.');
				return false;
			} else {
				this.value = 'Processing...';
				return true;
			}
EOJS
}

sub oof_nav_menu {
	my ($this, %prefs) = @_;
	my $offset = $prefs{offset} && $prefs{offset} =~ /^\d+$/ ? $prefs{offset} : 0;
	my $total  = $prefs{total}  && $prefs{total}  =~ /^\d+$/ ? $prefs{total}  : 0;
	my %url_prefs = %{ $prefs{url} };
	my $action = $prefs{action};
	my $limit = $this->result_limit;
	my $plimit = $this->page_limit;
	my $output = "";
	my $oof = $this->{oof};
	$offset = 0 if $offset >= $total;

	if ($offset > $limit) {
		$output .=
			$oof->link("First",	$this->build_url($action, %url_prefs, offset=>0)) . " | " .
			$oof->link("Previous",	$this->build_url($action, %url_prefs, offset=>$offset-$limit)) . " | ";
	} else {
		$output .= "First | Previous ";
	}

	my $page = POSIX::floor($offset / $limit);
	my $last_page = POSIX::ceil($total / $limit);
	my $start = 0;

	if ($page < $plimit/2) {
		# The "left boundary" exists when we can't evenly distribute
		# "previous page" links.
		$start = 0;
	} elsif ($page > $last_page - ceil($plimit/2)) {
		# The "right boundary" exists when we can't evenly distribute
		# "next page" links.
		$start = $last_page - $plimit;
	} else {
		$start = $page - floor($plimit/2);
	}

	for (my $i = $start; $i < $start + $plimit && $i < $last_page; $i++) {
		if ($i == $page) {
			$output .= $oof->strong($i+1) . " | ";
		} else {
			$output .= $oof->link($i+1, $this->build_url($action, %url_prefs, offset=>$i*$limit)) . " | ";
		}
	}

	if ($offset + $limit < $total) {
		$output .=	$oof->link("Next", $this->build_url($action, %url_prefs, offset=>$offset+$limit)) . " | " .
				$oof->link("Last", $this->build_url($action, %url_prefs, offset=>$total-$limit));
	} else {
		$output .= "Next | Last";
	}

	return $output;
}

sub oof_error {
	my ($this, $msg) = @_;

	print 	$this->header("Error"),
		$this->{oof}->p($msg),
		$this->footer();

	exit 0;
}

return TRUE();
