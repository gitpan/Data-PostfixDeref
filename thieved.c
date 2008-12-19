/* 
 * Stuff I have blatantly stolen from the perl source. Who thought
 * having to explicitly export functions was a good idea???
 */

#include "EXTERN.h"
#include "perl.h"

#include "ppport.h"
#include "thieved.h"

/* for some reason ppport redefines this... */
#undef Perl_warner

/* functions that were originally static */

/* toke.c */

char *
PFDR_filter_gets(pTHX_ register SV *sv, register PerlIO *fp, STRLEN append)
{
#ifdef PERL_CR_FILTER
    if (!PL_rsfp_filters) {
	filter_add(S_cr_textfilter,NULL);
    }
#endif
    if (PL_rsfp_filters) {
	if (!append)
            SvCUR_set(sv, 0);	/* start with empty line	*/
        if (FILTER_READ(0, sv, 0) > 0)
            return ( SvPVX(sv) ) ;
        else
	    return Nullch ;
    }
    else
        return (sv_gets(sv, fp, append));
}

#if PERL_VERSION == 8

void
PFDR_incline(pTHX_ char *s)
{
    char *t;
    char *n;
    char *e;
    char ch;

    CopLINE_inc(PL_curcop);
    if (*s++ != '#')
	return;
    while (SPACE_OR_TAB(*s)) s++;
    if (strnEQ(s, "line", 4))
	s += 4;
    else
	return;
    if (SPACE_OR_TAB(*s))
	s++;
    else
	return;
    while (SPACE_OR_TAB(*s)) s++;
    if (!isDIGIT(*s))
	return;
    n = s;
    while (isDIGIT(*s))
	s++;
    while (SPACE_OR_TAB(*s))
	s++;
    if (*s == '"' && (t = strchr(s+1, '"'))) {
	s++;
	e = t + 1;
    }
    else {
	for (t = s; !isSPACE(*t); t++) ;
	e = t;
    }
    while (SPACE_OR_TAB(*e) || *e == '\r' || *e == '\f')
	e++;
    if (*e != '\n' && *e != '\0')
	return;		/* false alarm */

    ch = *t;
    *t = '\0';
    if (t - s > 0) {
#ifndef USE_ITHREADS
	const char *cf = CopFILE(PL_curcop);
	if (cf && strlen(cf) > 7 && strnEQ(cf, "(eval ", 6)) {
	    /* must copy *{"::_<(eval N)[oldfilename:L]"}
	     * to *{"::_<newfilename"} */
	    char smallbuf[256], smallbuf2[256];
	    char *tmpbuf, *tmpbuf2;
	    GV **gvp, *gv2;
	    STRLEN tmplen = strlen(cf);
	    STRLEN tmplen2 = strlen(s);
	    if (tmplen + 3 < sizeof smallbuf)
		tmpbuf = smallbuf;
	    else
		Newx(tmpbuf, tmplen + 3, char);
	    if (tmplen2 + 3 < sizeof smallbuf2)
		tmpbuf2 = smallbuf2;
	    else
		Newx(tmpbuf2, tmplen2 + 3, char);
	    tmpbuf[0] = tmpbuf2[0] = '_';
	    tmpbuf[1] = tmpbuf2[1] = '<';
	    memcpy(tmpbuf + 2, cf, ++tmplen);
	    memcpy(tmpbuf2 + 2, s, ++tmplen2);
	    ++tmplen; ++tmplen2;
	    gvp = (GV**)hv_fetch(PL_defstash, tmpbuf, tmplen, FALSE);
	    if (gvp) {
		gv2 = *(GV**)hv_fetch(PL_defstash, tmpbuf2, tmplen2, TRUE);
		if (!isGV(gv2))
		    gv_init(gv2, PL_defstash, tmpbuf2, tmplen2, FALSE);
		/* adjust ${"::_<newfilename"} to store the new file name */
		GvSV(gv2) = newSVpvn(tmpbuf2 + 2, tmplen2 - 2);
		GvHV(gv2) = (HV*)SvREFCNT_inc(GvHV(*gvp));
		GvAV(gv2) = (AV*)SvREFCNT_inc(GvAV(*gvp));
	    }
	    if (tmpbuf != smallbuf) Safefree(tmpbuf);
	    if (tmpbuf2 != smallbuf2) Safefree(tmpbuf2);
	}
#endif
	CopFILE_free(PL_curcop);
	CopFILE_set(PL_curcop, s);
    }
    *t = ch;
    CopLINE_set(PL_curcop, atoi(n)-1);
}

#else /* 5.10 */

