#!/usr/bin/perl -W
# $Id$

use WASP qw(:std);
use Babs;
use strict;
use warnings;

my $babs = Babs->new(shift);
my $oof = $babs->{oof};

my $user_id = $babs->require_login(Babs::PERM_STORY_ADD);
my $story_id = $babs->req("story_id");

my $doform = TRUE;

if ($babs->req("submitted")) {
	
}

if ($doform) {
}

EXIT_SUCCESS;
