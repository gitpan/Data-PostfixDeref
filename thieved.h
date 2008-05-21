/* 
 * Stuff I have blatantly stolen from the perl source. Who thought
 * having to explicitly export functions was a good idea???
 */

/* macros */

#ifdef MULTIPLICITY
EXTERN_C SV *Perl_Gsv_placeholder_ptr(pTHX);
#undef PL_sv_placeholder
#define PL_sv_placeholder (*Perl_Gsv_placeholder_ptr(NULL))
#endif

/* toke.c */

#if PERL_VERSION > 8    /* 5.10 moved all this stuff */

#define op_pmreplroot op_pmreplrootu.op_pmreplroot

#define PL_yychar               (PL_parser->yychar)

/* ppport.h tries to do these for us */
#undef PL_expect
#undef PL_copline
#undef PL_rsfp
#undef PL_rsfp_filters

/* XXX temporary backwards compatibility */
#define PL_lex_brackets		(PL_parser->lex_brackets)
#define PL_lex_brackstack	(PL_parser->lex_brackstack)
#define PL_lex_casemods		(PL_parser->lex_casemods)
#define PL_lex_casestack        (PL_parser->lex_casestack)
#define PL_lex_defer		(PL_parser->lex_defer)
#define PL_lex_dojoin		(PL_parser->lex_dojoin)
#define PL_lex_expect		(PL_parser->lex_expect)
#define PL_lex_formbrack        (PL_parser->lex_formbrack)
#define PL_lex_inpat		(PL_parser->lex_inpat)
#define PL_lex_inwhat		(PL_parser->lex_inwhat)
#define PL_lex_op		(PL_parser->lex_op)
#define PL_lex_repl		(PL_parser->lex_repl)
#define PL_lex_starts		(PL_parser->lex_starts)
#define PL_lex_stuff		(PL_parser->lex_stuff)
#define PL_multi_start		(PL_parser->multi_start)
#define PL_multi_open		(PL_parser->multi_open)
#define PL_multi_close		(PL_parser->multi_close)
#define PL_pending_ident        (PL_parser->pending_ident)
#define PL_preambled		(PL_parser->preambled)
#define PL_sublex_info		(PL_parser->sublex_info)
#define PL_linestr		(PL_parser->linestr)
#define PL_expect		(PL_parser->expect)
#define PL_copline		(PL_parser->copline)
#define PL_bufptr		(PL_parser->bufptr)
#define PL_oldbufptr		(PL_parser->oldbufptr)
#define PL_oldoldbufptr		(PL_parser->oldoldbufptr)
#define PL_linestart		(PL_parser->linestart)
#define PL_bufend		(PL_parser->bufend)
#define PL_last_uni		(PL_parser->last_uni)
#define PL_last_lop		(PL_parser->last_lop)
#define PL_last_lop_op		(PL_parser->last_lop_op)
#define PL_lex_state		(PL_parser->lex_state)
#define PL_rsfp			(PL_parser->rsfp)
#define PL_rsfp_filters		(PL_parser->rsfp_filters)
#define PL_in_my		(PL_parser->in_my)
#define PL_in_my_stash		(PL_parser->in_my_stash)
#define PL_tokenbuf		(PL_parser->tokenbuf)
#define PL_multi_end		(PL_parser->multi_end)
#define PL_error_count		(PL_parser->error_count)

