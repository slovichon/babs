# crypt.inc
# $Id$

use constant BABS_RAND_ALNUM => 1;
use constant BABS_RAND_VIS_NQ => 2;
push @EXPORT, qw(BABS_RAND_ALNUM BABS_RAND_VIS_NQ);

my %methods = (
	"3des"		=> 48,
	aes		=> 32,
	blowfish 	=> 56,
	cast5 		=> 16,
	ciphersaber 	=> 53,
	des 		=> 8,
	gost 		=> 32,
	idea 		=> 16,
	md5 		=> 32,
	null		=> 16,
	rc4		=> undef,
	rc5		=> undef,
	rc6		=> undef,
	rmd160		=> undef,
	rsa		=> undef,
	serpent		=> undef,
	sha1		=> undef,
	solitaire	=> undef,
	tea		=> undef,
	twofish		=> undef,
	vigenere	=> undef,
);

sub crypt
{
	my ($this,$data) = @_;

	# replace method for subsequent calls
	*{__PACKAGE__."::crypt"} = "crypt_$type";
}

sub crypt_key_len
{
	my ($this) = shift;

}

sub crypt_gen_key
{
	my ($this,$type) = @_;
}

sub crypt_rand_str
{
}

# algorithm-specific methods
sub crypt_3des
{
	my ($this,$data) = @_;
	require Crypt::TripleDES;
	return Crypt::TripleDES->new()->encrypt3($data,$this->crypt_key);
}

sub crypt_aes
{
	my ($this,$data) = @_;
	require Crypt::Rijndael;
	my $cipher = Crypt::Rijndael->new($this->crypt_key,Crypt::Rijndael::MODE_CBC);
#	$cipher->set_iv($iv);
	return $cipher->encrypt($data);
}

sub crypt_blowfish
{
	my ($this,$data) = @_;
	require Crypt::Blowfish;
	return Crypt::Blowfish->new($this->crypt_key)->encrypt($data);
}

sub crypt_cast5
{
	my ($this,$data) = @_;
	require Crypt::CAST5;
	return Crypt::CAST5->new($this->crypt_key)->encrypt($data);
}

sub crypt_ciphersaber
{
	my ($this,$data) = @_;
	require Crypt::CipherSaber;
	return Crypt::CipherSaber->new($this->crypt_key,20)->encrypt($data);
}

sub crypt_des
{
	my ($this,$data) = @_;
	require Crypt::DES;
	return Crypt::DES->new($this->crypt_key)->encrypt($data);
}

sub crypt_gost
{
	my ($this,$data) = @_;
	require Crypt::GOST;
	return Crypt::GOST->new($this->crypt_key)->encrypt($data);
}

sub crypt_idea
{
	my ($this,$data) = @_;
	require Crypt::IDEA;
	return IDEA->new($this->crypt_key)->encrypt($data);
}

sub crypt_md5
{
	my ($this,$data) = @_;
	require Digest::MD5;
	my $ctx = Digest::MD5->new();
	$ctx->add($data);
	return $ctx->hexdigest;
}

sub crypt_null
{
	my ($this,$data) = @_;
	require Crypt::NULL;
	return Crypt::NULL->new($this->crypt_key)->encrypt($data);
}

sub crypt_rc4
{
	my ($this,$data) = @_;
	require Crypt::RC4;
	return Crypt::RC4->new($this->crypt_key)->RC4($data);
}

sub crypt_rc5
{
	my ($this,$data) = @_;
	require Crypt::RC5;
	return Crypt::RC5->new($this->crypt_key,12)->encrypt($data);
}

sub crypt_rc6
{
}

sub crypt_rsa
{
}

sub crypt_serpent
{
}

sub crypt_solitaire
{
}

sub crypt_tea
{
}

sub crypt_twofish
{
}

sub crypt_vigenere
{
}

return 1;