void
PFDR_incline(pTHX_ const char *s)
{
    dVAR;
    const char *t;
    const char *n;
    const char *e;

    CopLINE_inc(PL_curcop);
    if (*s++ != '#')
	return;
    while (SPACE_OR_TAB(*s))
	s++;
    if (strnEQ(s, "line", 4))
	s += 4;
    else
	return;
    if (SPACE_OR_TAB(*s))
	s++;
    else
	return;
    while (SPACE_OR_TAB(*s))
	s++;
    if (!isDIGIT(*s))
	return;

    n = s;
    while (isDIGIT(*s))
	s++;
    while (SPACE_OR_TAB(*s))
	s++;
    if (*s == '"' && (t = strchr(s+1, '"'))) {
	s++;
	e = t + 1;
    }
    else {
	t = s;
	while (!isSPACE(*t))
	    t++;
	e = t;
    }
    while (SPACE_OR_TAB(*e) || *e == '\r' || *e == '\f')
	e++;
    if (*e != '\n' && *e != '\0')
	return;		/* false alarm */

    if (t - s > 0) {
	const STRLEN len = t - s;
#ifndef USE_ITHREADS
	SV *const temp_sv = CopFILESV(PL_curcop);
	const char *cf;
	STRLEN tmplen;

	if (temp_sv) {
	    cf = SvPVX(temp_sv);
	    tmplen = SvCUR(temp_sv);
	} else {
	    cf = NULL;
	    tmplen = 0;
	}

	if (tmplen > 7 && strnEQ(cf, "(eval ", 6)) {
	    /* must copy *{"::_<(eval N)[oldfilename:L]"}
	     * to *{"::_<newfilename"} */
	    /* However, the long form of evals is only turned on by the
	       debugger - usually they're "(eval %lu)" */
	    char smallbuf[128];
	    char *tmpbuf;
	    GV **gvp;
	    STRLEN tmplen2 = len;
	    if (tmplen + 2 <= sizeof smallbuf)
		tmpbuf = smallbuf;
	    else
		Newx(tmpbuf, tmplen + 2, char);
	    tmpbuf[0] = '_';
	    tmpbuf[1] = '<';
	    memcpy(tmpbuf + 2, cf, tmplen);
	    tmplen += 2;
	    gvp = (GV**)hv_fetch(PL_defstash, tmpbuf, tmplen, FALSE);
	    if (gvp) {
		char *tmpbuf2;
		GV *gv2;

		if (tmplen2 + 2 <= sizeof smallbuf)
		    tmpbuf2 = smallbuf;
		else
		    Newx(tmpbuf2, tmplen2 + 2, char);

		if (tmpbuf2 != smallbuf || tmpbuf != smallbuf) {
		    /* Either they malloc'd it, or we malloc'd it,
		       so no prefix is present in ours.  */
		    tmpbuf2[0] = '_';
		    tmpbuf2[1] = '<';
		}

		memcpy(tmpbuf2 + 2, s, tmplen2);
		tmplen2 += 2;

		gv2 = *(GV**)hv_fetch(PL_defstash, tmpbuf2, tmplen2, TRUE);
		if (!isGV(gv2)) {
		    gv_init(gv2, PL_defstash, tmpbuf2, tmplen2, FALSE);
		    /* adjust ${"::_<newfilename"} to store the new file name */
		    GvSV(gv2) = newSVpvn(tmpbuf2 + 2, tmplen2 - 2);
		    GvHV(gv2) = (HV*)SvREFCNT_inc(GvHV(*gvp));
		    GvAV(gv2) = (AV*)SvREFCNT_inc(GvAV(*gvp));
		}

		if (tmpbuf2 != smallbuf) Safefree(tmpbuf2);
	    }
	    if (tmpbuf != smallbuf) Safefree(tmpbuf);
	}
#endif
	CopFILE_free(PL_curcop);
	CopFILE_setn(PL_curcop, s, len);
    }
    CopLINE_set(PL_curcop, atoi(n)-1);
}

#endif /* 5.10 */

/* op.c */

void
PFDR_no_bareword_allowed(pTHX_ const OP *o)
{
    qerror(Perl_mess(aTHX_
		     "Bareword \"%"SVf"\" not allowed while \"strict subs\" in use",
		     SVfARG(cSVOPo_sv)));
}

/* toke.c */

