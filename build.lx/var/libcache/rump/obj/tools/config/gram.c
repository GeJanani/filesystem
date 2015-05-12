#if HAVE_NBTOOL_CONFIG_H
#include "nbtool_config.h"
#endif
#ifndef lint
#if __GNUC__ - 0 >= 4 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ >= 1)
__attribute__((__used__))
#endif
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#ifdef _LIBC
#include "namespace.h"
#endif
#include <stdlib.h>
#include <string.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)

#define YYPREFIX "yy"

#define YYPURE 0

#line 2 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
/*	$NetBSD: gram.y,v 1.46 2014/11/04 23:01:23 joerg Exp $	*/

/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Lawrence Berkeley Laboratories.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)gram.y	8.1 (Berkeley) 6/6/93
 */

#include <sys/cdefs.h>
__RCSID("$NetBSD: gram.y,v 1.46 2014/11/04 23:01:23 joerg Exp $");

#include <sys/types.h>
#include <sys/param.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "defs.h"
#include "sem.h"

#define	FORMAT(n) (((n).fmt == 8 && (n).val != 0) ? "0%llo" : \
    ((n).fmt == 16) ? "0x%llx" : "%lld")

#define	stop(s)	cfgerror(s), exit(1)

static	struct	config conf;	/* at most one active at a time */


/*
 * Allocation wrapper functions
 */
static void wrap_alloc(void *ptr, unsigned code);
static void wrap_continue(void);
static void wrap_cleanup(void);

/*
 * Allocation wrapper type codes
 */
#define WRAP_CODE_nvlist	1
#define WRAP_CODE_defoptlist	2
#define WRAP_CODE_loclist	3
#define WRAP_CODE_attrlist	4
#define WRAP_CODE_condexpr	5

/*
 * The allocation wrappers themselves
 */
#define DECL_ALLOCWRAP(t)	static struct t *wrap_mk_##t(struct t *arg)

DECL_ALLOCWRAP(nvlist);
DECL_ALLOCWRAP(defoptlist);
DECL_ALLOCWRAP(loclist);
DECL_ALLOCWRAP(attrlist);
DECL_ALLOCWRAP(condexpr);

/* allow shorter names */
#define wrap_mk_loc(p) wrap_mk_loclist(p)
#define wrap_mk_cx(p) wrap_mk_condexpr(p)

/*
 * Macros for allocating new objects
 */

/* old-style for struct nvlist */
#define	new0(n,s,p,i,x)	wrap_mk_nvlist(newnv(n, s, p, i, x))
#define	new_n(n)	new0(n, NULL, NULL, 0, NULL)
#define	new_nx(n, x)	new0(n, NULL, NULL, 0, x)
#define	new_ns(n, s)	new0(n, s, NULL, 0, NULL)
#define	new_si(s, i)	new0(NULL, s, NULL, i, NULL)
#define	new_nsi(n,s,i)	new0(n, s, NULL, i, NULL)
#define	new_np(n, p)	new0(n, NULL, p, 0, NULL)
#define	new_s(s)	new0(NULL, s, NULL, 0, NULL)
#define	new_p(p)	new0(NULL, NULL, p, 0, NULL)
#define	new_px(p, x)	new0(NULL, NULL, p, 0, x)
#define	new_sx(s, x)	new0(NULL, s, NULL, 0, x)
#define	new_nsx(n,s,x)	new0(n, s, NULL, 0, x)
#define	new_i(i)	new0(NULL, NULL, NULL, i, NULL)

/* new style, type-polymorphic; ordinary and for types with multiple flavors */
#define MK0(t)		wrap_mk_##t(mk_##t())
#define MK1(t, a0)	wrap_mk_##t(mk_##t(a0))
#define MK2(t, a0, a1)	wrap_mk_##t(mk_##t(a0, a1))
#define MK3(t, a0, a1, a2)	wrap_mk_##t(mk_##t(a0, a1, a2))

#define MKF0(t, f)		wrap_mk_##t(mk_##t##_##f())
#define MKF1(t, f, a0)		wrap_mk_##t(mk_##t##_##f(a0))
#define MKF2(t, f, a0, a1)	wrap_mk_##t(mk_##t##_##f(a0, a1))

/*
 * Data constructors
 */

static struct defoptlist *mk_defoptlist(const char *, const char *,
					const char *);
static struct loclist *mk_loc(const char *, const char *, long long);
static struct loclist *mk_loc_val(const char *, struct loclist *);
static struct attrlist *mk_attrlist(struct attrlist *, struct attr *);
static struct condexpr *mk_cx_atom(const char *);
static struct condexpr *mk_cx_not(struct condexpr *);
static struct condexpr *mk_cx_and(struct condexpr *, struct condexpr *);
static struct condexpr *mk_cx_or(struct condexpr *, struct condexpr *);

/*
 * Other private functions
 */

static	void	setmachine(const char *, const char *, struct nvlist *, int);
static	void	check_maxpart(void);

static struct loclist *present_loclist(struct loclist *ll);
static void app(struct loclist *, struct loclist *);
static struct loclist *locarray(const char *, int, struct loclist *, int);
static struct loclist *namelocvals(const char *, struct loclist *);

#line 153 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union {
	struct	attr *attr;
	struct	devbase *devb;
	struct	deva *deva;
	struct	nvlist *list;
	struct defoptlist *defoptlist;
	struct loclist *loclist;
	struct attrlist *attrlist;
	struct condexpr *condexpr;
	const char *str;
	struct	numconst num;
	int64_t	val;
	u_char	flag;
	devmajor_t devmajor;
	int32_t i32;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
#line 201 "gram.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();


