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

return 1;