char *
PFDR_skipspace(pTHX_ register char *s)
{
    if (PL_lex_formbrack && PL_lex_brackets <= PL_lex_formbrack) {
	while (s < PL_bufend && SPACE_OR_TAB(*s))
	    s++;
	return s;
    }
    for (;;) {
	STRLEN prevlen;
	SSize_t oldprevlen, oldoldprevlen;
	SSize_t oldloplen = 0, oldunilen = 0;
	while (s < PL_bufend && isSPACE(*s)) {
	    if (*s++ == '\n' && PL_in_eval && !PL_rsfp)
		incline(s);
	}

	/* comment */
	if (s < PL_bufend && *s == '#') {
	    while (s < PL_bufend && *s != '\n')
		s++;
	    if (s < PL_bufend) {
		s++;
		if (PL_in_eval && !PL_rsfp) {
		    incline(s);
		    continue;
		}
	    }
	}

	/* only continue to recharge the buffer if we're at the end
	 * of the buffer, we're not reading from a source filter, and
	 * we're in normal lexing mode
	 */
	if (s < PL_bufend || !PL_rsfp || PL_sublex_info.sub_inwhat ||
		PL_lex_state == LEX_FORMLINE)
	    return s;

	/* try to recharge the buffer */
	if ((s = filter_gets(PL_linestr, PL_rsfp,
			     (prevlen = SvCUR(PL_linestr)))) == Nullch)
	{
	    /* end of file.  Add on the -p or -n magic */
	    if (PL_minus_p) {
		sv_setpv(PL_linestr,
			 ";}continue{print or die qq(-p destination: $!\\n);}");
		PL_minus_n = PL_minus_p = 0;
	    }
	    else if (PL_minus_n) {
		sv_setpvn(PL_linestr, ";}", 2);
		PL_minus_n = 0;
	    }
	    else
		sv_setpvn(PL_linestr,";", 1);

	    /* reset variables for next time we lex */
	    PL_oldoldbufptr = PL_oldbufptr = PL_bufptr = s = PL_linestart
		= SvPVX(PL_linestr);
	    PL_bufend = SvPVX(PL_linestr) + SvCUR(PL_linestr);
	    PL_last_lop = PL_last_uni = Nullch;

	    /* Close the filehandle.  Could be from -P preprocessor,
	     * STDIN, or a regular file.  If we were reading code from
	     * STDIN (because the commandline held no -e or filename)
	     * then we don't close it, we reset it so the code can
	     * read from STDIN too.
	     */

	    if (PL_preprocess && !PL_in_eval)
		(void)PerlProc_pclose(PL_rsfp);
	    else if ((PerlIO*)PL_rsfp == PerlIO_stdin())
		PerlIO_clearerr(PL_rsfp);
	    else
		(void)PerlIO_close(PL_rsfp);
	    PL_rsfp = Nullfp;
	    return s;
	}

	/* not at end of file, so we only read another line */
	/* make corresponding updates to old pointers, for yyerror() */
	oldprevlen = PL_oldbufptr - PL_bufend;
	oldoldprevlen = PL_oldoldbufptr - PL_bufend;
	if (PL_last_uni)
	    oldunilen = PL_last_uni - PL_bufend;
	if (PL_last_lop)
	    oldloplen = PL_last_lop - PL_bufend;
	PL_linestart = PL_bufptr = s + prevlen;
	PL_bufend = s + SvCUR(PL_linestr);
	s = PL_bufptr;
	PL_oldbufptr = s + oldprevlen;
	PL_oldoldbufptr = s + oldoldprevlen;
	if (PL_last_uni)
	    PL_last_uni = s + oldunilen;
	if (PL_last_lop)
	    PL_last_lop = s + oldloplen;
	incline(s);

	/* debugger active and we're not compiling the debugger code,
	 * so store the line into the debugger's array of lines
	 */
	if (PERLDB_LINE && PL_curstash != PL_debstash) {
#if PERL_VERSION > 8 || PERL_SUBVERSION > 8
	    update_debugger_info(NULL, PL_bufptr, PL_bufend - PL_bufptr);
#else /* 5.8.8 or less */
	    SV * const sv = NEWSV(85,0);

	    sv_upgrade(sv, SVt_PVMG);
	    sv_setpvn(sv,PL_bufptr,PL_bufend-PL_bufptr);
            (void)SvIOK_on(sv);
            SvIV_set(sv, 0);
	    av_store(CopFILEAV(PL_curcop),(I32)CopLINE(PL_curcop),sv);
#endif /* 5.8.8 or less */
	}
    }
}

#if PERL_VERSION > 8 || PERL_SUBVERSION > 8

void
PFDR_update_debugger_info(pTHX_ SV *orig_sv, const char *buf, STRLEN len)
{
    AV *av = CopFILEAVx(PL_curcop);
    if (av) {
	SV * const sv = newSV_type(SVt_PVMG);
	if (orig_sv)
	    sv_setsv(sv, orig_sv);
	else
	    sv_setpvn(sv, buf, len);
	(void)SvIOK_on(sv);
	SvIV_set(sv, 0);
	av_store(av, (I32)CopLINE(PL_curcop), sv);
    }
}

#endif /* 5.8.9 or 5.10 */

/* functions that are extern, but not exported by perl dll */

#ifdef NEED_EXPLICIT_EXPORTS

/* op.c */

OP *
PFDR_append_elem(pTHX_ I32 type, OP *first, OP *last)
{
    if (!first)
	return last;

    if (!last)
	return first;

    if (first->op_type != (unsigned)type
	|| (type == OP_LIST && (first->op_flags & OPf_PARENS)))
    {
	return newLISTOP(type, 0, first, last);
    }

    if (first->op_flags & OPf_KIDS)
	((LISTOP*)first)->op_last->op_sibling = last;
    else {
	first->op_flags |= OPf_KIDS;
	((LISTOP*)first)->op_first = last;
    }
    ((LISTOP*)first)->op_last = last;
    return first;
}

OP *
PFDR_convert(pTHX_ I32 type, I32 flags, OP *o)
{
    if (!o || o->op_type != OP_LIST)
	o = newLISTOP(OP_LIST, 0, o, Nullop);
    else
	o->op_flags &= ~OPf_WANT;

    if (!(PL_opargs[type] & OA_MARK))
	op_null(cLISTOPo->op_first);

    o->op_type = (OPCODE)type;
    o->op_ppaddr = PL_ppaddr[type];
    o->op_flags |= flags;

    o = CHECKOP(type, o);
    if (o->op_type != (unsigned)type)
	return o;

    return fold_constants(o);
}

#if PERL_VERSION > 8

/* pp_ctl.c */

