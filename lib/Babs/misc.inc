# $Id$

=head2 Miscellaneous Routines

This sub-module provides a group of mostly unrelated routines whose behaviors
are rather unique and thus contained within this I<miscellaneous> sub-module.

=over

=cut

use File::Copy ();
use strict;
use warnings;

=item $contents = $babs->slurp_file($filename);

This method returns as a single string the entire contents of the argued
C<$filename>.

=cut

sub slurp_file {
	my ($this, $file) = @_;
	local (*F, $/);
	unless (open F, "< $file") {
		$this->throw("Cannot open file; file: $file; mode: read-only");
	}
	my $data = <F>;
	close F;
	return $data;
}

use constant EMAIL_CHK_MX	=> 1;
use constant EMAIL_CHK_NONE	=> 2;

=item $valid = $babs->valid_email($email);

This method tries to determine if the given C<$email> address is valid or
not. It will perform different tests depending on the value of the Babs
configuration option C<email_check>.

=over

=item C<EMAIL_CHK_NONE>

This option provides only basic, common-sense parsing for the e-mail
address.  This type of checking is very fast.

=item C<EMAIL_CHK_MX>

This option provides mail exchange record lookups for the domain of
the argued e-mail address.  This may take awhile, as it has to perform
name lookups. This is the default value after a fresh Babs installation.

=back

=cut

sub valid_email {
	my ($this, $email) = @_;
	return FALSE() unless defined $email;
	return FALSE() unless $email =~ /^.+@(.+)$/;
	my $host = $1;
	if ($this->{email_check} == EMAIL_CHK_MX) {
		require Email::Valid;
		return Email::Valid->address(-address=>$email, -mxcheck=>1);
	}
	return TRUE();
}

=item $equal = $babs->hasheq(\%one, \%two);

This method determines whether the two argued references to hashes contain
the exact key/value pairs of the hashes to which they refer.

=cut

sub hasheq {
	my ($a, $b) = @_;
	my %dup = %$a;
	my ($k, $v, $val);
	while (($k, $v) = each %$b) {
		return FALSE() unless exists $dup{$k};
		$val = $dup{$k};
		return FALSE() unless $val xor defined $v;
		return FALSE() unless $val eq $v;

		# Remove the field if it exists
		delete $dup{$k};
	}
	# They're equal if we removed all the fields and none remain
	return %dup == ();
}

use constant PATH_ABS	=> 1;
use constant PATH_REL	=> 2;
use constant PATH_SYS	=> 3;

=item $url = $babs->build_url([$type,] $file[, %args]);

This method I<builds> URLs that will point to the recommended Babs
pages. The first optional argument is the type of URL that will be
built.

=over

=item C<PATH_ABS>

This specifies that an absolute URI should be built.  The resulting
URI should be able to access the document in any circumstances.

=item C<PATH_REL>

This specifies that a relative URL should be built.  The resulting
URL should be able to access relative to the location in which the
calling location exists.  This is the default.

=item C<PATH_SYS>

This specifies that an absolute path on the system be built.  The
resulting path will show the filesystem location of the file.

=back

The URL (or path) that will be returned shall point to C<$file>
in any way that it is specified (whether relative or absolute).

The optional C<%args> can be used to construct a query string for
the desired URL.  All name/value pairs in the hash become name/value
query string parameters, properly URI-escaped.

=cut

sub build_url {
	my ($this) = shift;
	# Default to PATH_REL if none is specified
	my $type = (grep {$_ == $_[0]} (PATH_ABS, PATH_REL, PATH_SYS)) ? shift : PATH_REL;
	my ($file, %args) = @_;
	my $url = $this->build_path($type) . $file;
	$url .= ".pl?" unless $file =~ /\./;
	my ($k, $v);
	while (($k, $v) = each %args) {
		$url .= $k . "=" . $this->escape_url($v) . "&amp;";
	}
	# Remove trailing '&amp;'
	$url =~ s/&amp;$//;
	return $url;
}

=item $path = $babs->build_path([$type]);

This method I<builds> a path to the location it is being called from.
C<$type> may be one of:

=over

=item C<PATH_ABS>

An absolute URI path will be provided.

=item C<PATH_REL>

A relative URL path will be provided.  This is the default.

=item C<PATH_SYS>

A filesystem path will be provided.

=back

=cut