#define AND 257
#define AT 258
#define ATTACH 259
#define BLOCK 260
#define BUILD 261
#define CHAR 262
#define COLONEQ 263
#define COMPILE_WITH 264
#define CONFIG 265
#define DEFFS 266
#define DEFINE 267
#define DEFOPT 268
#define DEFPARAM 269
#define DEFFLAG 270
#define DEFPSEUDO 271
#define DEFPSEUDODEV 272
#define DEVICE 273
#define DEVCLASS 274
#define DUMPS 275
#define DEVICE_MAJOR 276
#define ENDFILE 277
#define XFILE 278
#define FILE_SYSTEM 279
#define FLAGS 280
#define IDENT 281
#define IOCONF 282
#define LINKZERO 283
#define XMACHINE 284
#define MAJOR 285
#define MAKEOPTIONS 286
#define MAXUSERS 287
#define MAXPARTITIONS 288
#define MINOR 289
#define NEEDS_COUNT 290
#define NEEDS_FLAG 291
#define NO 292
#define XOBJECT 293
#define OBSOLETE 294
#define ON 295
#define OPTIONS 296
#define PACKAGE 297
#define PLUSEQ 298
#define PREFIX 299
#define PSEUDO_DEVICE 300
#define PSEUDO_ROOT 301
#define ROOT 302
#define SELECT 303
#define SINGLE 304
#define SOURCE 305
#define TYPE 306
#define VECTOR 307
#define VERSION 308
#define WITH 309
#define NUMBER 310
#define PATHNAME 311
#define QSTRING 312
#define WORD 313
#define EMPTYSTRING 314
#define ENDDEFS 315
#define YYERRCODE 256
static const short yylhs[] = {                           -1,
    0,   57,   57,   61,   61,   61,   58,   58,   58,   58,
   58,   46,   46,   59,   62,   62,   62,   62,   62,   63,
   63,   63,   63,   63,   63,   63,   63,   63,   63,   63,
   63,   63,   63,   63,   63,   63,   63,   63,   63,   63,
   64,   65,   66,   67,   67,   68,   69,   70,   71,   72,
   73,   74,   75,   76,   77,   78,   79,   80,   81,   82,
   83,   84,    1,    1,    9,    9,   10,   10,   13,   13,
   11,   11,   12,   52,   52,   51,   51,   53,   53,   53,
   54,   54,   56,   56,   55,   39,   39,   38,   18,   18,
   18,   20,   20,   21,   21,   21,   21,   21,   21,   49,
   49,   22,   24,   25,   25,   26,   26,   14,   44,   44,
   43,   43,   42,   17,   17,   19,   19,   41,   41,   40,
   40,   40,   40,   85,   85,   87,   15,   16,   16,   86,
   86,   88,   35,   60,   89,   89,   89,   89,   90,   90,
   90,   90,   90,   90,   90,   90,   90,   90,   90,   90,
   90,   90,   90,   90,   90,   90,   90,   90,   90,   91,
   92,   93,   94,   95,   96,   97,   98,   99,  100,  101,
  102,  103,  104,  105,  106,  107,  108,  109,  110,  112,
  112,  120,  111,  111,  121,  114,  114,  122,  122,  113,
  113,  123,  116,  116,  124,  124,  115,  115,  125,  117,
  118,  118,   29,   29,   29,   33,    8,    8,  119,  119,
  127,   36,   36,   30,   30,   31,   31,   31,   27,   27,
   28,   28,   37,   37,    2,    4,    4,    5,    5,    6,
    7,    7,    7,    3,   50,   50,   45,   45,   47,   47,
   32,   32,   32,   32,   48,   48,   23,   23,   34,   34,
  126,  126,
};
static const short yylen[] = {                            2,
    4,    0,    2,    1,    3,    3,    3,    4,    5,    3,
    1,    1,    2,    2,    0,    2,    3,    3,    2,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    5,    4,    6,    2,    1,    2,    3,    4,    4,    4,
    4,    4,    4,    4,    6,    2,    4,    2,    4,    4,
    4,    2,    0,    1,    0,    2,    1,    1,    0,    2,
    0,    2,    1,    0,    2,    0,    2,    0,    3,    1,
    1,    3,    1,    3,    1,    1,    2,    1,    0,    2,
    3,    1,    3,    2,    1,    4,    4,    5,    7,    1,
    1,    2,    4,    0,    2,    1,    3,    1,    0,    2,
    1,    3,    1,    1,    3,    1,    1,    1,    2,    1,
    3,    3,    5,    1,    3,    4,    1,    0,    2,    1,
    3,    3,    1,    1,    0,    2,    3,    3,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    2,
    3,    3,    2,    3,    2,    3,    2,    2,    2,    2,
    4,    3,    3,    3,    2,    4,    4,    2,    5,    1,
    3,    1,    1,    3,    1,    1,    3,    3,    3,    1,
    3,    1,    1,    3,    1,    3,    1,    3,    1,    1,
    3,    4,    1,    1,    1,    4,    2,    2,    0,    2,
    3,    0,    1,    1,    2,    1,    1,    2,    0,    2,
    2,    2,    0,    2,    1,    1,    3,    1,    3,    1,
    1,    2,    3,    1,    1,    1,    0,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    3,    1,    2,
    0,    1,
};
static const short yydefred[] = {                         2,
    0,    0,   11,    0,    0,    0,    0,    4,   15,    3,
  240,  239,    0,    0,    0,    0,  135,    0,    6,   10,
    0,    7,    5,    1,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   19,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   14,   16,    0,
   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,
   40,   12,    8,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  136,    0,  139,    0,
  140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
  150,  151,  152,  153,  154,  155,  156,  157,  158,  159,
   18,  127,    0,   88,   86,    0,    0,    0,  238,    0,
    0,    0,    0,    0,   46,    0,    0,    0,  234,    0,
    0,    0,  231,    0,    0,  228,  230,    0,  124,  133,
    0,   56,    0,    0,    0,   44,   62,   17,   13,    9,
  138,  200,    0,  182,    0,  180,  245,  246,  169,  235,
    0,    0,    0,  186,    0,    0,    0,    0,  170,    0,
    0,    0,    0,    0,    0,    0,  193,    0,  175,  160,
  215,    0,  137,    0,    0,   87,   47,    0,    0,    0,
  118,    0,    0,    0,    0,    0,    0,    0,    0,   65,
   64,    0,    0,  130,  232,    0,  236,    0,    0,    0,
    0,    0,   71,    0,    0,    0,  209,    0,    0,    0,
    0,  172,    0,  185,    0,  183,  192,    0,  190,  199,
    0,  197,  173,  161,    0,    0,    0,  213,  174,  216,
    0,  219,  117,  116,    0,  114,  113,  111,    0,  101,
  100,   90,    0,    0,    0,    0,    0,   48,    0,    0,
  119,   49,   52,   50,   59,   60,   54,   75,    0,    0,
    0,    0,   61,    0,  233,    0,    0,  229,  125,   57,
    0,    0,    0,  252,    0,    0,  181,  249,  241,  242,
  243,    0,  189,  244,  188,  187,  177,    0,    0,    0,
  176,  196,  194,  218,    0,    0,    0,    0,    0,    0,
   91,    0,    0,    0,   94,  108,  106,    0,  122,    0,
   77,    0,    0,   67,   68,   66,   41,  132,  131,  126,
   73,   72,    0,  204,  203,    0,  205,    0,  210,  250,
  184,  191,  198,    0,    0,  220,  179,  129,  115,   55,
  112,    0,    0,   93,  102,    0,    0,    0,   81,    0,
   43,    0,   70,    0,    0,  202,    0,  224,  221,  222,
    0,    0,   96,    0,  107,  123,    0,    0,    0,  208,
  207,  211,    0,    0,    0,   98,    0,   82,   85,   79,
  206,  248,    0,    0,    0,   99,    0,   84,  103,
};
static const short yydgoto[] = {                          1,
  200,  132,  133,  134,  135,  136,  137,  366,  271,  326,
  281,  332,  327,  317,  202,  308,  245,  189,  246,  254,
  255,  315,  370,  386,  258,  318,  305,  346,  336,   88,
  242,  371,  337,  294,  212,  239,  347,  115,  116,  191,
  192,  248,  249,  187,  118,   74,  119,  159,  256,  162,
  270,  199,  361,  362,  390,  388,    2,    9,   17,   24,
   10,   18,   50,   51,   52,   53,   54,   55,   56,   57,
   58,   59,   60,   61,   62,   63,   64,   65,   66,   67,
   68,   69,   70,   71,  138,  203,  139,  204,   25,   90,
   91,   92,   93,   94,   95,   96,   97,   98,   99,  100,
  101,  102,  103,  104,  105,  106,  107,  108,  109,  110,
  225,  155,  228,  163,  231,  176,  153,  217,  286,  156,
  226,  164,  229,  177,  232,  285,  339,
};
static const short yysindex[] = {                         0,
    0,  100,    0, -189, -270, -251, -189,    0,    0,    0,
    0,    0,   60,   81,   16,   92,    0,   -8,    0,    0,
   17,    0,    0,    0,   47,   97, -199, -187, -182, -189,
 -189, -189, -199, -199, -199, -168, -166,    0, -189,   31,
  -18, -194, -194, -189, -135, -189, -194,    0,    0,  140,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   23,  147, -151, -150, -171,  -20, -194,
 -114, -149, -144, -140, -138,  116,    0,  -87,    0,  166,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -81,    0,    0,  -46,   56, -133,    0, -133,
 -133,   56,   56,   56,    0,  -84,  -18, -199,    0, -132,
  -18, -169,    0,   59,  146,    0,    0,  141,    0,    0,
 -194,    0,  -18, -189, -189,    0,    0,    0,    0,    0,
    0,    0, -112,    0,  144,    0,    0,    0,    0,    0,
    0,  -11,  148,    0, -194, -120,  -64, -118,    0, -117,
 -116, -113, -111,  -60,  143,  157,    0, -194,    0,    0,
    0, -223,    0, -215, -108,    0,    0,  -80,  149,  -44,
    0,  -40,  -40,  -40,  149,  149,  149, -194,  -54,    0,
    0,  150,    2,    0,    0,  167,    0,  -89,  -18,  -18,
  -18, -194,    0, -133, -133,  -85,    0, -150,   88,   88,
 -169,    0, -223,    0,  168,    0,    0,  169,    0,    0,
  170,    0,    0,    0, -223,   88, -149,    0,    0,    0,
  152,    0,    0,    0,  -34,    0,    0,    0,  172,    0,
    0,    0, -152,   93,  173,  -19,  -93,    0,   88,   88,
    0,    0,    0,    0,    0,    0,    0,    0, -194,  -18,
 -197, -194,    0, -199,    0,   88,  146,    0,    0,    0,
  -70, -133, -133,    0,  -59,  -53,    0,    0,    0,    0,
    0,  -86,    0,    0,    0,    0,    0, -118, -117, -116,
    0,    0,    0,    0, -214,  -88, -215,  149, -108,  -17,
    0,  -77,   88, -194,    0,    0,    0,  179,    0,  -35,
    0, -221, -171,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -79,    0,    0,  -72,    0,  -85,    0,    0,
    0,    0,    0, -194,   83,    0,    0,    0,    0,    0,
    0, -194,  136,    0,    0,  145,  -93,   88,    0,  176,
    0,  195,    0,  -49,  -31,    0,  -59,    0,    0,    0,
  197,  151,    0,  181,    0,    0,  -67,  -38,  -63,    0,
    0,    0,   88,  181,  126,    0,  192,    0,    0,    0,
    0,    0,  159,   88,  -57,    0,  130,    0,    0,
};
static const short yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  256,    0,    0,    0,    0,  -25,
  -25,  -25,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  284,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   13,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  286,   24,    0,    0,    0,
    0,   24,   24,   24,    0,   38,   -7,    0,    0,    0,
    0,    0,    0,   74,   63,    0,    0,  287,    0,    0,
    0,    0,   -1,  -25,  -25,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  289,    0,    0,    0,    0,    0,
   79,    0,  295,    0,  298,    0,    0,    0,    0,    0,
    0,    0,    0,  327,   37,  334,    0,  339,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  342,   -2,
    0,  286,  286,  286,  342,  342,  342,    0,   55,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  -39,    0,    0,    0,    0,
    0,    0,    0,    0,  353,    0,    0,  356,    0,    0,
  359,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    9,    0,    0,    0,   27,    0,    0,    0,  362,    0,
    0,    0,    0,    0,  248,    5,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   -3,
  373,    0,    0,    0,    0,    0,   67,    0,    0,    0,
  375,  378,  379,    0,    0,  380,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  384,    0,    0,  342,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  389,    0,   11,
    0,  393,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    6,    0,  -39,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  394,    0,    0,    0,    0,    0,    0,    0,    0,
   -4,    0,    0,    7,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   53,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
