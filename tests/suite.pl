#!/usr/bin/perl -W

use strict;
use Babs;
use WASP;
use Thraxx;
use OF::HTML;

my $w = WASP->new;
$w->display(1);
my $t = Thraxx->construct(wasp=>$w, dbh=>1, skip_init=>1);
my $b = Babs->construct(thraxx=>$t, wasp=>$t, dbh=>undef, skip_init=>1,
			isapi=>CGI->new, of=>OF::HTML->new($w), dbh=>1);

print Babs::FOOBAR();

exit 0;
