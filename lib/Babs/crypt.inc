# Babs cryptographic routines
# $Id$

=head2 Cryptographic Routines

This sub-module provides cryptographic-related routines for the
secure transfer and storage of important information, such as
relevant user passwords.

=over

=cut

use strict;
use warnings;

use constant RAND_VIS_ALNUM	=> 1;
use constant RAND_VIS_NQ	=> 2;

use constant CRYPT_DES		=> 1;
use constant CRYPT_EXT_DES	=> 2;
use constant CRYPT_MD5		=> 3;
use constant CRYPT_BLOWFISH	=> 4;

=item C<< $encrypted = $babs->crypt($data); >>

This method will encrypt the argued data according to the configured
cryptographic settings.

=cut

sub crypt {
	my ($this, $data) = @_;
	return crypt($data, $this->{crypt_key});
}

=item C<< $str = $babs->rand_str($size, $type); >>

This method will generate a string of length C<$size> of random characters
from the character domain specified by C<$type>. The following values for
C<$type> are understood.

=over

=item C<RAND_VIS_NQ>

This type specifies that any visual character that is not a quote or
backslash (none of ' " or \) will be made available to the characteer
selection domain.

=item C<RAND_VIS_ALNUM>

This type specifies that any alphanumeric (numbers and letters only)
will be made available to the character selection domain.

=back

=cut

sub rand_str {
	my ($this, $size, $type) = @_;
	$type = RAND_VIS_NQ unless defined $type;
	my ($min, $max, @excluded);
	if ($type == RAND_VIS_NQ) {
		$min = 33;  # '!'
		$max = 126; # '~'
		@excluded = (34, 39, 92); # ' " \
	} elsif ($type == RAND_VIS_ALNUM) {
		# Just alphanumerics
		$min = 48;  # '0'
		$max = 122; # 'z'
		@excluded = (58 .. 64, 91 .. 96);
	} else {
		$this->throw("Invalid type; type: $type");
	}

	my $str = "";
	my $ch = "";

	for (1 .. $size) {
		do {
			$ch = $min + int rand($max-$min+1);
		} while (in_array($ch, \@excluded));
		$str .= chr($ch);
	}

	return $str;
}

=item C<< $key = $babs->gen_key($ktype); >>

This method generates a cryptographic key for future encryption
operations.  The following key types are recognized.

=over

=item C<CRYPT_DES>

This specifies that a key suitable for the Data Encryption Standard
algorithm should be generated.  This is the most primitive and most
likely widely available, but coincidentally the weakest as well.

=item C<CRYPT_EXTDES>

This specifies that a key suitable for an extended version of the
Data Encryption Standard algorithm should be generated.  This may only
be available on certain systems.  Consult your system's L<crypt(3)>
manual.

=item C<CRYPT_MD5>

This specifies that a key suitable for the Message-Digest version five
algorithm should be generated.  This may only be available on certain
systems.  Consult your system's L<crypt(3)> manual.

=item C<CRYPT_BLOWFISH>

This specifies that a key suitable for the Blowfish algorithm should be
generated.  This may only be available on certain systems.  Consult your
system's L<crypt(3)> manual.  The Blowfish algorithm is a strong, fast
algorithm that you should consider using, should your system support it.

=back

=cut

sub gen_key {
	my ($this, $ktype) = @_;
	my %map = (
		CRYPT_DES()      => {len=>2,  type=>RAND_VIS_NQ, prefix=>''},
		CRYPT_EXT_DES()  => {len=>9,  type=>RAND_VIS_NQ, prefix=>''},
		CRYPT_MD5()      => {len=>12, type=>RAND_VIS_NQ, prefix=>'$1$'},
		CRYPT_BLOWFISH() => {len=>22, type=>RAND_VIS_ALNUM, prefix=>'$2a$05$'},
	);
	$this->throw("Unknown key type; type: $ktype") unless exists $map{$ktype};
	return $map{$ktype}{prefix} .
		$this->rand_str($map{$ktype}{len}, $map{$ktype}{type});
}

=back

=cut

return TRUE();
