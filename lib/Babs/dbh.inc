# $Id$

=head2 Database Routines

This sub-module provides database-related methods.

=over

=cut

use strict;
use warnings;

=item $babs->dbh_selectcol($stmt, @bindvals);

This method is essentially a wrapper around C<DBI::selectcol_arrayref()>.
See L<DBI> for more information.

The SQL statement and an optional list of bind parameters are arguments
to this method, and the query is executed on the database and the
first column of the first row of the resultant data is returned. If
no such value would be returned, an empty string is instead returned.

=cut

sub dbh_selectcol {
	my ($this, $stmt, @bindvals) = @_;
	my $ret = $this->{dbh}->selectcol_arrayref($stmt,
				{ MaxRows=>1 }, @bindvals);
	return ref $ret eq "HASH" && defined $ret->[0] ? $ret->[0] : "";
}

=back

=cut

return TRUE();
