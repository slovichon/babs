# Babs template routines
# $Id$

use strict;

sub template_expand
{
	my ($__this, $__type, $__args) = @_;

	my $__file  = $__this->{templates_dir} . "/$__type";

	# Load template include file
	delete $INC{"$__file.inc"} if exists $INC{"$__file.inc"};

	package Babs::NullTemplate;

	no strict 'vars';

	# Import args to natural name
	if (ref $__args eq "HASH")
	{
		my ($__k, $__v);
		while (($__k, $__v) = each %$__args)
		{
			if ($__k =~ /^[a-zA-Z]+$/)
			{
				our $$__k = $__v;
			}
		}
	}

	eval {
		require "$__file.inc";
	};

	# Check for errors
	return "" if $@;

	my $__template = $__this->slurp_file("$__file.t");

	# Substitute variables
	$__template =~	s~
				({?)			# Optional { ... } delimiter
				(
					[\$@%]		# Scalar, array, hash
					[a-zA-Z_]	# First char
					[a-zA-Z0-9_]+	# Subsequent chars
					(?:->)?		# Optional dereference
					(?:
						(?: \[ .*? \] )
						|
						(?: { .*? } )
					)*		# Any amount of subscripts
				)
				\\1
			~$1~gxee;

	package Babs;

	# Clear Babs::NullTemplate - if for no other reasons
	# than cleanliness and future conflicts
	my ($__k);
	while ( ($__k) = each %Babs::NullTemplate::)
	{
		delete $Babs::NullTemplate::{$__k};
	}

	return $__template;
}

sub template_exists
{
	my ($this, $type) = @_;
	return -f $this->{template_dir} . "/$type.inc";
}

sub header
{
	my ($this, $title) = @_;
	return $this->template_expand("header", {title => $title});
}

sub footer
{
	my ($this) = @_;
	return $this->template_expand("footer");
}

return TRUE();