static const short yygindex[] = {                         0,
 -115, -102,  276,    0,  198,  199,    0,    0,    0,    0,
    0,    0,    0,   51,   78,    0,    0,   15,  103,   99,
    0,  102, -294,   29, -142,    0,    0,    0,   48,   25,
 -155, -184,    0,    0,  -42,    0,    0,  300,    0, -134,
  -90,  105,    0,  -74,    8,    0,   57,   94,  165,  288,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  396,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  208,  153,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  204,
  125,  203,  127,  188,  128,   91,    0,
};
#define YYTABLESIZE 429
static const short yytable[] = {                        141,
  142,   49,   63,  335,  147,  247,   63,  120,   63,  307,
  253,  185,  130,  253,  130,  201,  260,  185,  217,  131,
  121,  131,  214,  251,  201,   22,   73,  213,  206,  193,
  194,  381,  150,   89,  293,  295,  128,  165,  120,  121,
  201,  313,   14,  313,  252,  274,  195,   74,   95,  220,
   97,  302,  265,  266,  267,  120,   87,  261,  261,  261,
   13,   15,   83,   16,   76,  344,  323,  297,  121,   19,
   74,  314,  226,  352,  319,  320,  227,   74,  240,  301,
  195,   89,  359,  225,  128,  360,  243,   76,  392,  241,
   20,  330,  324,  325,   76,  127,   83,  244,  345,  397,
  143,   23,  146,  226,  113,  174,  111,  227,  179,    8,
  122,  123,  124,  112,  225,  140,  234,  262,  263,  264,
  247,   11,   12,  282,  283,  114,  273,  292,  355,   95,
  117,   97,  292,  144,  145,  238,  195,  196,  197,  236,
  157,  158,  160,  207,  125,  369,  126,  261,  261,  148,
  166,  214,  215,  128,  322,  268,  151,  181,  167,  250,
  251,  152,  154,  175,  168,  350,  169,  201,  178,  280,
  182,  170,   86,  376,  180,  183,  184,  198,  188,  190,
  129,  171,  209,  210,  211,  172,  226,  218,  173,  216,
  227,  221,  222,  223,  224,  227,  230,  235,   86,  233,
  237,  234,  234,  236,  247,  269,  257,  275,  276,  284,
  272,  298,  299,  300,  304,  309,  312,  311,  259,  316,
  331,  338,  357,  340,  348,  333,  321,  358,  373,  328,
  364,  250,  251,  365,  250,  251,  377,  374,  378,  379,
  383,  385,  387,  384,  389,  251,  391,   26,  394,  395,
   27,  396,  398,  334,  399,  134,   63,   28,   29,   30,
   31,   32,   33,   34,   35,   36,  114,   37,   38,   39,
  214,  356,  190,  251,  306,  247,   40,   41,   42,   43,
  201,  380,   63,   63,   44,   45,  219,  237,  217,   63,
   46,  160,  161,   45,  129,  109,   58,   74,  163,   47,
   63,  368,   75,   63,  165,   27,   48,  168,  247,  372,
  120,   76,   28,   29,   30,   31,   32,   33,   34,   35,
   36,  217,   37,  121,   39,   77,  226,   78,   21,   72,
  227,   40,   79,   80,   43,  149,  178,  225,   81,   44,
   45,   74,   82,  167,   74,   46,   83,   84,  212,   85,
   74,  104,  226,  226,   47,    3,  227,  227,   76,   86,
    4,   76,  162,  225,  225,  164,  226,   76,  166,  226,
  227,  110,   92,  227,  226,  226,  236,  225,  227,  227,
  225,    5,   69,    6,   42,  225,  225,   53,   51,  171,
  234,  234,  288,  223,  289,  290,  291,  288,  105,  289,
  290,  291,   78,   80,    7,  205,  277,  375,  278,  349,
  354,  353,  393,  351,  382,  186,  363,  310,  279,  208,
   89,  287,  341,  296,  303,  342,  329,  343,  367,
};
static const short yycheck[] = {                         42,
   43,   10,   10,   63,   47,   10,   10,   10,   10,   44,
   91,   58,   33,   91,   33,   10,   61,   58,   10,   40,
   10,   40,   10,   63,  127,   10,   10,  143,  131,  120,
  121,   63,   10,   10,  219,  220,   10,   80,   31,   32,
  143,   61,  313,   61,  125,   44,   10,   10,   44,   61,
   44,  236,  195,  196,  197,   58,   10,  192,  193,  194,
    4,  313,   10,    7,   10,  280,  264,  223,   58,   10,
   33,   91,   10,   91,  259,  260,   10,   40,  302,  235,
   44,   58,  304,   10,   58,  307,  302,   33,  383,  313,
   10,  276,  290,  291,   40,   39,   44,  313,  313,  394,
   44,   10,   46,   41,   27,   81,   10,   41,   84,   10,
   33,   34,   35,  313,   41,  310,   38,  192,  193,  194,
  125,  311,  312,  214,  215,  313,  125,   45,  313,  125,
  313,  125,   45,  269,  270,  178,  122,  123,  124,   61,
  312,  313,  312,  313,  313,   63,  313,  282,  283,   10,
  265,  144,  145,  123,  270,  198,   10,   42,  273,  312,
  313,  313,  313,  313,  279,  308,  281,  270,  313,  212,
  258,  286,  313,  358,  313,   10,  258,  262,  123,  313,
  313,  296,  124,   38,   44,  300,  124,   44,  303,  302,
  124,   44,  313,  258,  313,  313,  313,  258,  313,  313,
   44,  313,  124,   61,  313,  260,   58,   41,  298,  295,
   61,   44,   44,   44,   63,   44,   44,  125,  263,  313,
  291,  275,   44,  310,  313,  285,  269,  263,   93,  272,
  310,  312,  313,  306,  312,  313,   61,   93,   44,  289,
   44,   61,  310,   93,  283,  285,  310,  256,  123,   58,
  259,   93,  310,  313,  125,    0,  264,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  313,  276,  277,  278,
  258,  314,  313,  313,  309,  280,  285,  286,  287,  288,
  275,  313,  290,  291,  293,  294,  298,  313,  280,  291,
  299,  312,  313,   10,  313,   10,   10,  260,   10,  308,
  304,  344,  256,  307,   10,  259,  315,   10,  313,  352,
  313,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  313,  276,  313,  278,  279,  264,  281,  313,  313,
  264,  285,  286,  287,  288,  313,   10,  264,  292,  293,
  294,  304,  296,   10,  307,  299,  300,  301,   10,  303,
  313,   10,  290,  291,  308,  256,  290,  291,  304,  313,
  261,  307,   10,  290,  291,   10,  304,  313,   10,  307,
  304,   10,  125,  307,  312,  313,  298,  304,  312,  313,
  307,  282,   10,  284,   10,  312,  313,   10,   10,   10,
  312,  313,  310,   10,  312,  313,  314,  310,   10,  312,
  313,  314,   10,   10,  305,  130,  209,  357,  210,  307,
  312,  310,  384,  309,  367,  116,  323,  253,  211,  132,
   25,  218,  298,  221,  237,  299,  274,  300,  338,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 315
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? (YYMAXTOKEN + 1) : (a))
#if YYDEBUG
static const char *yytname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,"'\\n'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,"'!'",0,0,0,0,"'&'",0,"'('","')'","'*'",0,"','","'-'",0,0,0,0,0,0,0,0,0,0,0,
0,"':'",0,0,"'='",0,"'?'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'['",0,"']'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",
"'|'","'}'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"AND","AT","ATTACH","BLOCK","BUILD","CHAR",
"COLONEQ","COMPILE_WITH","CONFIG","DEFFS","DEFINE","DEFOPT","DEFPARAM",
"DEFFLAG","DEFPSEUDO","DEFPSEUDODEV","DEVICE","DEVCLASS","DUMPS","DEVICE_MAJOR",
"ENDFILE","XFILE","FILE_SYSTEM","FLAGS","IDENT","IOCONF","LINKZERO","XMACHINE",
"MAJOR","MAKEOPTIONS","MAXUSERS","MAXPARTITIONS","MINOR","NEEDS_COUNT",
"NEEDS_FLAG","NO","XOBJECT","OBSOLETE","ON","OPTIONS","PACKAGE","PLUSEQ",
"PREFIX","PSEUDO_DEVICE","PSEUDO_ROOT","ROOT","SELECT","SINGLE","SOURCE","TYPE",
"VECTOR","VERSION","WITH","NUMBER","PATHNAME","QSTRING","WORD","EMPTYSTRING",
"ENDDEFS","illegal-token",
};
static const char *yyrule[] = {
"$accept : configuration",
"configuration : topthings machine_spec definition_part selection_part",
"topthings :",
"topthings : topthings topthing",
"topthing : '\\n'",
"topthing : SOURCE filename '\\n'",
"topthing : BUILD filename '\\n'",
"machine_spec : XMACHINE WORD '\\n'",
"machine_spec : XMACHINE WORD WORD '\\n'",
"machine_spec : XMACHINE WORD WORD subarches '\\n'",
"machine_spec : IOCONF WORD '\\n'",
"machine_spec : error",
"subarches : WORD",
"subarches : subarches WORD",
"definition_part : definitions ENDDEFS",
"definitions :",
"definitions : definitions '\\n'",
"definitions : definitions definition '\\n'",
"definitions : definitions error '\\n'",
"definitions : definitions ENDFILE",
"definition : define_file",
"definition : define_object",
"definition : define_device_major",
"definition : define_prefix",
"definition : define_devclass",
"definition : define_filesystems",
"definition : define_attribute",
"definition : define_option",
"definition : define_flag",
"definition : define_obsolete_flag",
"definition : define_param",
"definition : define_obsolete_param",
"definition : define_device",
"definition : define_device_attachment",
"definition : define_maxpartitions",
"definition : define_maxusers",
"definition : define_makeoptions",
"definition : define_pseudo",
"definition : define_pseudodev",
"definition : define_major",
"definition : define_version",
"define_file : XFILE filename fopts fflags rule",
"define_object : XOBJECT filename fopts oflags",
"define_device_major : DEVICE_MAJOR WORD device_major_char device_major_block fopts devnodes",
"define_prefix : PREFIX filename",
"define_prefix : PREFIX",
"define_devclass : DEVCLASS WORD",
"define_filesystems : DEFFS deffses optdepend_list",
"define_attribute : DEFINE WORD interface_opt depend_list",
"define_option : DEFOPT optfile_opt defopts optdepend_list",
"define_flag : DEFFLAG optfile_opt defopts optdepend_list",
"define_obsolete_flag : OBSOLETE DEFFLAG optfile_opt defopts",
"define_param : DEFPARAM optfile_opt defopts optdepend_list",
"define_obsolete_param : OBSOLETE DEFPARAM optfile_opt defopts",
"define_device : DEVICE devbase interface_opt depend_list",
"define_device_attachment : ATTACH devbase AT atlist devattach_opt depend_list",
"define_maxpartitions : MAXPARTITIONS int32",
"define_maxusers : MAXUSERS int32 int32 int32",
"define_makeoptions : MAKEOPTIONS condmkopt_list",
"define_pseudo : DEFPSEUDO devbase interface_opt depend_list",
"define_pseudodev : DEFPSEUDODEV devbase interface_opt depend_list",
"define_major : MAJOR '{' majorlist '}'",
"define_version : VERSION int32",
"fopts :",
"fopts : condexpr",
"fflags :",
"fflags : fflags fflag",
"fflag : NEEDS_COUNT",
"fflag : NEEDS_FLAG",
"rule :",
"rule : COMPILE_WITH stringvalue",
"oflags :",
"oflags : oflags oflag",
"oflag : NEEDS_FLAG",
"device_major_char :",
"device_major_char : CHAR int32",
"device_major_block :",
"device_major_block : BLOCK int32",
"devnodes :",
"devnodes : devnodetype ',' devnodeflags",
"devnodes : devnodetype",
"devnodetype : SINGLE",
"devnodetype : VECTOR '=' devnode_dims",
"devnode_dims : NUMBER",
"devnode_dims : NUMBER ':' NUMBER",
"devnodeflags : LINKZERO",
"deffses : deffs",
"deffses : deffses deffs",
"deffs : WORD",
"interface_opt :",
"interface_opt : '{' '}'",
"interface_opt : '{' loclist '}'",
"loclist : locdef",
"loclist : locdef ',' loclist",
"locdef : locname locdefault",
"locdef : locname",
"locdef : '[' locname locdefault ']'",
"locdef : locname '[' int32 ']'",
"locdef : locname '[' int32 ']' locdefaults",
"locdef : '[' locname '[' int32 ']' locdefaults ']'",
"locname : WORD",
"locname : QSTRING",
"locdefault : '=' value",
"locdefaults : '=' '{' values '}'",
"depend_list :",
"depend_list : ':' depends",
"depends : depend",
"depends : depends ',' depend",
"depend : WORD",
"optdepend_list :",
"optdepend_list : ':' optdepends",
"optdepends : optdepend",
"optdepends : optdepends ',' optdepend",
"optdepend : WORD",
"atlist : atname",
"atlist : atlist ',' atname",
"atname : WORD",
"atname : ROOT",
"defopts : defopt",
"defopts : defopts defopt",
"defopt : WORD",
"defopt : WORD '=' value",
"defopt : WORD COLONEQ value",
"defopt : WORD '=' value COLONEQ value",
"condmkopt_list : condmkoption",
"condmkopt_list : condmkopt_list ',' condmkoption",
"condmkoption : condexpr mkvarname PLUSEQ value",
"devbase : WORD",
"devattach_opt :",
"devattach_opt : WITH WORD",
"majorlist : majordef",
"majorlist : majorlist ',' majordef",
"majordef : devbase '=' int32",
"int32 : NUMBER",
"selection_part : selections",
"selections :",
"selections : selections '\\n'",
"selections : selections selection '\\n'",
"selections : selections error '\\n'",
"selection : definition",
"selection : select_attr",
"selection : select_no_attr",
"selection : select_no_filesystems",
"selection : select_filesystems",
"selection : select_no_makeoptions",
"selection : select_makeoptions",
"selection : select_no_options",
"selection : select_options",
"selection : select_maxusers",
"selection : select_ident",
"selection : select_no_ident",
"selection : select_config",
"selection : select_no_config",
"selection : select_no_pseudodev",
"selection : select_pseudodev",
"selection : select_pseudoroot",
"selection : select_no_device_instance_attachment",
"selection : select_no_device_attachment",
"selection : select_no_device_instance",
"selection : select_device_instance",
"select_attr : SELECT WORD",
"select_no_attr : NO SELECT WORD",
"select_no_filesystems : NO FILE_SYSTEM no_fs_list",
"select_filesystems : FILE_SYSTEM fs_list",
"select_no_makeoptions : NO MAKEOPTIONS no_mkopt_list",
"select_makeoptions : MAKEOPTIONS mkopt_list",
"select_no_options : NO OPTIONS no_opt_list",
"select_options : OPTIONS opt_list",
"select_maxusers : MAXUSERS int32",
"select_ident : IDENT stringvalue",
"select_no_ident : NO IDENT",
"select_config : CONFIG conf root_spec sysparam_list",
"select_no_config : NO CONFIG WORD",
"select_no_pseudodev : NO PSEUDO_DEVICE WORD",
"select_pseudodev : PSEUDO_DEVICE WORD npseudo",
"select_pseudoroot : PSEUDO_ROOT device_instance",
"select_no_device_instance_attachment : NO device_instance AT attachment",
"select_no_device_attachment : NO DEVICE AT attachment",
"select_no_device_instance : NO device_instance",
"select_device_instance : device_instance AT attachment locators device_flags",
"fs_list : fsoption",
"fs_list : fs_list ',' fsoption",
"fsoption : WORD",
"no_fs_list : no_fsoption",
"no_fs_list : no_fs_list ',' no_fsoption",
"no_fsoption : WORD",
"mkopt_list : mkoption",
"mkopt_list : mkopt_list ',' mkoption",
"mkoption : mkvarname '=' value",
"mkoption : mkvarname PLUSEQ value",
"no_mkopt_list : no_mkoption",
"no_mkopt_list : no_mkopt_list ',' no_mkoption",
"no_mkoption : WORD",
"opt_list : option",
"opt_list : opt_list ',' option",
"option : WORD",
"option : WORD '=' value",
"no_opt_list : no_option",
"no_opt_list : no_opt_list ',' no_option",
"no_option : WORD",
"conf : WORD",
"root_spec : ROOT on_opt dev_spec",
"root_spec : ROOT on_opt dev_spec fs_spec",
"dev_spec : '?'",
"dev_spec : WORD",
"dev_spec : major_minor",
"major_minor : MAJOR NUMBER MINOR NUMBER",
"fs_spec : TYPE '?'",
"fs_spec : TYPE WORD",
"sysparam_list :",
"sysparam_list : sysparam_list sysparam",
"sysparam : DUMPS on_opt dev_spec",
"npseudo :",
"npseudo : int32",
"device_instance : WORD",
"device_instance : WORD '*'",
"attachment : ROOT",
"attachment : WORD",
"attachment : WORD '?'",
"locators :",
"locators : locators locator",
"locator : WORD '?'",
"locator : WORD values",
"device_flags :",
"device_flags : FLAGS int32",
"condexpr : cond_or_expr",
"cond_or_expr : cond_and_expr",
"cond_or_expr : cond_or_expr '|' cond_and_expr",
"cond_and_expr : cond_prefix_expr",
"cond_and_expr : cond_and_expr '&' cond_prefix_expr",
"cond_prefix_expr : cond_base_expr",
"cond_base_expr : condatom",
"cond_base_expr : '!' condatom",
"cond_base_expr : '(' condexpr ')'",
"condatom : WORD",
"mkvarname : QSTRING",
"mkvarname : WORD",
"optfile_opt :",
"optfile_opt : filename",
"filename : QSTRING",
"filename : PATHNAME",
"value : QSTRING",
"value : WORD",
"value : EMPTYSTRING",
"value : signed_number",
"stringvalue : QSTRING",
"stringvalue : WORD",
"values : value",
"values : value ',' values",
"signed_number : NUMBER",
"signed_number : '-' NUMBER",
"on_opt :",
"on_opt : ON",

};
#endif

