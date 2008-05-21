#!/usr/bin/perl

use warnings;
use strict;

use Test::More;
use Data::PostfixDeref;

my $tests;

sub array_ref (\@) {
    my $nok;
    my $name = do { local $" = ','; "array_ref(@_)" };

    if (!ref($_[0])) {
        $nok = 1;
    }
    elsif (ref($_[0]) eq 'ARRAY') {
        $nok = 1 unless 1 == @{$_[0]};
        push @{$_[0]}, 'b';
    }
    else {
        $nok = 1;
    }
    ok !$nok, $name;
}

{
    BEGIN { $tests += 4 }

    # we need a nested aref as $aref->[] fails

    my $aref = [['a']];
    my $href = [{a => 1}];

    array_ref $aref->[0][];
    is $aref->[0][1], 'b', 'args decoded correctly';

    ok !eval q{ array_ref $href->[0]{}; 1}, 'can\'t pass ->{} to (\@) sub';
    like $@, qr/must be array \(not hash/, '...correct error';
}

sub hash_ref (\%) {
    my $nok;
    my $name = do { local $" = ','; "hash_ref(@_)" };

    if (!ref($_[0])) {
        $nok = 1;
    }
    elsif (ref($_[0]) eq 'HASH') {
        $nok = 1 unless 1 == $_[0]{a};
        $_[0]{b} = 2;
    }
    else {
        $nok = 1;
    }
    ok !$nok, $name;
}

{
    BEGIN { $tests += 4 }

    # we need a nested aref as $aref->[] fails

    my $aref = [['a']];
    my $href = [{a => 1}];

    hash_ref $href->[0]{};
    is $href->[0]{b}, 2, 'args decoded correctly';

    ok !eval q{ hash_ref $aref->[0][]; 1}, 'can\'t pass ->[] to (\%) sub';
    like $@, qr/must be hash \(not array/, '...correct error';
}

sub array_or_hash_ref (\[@%]) {
    my $nok;
    my $name = do {
        local $" = ',';
        "array_or_hash_ref(@_)";
    };

    if (!ref($_[0])) {
        $nok = 1;
    }
    elsif (ref($_[0]) eq 'ARRAY') {
        $nok = 1 unless 1 == @{$_[0]};
        push @{$_[0]}, 'b';
    }
    elsif (ref($_[0]) eq 'HASH') {
        $nok = 1 unless $_[0]->{'a'};
        $_[0]->{'b'} = 2;
    }
    else {
        $nok = 1;
    }
    ok !$nok, $name;
}

{
    BEGIN { $tests += 4 }

    my $aref = [['a']];
    array_or_hash_ref $aref->[0][];
    is $aref->[0][1], 'b', 'args decoded correctly';

    my $href = [{'a' => 1}];
    array_or_hash_ref $href->[0]{};
    is $href->[0]{b}, 2, 'args decoded correctly';
}

BEGIN { plan tests => $tests }