PERL_CONTEXT *
PFDR_create_eval_scope(pTHX_ U32 flags)
{
    PERL_CONTEXT *cx;
    const I32 gimme = GIMME_V;
	
    ENTER;
    SAVETMPS;

    PUSHBLOCK(cx, (CXt_EVAL|CXp_TRYBLOCK), PL_stack_sp);
    PUSHEVAL(cx, 0, 0);

    PL_in_eval = EVAL_INEVAL;
    if (flags & G_KEEPERR)
	PL_in_eval |= EVAL_KEEPERR;
    else
	sv_setpvn(ERRSV,"",0);
    if (flags & G_FAKINGEVAL) {
	PL_eval_root = PL_op; /* Only needed so that goto works right. */
    }
    return cx;
}
    
void
PFDR_delete_eval_scope(pTHX)
{
    SV **newsp;
    PMOP *newpm;
    I32 gimme;
    register PERL_CONTEXT *cx;
    I32 optype;
	
    POPBLOCK(cx,newpm);
    POPEVAL(cx);
    PL_curpm = newpm;
    LEAVE;
    PERL_UNUSED_VAR(newsp);
    PERL_UNUSED_VAR(gimme);
    PERL_UNUSED_VAR(optype);
}

#endif /* 5.8 */

/* toke.c */

void
PFDR_deprecate_old(pTHX_ char *s)
{
    /* This function should NOT be called for any new deprecated warnings */
    /* Use Perl_deprecate instead                                         */
    /*                                                                    */
    /* It is here to maintain backward compatibility with the pre-5.8     */
    /* warnings category hierarchy. The "deprecated" category used to     */
    /* live under the "syntax" category. It is now a top-level category   */
    /* in its own right.                                                  */

    if (ckWARN2(WARN_DEPRECATED, WARN_SYNTAX))
	Perl_warner(aTHX_ packWARN2(WARN_DEPRECATED, WARN_SYNTAX),
			"Use of %s is deprecated", s);
}

#if PERL_VERSION == 8

OP *
PFDR_fold_constants(pTHX_ register OP *o)
{
    register OP *curop;
    I32 type = o->op_type;
    SV *sv;

    if (PL_opargs[type] & OA_RETSCALAR)
	scalar(o);
    if (PL_opargs[type] & OA_TARGET && !o->op_targ)
	o->op_targ = pad_alloc(type, SVs_PADTMP);

    /* integerize op, unless it happens to be C<-foo>.
     * XXX should pp_i_negate() do magic string negation instead? */
    if ((PL_opargs[type] & OA_OTHERINT) && (PL_hints & HINT_INTEGER)
	&& !(type == OP_NEGATE && cUNOPo->op_first->op_type == OP_CONST
	     && (cUNOPo->op_first->op_private & OPpCONST_BARE)))
    {
	o->op_ppaddr = PL_ppaddr[type = ++(o->op_type)];
    }

    if (!(PL_opargs[type] & OA_FOLDCONST))
	goto nope;

    switch (type) {
    case OP_NEGATE:
	/* XXX might want a ck_negate() for this */
	cUNOPo->op_first->op_private &= ~OPpCONST_STRICT;
	break;
    case OP_UCFIRST:
    case OP_LCFIRST:
    case OP_UC:
    case OP_LC:
    case OP_SLT:
    case OP_SGT:
    case OP_SLE:
    case OP_SGE:
    case OP_SCMP:
	/* XXX what about the numeric ops? */
	if (PL_hints & HINT_LOCALE)
	    goto nope;
    }

    if (PL_error_count)
	goto nope;		/* Don't try to run w/ errors */

    for (curop = LINKLIST(o); curop != o; curop = LINKLIST(curop)) {
	if ((curop->op_type != OP_CONST ||
	     (curop->op_private & OPpCONST_BARE)) &&
	    curop->op_type != OP_LIST &&
	    curop->op_type != OP_SCALAR &&
	    curop->op_type != OP_NULL &&
	    curop->op_type != OP_PUSHMARK)
	{
	    goto nope;
	}
    }

    curop = LINKLIST(o);
    o->op_next = 0;
    PL_op = curop;
    CALLRUNOPS(aTHX);
    sv = *(PL_stack_sp--);
    if (o->op_targ && sv == PAD_SV(o->op_targ))	/* grab pad temp? */
	pad_swipe(o->op_targ,  FALSE);
    else if (SvTEMP(sv)) {			/* grab mortal temp? */
	(void)SvREFCNT_inc(sv);
	SvTEMP_off(sv);
    }
    op_free(o);
    if (type == OP_RV2GV)
	return newGVOP(OP_GV, 0, (GV*)sv);
    return newSVOP(OP_CONST, 0, sv);

  nope:
    return o;
}

#else /* 5.10 */