int      yydebug;
int      yynerrs;

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

typedef struct {
    unsigned stacksize;
    short    *s_base;
    short    *s_mark;
    short    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;
#line 1076 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"

void
yyerror(const char *s)
{

	cfgerror("%s", s);
}

/************************************************************/

/*
 * Wrap allocations that live on the parser stack so that we can free
 * them again on error instead of leaking.
 */

#define MAX_WRAP 1000

struct wrap_entry {
	void *ptr;
	unsigned typecode;
};

static struct wrap_entry wrapstack[MAX_WRAP];
static unsigned wrap_depth;

/*
 * Remember pointer PTR with type-code CODE.
 */
static void
wrap_alloc(void *ptr, unsigned code)
{
	unsigned pos;

	if (wrap_depth >= MAX_WRAP) {
		panic("allocation wrapper stack overflow");
	}
	pos = wrap_depth++;
	wrapstack[pos].ptr = ptr;
	wrapstack[pos].typecode = code;
}

/*
 * We succeeded; commit to keeping everything that's been allocated so
 * far and clear the stack.
 */
static void
wrap_continue(void)
{
	wrap_depth = 0;
}

/*
 * We failed; destroy all the objects allocated.
 */
static void
wrap_cleanup(void)
{
	unsigned i;

	/*
	 * Destroy each item. Note that because everything allocated
	 * is entered on the list separately, lists and trees need to
	 * have their links blanked before being destroyed. Also note
	 * that strings are interned elsewhere and not handled by this
	 * mechanism.
	 */

	for (i=0; i<wrap_depth; i++) {
		switch (wrapstack[i].typecode) {
		    case WRAP_CODE_nvlist:
			nvfree(wrapstack[i].ptr);
			break;
		    case WRAP_CODE_defoptlist:
			{
				struct defoptlist *dl = wrapstack[i].ptr;

				dl->dl_next = NULL;
				defoptlist_destroy(dl);
			}
			break;
		    case WRAP_CODE_loclist:
			{
				struct loclist *ll = wrapstack[i].ptr;

				ll->ll_next = NULL;
				loclist_destroy(ll);
			}
			break;
		    case WRAP_CODE_attrlist:
			{
				struct attrlist *al = wrapstack[i].ptr;

				al->al_next = NULL;
				al->al_this = NULL;
				attrlist_destroy(al);
			}
			break;
		    case WRAP_CODE_condexpr:
			{
				struct condexpr *cx = wrapstack[i].ptr;

				cx->cx_type = CX_ATOM;
				cx->cx_atom = NULL;
				condexpr_destroy(cx);
			}
			break;
		    default:
			panic("invalid code %u on allocation wrapper stack",
			      wrapstack[i].typecode);
		}
	}

	wrap_depth = 0;
}

/*
 * Instantiate the wrapper functions.
 *
 * Each one calls wrap_alloc to save the pointer and then returns the
 * pointer again; these need to be generated with the preprocessor in
 * order to be typesafe.
 */
#define DEF_ALLOCWRAP(t) \
	static struct t *				\
	wrap_mk_##t(struct t *arg)			\
	{						\
		wrap_alloc(arg, WRAP_CODE_##t);		\
		return arg;				\
	}

DEF_ALLOCWRAP(nvlist);
DEF_ALLOCWRAP(defoptlist);
DEF_ALLOCWRAP(loclist);
DEF_ALLOCWRAP(attrlist);
DEF_ALLOCWRAP(condexpr);

/************************************************************/

/*
 * Data constructors
 *
 * (These are *beneath* the allocation wrappers.)
 */

static struct defoptlist *
mk_defoptlist(const char *name, const char *val, const char *lintval)
{
	return defoptlist_create(name, val, lintval);
}

static struct loclist *
mk_loc(const char *name, const char *str, long long num)
{
	return loclist_create(name, str, num);
}

static struct loclist *
mk_loc_val(const char *str, struct loclist *next)
{
	struct loclist *ll;

	ll = mk_loc(NULL, str, 0);
	ll->ll_next = next;
	return ll;
}

static struct attrlist *
mk_attrlist(struct attrlist *next, struct attr *a)
{
	return attrlist_cons(next, a);
}

static struct condexpr *
mk_cx_atom(const char *s)
{
	struct condexpr *cx;

	cx = condexpr_create(CX_ATOM);
	cx->cx_atom = s;
	return cx;
}

static struct condexpr *
mk_cx_not(struct condexpr *sub)
{
	struct condexpr *cx;

	cx = condexpr_create(CX_NOT);
	cx->cx_not = sub;
	return cx;
}

static struct condexpr *
mk_cx_and(struct condexpr *left, struct condexpr *right)
{
	struct condexpr *cx;

	cx = condexpr_create(CX_AND);
	cx->cx_and.left = left;
	cx->cx_and.right = right;
	return cx;
}

static struct condexpr *
mk_cx_or(struct condexpr *left, struct condexpr *right)
{
	struct condexpr *cx;

	cx = condexpr_create(CX_OR);
	cx->cx_or.left = left;
	cx->cx_or.right = right;
	return cx;
}

/************************************************************/

static void
setmachine(const char *mch, const char *mcharch, struct nvlist *mchsubarches,
	int isioconf)
{
	char buf[MAXPATHLEN];
	struct nvlist *nv;

	if (isioconf) {
		if (include(_PATH_DEVNULL, ENDDEFS, 0, 0) != 0)
			exit(1);
		ioconfname = mch;
		return;
	}

	machine = mch;
	machinearch = mcharch;
	machinesubarches = mchsubarches;

	/*
	 * Define attributes for all the given names
	 */
	if (defattr(machine, NULL, NULL, 0) != 0 ||
	    (machinearch != NULL &&
	     defattr(machinearch, NULL, NULL, 0) != 0))
		exit(1);
	for (nv = machinesubarches; nv != NULL; nv = nv->nv_next) {
		if (defattr(nv->nv_name, NULL, NULL, 0) != 0)
			exit(1);
	}

	/*
	 * Set up the file inclusion stack.  This empty include tells
	 * the parser there are no more device definitions coming.
	 */
	if (include(_PATH_DEVNULL, ENDDEFS, 0, 0) != 0)
		exit(1);

	/* Include arch/${MACHINE}/conf/files.${MACHINE} */
	(void)snprintf(buf, sizeof(buf), "arch/%s/conf/files.%s",
	    machine, machine);
	if (include(buf, ENDFILE, 0, 0) != 0)
		exit(1);

	/* Include any arch/${MACHINE_SUBARCH}/conf/files.${MACHINE_SUBARCH} */
	for (nv = machinesubarches; nv != NULL; nv = nv->nv_next) {
		(void)snprintf(buf, sizeof(buf), "arch/%s/conf/files.%s",
		    nv->nv_name, nv->nv_name);
		if (include(buf, ENDFILE, 0, 0) != 0)
			exit(1);
	}

	/* Include any arch/${MACHINE_ARCH}/conf/files.${MACHINE_ARCH} */
	if (machinearch != NULL)
		(void)snprintf(buf, sizeof(buf), "arch/%s/conf/files.%s",
		    machinearch, machinearch);
	else
		strlcpy(buf, _PATH_DEVNULL, sizeof(buf));
	if (include(buf, ENDFILE, 0, 0) != 0)
		exit(1);

	/*
	 * Include the global conf/files.  As the last thing
	 * pushed on the stack, it will be processed first.
	 */
	if (include("conf/files", ENDFILE, 0, 0) != 0)
		exit(1);

	oktopackage = 1;
}

static void
check_maxpart(void)
{

	if (maxpartitions <= 0 && ioconfname == NULL) {
		stop("cannot proceed without maxpartitions specifier");
	}
}

static void
check_version(void)
{
	/*
	 * In essence, version is 0 and is not supported anymore
	 */
	if (version < CONFIG_MINVERSION)
		stop("your sources are out of date -- please update.");
}

/*
 * Prepend a blank entry to the locator definitions so the code in
 * sem.c can distinguish "empty locator list" from "no locator list".
 * XXX gross.
 */
static struct loclist *
present_loclist(struct loclist *ll)
{
	struct loclist *ret;

	ret = MK3(loc, "", NULL, 0);
	ret->ll_next = ll;
	return ret;
}

static void
app(struct loclist *p, struct loclist *q)
{
	while (p->ll_next)
		p = p->ll_next;
	p->ll_next = q;
}

static struct loclist *
locarray(const char *name, int count, struct loclist *adefs, int opt)
{
	struct loclist *defs = adefs;
	struct loclist **p;
	char buf[200];
	int i;

	if (count <= 0) {
		fprintf(stderr, "config: array with <= 0 size: %s\n", name);
		exit(1);
	}
	p = &defs;
	for(i = 0; i < count; i++) {
		if (*p == NULL)
			*p = MK3(loc, NULL, "0", 0);
		snprintf(buf, sizeof(buf), "%s%c%d", name, ARRCHR, i);
		(*p)->ll_name = i == 0 ? name : intern(buf);
		(*p)->ll_num = i > 0 || opt;
		p = &(*p)->ll_next;
	}
	*p = 0;
	return defs;
}


static struct loclist *
namelocvals(const char *name, struct loclist *vals)
{
	struct loclist *p;
	char buf[200];
	int i;

	for (i = 0, p = vals; p; i++, p = p->ll_next) {
		snprintf(buf, sizeof(buf), "%s%c%d", name, ARRCHR, i);
		p->ll_name = i == 0 ? name : intern(buf);
	}
	return vals;
}

#line 1283 "gram.c"

#if YYDEBUG
#include <stdio.h>		/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (short *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack)) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = YYLEX) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = yytname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    yyerror("syntax error");

    goto yyerrlab;

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yystack.s_mark]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = yytname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 5:
#line 275 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ if (!srcdir) srcdir = yystack.l_mark[-1].str; }
break;
case 6:
#line 276 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ if (!builddir) builddir = yystack.l_mark[-1].str; }
break;
case 7:
#line 281 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setmachine(yystack.l_mark[-1].str,NULL,NULL,0); }
break;
case 8:
#line 282 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setmachine(yystack.l_mark[-2].str,yystack.l_mark[-1].str,NULL,0); }
break;
case 9:
#line 283 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setmachine(yystack.l_mark[-3].str,yystack.l_mark[-2].str,yystack.l_mark[-1].list,0); }
break;
case 10:
#line 284 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setmachine(yystack.l_mark[-1].str,NULL,NULL,1); }
break;
case 11:
#line 285 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ stop("cannot proceed without machine or ioconf specifier"); }
break;
case 12:
#line 290 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_n(yystack.l_mark[0].str); }
break;
case 13:
#line 291 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_nx(yystack.l_mark[0].str, yystack.l_mark[-1].list); }
break;
case 14:
#line 302 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ check_maxpart(); check_version(); }
break;
case 17:
#line 309 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ wrap_continue(); }
break;
case 18:
#line 310 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ wrap_cleanup(); }
break;
case 19:
#line 311 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ enddefs(); checkfiles(); }
break;
case 41:
#line 341 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ addfile(yystack.l_mark[-3].str, yystack.l_mark[-2].condexpr, yystack.l_mark[-1].flag, yystack.l_mark[0].str); }
break;
case 42:
#line 346 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ addobject(yystack.l_mark[-2].str, yystack.l_mark[-1].condexpr, yystack.l_mark[0].flag); }
break;
case 43:
#line 352 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{
		adddevm(yystack.l_mark[-4].str, yystack.l_mark[-3].devmajor, yystack.l_mark[-2].devmajor, yystack.l_mark[-1].condexpr, yystack.l_mark[0].list);
		do_devsw = 1;
	}
