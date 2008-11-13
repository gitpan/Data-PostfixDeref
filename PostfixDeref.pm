package Data::PostfixDeref;

use 5.008001;

use warnings;
use strict;

our $VERSION = '0.02';

use XSLoader;

#$ENV{DATA_PFDR_NOLOAD} or
    XSLoader::load __PACKAGE__, $VERSION;

1;

=head1 NAME

Data::PostfixDeref - Allow ->[] ->{} as an alternative to @{ } %{ }

=head1 SYNOPSIS

    use Data::PostfixDeref;

    my $x = { a => [1, {b => [2, 3]} ] };

    print for $x->{a}[1]{b}[];
    print for keys $x->{a}[1]{};
    push $x->{a}[], {c => 4};

=head1 DESCRIPTION

This module installs a hook into the Perl parser, that allows the syntax

    $x->[0][];

as an alternative to

    @{ $x->[0] };

and similarly C<< $x->[0]{} >> for C<< %{ $x->[0] } >>. The idea is to
make expressions like

    @{ $obj->{foo}{bar}{baz} }

less unwieldy. These expressions can be used anywhere the equivalent
C<@{ }> expression would have been valid; in particular, they can be
passed to C<(\@)>-prototyped functions, and builtins like C<keys>
and C<push>.

=head2 Disallowed syntax

Any further subscripts, such as

    $x->[0][][0]

will elict the error 'Additional subscripts after ->[] are forbidden'
(but see L</TODO> below).

Attempting to interpolate a hash into a string with

    "$x->[0]{}"

will fail with 'Can't interpolate hash'.

=head2 Switching it off

The hooks installed can be removed with

    no Data::PostfixDeref;

Note that all code compiled while the hooks are in effect will get the
new syntax, even code in different packages or different files. Also
note that once the hooks are removed, string-evals won't allow the
syntax, even if the surrounding code would.

=head1 LIMITATIONS

Since we don't actually replace the Perl parser, the new syntax is not
as general as it might be. The only cases that will work are

=over 4

=item *

Directly after another subscript, like

    $x->[0][]

=item *

Directly after a list slice, like

    ([1], [2])[1][]

=item *

Directly after a sub call (with or without parameters), like

    get_aref($x, $y)->[]

=item *

Directly after a method call (with or without parameters), like

    $obj->get_aref->[]

=back

In particular, neither

    $aref->[]

with no intervening subscript nor more complicated expressions like

    ($firstaref || $secondaref)->[]

will work (they will still be considered syntax errors); the first due
to an artefact of the Perl parser (specifically, that C<CHECKOP> is
never called for C<OP_PADSV>), and the second because the question of
precedence makes it impossible without being properly integrated into
the yacc parser.

=head1 TODO

=over 4

=item -

Use C<%^H> to activate the hooks lexically rather than globally. This
will only work under 5.10.

=item -

Allow the syntax

    $x->[0][][1, 2, 3]

for array slices, and

    $x->[0]{}{qw/a b c/}

for hash slices. (Currently these are both still syntax errors.)

=head1 BUGS

Please report any bugs via rt.cpan.org.

=head1 COPYRIGHT

Copyright 2008 Ben Morrow <ben@morrow.me.uk>

This module is released under the same terms as Perl itself.

