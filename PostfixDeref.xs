#define PERL_CORE
#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"
#include "thieved.h"

#ifdef DEBUG
#define debug warn
#else
#define debug PFDR_noop

/* I assume any competent compiler can optimize this out... */
STATIC void
PFDR_noop(const char *fmt, ...)
{
    return;
}
#endif

#define MY_CXT_KEY "Data::PostfixDeref::_guts" XS_VERSION
typedef struct {
    int in_ck;
} my_cxt_t;
START_MY_CXT

#ifndef YYEMPTY
#define YYEMPTY (-1)
#endif

#define LEX_IN_QQISH \
    ((PL_lex_inwhat == OP_SCALAR || \
      PL_lex_inwhat == OP_STRINGIFY || \
      PL_lex_inwhat == OP_MATCH || \
      PL_lex_inwhat == OP_SUBST || \
      PL_lex_inwhat == OP_BACKTICK) && \
     PL_lex_brackets == 0)

#define DO_ALL_OPS \
    /* PADSV doesn't get CHECKOPed, so we won't do RV2SV either */ \
    /* DO_ONE_OP(RV2SV) */ \
    /* DO_ONE_OP(PADSV) */ \
    DO_ONE_OP(AELEM) \
    DO_ONE_OP(HELEM) \
    DO_ONE_OP(LSLICE) \
    /* ASLICE and HSLICE are somewhat weird... */ \
    /* DO_ONE_OP(ASLICE) */ \
    /* DO_ONE_OP(HSLICE) */ \
    DO_ONE_OP(ENTERSUB)

#undef DO_ONE_OP
#define DO_ONE_OP(op) STATIC OP *(*pfdr_old_ck_ ## op)(pTHX_ OP *o);
DO_ALL_OPS

STATIC OP *
pfdr_do_ck(pTHX_ OP *o)
{
    dMY_CXT;
    char *s, *oldptr, b;
    int yychar;

#undef DO_ONE_OP
#define DO_ONE_OP(op) \
    case OP_ ## op : \
        o = pfdr_old_ck_ ## op (aTHX_ o); \
        break;

    switch (o->op_type) {
        DO_ALL_OPS
        default:
            Perl_croak(aTHX_ "Data::PostfixDeref: pdfr_do_ck called for invalid op");
    }

    if (MY_CXT.in_ck) {
        debug("pfdr_do_ck not recursing");
        return o;
    }

    MY_CXT.in_ck = 1;

    yychar = PL_yychar;
    oldptr = s = PL_bufptr;

    /* the parser may have read ahead */
    if (yychar != ARROW && yychar != YYEMPTY)
        goto nope;

    if (!LEX_IN_QQISH)
        s = skipspace(s);

    if (yychar == ARROW)
        yychar = YYEMPTY;
    else if (s[0] == '-' && s[1] == '>')
        s += 2;
    else if (
        o->op_type == OP_RV2SV || 
        o->op_type == OP_PADSV ||
        o->op_type == OP_ASLICE ||
        o->op_type == OP_ENTERSUB
    )
        goto nope;

    if (!LEX_IN_QQISH)
        s = skipspace(s);

    if (*s != '[' && *s != '{')
        goto nope;

    b = *s; s++; 
    if (!LEX_IN_QQISH)
        s = skipspace(s);

    if (b == '[' && *s != ']' || b == '{' && *s != '}')
        goto nope;

    s++;
    debug("got a ->%s", (b == '[') ? "[]" : "{}");

    if (LEX_IN_QQISH && b == '{') {
        MY_CXT.in_ck = 0;
        Perl_croak(aTHX_ "Can't interpolate hash");
    }

    /* newBINOP is supposed to do this, but it doesn't if we
     * return the wrong op */
    if (
        (o->op_type == OP_AELEM || o->op_type == OP_HELEM)
        && ! o->op_next
    ) {
        ((BINOP *)o)->op_last = ((BINOP *)o)->op_first->op_sibling;
        o = fold_constants(o);
    }

    o = (b == '[') ? newAVREF(o) : newHVREF(o);

    if (LEX_IN_QQISH) {

        o = convert(OP_JOIN, 0,
            append_elem(OP_LIST,
                newSVREF(newGVOP(OP_GV, 0,
                    gv_fetchpv("\"", TRUE, SVt_PV))),
                o));

        /* we might have finished interpolation */
        if (s == PL_bufend)
            PL_lex_state = LEX_INTERPEND;
        else
            PL_lex_state = LEX_INTERPCONCAT;
    }

    debug("Next char: '%c', yychar: '%d'", *s, yychar);
    PL_bufptr = s;
    PL_yychar = yychar;

    /* we need to skip forward to check for more subscripts, but we want
     * to leave that space to toke.c if there aren't any
     */
    if (!LEX_IN_QQISH) s = skipspace(s);

    /* slice syntax: ->[][1, 2, 3], ->{}{qw/a b c/}
     * transform into ->D::P::fakeslice([$D::P::fakeslice,
     * catch matching ]} in ANONLIST|HASH and add closing )
     * catch fakeslice in ENTERSUB and convert optree
     */

    if (
        s[0] == '[' || s[0] == '{'
        || s[0] == '-' && s[1] == '>'
    )
        Perl_croak(aTHX_ "Additional subscripts after ->%s are forbidden",
            (b == '[' ? "[]" : "{}"));

    MY_CXT.in_ck = 0;
    return o;

  nope:
    PL_bufptr = oldptr;
    MY_CXT.in_ck = 0;
    return o;
}

MODULE = Data::PostfixDeref  PACKAGE = Data::PostfixDeref

PROTOTYPES: DISABLE

BOOT:
{
    MY_CXT_INIT;
    MY_CXT.in_ck = 0;
}

void
import(class)
        char *class
    CODE:
#undef DO_ONE_OP
#define DO_ONE_OP(op) pfdr_old_ck_ ## op = PL_check[OP_ ## op];
            DO_ALL_OPS
#undef DO_ONE_OP
#define DO_ONE_OP(op) PL_check[OP_ ## op] =
            DO_ALL_OPS pfdr_do_ck;

void
unimport(class)
        char *class
    CODE:
#undef DO_ONE_OP
#define DO_ONE_OP(op) PL_check[OP_ ## op] = pfdr_old_ck_ ## op;
        DO_ALL_OPS

void
_breakpoint()
    CODE:
