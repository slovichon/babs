#!/usr/bin/perl -W
# $Id$

use Babs;
use strict;
use warnings;

my $babs = Babs->new(shift);
$babs->require_login(Babs::PERM_STORY_ADD);

my $oof = $babs->{oof};

if ($babs->req("submitted")) {
	# "Add Story" form submitted; add story
}

if () {
	# Simple request; display "Add Story" form
	print	$babs->expand_template(),
		$oof->header("Adding Story"),
		$babs->expand_template();
}
