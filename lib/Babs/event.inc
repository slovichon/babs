# $Id$

=head2 Event Routines

These routines provide an event-driven framework to Babs under which
event firing, notification, and handling can all be easily manipulated.

=over

=cut

use strict;
use warnings;

use constant EVT_STORY_ADD	=> 1;
use constant EVT_STORY_UPDATE	=> 2;
use constant EVT_STORY_REMOVE	=> 3;

use constant EVT_COMMENT_ADD	=> 4;
use constant EVT_COMMENT_UPDATE	=> 5;
use constant EVT_COMMENT_REMOVE	=> 6;

use constant EVT_USER_ADD	=> 7;
use constant EVT_USER_UPDATE	=> 8;
use constant EVT_USER_REMOVE	=> 9;

use constant EVT_UDF_UPDATE	=> 10;
# XXX: EVT_UDF_REMOVE/ADD ?

use constant _EVT_MAX		=> 10;

=item $babs->event_fire($type, @args);

This method fires the target event. Available events include:

=over

=item C<EVT_STORY_ADD>

This event is fired when a story is added. The only argument
is a hash reference containing all story fields.

=item EVT_STORY_UPDATE>

This event is fired when a story is updated. The only argument
is a hash reference containing all updated story fields.

=item C<EVT_STORY_REMOVE>

This event is fired when a story is removed. The only argument
is the story identifier.

=item C<EVT_COMMENT_ADD>

This event is fired when a comment is added. The only argument
is a hash reference containing all comment fields.

=item C<EVT_COMMENT_UPDATE>

This event is fired when a comment is updated. The only argument
is a hash reference containing all updated comment fields.

=item C<EVT_COMMENT_REMOVE>

This event is fired when a comment is removed. The only argument
is the comment identifier.

=item C<EVT_USER_ADD>

This event is fired when a user is added. The only argument
is a hash reference containing all user fields.

=item C<EVT_USER_UPDATE>

This event is fired when a user is updated. The only argument
is a hash reference containing all updated user fields.

=item C<EVT_USER_REMOVE>

This event is fired when a comment is removed. The only argument
is the user identifier.

=item C<EVT_UDF_UPDATE>

This event is fired when the user-defined fields fingerprint has
expired and is rebuilt.

=back

XXX: Note that you can determine if an event type is available with
XXX: L<Babs::event_register()>.

The associated argument list will be delivered to the event handler.

The associated handling actions will be performed in the order in
which they are registered.  See L<Babs::event_register()>.

=cut

sub event_fire {
	my $this = shift;
	my $type = shift;

	$this->throw("Unknown event type; type: $type") if $type <= 0 || $type > _EVT_MAX;
	
	foreach my $action (@{ $this->{events}[$type] }) {
		$action->(@_);
	}
}

=item $babs->event_has($type);

This method determines if the argued event type has been registered for
event propagation handling.

XXX: this should be able to determine if a given event type is available

=cut

sub event_has {
	my ($this, $type) = @_;
	return ref $this->{events}[$type] eq "ARRAY" && @{ $this->{events}[$type] };
}

=item $babs->event_register($type, $action);

This method registers an event for the specified action. The action and
all others previously and hereafter specified will be performed when the
event is fired. C<$action> may be a string or subroutine reference.

Event handling will be carried out in the order in which the actions
are registered.  See L<Babs::event_fire()>.

=cut

sub event_register {
	my ($this, $type, $action) = @_;
	$this->throw() if $type <= 0 || $type > _EVT_MAX;
	if (ref($this->{events}[$type]) eq "ARRAY") {
		push @{ $this->{events}[$type] }, $action;
	} else {
		$this->{events}[$type] = [$action];
	}
}

# XXX: event_unregister

=back

=cut

return TRUE();