sub build_path {
	my ($this, $type) = @_;
	$type = PATH_REL unless defined $type;
	if ($type == PATH_ABS) {
		return $this->{site_uri} . $this->{web_root} . $this->{path};
	} elsif ($type == PATH_REL) {
		return $this->{web_root} . $this->{path};
	} elsif ($type == PATH_SYS) {
		return $this->{sys_root} . $this->{sys_path};
	} else {
		$this->throw("Unrecognized path type; type: $type");
	}
}

=item C<< $exists = $babs->in_array($needle, \@hay); >>



=cut

sub in_array {
	my ($needle, $hay) = @_;

	foreach my $i (@$hay) {
		return TRUE() if $i == $needle;
	}

	return FALSE();
}

=item C<< $class = $babs->gen_class(); >>

=cut

sub gen_class {
	my ($this) = @_;
	return "BabsData" . (1 + ($this->{gen_class} = !$this->{gen_class}));
}

=item C<< $babs->file_move(); >>

=cut

sub file_move {
	my ($this, $src, $dst) = @_;

	unless (File::Copy::copy($src, $dst)) {
		$this->throw("Could not File::Copy::copy; src: $src; dst: $dst")
	}
	$this->file_remove($src);
}

=item C<< $babs->file_remove(); >>

=cut

sub file_remove {
	my ($this, $file) = @_;
	unless (unlink $file) {
		$this->throw("Could not unlink() file; file: $file");
	}
}

# Sends actual mail (lower level); use wrapper
# mail() for checking/error-handling instead
sub _mail {
	my ($this, %args) = @_;

	# Strip potential CLRFs from header
	$args{from}	=~ s/\r\n|[\r\n]//g;
	$args{subject}	=~ s/\r\n|[\r\n]//g;
	$args{to}	=~ s/\r\n|[\r\n]//g;

	
}

=item C<< $babs->mail(); >>

=cut

sub mail {
	my ($this, %args) = @_;
	unless ($this->_mail(%args)) {
		$this->throw("Could not mail()");
	}
}

=item C<< $url = $babs->get_url(); >>

=cut

sub get_url {
	my ($this) = @_;
	return $this->{cgi}->url(-absolute=>1);
}

=item C<< $babs->redirect($url); >>

=item

sub redirect {
	my ($this, $url) = @_;

	# Turn it into an absolute URI
	if ($url =~ /^[a-z]+:!/) {
		# Matches protocol; must be complete
	} elsif ($url =~ m!^/!) {
		# Absolute path
		$url = $this->build_path(PATH_ABS) . $url;
	} else {
		# Relative path
		my $path = $this->get_url;
		$path =~ s{[^/]+$}{}; # get cwd
		$url = $this->build_path(PATH_ABS) . $path . $url;
	}

	print $this->{cgi}->redirect($url);
	exit(0);
}

=item C<< $equal = $babs>arrayeq(\@a, \@b); >>

=cut

sub arrayeq {
	my ($a, $b) = @_;
	my $i;

	for ($i = 0; $i < @$a; $i++) {
		return FALSE() unless $a->[$i] eq $b->[$i];
	}

	# Make sure one array doesn't contain more items than the other
	return @$b == $i;
}

# Reads from post data. Also checks if the HTTP_REFERER
# is valid (i.e., originating from the host that created
# the previous page) if set to do so.
sub post {
	my ($this, $var) = @_;
	return undef if $this->{check_http_referer} && !$this->valid_referer;
	return undef unless $this->{cgi}->request_method eq "POST";
	return $this->{cgi}->param($var);
}

sub req {
	my ($this, $var) = @_;
	return $this->{cgi}->param($var);
}

sub valid_referer {
	my ($this) = @_;
	# Cache
	return $this->{valid_referer} if exists $this->{valid_referer};
	return $this->{valid_referer} = $this->{cgi}->referer =~ /^\Q$this->{site_uri}/;
}

sub help_id {
	my ($this, $id) = @_;
	my %map = (
		"overview break"	=> 101,
		"auto-urls"		=> 102,
		"allowed html"		=> 103,
		"html attributes"	=> 104,
		"clear rss"		=> 105,
		"clear cache"		=> 106,
		"password confirm"	=> 107,
	);

	return 0 unless exists $map{$id};
	return $map{$id};
}

sub result_limit {
	my ($this, $try) = @_;
	my $limit = $this->{default_result_limit};
	$limit = $try if $try && $try <= $this->{max_result_limit};
	return $limit;
}

sub page_limit {
	my ($this, $try) = @_;
	my $limit = $this->{default_page_limit};
	$limit = $try if $try && $try <= $this->{max_page_limit};
	return $limit;
}

return TRUE();