break;
case 44:
#line 360 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ prefix_push(yystack.l_mark[0].str); }
break;
case 45:
#line 361 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ prefix_pop(); }
break;
case 46:
#line 365 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ (void)defdevclass(yystack.l_mark[0].str, NULL, NULL, 1); }
break;
case 47:
#line 369 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ deffilesystem(yystack.l_mark[-1].list, yystack.l_mark[0].list); }
break;
case 48:
#line 374 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ (void)defattr0(yystack.l_mark[-2].str, yystack.l_mark[-1].loclist, yystack.l_mark[0].attrlist, 0); }
break;
case 49:
#line 379 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ defoption(yystack.l_mark[-2].str, yystack.l_mark[-1].defoptlist, yystack.l_mark[0].list); }
break;
case 50:
#line 384 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ defflag(yystack.l_mark[-2].str, yystack.l_mark[-1].defoptlist, yystack.l_mark[0].list, 0); }
break;
case 51:
#line 389 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ defflag(yystack.l_mark[-1].str, yystack.l_mark[0].defoptlist, NULL, 1); }
break;
case 52:
#line 394 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ defparam(yystack.l_mark[-2].str, yystack.l_mark[-1].defoptlist, yystack.l_mark[0].list, 0); }
break;
case 53:
#line 399 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ defparam(yystack.l_mark[-1].str, yystack.l_mark[0].defoptlist, NULL, 1); }
break;
case 54:
#line 404 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ defdev(yystack.l_mark[-2].devb, yystack.l_mark[-1].loclist, yystack.l_mark[0].attrlist, 0); }
break;
case 55:
#line 409 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ defdevattach(yystack.l_mark[-1].deva, yystack.l_mark[-4].devb, yystack.l_mark[-2].list, yystack.l_mark[0].attrlist); }
break;
case 56:
#line 413 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ maxpartitions = yystack.l_mark[0].i32; }
break;
case 57:
#line 418 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setdefmaxusers(yystack.l_mark[-2].i32, yystack.l_mark[-1].i32, yystack.l_mark[0].i32); }
break;
case 59:
#line 428 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ defdev(yystack.l_mark[-2].devb, yystack.l_mark[-1].loclist, yystack.l_mark[0].attrlist, 1); }
break;
case 60:
#line 433 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ defdev(yystack.l_mark[-2].devb, yystack.l_mark[-1].loclist, yystack.l_mark[0].attrlist, 2); }
break;
case 62:
#line 441 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setversion(yystack.l_mark[0].i32); }
break;
case 63:
#line 446 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.condexpr = NULL; }
break;
case 64:
#line 447 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.condexpr = yystack.l_mark[0].condexpr; }
break;
case 65:
#line 452 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.flag = 0; }
break;
case 66:
#line 453 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.flag = yystack.l_mark[-1].flag | yystack.l_mark[0].flag; }
break;
case 67:
#line 458 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.flag = FI_NEEDSCOUNT; }
break;
case 68:
#line 459 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.flag = FI_NEEDSFLAG; }
break;
case 69:
#line 464 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = NULL; }
break;
case 70:
#line 465 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 71:
#line 470 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.flag = 0; }
break;
case 72:
#line 471 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.flag = yystack.l_mark[-1].flag | yystack.l_mark[0].flag; }
break;
case 73:
#line 476 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.flag = OI_NEEDSFLAG; }
break;
case 74:
#line 481 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.devmajor = -1; }
break;
case 75:
#line 482 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.devmajor = yystack.l_mark[0].i32; }
break;
case 76:
#line 487 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.devmajor = -1; }
break;
case 77:
#line 488 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.devmajor = yystack.l_mark[0].i32; }
break;
case 78:
#line 493 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_s("DEVNODE_DONTBOTHER"); }
break;
case 79:
#line 494 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = nvcat(yystack.l_mark[-2].list, yystack.l_mark[0].list); }
break;
case 80:
#line 495 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = yystack.l_mark[0].list; }
break;
case 81:
#line 500 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_s("DEVNODE_SINGLE"); }
break;
case 82:
#line 501 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = nvcat(new_s("DEVNODE_VECTOR"), yystack.l_mark[0].list); }
break;
case 83:
#line 506 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_i(yystack.l_mark[0].num.val); }
break;
case 84:
#line 507 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{
		struct nvlist *__nv1, *__nv2;

		__nv1 = new_i(yystack.l_mark[-2].num.val);
		__nv2 = new_i(yystack.l_mark[0].num.val);
		yyval.list = nvcat(__nv1, __nv2);
	  }