#ifdef PERL_MAD
#  define PL_endwhite		(PL_parser->endwhite)
#  define PL_faketokens		(PL_parser->faketokens)
#  define PL_lasttoke		(PL_parser->lasttoke)
#  define PL_nextwhite		(PL_parser->nextwhite)
#  define PL_realtokenstart	(PL_parser->realtokenstart)
#  define PL_skipwhite		(PL_parser->skipwhite)
#  define PL_thisclose		(PL_parser->thisclose)
#  define PL_thismad		(PL_parser->thismad)
#  define PL_thisopen		(PL_parser->thisopen)
#  define PL_thisstuff		(PL_parser->thisstuff)
#  define PL_thistoken		(PL_parser->thistoken)
#  define PL_thiswhite		(PL_parser->thiswhite)
#  define PL_thiswhite		(PL_parser->thiswhite)
#  define PL_nexttoke		(PL_parser->nexttoke)
#  define PL_curforce		(PL_parser->curforce)
#else
#  define PL_nexttoke		(PL_parser->nexttoke)
#  define PL_nexttype		(PL_parser->nexttype)
#  define PL_nextval		(PL_parser->nextval)
#endif

#else /* 5.8 */

#define SVfARG(f) (f)

#endif /* 5.8 */

/* On MacOS, respect nonbreaking spaces */
#ifdef MACOS_TRADITIONAL
#define SPACE_OR_TAB(c) ((c)==' '||(c)=='\312'||(c)=='\t')
#else
#define SPACE_OR_TAB(c) ((c)==' '||(c)=='\t')
#endif

#define LEX_NORMAL		10 /* normal code (ie not within "...")     */
#define LEX_INTERPNORMAL	 9 /* code within a string, eg "$foo[$x+1]" */
#define LEX_INTERPCASEMOD	 8 /* expecting a \U, \Q or \E etc          */
#define LEX_INTERPPUSH		 7 /* starting a new sublex parse level     */
#define LEX_INTERPSTART		 6 /* expecting the start of a $var         */

				   /* at end of code, eg "$x" followed by:  */
#define LEX_INTERPEND		 5 /* ... eg not one of [, { or ->          */
#define LEX_INTERPENDMAYBE	 4 /* ... eg one of [, { or ->              */

#define LEX_INTERPCONCAT	 3 /* expecting anything, eg at start of
				        string or after \E, $foo, etc       */
#define LEX_INTERPCONST		 2 /* NOT USED */
#define LEX_FORMLINE		 1 /* expecting a format line               */
#define LEX_KNOWNEXT		 0 /* next token known; just return it      */

/* op.c */

#define CHECKOP(type,o) \
    ((PL_op_mask && PL_op_mask[type])                                   \
     ? ( op_free((OP*)o),                                       \
         Perl_croak(aTHX_ "'%s' trapped by operation mask", PL_op_desc[type]),  \
         Nullop )                                               \
     : CALL_FPTR(PL_check[type])(aTHX_ (OP*)o))

#define LINKLIST(o) ((o)->op_next ? (o)->op_next : linklist((OP*)o))

/* prototypes */

char *PFDR_filter_gets(pTHX_ register SV *sv, register PerlIO *fp, STRLEN append);
#undef filter_gets
#define filter_gets(sv, fp, append) PFDR_filter_gets(aTHX_ sv, fp, append)

#if PERL_VERSION == 8
void PFDR_incline(pTHX_ char *s);
#else
void PFDR_incline(pTHX_ const char *s);
#endif
#undef incline
#define incline(s) PFDR_incline(aTHX_ s)

void PFDR_no_bareword_allowed(pTHX_ const OP *o);
#undef no_bareword_allowed
#define no_bareword_allowed(o) PFDR_no_bareword_allowed(aTHX_ o)

char * PFDR_skipspace(pTHX_ register char *s);
#undef skipspace
#define skipspace(s) PFDR_skipspace(aTHX_ s)

#if PERL_VERSION > 8

void PFDR_update_debugger_info(pTHX_ SV *orig_sv, const char *buf, STRLEN len);
#undef update_debugger_info
#define update_debugger_info(orig_sv, buf, len) PFDR_update_debugger_info(aTHX_ orig_sv, buf, len)

#endif /* 5.10 */

#ifdef NEED_EXPLICIT_EXPORTS

/* for win32 and the like, these need implementations as well */