OP *
PFDR_fold_constants(pTHX_ register OP *o)
{
    dVAR;
    register OP *curop;
    OP *newop;
    VOL I32 type = o->op_type;
    SV * VOL sv = NULL;
    int ret = 0;
    I32 oldscope;
    OP *old_next;
    SV * const oldwarnhook = PL_warnhook;
    SV * const olddiehook  = PL_diehook;
    dJMPENV;

    if (PL_opargs[type] & OA_RETSCALAR)
	scalar(o);
    if (PL_opargs[type] & OA_TARGET && !o->op_targ)
	o->op_targ = pad_alloc(type, SVs_PADTMP);

    /* integerize op, unless it happens to be C<-foo>.
     * XXX should pp_i_negate() do magic string negation instead? */
    if ((PL_opargs[type] & OA_OTHERINT) && (PL_hints & HINT_INTEGER)
	&& !(type == OP_NEGATE && cUNOPo->op_first->op_type == OP_CONST
	     && (cUNOPo->op_first->op_private & OPpCONST_BARE)))
    {
	o->op_ppaddr = PL_ppaddr[type = ++(o->op_type)];
    }

    if (!(PL_opargs[type] & OA_FOLDCONST))
	goto nope;

    switch (type) {
    case OP_NEGATE:
	/* XXX might want a ck_negate() for this */
	cUNOPo->op_first->op_private &= ~OPpCONST_STRICT;
	break;
    case OP_UCFIRST:
    case OP_LCFIRST:
    case OP_UC:
    case OP_LC:
    case OP_SLT:
    case OP_SGT:
    case OP_SLE:
    case OP_SGE:
    case OP_SCMP:
	/* XXX what about the numeric ops? */
	if (PL_hints & HINT_LOCALE)
	    goto nope;
    }

    if (PL_parser && PL_parser->error_count)
	goto nope;		/* Don't try to run w/ errors */

    for (curop = LINKLIST(o); curop != o; curop = LINKLIST(curop)) {
	const OPCODE type = curop->op_type;
	if ((type != OP_CONST || (curop->op_private & OPpCONST_BARE)) &&
	    type != OP_LIST &&
	    type != OP_SCALAR &&
	    type != OP_NULL &&
	    type != OP_PUSHMARK)
	{
	    goto nope;
	}
    }

    curop = LINKLIST(o);
    old_next = o->op_next;
    o->op_next = 0;
    PL_op = curop;

    oldscope = PL_scopestack_ix;
    create_eval_scope(G_FAKINGEVAL);

    PL_warnhook = PERL_WARNHOOK_FATAL;
    PL_diehook  = NULL;
    JMPENV_PUSH(ret);

    switch (ret) {
    case 0:
	CALLRUNOPS(aTHX);
	sv = *(PL_stack_sp--);
	if (o->op_targ && sv == PAD_SV(o->op_targ))	/* grab pad temp? */
	    pad_swipe(o->op_targ,  FALSE);
	else if (SvTEMP(sv)) {			/* grab mortal temp? */
	    SvREFCNT_inc_simple_void(sv);
	    SvTEMP_off(sv);
	}
	break;
    case 3:
	/* Something tried to die.  Abandon constant folding.  */
	/* Pretend the error never happened.  */
	sv_setpvn(ERRSV,"",0);
	o->op_next = old_next;
	break;
    default:
	JMPENV_POP;
	/* Don't expect 1 (setjmp failed) or 2 (something called my_exit)  */
	PL_warnhook = oldwarnhook;
	PL_diehook  = olddiehook;
	/* XXX note that this croak may fail as we've already blown away
	 * the stack - eg any nested evals */
	Perl_croak(aTHX_ "panic: fold_constants JMPENV_PUSH returned %d", ret);
    }
    JMPENV_POP;
    PL_warnhook = oldwarnhook;
    PL_diehook  = olddiehook;

    if (PL_scopestack_ix > oldscope)
	delete_eval_scope();

    if (ret)
	goto nope;

#ifndef PERL_MAD
    op_free(o);
#endif
    assert(sv);
    if (type == OP_RV2GV)
	newop = newGVOP(OP_GV, 0, (GV*)sv);
    else
	newop = newSVOP(OP_CONST, 0, (SV*)sv);
    op_getmad(o,newop,'f');
    return newop;

 nope:
    return o;
}

#endif /* 5.10 */

#if PERL_VERSION == 8

OP *
PFDR_linklist(pTHX_ OP *o)
{

    if (o->op_next)
	return o->op_next;

    /* establish postfix order */
    if (cUNOPo->op_first) {
        register OP *kid;
	o->op_next = LINKLIST(cUNOPo->op_first);
	for (kid = cUNOPo->op_first; kid; kid = kid->op_sibling) {
	    if (kid->op_sibling)
		kid->op_next = LINKLIST(kid->op_sibling);
	    else
		kid->op_next = o;
	}
    }
    else
	o->op_next = o;

    return o->op_next;
}

#else /* 5.10 */

OP *
PFDR_linklist(pTHX_ OP *o)
{
    OP *first;

    if (o->op_next)
	return o->op_next;

    /* establish postfix order */
    first = cUNOPo->op_first;
    if (first) {
        register OP *kid;
	o->op_next = LINKLIST(first);
	kid = first;
	for (;;) {
	    if (kid->op_sibling) {
		kid->op_next = LINKLIST(kid->op_sibling);
		kid = kid->op_sibling;
	    } else {
		kid->op_next = o;
		break;
	    }
	}
    }
    else
	o->op_next = o;

    return o->op_next;
}

#endif /* 5.10 */

/* pad.c */

