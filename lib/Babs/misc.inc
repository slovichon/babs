# Babs miscellaneous routines
# $Id$

use strict;

use File::Copy ();

sub slurp_file
{
	my ($this, $file) = @_;
	local *F;
	local $/;
	unless (open F, "< $file")
	{
		$this->throw("Cannot open file; file: $file; mode: read-only");
	}
	my $data = <F>;
	close F;
	return $data;
}

use constant EMAIL_CHK_MX	=> 1;
#use constant EMAIL_CHK_HOST	=> 2;
use constant EMAIL_CHK_NONE	=> 3;

sub valid_email
{
	my ($this, $email) = @_;
	return FALSE() unless defined $email;
	return FALSE() unless $email =~ /^.+@(.+)$/;
	my $host = $1;
	if ($this->{email_check} == EMAIL_CHK_MX) {
		require Email::Valid;
		return Email::Valid->address(-address=>$email, -mxcheck=>1);
	}
#	return defined gethostbyname($host)	if $this->{email_check} == EMAIL_CHK_HOST;
	return TRUE();
}

sub hasheq
{
	my ($a, $b) = @_;
	my %dup = %$a;
	my ($k, $v);
	while (($k, $v) = each %$b)
	{
		if (exists $dup{$k} && $dup{$k} eq $b->{$k})
		{
			# Remove the field if it exists
			delete $dup{$k};
		} else {
			# Field doesn't exist; hashes aren't equal
			return FALSE();
		}
	}
	# They're equal if we removed all the fields and none remain
	return %dup == ();
}

use constant PATH_ABS	=> 1;
use constant PATH_REL	=> 2;
use constant PATH_SYS	=> 3;

sub build_url
{
	my ($this) = shift;
	# Default to PATH_REL if none is specified
	my $type = (grep {$_ == $_[0]} (PATH_ABS, PATH_REL, PATH_SYS)) ? shift : PATH_REL;
	my ($file, %args) = @_;
	my $url = $this->build_path($type) . $file . ".pl?";
	my ($k, $v);
	while (($k, $v) = each %args)
	{
		$url .= $k . "=" . $this->encode_http($v) . "&";
	}
	# Remove trailing '&'
	$url =~ s/&$//;
	return $url;
}

sub build_path
{
	my ($this, $type) = @_;
	$type = PATH_REL unless defined $type;
	   if ($type == PATH_ABS) { return $this->{site_uri} . $this->{web_root} . $this->{path}; }
	elsif ($type == PATH_REL) { return $this->{web_root} . $this->{path}; }
	elsif ($type == PATH_SYS) { return $this->{sys_root} . $this->{sys_path}; }
	else {
		$this->throw("Unrecognized path type; type: $type");
	}
}

sub in_array
{
	my ($needle, $hay) = @_;

	foreach my $i (@$hay)
	{
		return TRUE() if $i == $needle;
	}

	return FALSE();
}

sub gen_class
{
	my ($this) = @_;
	return "BabsData" . (1+($this->{gen_class} = !$this->{gen_class}));
}

sub file_move
{
	my ($this, $src, $dst) = @_;

	unless (File::Copy::copy($src, $dst))
	{
		$this->throw("Could not File::Copy::copy; src: $src; dst: $dst")
	}
	$this->file_remove($src);
}

sub file_remove
{
	my ($this, $file) = @_;
	unless (unlink $file)
	{
		$this->throw("Could not unlink() file; file: $file");
	}
}

# Sends actual mail (lower level); use wrapper
# mail() for checking/error-handling instead
sub _mail
{
	my ($this, %args) = @_;

	# Strip potential CLRFs from header
	$args{from}	=~ s/\r\n|[\r\n]//g;
	$args{subject}	=~ s/\r\n|[\r\n]//g;
}

sub mail
{
	my ($this, %args) = @_;
	unless ($this->_mail(%args))
	{
		$this->throw("Could not mail()");
	}
}

sub get_url
{
	my ($this) = @_;
	return $ENV{MOD_PERL} ? $this->{isapi}->uri :
		$this->{isapi}->url(-absolute=>1);
}

sub redirect
{
	my ($this, $url) = @_;

	# Turn it into an absolute URI
	if ($url =~ /^[a-z]+:!/)
	{
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

	print $this->{isapi}->redirect($url);
	exit(0);
}

sub arrayeq
{
	my ($a1, $a2) = @_;
	my $i;

	for ($i = 0; $i < @$a1; $i++)
	{
		return FALSE() unless $a1->[$i] eq $a2->[$i]
	}

	return @$a2 == $i;
}

# Reads from post data. Also checks if the HTTP_REFERER
# is valid (i.e., originating from the host that created
# the previous page) if set to do so.
sub post
{
	my ($this, $var) = @_;
	return undef if $this->{check_http_referer} && !$this->valid_referer;
	return undef unless $this->{isapi}->request_method eq "POST";
	return $this->{isapi}->param($var);
}

sub request
{
	my ($this, $var) = @_;
	return $this->{isapi}->param($var);
}

sub valid_referer
{
	my ($this) = @_;
	# Cache
	return $this->{valid_referer} if exists $this->{valid_referer};
	return $this->{valid_referer} = $this->{isapi}->referer =~ /^\Q$this->{site_uri}/;
}

sub help_id
{
	my ($this, $id) = @_;
	my %map = 
	(
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

sub result_limit
{
	my ($this, $try) = @_;
	my $limit = $this->{default_result_limit};
	$limit = $try if $try && $try <= $this->{max_result_limit};
	return $limit;
}

sub page_limit
{
	my ($this, $try) = @_;
	my $limit = $this->{default_page_limit};
	$limit = $try if $try && $try <= $this->{max_page_limit};
	return $limit;
}

return TRUE();