OP *PFDR_append_elem(pTHX_ I32 type, OP *first, OP *last);
#undef append_elem
#define append_elem(type, first, last) PFDR_append_elem(aTHX_ type, first, last)

OP *PFDR_convert(pTHX_ I32 type, I32 flags, OP *o);
#undef convert
#define convert(type, flags, o) PFDR_convert(aTHX_ type, flags, o)

#if PERL_VERSION > 8

PERL_CONTEXT *PFDR_create_eval_scope(pTHX_ U32 flags);
#undef create_eval_scope
#define create_eval_scope(flags) PFDR_create_eval_scope(aTHX_ flags)

void PFDR_delete_eval_scope(pTHX);
#undef delete_eval_scope
#define delete_eval_scope() PFDR_delete_eval_scope(aTHX)

#endif /* 5.10 */

void PFDR_deprecate_old(pTHX_ char *s);
#undef deprecate_old
#define deprecate_old(s) PFDR_deprecate_old(aTHX_ s)

OP *PFDR_fold_constants(pTHX_ register OP *o);
#undef fold_constants
#define fold_constants(o) PFDR_fold_constants(aTHX_ o)

OP *PFDR_linklist(pTHX_ OP *o);
#undef linklist
#define linklist(o) PFDR_linklist(aTHX_ o)

PADOFFSET PFDR_pad_alloc(pTHX_ I32 optype, U32 tmptype);
#undef pad_alloc
#define pad_alloc(optype, tmptype) PFDR_pad_alloc(aTHX_ optype, tmptype)

void PFDR_pad_reset(pTHX);
#undef pad_reset
#define pad_reset() PFDR_pad_reset(aTHX)

void PFDR_pad_swipe(pTHX_ PADOFFSET po, bool refadjust);
#undef pad_swipe
#define pad_swipe(po, refadjust) PFDR_pad_swipe(aTHX_ po, refadjust)

void PFDR_qerror(pTHX_ SV *err);
#undef qerror
#define qerror(err) PFDR_qerror(aTHX_ err)

OP *PFDR_scalar(pTHX_ OP *o);
#undef scalar
#define scalar(o) PFDR_scalar(aTHX_ o)

OP *PFDR_scalarkids(pTHX_ OP *o);
#undef scalarkids
#define scalarkids(o) PFDR_scalarkids(aTHX_ o)

OP *PFDR_scalarvoid(pTHX_ OP *o);
#undef scalarvoid
#define scalarvoid(o) PFDR_scalarvoid(aTHX_ o)

#else /* NEED_EXPLICIT_EXPORTS */

/* on platforms that don't need explicit exports from dlls, we can just
 * link directly to perl's implementation
 */

#undef append_elem
#define append_elem(type, first, last) Perl_append_elem(aTHX_ type, first, last)
#undef convert
#define convert(type, flags, o) Perl_convert(aTHX_ type, flags, o)
#undef deprecate_old
#define deprecate_old(s) Perl_deprecate_old(pTHX_ s)
#undef fold_constants
#define fold_constants(o) Perl_fold_constants(aTHX_ o)
#undef linklist
#define linklist(o) Perl_linklist(aTHX_ o)
#undef pad_alloc
#define pad_alloc(optype, tmptype) Perl_pad_alloc(aTHX_ optype, tmptype)
#undef pad_reset
#define pad_reset() Perl_pad_reset(aTHX)
#undef pad_swipe
#define pad_swipe(po, refadjust) Perl_pad_swipe(aTHX_ po, refadjust)
#undef qerror
#define qerror(err) Perl_qerror(aTHX_ err)
#undef scalar
#define scalar(o) Perl_scalar(aTHX_ o)
#undef scalarkids
#define scalarkids(o) Perl_scalarkids(aTHX_ o)
#undef scalarvoid
#define scalarvoid(o) Perl_scalarvoid(aTHX_ o)

#endif /* !NEED_EXPLICIT_EXPORTS */

