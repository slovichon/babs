# Babs internal structure representation routines
# $Id$

use strict;

use constant ISR_TYPE_STR => 1;
use constant ISR_TYPE_NUM => 2;

sub isr_check_field
{
	my ($this, $table, $field, $op, $val) = @_;

	my $num = sub { $_[0] && $_[0] =~ /^\d+$/ ? $_[0] : 0 };

	my %ops = {
		ISR_TYPE_NUM()	=> {
			'='	=> $num,
			'<'	=> $num,
			'>'	=> $num,
			'<='	=> $num,
			'>='	=> $num,
			'<>'	=> $num,
		},
		ISR_TYPE_STR()	=> {
			'LIKE'	=> sub { q(') . $this->{dbh}->prepare_str($_[0], SQL_WILD) . q(') },
			'='	=> sub { q(') . $this->{dbh}->prepare_str($_[0], SQL_REG)  . q(') },
		},
	};

	my %fields = (
		babs_users => {
			user_id		=> ISR_TYPE_NUM,
			username	=> ISR_TYPE_STR,
			type		=> ISR_TYPE_NUM,
			new_password	=> ISR_TYPE_STR,
			recover_key	=> ISR_TYPE_STR,
			cancel_key	=> ISR_TYPE_STR,
			email		=> ISR_TYPE_STR,
		},
		babs_stories => {
			story_id	=> ISR_TYPE_NUM,
			author_id	=> ISR_TYPE_NUM,
			headline	=> ISR_TYPE_STR,
			story		=> ISR_TYPE_STR,
			create_date	=> ISR_TYPE_NUM,
			mod_date	=> ISR_TYPE_NUM,
			num_comments	=> ISR_TYPE_NUM,
			allow_comments	=> ISR_TYPE_NUM,
		},
		babs_comments => {
		},
	);

	return undef unless exists $fields{$table};
	return undef unless exists $fields{$table}{$field};
	return undef unless in_array($op, [ keys %{ $ops{$fields{$table}{$field}} } ]);
	return $ops{$fields{$table}{$field}}{$op}->($val);
}

return TRUE();