PADOFFSET
PFDR_pad_alloc(pTHX_ I32 optype, U32 tmptype)
{
    SV *sv;
    I32 retval;

    ASSERT_CURPAD_ACTIVE("pad_alloc");

    if (AvARRAY(PL_comppad) != PL_curpad)
	Perl_croak(aTHX_ "panic: pad_alloc");
    if (PL_pad_reset_pending)
	pad_reset();
    if (tmptype & SVs_PADMY) {
#if PERL_VERSION == 8
	do {
#endif
	    sv = *av_fetch(PL_comppad, AvFILLp(PL_comppad) + 1, TRUE);
#if PERL_VERSION == 8
	} while (SvPADBUSY(sv));		/* need a fresh one */
#endif
	retval = AvFILLp(PL_comppad);
    }
    else {
	SV * const * const names = AvARRAY(PL_comppad_name);
        const SSize_t names_fill = AvFILLp(PL_comppad_name);
	for (;;) {
	    /*
	     * "foreach" index vars temporarily become aliases to non-"my"
	     * values.  Thus we must skip, not just pad values that are
	     * marked as current pad values, but also those with names.
	     */
	    /* HVDS why copy to sv here? we don't seem to use it */
	    if (++PL_padix <= names_fill &&
		   (sv = names[PL_padix]) && sv != &PL_sv_undef)
		continue;
	    sv = *av_fetch(PL_comppad, PL_padix, TRUE);
	    if (!(SvFLAGS(sv) & (SVs_PADTMP | SVs_PADMY)) &&
		!IS_PADGV(sv) && !IS_PADCONST(sv))
		break;
	}
	retval = PL_padix;
    }
    SvFLAGS(sv) |= tmptype;
    PL_curpad = AvARRAY(PL_comppad);

    DEBUG_X(PerlIO_printf(Perl_debug_log,
	  "Pad 0x%"UVxf"[0x%"UVxf"] alloc:   %ld for %s\n",
	  PTR2UV(PL_comppad), PTR2UV(PL_curpad), (long) retval,
	  PL_op_name[optype]));
    return (PADOFFSET)retval;
}

void
PFDR_pad_reset(pTHX)
{
#ifdef USE_BROKEN_PAD_RESET
    if (AvARRAY(PL_comppad) != PL_curpad)
	Perl_croak(aTHX_ "panic: pad_reset curpad");

    DEBUG_X(PerlIO_printf(Perl_debug_log,
	    "Pad 0x%"UVxf"[0x%"UVxf"] reset:     padix %ld -> %ld",
	    PTR2UV(PL_comppad), PTR2UV(PL_curpad),
		(long)PL_padix, (long)PL_padix_floor
	    )
    );

    if (!PL_tainting) {	/* Can't mix tainted and non-tainted temporaries. */
        register I32 po;
	for (po = AvMAX(PL_comppad); po > PL_padix_floor; po--) {
	    if (PL_curpad[po] && !SvIMMORTAL(PL_curpad[po]))
		SvPADTMP_off(PL_curpad[po]);
	}
	PL_padix = PL_padix_floor;
    }
#endif
    PL_pad_reset_pending = FALSE;
}

void
PFDR_pad_swipe(pTHX_ PADOFFSET po, bool refadjust)
{
    ASSERT_CURPAD_LEGAL("pad_swipe");
    if (!PL_curpad)
	return;
    if (AvARRAY(PL_comppad) != PL_curpad)
	Perl_croak(aTHX_ "panic: pad_swipe curpad");
    if (!po)
	Perl_croak(aTHX_ "panic: pad_swipe po");

    DEBUG_X(PerlIO_printf(Perl_debug_log,
		"Pad 0x%"UVxf"[0x%"UVxf"] swipe:   %ld\n",
		PTR2UV(PL_comppad), PTR2UV(PL_curpad), (long)po));

    if (PL_curpad[po])
	SvPADTMP_off(PL_curpad[po]);
    if (refadjust)
	SvREFCNT_dec(PL_curpad[po]);


    /* if pad tmps aren't shared between ops, then there's no need to
     * create a new tmp when an existing op is freed */
#ifdef USE_BROKEN_PAD_RESET
    PL_curpad[po] = NEWSV(1107,0);
    SvPADTMP_on(PL_curpad[po]);
#else
    PL_curpad[po] = &PL_sv_undef;
#endif
    if ((I32)po < PL_padix)
	PL_padix = po - 1;
}

/* pp_ctl.c */

void
PFDR_qerror(pTHX_ SV *err)
{
    if (PL_in_eval)
	sv_catsv(ERRSV, err);
    else if (PL_errors)
	sv_catsv(PL_errors, err);
    else
	Perl_warn(aTHX_ "%"SVf, err);
    ++PL_error_count;
}

/* op.c */

