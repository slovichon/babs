# Babs event routines
# $Id$

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

use constant _EVT_MAX		=> 10;

sub event_fire
{
	my $this = shift;
	my $type = shift;

	$this->throw() if $type <= 0 || $type > _EVT_MAX;
	
	foreach my $action (@{ $this->{events}[$type] })
	{
		$action->(@_);
	}
}

sub event_has
{
	my ($this, $type) = @_;
	return ref $this->{events}[$type] eq "ARRAY" && @{ $this->{events}[$type] };
}

sub event_register
{
	my ($this, $type, $action) = @_;
	$this->throw() if $type <= 0 || $type > _EVT_MAX;
	if (ref($this->{events}[$type]) eq "ARRAY")
	{
		push @{ $this->{events}[$type] }, $action;
	} else {
		$this->{events}[$type] = [$action];
	}
}

return Babs::TRUE;
