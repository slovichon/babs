#!/usr/bin/perl -W

use Babs;
use strict;

my $babs = Babs->new(shift);
my $of = $babs->{of};

my $user_id = $babs->{isapi}->param("user_id") || 0;

if ($babs->user_exists($user_id))
{
	my $user = $babs->user_get($user_id);
	print	$babs->header("User Profile"),
		$of->header("User Profile"),
		$babs->template_expand("user", $user),
		$babs->footer();

} else {
	print	$babs->header("Error"),
		$of->header("Error"),
		$of->p("The requested user could not be found."),
		$babs->footer();
}

exit 0;