OP *
PFDR_scalar(pTHX_ OP *o)
{
    OP *kid;

    /* assumes no premature commitment */
    if (!o || PL_error_count || (o->op_flags & OPf_WANT)
	 || o->op_type == OP_RETURN)
    {
	return o;
    }

    o->op_flags = (o->op_flags & ~OPf_WANT) | OPf_WANT_SCALAR;

    switch (o->op_type) {
    case OP_REPEAT:
	scalar(cBINOPo->op_first);
	break;
    case OP_OR:
    case OP_AND:
    case OP_COND_EXPR:
	for (kid = cUNOPo->op_first->op_sibling; kid; kid = kid->op_sibling)
	    scalar(kid);
	break;
    case OP_SPLIT:
	if ((kid = cLISTOPo->op_first) && kid->op_type == OP_PUSHRE) {
	    if (!kPMOP->op_pmreplroot)
		deprecate_old("implicit split to @_");
	}
	/* FALL THROUGH */
    case OP_MATCH:
    case OP_QR:
    case OP_SUBST:
    case OP_NULL:
    default:
	if (o->op_flags & OPf_KIDS) {
	    for (kid = cUNOPo->op_first; kid; kid = kid->op_sibling)
		scalar(kid);
	}
	break;
    case OP_LEAVE:
    case OP_LEAVETRY:
	kid = cLISTOPo->op_first;
	scalar(kid);
	while ((kid = kid->op_sibling)) {
	    if (kid->op_sibling)
		scalarvoid(kid);
	    else
		scalar(kid);
	}
	WITH_THR(PL_curcop = &PL_compiling);
	break;
    case OP_SCOPE:
    case OP_LINESEQ:
    case OP_LIST:
	for (kid = cLISTOPo->op_first; kid; kid = kid->op_sibling) {
	    if (kid->op_sibling)
		scalarvoid(kid);
	    else
		scalar(kid);
	}
	WITH_THR(PL_curcop = &PL_compiling);
	break;
    case OP_SORT:
	if (ckWARN(WARN_VOID))
	    Perl_warner(aTHX_ packWARN(WARN_VOID), "Useless use of sort in scalar context");
    }
    return o;
}

OP *
PFDR_scalarkids(pTHX_ OP *o)
{
    if (o && o->op_flags & OPf_KIDS) {
        OP *kid;
	for (kid = cLISTOPo->op_first; kid; kid = kid->op_sibling)
	    scalar(kid);
    }
    return o;
}