break;
case 85:
#line 518 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_s("DEVNODE_FLAG_LINKZERO");}
break;
case 86:
#line 523 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_n(yystack.l_mark[0].str); }
break;
case 87:
#line 524 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_nx(yystack.l_mark[0].str, yystack.l_mark[-1].list); }
break;
case 88:
#line 529 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 89:
#line 534 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = NULL; }
break;
case 90:
#line 535 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = present_loclist(NULL); }
break;
case 91:
#line 536 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = present_loclist(yystack.l_mark[-1].loclist); }
break;
case 92:
#line 546 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = yystack.l_mark[0].loclist; }
break;
case 93:
#line 547 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = yystack.l_mark[-2].loclist; app(yystack.l_mark[-2].loclist, yystack.l_mark[0].loclist); }
break;
case 94:
#line 556 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = MK3(loc, yystack.l_mark[-1].str, yystack.l_mark[0].str, 0); }
break;
case 95:
#line 557 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = MK3(loc, yystack.l_mark[0].str, NULL, 0); }
break;
case 96:
#line 558 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = MK3(loc, yystack.l_mark[-2].str, yystack.l_mark[-1].str, 1); }
break;
case 97:
#line 559 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = locarray(yystack.l_mark[-3].str, yystack.l_mark[-1].i32, NULL, 0); }
break;
case 98:
#line 561 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = locarray(yystack.l_mark[-4].str, yystack.l_mark[-2].i32, yystack.l_mark[0].loclist, 0); }
break;
case 99:
#line 563 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = locarray(yystack.l_mark[-5].str, yystack.l_mark[-3].i32, yystack.l_mark[-1].loclist, 1); }
break;
case 100:
#line 568 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 101:
#line 569 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 102:
#line 574 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 103:
#line 579 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = yystack.l_mark[-1].loclist; }
break;
case 104:
#line 584 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.attrlist = NULL; }
break;
case 105:
#line 585 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.attrlist = yystack.l_mark[0].attrlist; }
break;
case 106:
#line 590 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.attrlist = MK2(attrlist, NULL, yystack.l_mark[0].attr); }
break;
case 107:
#line 591 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.attrlist = MK2(attrlist, yystack.l_mark[-2].attrlist, yystack.l_mark[0].attr); }
break;
case 108:
#line 596 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.attr = refattr(yystack.l_mark[0].str); }
break;
case 109:
#line 601 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = NULL; }
break;
case 110:
#line 602 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = yystack.l_mark[0].list; }
break;
case 111:
#line 607 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_n(yystack.l_mark[0].str); }
break;
case 112:
#line 608 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_nx(yystack.l_mark[0].str, yystack.l_mark[-2].list); }
break;
case 113:
#line 613 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 114:
#line 619 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_n(yystack.l_mark[0].str); }
break;
case 115:
#line 620 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_nx(yystack.l_mark[0].str, yystack.l_mark[-2].list); }
break;
case 116:
#line 625 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 117:
#line 626 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = NULL; }
break;
case 118:
#line 631 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.defoptlist = yystack.l_mark[0].defoptlist; }
break;
case 119:
#line 632 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.defoptlist = defoptlist_append(yystack.l_mark[0].defoptlist, yystack.l_mark[-1].defoptlist); }
break;
case 120:
#line 637 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.defoptlist = MK3(defoptlist, yystack.l_mark[0].str, NULL, NULL); }
break;
case 121:
#line 638 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.defoptlist = MK3(defoptlist, yystack.l_mark[-2].str, yystack.l_mark[0].str, NULL); }
break;
case 122:
#line 639 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.defoptlist = MK3(defoptlist, yystack.l_mark[-2].str, NULL, yystack.l_mark[0].str); }
break;
case 123:
#line 640 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.defoptlist = MK3(defoptlist, yystack.l_mark[-4].str, yystack.l_mark[-2].str, yystack.l_mark[0].str); }
break;
case 126:
#line 651 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ appendcondmkoption(yystack.l_mark[-3].condexpr, yystack.l_mark[-2].str, yystack.l_mark[0].str); }
break;
case 127:
#line 656 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.devb = getdevbase(yystack.l_mark[0].str); }
break;
case 128:
#line 661 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.deva = NULL; }
break;
case 129:
#line 662 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.deva = getdevattach(yystack.l_mark[0].str); }
break;
case 132:
#line 674 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setmajor(yystack.l_mark[-2].devb, yystack.l_mark[0].i32); }
break;
case 133:
#line 678 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{
		if (yystack.l_mark[0].num.val > INT_MAX || yystack.l_mark[0].num.val < INT_MIN)
			cfgerror("overflow %" PRId64, yystack.l_mark[0].num.val);
		else
			yyval.i32 = (int32_t)yystack.l_mark[0].num.val;
	}
