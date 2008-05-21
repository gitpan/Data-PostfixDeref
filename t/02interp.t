#!/usr/bin/perl

use warnings;
use strict;

use Test::More;
use Data::PostfixDeref;

my $tests;

# test subscript interpolation

my  $aref  = [1, 2, 3, 4];
my  $nesta = [1, [2, 3], {a => 4, b => 5}];
my  $href  = {a => 1, b => 2};
my  $nesth = {e => [1, 2], f => {g => 3}};

local $" = 'x';

BEGIN { $tests += 4 }

is (eval q/"$aref-> []"/, $aref.'-> []', '"$aref-> []"');
is (eval q/"$aref ->[]"/, $aref.' ->[]', '"$aref ->[]"');
is (eval q/"$href-> {}"/, $href.'-> {}', '"$href-> {}"');
is (eval q/"$href ->{}"/, $href.' ->{}', '"$href ->{}"');

BEGIN { $tests += 4 }

is (eval q/"$$nesta[1] []"/, $$nesta[1].' []', '"$$aref[1] []"');
is (eval q/"$$nesta[2] {}"/, $$nesta[2].' {}', '"$$aref[1] {}"');
is (eval q/"$$nesth{e} []"/, $$nesth{e}.' []', '"$$href{a} []"');
is (eval q/"$$nesth{f} {}"/, $$nesth{f}.' {}', '"$$href{a} {}"');

BEGIN { $tests += 7 }

TODO: {
    local $TODO = 'scalars are not subject to CHECKOP';

    is (eval q/"$aref->[]"/, '1x2x3x4', '"$aref->[]"');
    is (eval q/"$aref->[] a"/, '1x2x3x4 a', '"$ar->[] a"');
}

is (eval q/"$nesta->[1][]"/,    '2x3',      '"$aref->[1][]"');
is (eval q/"$nesta->[1][] a"/,  '2x3 a',    '"$aref->[1][] a"');
is (eval q/"$nesth->{e}[]"/,    '1x2',      '"$href->{a}[]"');
is (eval q/"$$nesta[1][]"/,     '2x3',      '"$$aref[1][]"');
is (eval q/"$$nesth{e}[]"/,     '1x2',      '"$$href{a}[]"');

{
    my $x = '2x3';

    BEGIN { $tests += 5 }

    # I use $nesta rather than $aref since $aref-[] doesn't work

    ok ($x =~ /$nesta->[1][]/, '/$aref->[1][]/'); #' grr vim

    is (<<E, "2x3\n", '$aref->[1][] in heredoc');
$nesta->[1][]
E

    is (`$^X -e "print q/$nesta->[1][]/"`, '2x3', '`$aref->[1][]`');

    $x =~ s/$nesta->[1][]/foo/;
    is ($x, 'foo', 's/$aref->[1][]//');

    $x =~ s/foo/$nesta->[1][]/;
    is ($x, '2x3', 's//$aref->[1][]/');
}

BEGIN { $tests += 7 }

ok (!eval q/"$href->{}"; 1/, '"$href->{}" fails');

TODO: {
    local $TODO = 'scalars not subject to CHECKOP';

    like ($@, qr/Can't interpolate hash/, '...with the correct error');
}

is (eval q/scalar $nesta->[1][]/, 2, '$aref->[1][] still works');

ok (!eval q/"$nesta->[1]{}"; 1/, '"$aref->[1]{}" fails');
like ($@, qr/Can't interpolate hash/, '...with the correct error');

ok (!eval q/"$nesth->{e}{}"; 1/, '"$href->{a}{}" fails');
like ($@, qr/Can't interpolate hash/, '...with the correct error');

BEGIN { plan tests => $tests }
