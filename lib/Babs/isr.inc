# $Id$

=head2 ISR Routines

This sub-module provides a representation and framework for manipulating
the internal structures of the database tables.

=over

=cut

use strict;
use warnings;

use constant ISR_TYPE_STR  => 1;
use constant ISR_TYPE_NUM  => 2;
use constant ISR_TYPE_BOOL => 3;

=item C<< $safe = $babs->isr_check_field($table, $field, $op, $val); >>

This method checks a value to be run in an SQL query against a certain field of
a Babs database table, under a specific operation.  This can be used to determine
if such a database table or field exists, and to properly encode data that will
be used to query such a field.

C<$table> and C<$field> denote the structure of Babs' internally-used database,
C<$op> is the operator (such as +, -, LIKE, etc.), and C<$val> is the value
that the test will be run on.

C<undef> will be returned only if the field does not exist.

=cut

sub isr_check_field {
	my ($this, $table, $field, $op, $val) = @_;

	my $esc_num  = sub { return $_[0] && $_[0] =~ /^\d+$/ ? $_[0] : 0; };
	my $esc_reg  = sub { return $this->dbh_quote($_[0], SQL_REG); };
	my $esc_wild = sub { return $this->dbh_quote($_[0], SQL_WILD); };
	my $esc_bool = sub { return $_[0] ? 1 : 0; };

	my %ops = {
		ISR_TYPE_NUM()	=> {
			'='	=> $esc_num,
			'<'	=> $esc_num,
			'>'	=> $esc_num,
			'<='	=> $esc_num,
			'>='	=> $esc_num,
			'<>'	=> $esc_num,
		},
		ISR_TYPE_STR()	=> {
			'LIKE'	=> $esc_wild,
			'='	=> $esc_reg,
		},
		ISR_TYPE_BOOL() => {
			'='	=> $esc_bool,
			'<>'	=> $esc_bool,
		},
	};

	my %fields = (
		babs_users => {
			user_id			=> ISR_TYPE_NUM,
			username		=> ISR_TYPE_STR,
			type			=> ISR_TYPE_NUM,
			new_password		=> ISR_TYPE_STR,
			recover_key		=> ISR_TYPE_STR,
			cancel_key		=> ISR_TYPE_STR,
			email			=> ISR_TYPE_STR,
		},
		babs_stories => {
			story_id		=> ISR_TYPE_NUM,
			author_id		=> ISR_TYPE_NUM,
			headline		=> ISR_TYPE_STR,
			story			=> ISR_TYPE_STR,
			create_date		=> ISR_TYPE_NUM,
			mod_date		=> ISR_TYPE_NUM,
			num_comments		=> ISR_TYPE_NUM,
			allow_comments		=> ISR_TYPE_BOOL,
		},
		babs_comments => {
			story_id		=> ISR_TYPE_NUM,
			comment_id		=> ISR_TYPE_NUM,
			author_id		=> ISR_TYPE_NUM,
			subject			=> ISR_TYPE_STR,
			comment			=> ISR_TYPE_STR,
			create_date		=> ISR_TYPE_NUM,
			mod_date		=> ISR_TYPE_NUM,
			parent_comment_id	=> ISR_TYPE_NUM,
		},
	);

	return undef unless exists $fields{$table};
	return undef unless exists $fields{$table}{$field};
	return undef unless in_array($op, [ keys %{ $ops{$fields{$table}{$field}} } ]);
	return $ops{$fields{$table}{$field}}{$op}->($val);
}

return TRUE();
