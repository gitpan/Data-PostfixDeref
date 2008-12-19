#!/usr/bin/perl

use warnings;
use strict;

use Test::More;
use Data::PostfixDeref;

my $tests;

{
    package DB; # so eval sees surrounding scope

    my $T = Test::Builder->new;

    sub ::eval_is {
        my ($perl, $exp, $name) = @_;

        $T->todo;
        my $rv = $T->is_eq(eval "#line 1 '$name'\n$perl", $exp, $name);
        $rv or $T->diag("tried: $perl");
        $@ and $T->diag("\$\@: $@");
        return $rv;
    }
}

my $T = Test::Builder->new;

sub do_is {
    my ($file, $exp, $name) = @_;

    $T->todo;
    my $rv = $T->is_eq(do $file, $exp, $name);
    unless ($rv) {
        $T->diag("tried: do '$file'");
        my $perl = do {
            local $/;
            my $F;
            open $F, "<", $file and <$F>;
        };
        $T->diag($perl);
    }
    $@ and $T->diag("\$\@: $@");
    return $rv;
}

{
    package t::Foo;

    sub aref { return [1, 2] }
    sub href { return {f => 6} }
    sub self { return $_[0] }
    sub four { return 4 }
}

{
    my $aref = [1, [2, 3], {a => "b"}];
    my $href = {a => 1, b => [2, 3], c => {d => 4, e => 5}, '' => 6};

    sub aref { return [3, 4] }
    sub href { return {e => 5} }

    our $obj = bless [], "t::Foo";

    BEGIN { $tests += 9 }

    TODO: {
        local $TODO = 'scalars not subject to CHECKOP', 1;
        eval_is ( q/scalar $aref->[]/,      3, '$aref->[]');
    }
    eval_is ( q/scalar $aref->[1]->[]/,     2, '$aref->[1]->[]');
    eval_is ( <<'PERL',                     2, 'with spaces and comments');
scalar $aref->[1]  #foo
    -> #foo
    [ #foo
    ]
PERL
    do_is   ( 't/multiline',                4, 'don\'t break method calls');
    eval_is ( q/scalar $aref->[1][]/,       2, '$aref->[1][]');
    eval_is ( q/scalar $$aref[1][]/,        2, '$$aref[1][]');
    eval_is ( q/no warnings; $aref->[()]/,  1, '$aref->[()]');
    eval_is ( q/scalar main->aref()->[]/,   2, 'main->aref()->[]');
    eval_is ( q/scalar main->aref->[]/,     2,  'main->aref->[]');

    BEGIN { $tests += 7 }

    TODO: {
        local $TODO = 'scalars not subject to CHECKOP', 1;
        eval_is ( q/scalar keys $href->{}/,     4, '$href->{}');
    }
    eval_is ( q/scalar keys $href->{c}->{}/,    2, '$href->{c}->{}');
    eval_is ( q/scalar keys $href->{c}{}/,      2, '$href->{c}{}');
    eval_is ( q/scalar keys $$href{c}{}/,       2, '$$href{c}{}');
    eval_is ( q/no warnings; $href->{()}/,      6, '$href->{()}');
    eval_is ( q/scalar keys main->href()->{}/,  1, 'main->href()->{}');
    eval_is ( q/scalar keys main->href->{}/,    1, 'main->href->{}');


    BEGIN { $tests += 4 }

    eval_is ( q/scalar $href->{b}->[]/,      2, '$href->{b}->[]');
    eval_is ( q/scalar $href->{b}[]/,        2, '$href->{b}[]');
    eval_is ( q/scalar keys $aref->[2]->{}/, 1, '$aref->[2]->{}');
    eval_is ( q/scalar keys $aref->[2]{}/,   1, '$aref->[2]{}');

    BEGIN { $tests += 3 }

    TODO: {
        local $TODO = 'scalars not subject to CHECKOP', 1;
        eval_is ( q/push $aref->[], 6; $aref->[3]/, 6, 'push $aref->[]');
    }
    eval_is ( q/push $aref->[1][], 7; $aref->[1][2]/, 7, 'push $aref->[1][]');
    eval_is ( q/push $href->{b}[], 8; $href->{b}[2]/, 8, 'push $href->{b}[]');
}

# Test slices

{
    BEGIN { $tests += 4 }

    eval_is q{scalar(([qw/foo bar/])[0]->[])},  2,  '()[0]->[]';
    eval_is q{scalar(([qw/foo bar/])[0][])},    2,  '()[0][]';

    eval_is q{keys(({foo => "bar"})[0]->{})},   "foo",  '()[0]->{}';
    eval_is q{keys(({foo => "bar"})[0]{})},     "foo",  '()[0]{}';

}

# Test bad derefs

{
    my $ref = [\1];

    BEGIN { $tests += 2 }

    eval q/ $ref->[0][] /;
    like($@, qr/Not an ARRAY reference/, "[] array dereference");
    eval q/ $ref->[0]{} /;
    like($@, qr/Not a HASH reference/, "{} hash dereference");
}

BAIL_OUT 'basic syntax not working'
    if grep !$_, Test::More->builder->summary;

BEGIN { plan tests => $tests }
