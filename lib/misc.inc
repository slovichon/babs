# misc.inc - Miscellaneous Babs routines
# $Id$

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
use constant EMAIL_CHK_HOST	=> 2;
use constant EMAIL_CHK_NONE	=> 3;

sub valid_email
{
	my ($this, $email) = @_;
	return FALSE unless defined $email;
	return FALSE unless $email =~ /^.+@(.+)$/;
	my $host = $1;
	return checkmx($host)			if $this->{email_check} == EMAIL_CHK_MX;
	return defined gethostbyname($host)	if $this->{email_check} == EMAIL_CHK_HOST;
	return TRUE;
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
			return FALSE;
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
	my $type = grep {$_ == $_[0]} (PATH_ABS PATH_REL PATH_SYS) ? shift : PATH_REL;
	my ($file, %args) = @_;
	my $url = $this->build_path($type) . $file . ".pl?";
	my ($k, $v);
	while (($k, $v) = each %args)
	{
		$url .= $k . "=" . $this->http_encode($v) . "&";
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

sub encode_http
{
	my ($this, $data) = @_;
	$data =~ s/[^a-zA-Z0-9]/"%" . unpack("H", $1)/ge;
	return $data;
}

sub decode_http
{
	my ($this, $data) = @_;
	$data =~ s/%(\d\d)/pack("H", $1)/g;
	return $data;
}

sub encode_html
{
	my ($this, $data) = @_;
	my %tbl =
	(
		'"' => "quot",
		"'" => "apos",
		'<' => "lt",
		'>' => "gt",
		'&' => "amp",
	);
	my $c = qr![^a-zA-Z0-9_=()\[\]{}!@#\$%^\*:;.,\/\\\|+-]!;
	$data =~ s/$c/"$" . (exists $tbl{$&} ? $tbl{$&} : "#" . ord($&)) . ";"/ge;
	return $data;
}

sub decode_html
{
	my ($this, $data) = $_;
	my %tbl =
	(
		"quot"	=> '"',
		"apos"	=> "'",
		"lt"	=> '<',
		"gt"	=> '>',
		"amp"	=> '&',
	);
	$data =~ s/$#(\d+);/chr($1)/ge;
	$data =~ s/&(.*);/exists $tbl{$1} ? $tbl{$1} : $&/ge;
	return $data;
}

sub in_array
{
	my ($needle, $hay) = @_;

	foreach my $i (@$hay)
	{
		return TRUE if $i == $needle;
	}

	return FALSE;
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
	return $this->{isapi}->url(-absolute=>1);
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
		my $path = $this->{isapi}->url(-absolute=>1);
		$path =~ s{[^/]+$}{}; # get cwd
		$url = $this->build_path(PATH_ABS) . $path . $url;
	}

	print $this->{isapi}->redirect($url);
	exit(0);
}

# This sub is used by str_parse() to clean up HTML
# attributes. It has no other immediate usefulness
sub _str_attr_clean
{
	my ($this, $name, $val) = @_;

	# Attributes will have been matched by their delimiters ['"]
	# Note that this should be one of the other; not both
	$val =~ s/^&quot;(.*)&quot;$|/$1/; #xor
	$val =~ s/^'(.*)'$/$1/g;

	# Attributes should have been subjected to htmlEntities()
#	$val = $this->decode_html($val);

	# Strip dangerous "expression()"s in CSS
	my @mat;
	if ($name eq "style" && $this->{story_strip_expr} &&
		(@mat = m!([a-zA-Z0-9-]+)\s*:\s*expression\(!ig))
	{
		$val = $this->str_remove_css($val, \@mat);
	}

	return $val;
}

sub str_remove_css
{
}

sub str_parse
{
}


return 1;
