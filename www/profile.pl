#!/usr/bin/perl -W
# $Id$

use WASP qw(:std);
use Babs;
use strict;
use warnings;

my $babs = Babs->new(shift);
my $oof = $babs->{oof};

my $user_id = $babs->req("user_id") || 0;

if ($user_id && $babs->user_exists($user_id))
{
	my $user = $babs->user_get($user_id);
	print	$babs->header("User Profile"),
		$oof->header("User Profile"),
		$babs->template_expand("user", $user),
		$babs->footer();

} else {
	print	$babs->header("Error"),
		$oof->header("Error"),
		$oof->p("The requested user could not be found."),
		$babs->footer();
}

EXIT_SUCCESS;
