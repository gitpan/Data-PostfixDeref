#!/usr/bin/perl

use strict;
use warnings;

use Test::More;

my $tests;

BEGIN { $tests += 5 }

require_ok  'Data::PostfixDeref';
can_ok      'Data::PostfixDeref', 'import';
can_ok      'Data::PostfixDeref', 'unimport';

ok eval q/Data::PostfixDeref->import; 1/,   'can install handlers';
ok eval q/Data::PostfixDeref->unimport; 1/, 'can uninstall handlers';

BAIL_OUT 'module will not load'
    if grep !$_, Test::More->builder->summary;

BEGIN { plan tests => $tests }