break;
case 137:
#line 701 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ wrap_continue(); }
break;
case 138:
#line 702 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ wrap_cleanup(); }
break;
case 160:
#line 731 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ addattr(yystack.l_mark[0].str); }
break;
case 161:
#line 735 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ delattr(yystack.l_mark[0].str); }
break;
case 168:
#line 763 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setmaxusers(yystack.l_mark[0].i32); }
break;
case 169:
#line 767 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setident(yystack.l_mark[0].str); }
break;
case 170:
#line 771 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setident(NULL); }
break;
case 171:
#line 776 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ addconf(&conf); }
break;
case 172:
#line 780 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ delconf(yystack.l_mark[0].str); }
break;
case 173:
#line 784 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ delpseudo(yystack.l_mark[0].str); }
break;
case 174:
#line 788 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ addpseudo(yystack.l_mark[-1].str, yystack.l_mark[0].i32); }
break;
case 175:
#line 792 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ addpseudoroot(yystack.l_mark[0].str); }
break;
case 176:
#line 797 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ deldevi(yystack.l_mark[-2].str, yystack.l_mark[0].str); }
break;
case 177:
#line 801 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ deldeva(yystack.l_mark[0].str); }
break;
case 178:
#line 805 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ deldev(yystack.l_mark[0].str); }
break;
case 179:
#line 810 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ adddev(yystack.l_mark[-4].str, yystack.l_mark[-2].str, yystack.l_mark[-1].loclist, yystack.l_mark[0].i32); }
break;
case 182:
#line 821 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ addfsoption(yystack.l_mark[0].str); }
break;
case 185:
#line 832 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ delfsoption(yystack.l_mark[0].str); }
break;
case 188:
#line 844 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ addmkoption(yystack.l_mark[-2].str, yystack.l_mark[0].str); }
break;
case 189:
#line 845 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ appendmkoption(yystack.l_mark[-2].str, yystack.l_mark[0].str); }
break;
case 192:
#line 857 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ delmkoption(yystack.l_mark[0].str); }
break;
case 195:
#line 868 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ addoption(yystack.l_mark[0].str, NULL); }
break;
case 196:
#line 869 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ addoption(yystack.l_mark[-2].str, yystack.l_mark[0].str); }
break;
case 199:
#line 880 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ deloption(yystack.l_mark[0].str); }
break;
case 200:
#line 885 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{
		conf.cf_name = yystack.l_mark[0].str;
		conf.cf_lineno = currentline();
		conf.cf_fstype = NULL;
		conf.cf_root = NULL;
		conf.cf_dump = NULL;
	}
