# Babs template routines
# $Id$

use strict;

sub template_expand {
	my ($this, $type, $args) = @_;

	my $file = $this->{templates_dir} . "/$type";

	# Allow multiple inclusion
	delete $INC{"$file.inc"} if exists $INC{"$file.inc"};

	package Babs::Null;
	use constant TRUE  => 1;
	use constant FALSE => "";

	our $babs;
	
	$babs = $this;

	no strict 'vars';

	# Import args into package
	if (ref $args eq "HASH") {
		my ($k, $v);
		while (($k, $v) = each %$args) {
			$$k = $v if $k =~ /^[a-zA-Z_][a-zA-Z0-9_]+$/;
		}
	}

	my $ret = eval {
		require "$file.inc";
	};

#	if ($@) {
#		package Babs;
#		$this->throw("Compile error in $file.inc: $@")
#	}

	# Check for errors
	unless ($ret) {
		package Babs;
		return "";
	}

	my $template = $this->slurp_file("$file.t");

	# Expand variables
	$template =~ s~
		[\$@%]+			# Scalar, array, hash
		[a-zA-Z_]		# First char
		[a-zA-Z0-9_]*		# Subsequent chars
		(?:
			(?: -> )?	# Dereference
			(?: \[ .*? \] )	# Array
			|		# or
			(?: { .*? } )	# Hash
		)+			# Any amount of nest
	~$&~gxe;

	package Babs;

	# Clear Babs::Null to prevent future conflicts
	# than cleanliness and future conflicts
	while (($k) = each %Babs::Null::) {
		undef *{"Babs::Null::$k"};
	}
	# %Babs::Null:: = (); # ?

	return $template;
}

sub template_exists {
	my ($this, $type) = @_;
	return -f $this->{template_dir} . "/$type.inc";
}

sub header {
	my ($this, $title) = @_;
	return $this->template_expand("header", {title => $title});
}

sub footer {
	my ($this) = @_;
	return $this->template_expand("footer");
}

return TRUE();
