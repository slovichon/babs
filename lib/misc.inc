# misc.inc - Miscellaneous Babs routines
# $Id$

sub slurp_file
{
	my ($this, $file) = @_;
	local *F;
	local $/;
	unless (open F, "< $file")
	{
		$this->{wasp}->throw("Cannot open file; file: $file; mode: read-only");
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
	my $type = grep {$_ == $_[0]} (PATH_ABS PATH_REL PATH_ABS) ? shift : PATH_REL;
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

sub http_encode
{
	my ($this, $data) = @_;
	return $this->{}
}

return 1;