break;
case 201:
#line 896 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setconf(&conf.cf_root, "root", yystack.l_mark[0].list); }
break;
case 202:
#line 897 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setconf(&conf.cf_root, "root", yystack.l_mark[-1].list); }
break;
case 203:
#line 902 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_si(intern("?"),
					    (long long)NODEV); }
break;
case 204:
#line 904 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_si(yystack.l_mark[0].str,
					    (long long)NODEV); }
break;
case 205:
#line 906 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.list = new_si(NULL, yystack.l_mark[0].val); }
break;
case 206:
#line 911 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.val = (int64_t)makedev(yystack.l_mark[-2].num.val, yystack.l_mark[0].num.val); }
break;
case 207:
#line 916 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setfstype(&conf.cf_fstype, intern("?")); }
break;
case 208:
#line 917 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setfstype(&conf.cf_fstype, yystack.l_mark[0].str); }
break;
case 211:
#line 928 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ setconf(&conf.cf_dump, "dumps", yystack.l_mark[0].list); }
break;
case 212:
#line 933 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.i32 = 1; }
break;
case 213:
#line 934 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.i32 = yystack.l_mark[0].i32; }
break;
case 214:
#line 939 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 215:
#line 940 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = starref(yystack.l_mark[-1].str); }
break;
case 216:
#line 945 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = NULL; }
break;
case 217:
#line 946 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 218:
#line 947 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = wildref(yystack.l_mark[-1].str); }
break;
case 219:
#line 952 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = NULL; }
break;
case 220:
#line 953 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = yystack.l_mark[0].loclist; app(yystack.l_mark[0].loclist, yystack.l_mark[-1].loclist); }
break;
case 221:
#line 958 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = MK3(loc, yystack.l_mark[-1].str, NULL, 0); }
break;
case 222:
#line 959 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = namelocvals(yystack.l_mark[-1].str, yystack.l_mark[0].loclist); }
break;
case 223:
#line 964 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.i32 = 0; }
break;
case 224:
#line 965 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.i32 = yystack.l_mark[0].i32; }
break;
case 227:
#line 988 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.condexpr = MKF2(cx, or, yystack.l_mark[-2].condexpr, yystack.l_mark[0].condexpr); }
break;
case 229:
#line 993 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.condexpr = MKF2(cx, and, yystack.l_mark[-2].condexpr, yystack.l_mark[0].condexpr); }
break;
case 231:
#line 1003 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.condexpr = yystack.l_mark[0].condexpr; }
break;
case 232:
#line 1004 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.condexpr = MKF1(cx, not, yystack.l_mark[0].condexpr); }
break;
case 233:
#line 1005 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.condexpr = yystack.l_mark[-1].condexpr; }
break;
case 234:
#line 1010 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.condexpr = MKF1(cx, atom, yystack.l_mark[0].str); }
break;
case 235:
#line 1021 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 236:
#line 1022 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 237:
#line 1027 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = NULL; }
break;
case 238:
#line 1028 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 239:
#line 1033 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 240:
#line 1034 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 241:
#line 1039 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 242:
#line 1040 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 243:
#line 1041 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 244:
#line 1042 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{
		char bf[40];

		(void)snprintf(bf, sizeof(bf), FORMAT(yystack.l_mark[0].num), (long long)yystack.l_mark[0].num.val);
		yyval.str = intern(bf);
	  }
break;
case 245:
#line 1052 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 246:
#line 1053 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.str = yystack.l_mark[0].str; }
break;
case 247:
#line 1059 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = MKF2(loc, val, yystack.l_mark[0].str, NULL); }
break;
case 248:
#line 1060 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.loclist = MKF2(loc, val, yystack.l_mark[-2].str, yystack.l_mark[0].loclist); }
break;
case 249:
#line 1065 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.num = yystack.l_mark[0].num; }
break;
case 250:
#line 1066 "/home/k/GeDONE/contrib/dde_rump-fe90aa7ca739eb69c0e0e422756f1d36f3d744ec/src/lib/dde_rump/src/tools/config/../../usr.bin/config/gram.y"
	{ yyval.num.fmt = yystack.l_mark[0].num.fmt; yyval.num.val = -yystack.l_mark[0].num.val; }
break;
#line 2188 "gram.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            if ((yychar = YYLEX) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = yytname[YYTRANSLATE(yychar)];
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
    {
        goto yyoverflow;
    }
    *++yystack.s_mark = (short) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
