# Babs output-formatting routines
# $Id$

use POSIX ();
use strict;

# Should this be a template?
sub of_login_form {
	my ($this) = @_;
	my $of = $this->{of};

	# Failed log-in attempt
	my $redirect = $this->post("babs_redirect");

	# If there wasn't a failed login, then it was the referring page
	$redirect = $this->{cgi}->referer unless $redirect;

	# Only redirect if it's not the login page
	$redirect = $redirect && $redirect eq $this->build_url("login") ?
		$this->{cgi}->escapeHTML($redirect) : "";

	my $of_join = $this->{allow_join} ?
		$of->link("Not a member?", $this->build_url("admin/user-add")) .
		$of->br : "";

	return	$of->form(
			{action => $this->build_url("login-post")},
			$of->table(
				{class => "babsTable"},
				$of->table_row({class=>"babsHeader", colspan=>2, value=>"Log In"}),
				$of->table_row({class=>"babsDesc", value=>"Username:"},
					{class=>"babsData1", value=>$of->input(type=>"text", name=>"babs_username")}),
				$of->table_row({class=>"babsDesc", value=>"Password:"},
					{class=>"babsData2", value=>$of->input(type=>"password", name=>"babs_password")}),
				$of->table_row({class=>"babsFooter", colspan=>2, value=>
					$of->input(type=>"hidden", name=>"babs_redirect", value=>$redirect) .
					$of->input(type=>"submit", name=>"babs_submitted", class=>"babsButton", value=>"Log In") .
					$of->input(type=>"reset", class=>"babsButton", value=>"Clear")}),
				$of->table_row({class=>"babsFooter", colspan=>2, value=>
					$of_join .
					$of->link("Forgot your password?", $this->build_url("lost-pass")) .
					$of->br .
					$of->link("Forgot your username?", $this->build_url("lost-user"))})
			)
		);
}

sub of_close_window {
	my ($this, $title) = @_;

	my $data = $this->encode_slashes($this->{of}->link(value=>$title,
			href=>"#", onclick=>"window.close()"));

	return	qq{<script type="text/javascript">} .
		"document.write('$data')" .
		"</script>";
}

sub of_popup {
	my ($this, $value, $href) = @_;
	$href = $this->encode_slashes($href);
	my $name = "babs" . $this->random_str(8, RAND_VIS_ALNUM());
	return $this->{of}->link(value=>$value, href=>$href,
		onclick=>"window.open('$href', '$name', 'width=500,height=300'); return false;");
}

# Prevent multiple submissions
sub of_js_pms {
	return qq<
			if (this.value == 'Processing...')
			{
				window.alert('Please allow sufficient time for processing.');
				return false;
			} else {
				this.value = 'Processing...';
				return true;
			}
	>;
}

sub of_nav_menu {
	my ($this, %prefs) = @_;
	my $offset	= $prefs{offset} && $prefs{offset} =~ /^\d+$/ ? $prefs{offset} : 0;
	my $total	= $prefs{total}  && $prefs{total}  =~ /^\d+$/ ? $prefs{total}  : 0;
	my %url_prefs	= %{ $prefs{url} };
	my $action	= $prefs{action};
	my $limit	= $this->result_limit;
	my $plimit	= $this->page_limit;
	my $output	= "";
	my $of		= $this->{of};
	$offset		= 0 if $offset >= $total;

	if ($offset > $limit) {
		$output .=
			$of->link("First",	$this->build_url($action, %url_prefs, offset=>0)) . " | " .
			$of->link("Previous",	$this->build_url($action, %url_prefs, offset=>$offset-$limit)) . " | ";
	} else {
		$output .= "First | Previous ";
	}

	my $page	= POSIX::floor($offset / $limit);
	my $last_page	= POSIX::ceil($total / $limit);
	my $start	= 0;

	if ($page < $plimit/2) {
		# The "left boundary" exists when we can't evenly
		# distribute "previous page" links.
		$start = 0;
	} elsif ($page > $last_page - ceil($plimit/2)) {
		# The "right boundary" exists when we can't
		# evenly distribute "next page" links.
		$start = $last_page - $plimit;
	} else {
		$start = $page - floor($plimit/2);
	}

	for (my $i = $start; $i < $start + $plimit && $i < $last_page; $i++) {
		if ($i == $page) {
			$output .= $of->strong($i+1) . " | ";
		} else {
			$output .= $of->link($i+1, $this->build_url($action, %url_prefs, offset=>$i*$limit)) . " | ";
		}
	}

	if ($offset + $limit < $total) {
		$output .=	$of->link("Next", $this->build_url($action, %url_prefs, offset=>$offset+$limit)) . " | " .
				$of->link("Last", $this->build_url($action, %url_prefs, offset=>$total-$limit));
	} else {
		$output .= "Next | Last";
	}

	return $output;
}

return TRUE();