OP *
PFDR_scalarvoid(pTHX_ OP *o)
{
    OP *kid;
    const char* useless = 0;
    SV* sv;
    U8 want;

    if (o->op_type == OP_NEXTSTATE
	|| o->op_type == OP_SETSTATE
	|| o->op_type == OP_DBSTATE
	|| (o->op_type == OP_NULL && (o->op_targ == OP_NEXTSTATE
				      || o->op_targ == OP_SETSTATE
				      || o->op_targ == OP_DBSTATE)))
	PL_curcop = (COP*)o;		/* for warning below */

    /* assumes no premature commitment */
    want = o->op_flags & OPf_WANT;
    if ((want && want != OPf_WANT_SCALAR) || PL_error_count
	 || o->op_type == OP_RETURN)
    {
	return o;
    }

    if ((o->op_private & OPpTARGET_MY)
	&& (PL_opargs[o->op_type] & OA_TARGLEX))/* OPp share the meaning */
    {
	return scalar(o);			/* As if inside SASSIGN */
    }

    o->op_flags = (o->op_flags & ~OPf_WANT) | OPf_WANT_VOID;

    switch (o->op_type) {
    default:
	if (!(PL_opargs[o->op_type] & OA_FOLDCONST))
	    break;
	/* FALL THROUGH */
    case OP_REPEAT:
	if (o->op_flags & OPf_STACKED)
	    break;
	goto func_ops;
    case OP_SUBSTR:
	if (o->op_private == 4)
	    break;
	/* FALL THROUGH */
    case OP_GVSV:
    case OP_WANTARRAY:
    case OP_GV:
    case OP_PADSV:
    case OP_PADAV:
    case OP_PADHV:
    case OP_PADANY:
    case OP_AV2ARYLEN:
    case OP_REF:
    case OP_REFGEN:
    case OP_SREFGEN:
    case OP_DEFINED:
    case OP_HEX:
    case OP_OCT:
    case OP_LENGTH:
    case OP_VEC:
    case OP_INDEX:
    case OP_RINDEX:
    case OP_SPRINTF:
    case OP_AELEM:
    case OP_AELEMFAST:
    case OP_ASLICE:
    case OP_HELEM:
    case OP_HSLICE:
    case OP_UNPACK:
    case OP_PACK:
    case OP_JOIN:
    case OP_LSLICE:
    case OP_ANONLIST:
    case OP_ANONHASH:
    case OP_SORT:
    case OP_REVERSE:
    case OP_RANGE:
    case OP_FLIP:
    case OP_FLOP:
    case OP_CALLER:
    case OP_FILENO:
    case OP_EOF:
    case OP_TELL:
    case OP_GETSOCKNAME:
    case OP_GETPEERNAME:
    case OP_READLINK:
    case OP_TELLDIR:
    case OP_GETPPID:
    case OP_GETPGRP:
    case OP_GETPRIORITY:
    case OP_TIME:
    case OP_TMS:
    case OP_LOCALTIME:
    case OP_GMTIME:
    case OP_GHBYNAME:
    case OP_GHBYADDR:
    case OP_GHOSTENT:
    case OP_GNBYNAME:
    case OP_GNBYADDR:
    case OP_GNETENT:
    case OP_GPBYNAME:
    case OP_GPBYNUMBER:
    case OP_GPROTOENT:
    case OP_GSBYNAME:
    case OP_GSBYPORT:
    case OP_GSERVENT:
    case OP_GPWNAM:
    case OP_GPWUID:
    case OP_GGRNAM:
    case OP_GGRGID:
    case OP_GETLOGIN:
    case OP_PROTOTYPE:
      func_ops:
	if (!(o->op_private & (OPpLVAL_INTRO|OPpOUR_INTRO)))
	    useless = OP_DESC(o);
	break;

#if PERL_VERSION > 8
    case OP_NOT:
       kid = cUNOPo->op_first;
       if (kid->op_type != OP_MATCH && kid->op_type != OP_SUBST &&
           kid->op_type != OP_TRANS) {
	        goto func_ops;
       }
       useless = "negative pattern binding (!~)";
       break;
#endif

    case OP_RV2GV:
    case OP_RV2SV:
    case OP_RV2AV:
    case OP_RV2HV:
	if (!(o->op_private & (OPpLVAL_INTRO|OPpOUR_INTRO)) &&
		(!o->op_sibling || o->op_sibling->op_type != OP_READLINE))
	    useless = "a variable";
	break;

    case OP_CONST:
	sv = cSVOPo_sv;
	if (cSVOPo->op_private & OPpCONST_STRICT)
	    no_bareword_allowed(o);
	else {
	    if (ckWARN(WARN_VOID)) {
		useless = "a constant";
#if PERL_VERSION > 8
		if (o->op_private & OPpCONST_ARYBASE)
		    useless = NULL;
#endif
#if PERL_VERSION > 8 || PERL_SUBVERSION > 3
		/* don't warn on optimised away booleans, eg 
		 * use constant Foo, 5; Foo || print; */
		if (cSVOPo->op_private & OPpCONST_SHORTCIRCUIT)
		    useless = 0;
                else
#endif
		/* the constants 0 and 1 are permitted as they are
		   conventionally used as dummies in constructs like
		        1 while some_condition_with_side_effects;  */
		if (SvNIOK(sv) && (SvNV(sv) == 0.0 || SvNV(sv) == 1.0))
		    useless = 0;
		else if (SvPOK(sv)) {
                  /* perl4's way of mixing documentation and code
                     (before the invention of POD) was based on a
                     trick to mix nroff and perl code. The trick was
                     built upon these three nroff macros being used in
                     void context. The pink camel has the details in
                     the script wrapman near page 319. */
		    if (strnEQ(SvPVX_const(sv), "di", 2) ||
			strnEQ(SvPVX_const(sv), "ds", 2) ||
			strnEQ(SvPVX_const(sv), "ig", 2))
			    useless = 0;
		}
	    }
	}
	op_null(o);		/* don't execute or even remember it */
	break;

    case OP_POSTINC:
	o->op_type = OP_PREINC;		/* pre-increment is faster */
	o->op_ppaddr = PL_ppaddr[OP_PREINC];
	break;

    case OP_POSTDEC:
	o->op_type = OP_PREDEC;		/* pre-decrement is faster */
	o->op_ppaddr = PL_ppaddr[OP_PREDEC];
	break;

    case OP_I_POSTINC:
	o->op_type = OP_I_PREINC;	/* pre-increment is faster */
	o->op_ppaddr = PL_ppaddr[OP_I_PREINC];
	break;

    case OP_I_POSTDEC:
	o->op_type = OP_I_PREDEC;	/* pre-decrement is faster */
	o->op_ppaddr = PL_ppaddr[OP_I_PREDEC];
	break;

    case OP_OR:
    case OP_AND:
    case OP_COND_EXPR:
#if PERL_VERSION > 8
    case OP_DOR:
    case OP_ENTERGIVEN:
    case OP_ENTERWHEN:
#endif
	for (kid = cUNOPo->op_first->op_sibling; kid; kid = kid->op_sibling)
	    scalarvoid(kid);
	break;

    case OP_NULL:
	if (o->op_flags & OPf_STACKED)
	    break;
	/* FALL THROUGH */
    case OP_NEXTSTATE:
    case OP_DBSTATE:
    case OP_ENTERTRY:
    case OP_ENTER:
	if (!(o->op_flags & OPf_KIDS))
	    break;
	/* FALL THROUGH */
    case OP_SCOPE:
    case OP_LEAVE:
    case OP_LEAVETRY:
    case OP_LEAVELOOP:
    case OP_LINESEQ:
    case OP_LIST:
#if PERL_VERSION > 8
    case OP_LEAVEGIVEN:
    case OP_LEAVEWHEN:
#endif
	for (kid = cLISTOPo->op_first; kid; kid = kid->op_sibling)
	    scalarvoid(kid);
	break;
    case OP_ENTEREVAL:
	scalarkids(o);
	break;
    case OP_REQUIRE:
	/* all requires must return a boolean value */
	o->op_flags &= ~OPf_WANT;
	/* FALL THROUGH */
    case OP_SCALAR:
	return scalar(o);
    case OP_SPLIT:
	if ((kid = cLISTOPo->op_first) && kid->op_type == OP_PUSHRE) {
	    if (!kPMOP->op_pmreplroot)
		deprecate_old("implicit split to @_");
	}
	break;
    }
    if (useless && ckWARN(WARN_VOID))
	Perl_warner(aTHX_ packWARN(WARN_VOID), "Useless use of %s in void context", useless);
    return o;
}

#endif /* NEED_EXPLICIT_EXPORTS */
