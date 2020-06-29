
/*  A Bison parser, made from /home/php/php4/php-4.0.3/main/configuration-parser.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse cfgparse
#define yylex cfglex
#define yyerror cfgerror
#define yylval cfglval
#define yychar cfgchar
#define yydebug cfgdebug
#define yynerrs cfgnerrs
#define	TC_STRING	257
#define	TC_ENCAPSULATED_STRING	258
#define	SECTION	259
#define	CFG_TRUE	260
#define	CFG_FALSE	261
#define	EXTENSION	262
#define	T_ZEND_EXTENSION	263
#define	T_ZEND_EXTENSION_TS	264
#define	T_ZEND_EXTENSION_DEBUG	265
#define	T_ZEND_EXTENSION_DEBUG_TS	266


/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
 */



/* $Id: configuration-parser.y,v 1.54 2000/08/31 22:23:53 andi Exp $ */

#define DEBUG_CFG_PARSER 0
#include "php.h"
#include "php_globals.h"
#include "php_ini.h"
#include "ext/standard/dl.h"
#include "ext/standard/file.h"
#include "ext/standard/php_browscap.h"
#include "zend_extensions.h"


#if WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#include "win32/wfile.h"
#endif

#define YYSTYPE zval

#define PARSING_MODE_CFG		0
#define PARSING_MODE_BROWSCAP	1
#define PARSING_MODE_STANDALONE	2

static HashTable configuration_hash;
extern HashTable browser_hash;
PHPAPI extern char *php_ini_path;
static HashTable *active_hash_table;
static zval *current_section;
static char *currently_parsed_filename;

static int parsing_mode;

zval yylval;

extern int cfglex(zval *cfglval);
extern FILE *cfgin;
extern int cfglineno;
extern void init_cfg_scanner(void);

zval *cfg_get_entry(char *name, uint name_length)
{
	zval *tmp;

	if (zend_hash_find(&configuration_hash, name, name_length, (void **) &tmp)==SUCCESS) {
		return tmp;
	} else {
		return NULL;
	}
}


PHPAPI int cfg_get_long(char *varname,long *result)
{
	zval *tmp,var;
	
	if (zend_hash_find(&configuration_hash,varname,strlen(varname)+1,(void **) &tmp)==FAILURE) {
		*result=(long)NULL;
		return FAILURE;
	}
	var = *tmp;
	zval_copy_ctor(&var);
	convert_to_long(&var);
	*result = var.value.lval;
	return SUCCESS;
}


PHPAPI int cfg_get_double(char *varname,double *result)
{
	zval *tmp,var;
	
	if (zend_hash_find(&configuration_hash,varname,strlen(varname)+1,(void **) &tmp)==FAILURE) {
		*result=(double)0;
		return FAILURE;
	}
	var = *tmp;
	zval_copy_ctor(&var);
	convert_to_double(&var);
	*result = var.value.dval;
	return SUCCESS;
}


PHPAPI int cfg_get_string(char *varname, char **result)
{
	zval *tmp;

	if (zend_hash_find(&configuration_hash,varname,strlen(varname)+1,(void **) &tmp)==FAILURE) {
		*result=NULL;
		return FAILURE;
	}
	*result = tmp->value.str.val;
	return SUCCESS;
}


static void yyerror(char *str)
{
	char *error_buf;
	int error_buf_len;

	error_buf_len = 128+strlen(currently_parsed_filename); /* should be more than enough */
	error_buf = (char *) emalloc(error_buf_len);
	
	sprintf(error_buf, "Error parsing %s on line %d\n", currently_parsed_filename, cfglineno);
#ifdef PHP_WIN32
	MessageBox(NULL, error_buf, "PHP Error", MB_OK|MB_TOPMOST|0x00200000L);
#else
	fprintf(stderr, "PHP:  %s", error_buf);
#endif
	efree(error_buf);
}


static void pvalue_config_destructor(zval *pvalue)
{
	if (pvalue->type == IS_STRING && pvalue->value.str.val != empty_string) {
		free(pvalue->value.str.val);
	}
}


static void pvalue_browscap_destructor(zval *pvalue)
{
	if (pvalue->type == IS_OBJECT || pvalue->type == IS_ARRAY) {
		zend_hash_destroy(pvalue->value.obj.properties);
		free(pvalue->value.obj.properties);
	}
}


int php_init_config(void)
{
	PLS_FETCH();

	if (zend_hash_init(&configuration_hash, 0, NULL, (dtor_func_t) pvalue_config_destructor, 1)==FAILURE) {
		return FAILURE;
	}

#if USE_CONFIG_FILE
	{
		char *env_location,*default_location,*php_ini_search_path;
		int safe_mode_state = PG(safe_mode);
		char *open_basedir = PG(open_basedir);
		char *opened_path;
		int free_default_location=0;
		
		env_location = getenv("PHPRC");
		if (!env_location) {
			env_location="";
		}
#ifdef PHP_WIN32
		{
			if (php_ini_path) {
				default_location = php_ini_path;
			} else {
				default_location = (char *) malloc(512);
			
				if (!GetWindowsDirectory(default_location,255)) {
					default_location[0]=0;
				}
				free_default_location=1;
			}
		}
#else
		if (!php_ini_path) {
			default_location = CONFIGURATION_FILE_PATH;
		} else {
			default_location = php_ini_path;
		}
#endif

/* build a path */
		php_ini_search_path = (char *) malloc(sizeof(".")+strlen(env_location)+strlen(default_location)+2+1);

		if (!php_ini_path) {
#ifdef PHP_WIN32
			sprintf(php_ini_search_path,".;%s;%s",env_location,default_location);
#else
			sprintf(php_ini_search_path,".:%s:%s",env_location,default_location);
#endif
		} else {
			/* if path was set via -c flag, only look there */
			strcpy(php_ini_search_path,default_location);
		}
		PG(safe_mode) = 0;
		PG(open_basedir) = NULL;
		cfgin = php_fopen_with_path("php.ini","r",php_ini_search_path,&opened_path);
		free(php_ini_search_path);
		if (free_default_location) {
			free(default_location);
		}
		PG(safe_mode) = safe_mode_state;
		PG(open_basedir) = open_basedir;

		if (!cfgin) {
			return SUCCESS;  /* having no configuration file is ok */
		}

		if (opened_path) {
			zval tmp;
			
			tmp.value.str.val = strdup(opened_path);
			tmp.value.str.len = strlen(opened_path);
			tmp.type = IS_STRING;
			zend_hash_update(&configuration_hash,"cfg_file_path",sizeof("cfg_file_path"),(void *) &tmp,sizeof(zval),NULL);
#if DEBUG_CFG_PARSER
			php_printf("INI file opened at '%s'\n",opened_path);
#endif
			efree(opened_path);
		}
			
		init_cfg_scanner();
		active_hash_table = &configuration_hash;
		parsing_mode = PARSING_MODE_CFG;
		currently_parsed_filename = "php.ini";
		yyparse();
		fclose(cfgin);
	}
	
#endif
	
	return SUCCESS;
}


PHP_MINIT_FUNCTION(browscap)
{
	char *browscap = INI_STR("browscap");

	if (browscap) {
		if (zend_hash_init(&browser_hash, 0, NULL, (dtor_func_t) pvalue_browscap_destructor, 1)==FAILURE) {
			return FAILURE;
		}

		cfgin = V_FOPEN(browscap, "r");
		if (!cfgin) {
			php_error(E_WARNING,"Cannot open '%s' for reading", browscap);
			return FAILURE;
		}
		init_cfg_scanner();
		active_hash_table = &browser_hash;
		parsing_mode = PARSING_MODE_BROWSCAP;
		currently_parsed_filename = browscap;
		yyparse();
		fclose(cfgin);
	}

	return SUCCESS;
}


/* {{{ proto void parse_ini_file(string filename)
   Parse configuration file */
PHP_FUNCTION(parse_ini_file)
{
#ifdef ZTS
	php_error(E_WARNING, "parse_ini_file() is not supported in multithreaded PHP");
	RETURN_FALSE;
#else
	zval **filename;

	if (ARG_COUNT(ht)!=1 || zend_get_parameters_ex(1, &filename)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string_ex(filename);
	cfgin = V_FOPEN((*filename)->value.str.val, "r");
	if (!cfgin) {
		php_error(E_WARNING,"Cannot open '%s' for reading", (*filename)->value.str.val);
		return;
	}
	array_init(return_value);
	init_cfg_scanner();
	active_hash_table = return_value->value.ht;
	parsing_mode = PARSING_MODE_STANDALONE;
	currently_parsed_filename = (*filename)->value.str.val;
	yyparse();
	fclose(cfgin);
#endif
}
/* }}} */

int php_shutdown_config(void)
{
	zend_hash_destroy(&configuration_hash);
	return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(browscap)
{
	if (INI_STR("browscap")) {
		zend_hash_destroy(&browser_hash);
	}
	return SUCCESS;
}


static void convert_browscap_pattern(zval *pattern)
{
	register int i,j;
	char *t;

	for (i=0; i<pattern->value.str.len; i++) {
		if (pattern->value.str.val[i]=='*' || pattern->value.str.val[i]=='?' || pattern->value.str.val[i]=='.') {
			break;
		}
	}

	if (i==pattern->value.str.len) { /* no wildcards */
		pattern->value.str.val = zend_strndup(pattern->value.str.val, pattern->value.str.len);
		return;
	}

	t = (char *) malloc(pattern->value.str.len*2);
	
	for (i=0,j=0; i<pattern->value.str.len; i++,j++) {
		switch (pattern->value.str.val[i]) {
			case '?':
				t[j] = '.';
				break;
			case '*':
				t[j++] = '.';
				t[j] = '*';
				break;
			case '.':
				t[j++] = '\\';
				t[j] = '.';
				break;
			default:
				t[j] = pattern->value.str.val[i];
				break;
		}
	}
	t[j]=0;
	pattern->value.str.val = t;
	pattern->value.str.len = j;
}


void do_cfg_op(char type, zval *result, zval *op1, zval *op2)
{
	int i_result;
	int i_op1, i_op2;
	char str_result[MAX_LENGTH_OF_LONG];

	i_op1 = atoi(op1->value.str.val);
	free(op1->value.str.val);
	if (op2) {
		i_op2 = atoi(op2->value.str.val);
		free(op2->value.str.val);
	} else {
		i_op2 = 0;
	}

	switch (type) {
		case '|':
			i_result = i_op1 | i_op2;
			break;
		case '&':
			i_result = i_op1 & i_op2;
			break;
		case '~':
			i_result = ~i_op1;
			break;
		case '!':
			i_result = !i_op1;
			break;
		default:
			i_result = 0;
			break;
	}

	result->value.str.len = zend_sprintf(str_result, "%d", i_result);
	result->value.str.val = (char *) malloc(result->value.str.len+1);
	memcpy(result->value.str.val, str_result, result->value.str.len);
	result->value.str.val[result->value.str.len] = 0;
	result->type = IS_STRING;
}


void do_cfg_get_constant(zval *result, zval *name)
{
	zval z_constant;

	if (zend_get_constant(name->value.str.val, name->value.str.len, &z_constant)) {
		/* z_constant is emalloc()'d */
		convert_to_string(&z_constant);
		result->value.str.val = zend_strndup(z_constant.value.str.val, z_constant.value.str.len);
		result->value.str.len = z_constant.value.str.len;
		result->type = z_constant.type;
		zval_dtor(&z_constant);
		free(name->value.str.val);	
	} else {
		*result = *name;
	}
}


#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		45
#define	YYFLAG		-32768
#define	YYNTBASE	22

#define YYTRANSLATE(x) ((unsigned)(x) <= 267 ? yytranslate[x] : 28)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    18,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    16,     2,     2,     2,     2,    14,     2,    20,
    21,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    17,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    13,     2,    15,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    19
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     4,     8,    10,    14,    18,    22,    26,    30,
    32,    34,    36,    38,    40,    42,    44,    46,    48,    50,
    52,    56,    60,    63,    66,    70
};

static const short yyrhs[] = {    22,
    23,     0,     0,     3,    17,    25,     0,     3,     0,     8,
    17,    24,     0,     9,    17,    24,     0,    10,    17,    24,
     0,    11,    17,    24,     0,    12,    17,    24,     0,     5,
     0,    18,     0,     3,     0,     4,     0,    26,     0,     4,
     0,     6,     0,     7,     0,    18,     0,    19,     0,    27,
     0,    26,    13,    26,     0,    26,    14,    26,     0,    15,
    26,     0,    16,    26,     0,    20,    26,    21,     0,     3,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   441,   443,   446,   490,   491,   501,   509,   517,   525,   533,
   557,   561,   563,   566,   568,   569,   570,   571,   572,   575,
   577,   578,   579,   580,   581,   584
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","TC_STRING",
"TC_ENCAPSULATED_STRING","SECTION","CFG_TRUE","CFG_FALSE","EXTENSION","T_ZEND_EXTENSION",
"T_ZEND_EXTENSION_TS","T_ZEND_EXTENSION_DEBUG","T_ZEND_EXTENSION_DEBUG_TS","'|'",
"'&'","'~'","'!'","'='","'\\n'","'\\000'","'('","')'","statement_list","statement",
"cfg_string","string_or_value","expr","constant_string", NULL
};
#endif

static const short yyr1[] = {     0,
    22,    22,    23,    23,    23,    23,    23,    23,    23,    23,
    23,    24,    24,    25,    25,    25,    25,    25,    25,    26,
    26,    26,    26,    26,    26,    27
};

static const short yyr2[] = {     0,
     2,     0,     3,     1,     3,     3,     3,     3,     3,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     3,     3,     2,     2,     3,     1
};

static const short yydefact[] = {     2,
     0,     4,    10,     0,     0,     0,     0,     0,    11,     1,
     0,     0,     0,     0,     0,     0,    26,    15,    16,    17,
     0,     0,    18,    19,     0,     3,    14,    20,    12,    13,
     5,     6,     7,     8,     9,    23,    24,     0,     0,     0,
    25,    21,    22,     0,     0
};

static const short yydefgoto[] = {     1,
    10,    31,    26,    27,    28
};

static const short yypact[] = {-32768,
     2,   -14,-32768,   -11,     0,     6,     9,    28,-32768,-32768,
    18,    27,    27,    27,    27,    27,-32768,-32768,-32768,-32768,
    12,    12,-32768,-32768,    12,-32768,    30,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,    -5,    12,    12,
-32768,-32768,-32768,    29,-32768
};

static const short yypgoto[] = {-32768,
-32768,    26,-32768,   -21,-32768
};


#define	YYLAST		45


static const short yytable[] = {    36,
    37,    44,    11,    38,     2,    12,     3,    39,    40,     4,
     5,     6,     7,     8,    17,    41,    13,    42,    43,     9,
    17,    18,    14,    19,    20,    15,    21,    22,    45,    29,
    30,    25,    21,    22,     0,    23,    24,    25,    32,    33,
    34,    35,    39,    40,    16
};

static const short yycheck[] = {    21,
    22,     0,    17,    25,     3,    17,     5,    13,    14,     8,
     9,    10,    11,    12,     3,    21,    17,    39,    40,    18,
     3,     4,    17,     6,     7,    17,    15,    16,     0,     3,
     4,    20,    15,    16,    -1,    18,    19,    20,    13,    14,
    15,    16,    13,    14,    17
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */

/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 3:
{
#if DEBUG_CFG_PARSER
			printf("'%s' = '%s'\n",yyvsp[-2].value.str.val,yyvsp[0].value.str.val);
#endif
			yyvsp[0].type = IS_STRING;
			switch (parsing_mode) {
				case PARSING_MODE_CFG:
					zend_hash_update(active_hash_table, yyvsp[-2].value.str.val, yyvsp[-2].value.str.len+1, &yyvsp[0], sizeof(zval), NULL);
					if (active_hash_table == &configuration_hash) {
						php_alter_ini_entry(yyvsp[-2].value.str.val, yyvsp[-2].value.str.len+1, yyvsp[0].value.str.val, yyvsp[0].value.str.len+1, PHP_INI_SYSTEM, PHP_INI_STAGE_STARTUP);
					}
					break;
				case PARSING_MODE_BROWSCAP:
					if (current_section) {
						zval *new_property;
						char *new_key;

						new_property = (zval *) malloc(sizeof(zval));
						INIT_PZVAL(new_property);
						new_property->value.str.val = yyvsp[0].value.str.val;
						new_property->value.str.len = yyvsp[0].value.str.len;
						new_property->type = IS_STRING;
						
						new_key = zend_strndup(yyvsp[-2].value.str.val, yyvsp[-2].value.str.len);
						zend_str_tolower(new_key,yyvsp[-2].value.str.len);
						zend_hash_update(current_section->value.obj.properties, new_key, yyvsp[-2].value.str.len+1, &new_property, sizeof(zval *), NULL);
						free(new_key);
					}
					break;
				case PARSING_MODE_STANDALONE: {
						zval *entry;

						MAKE_STD_ZVAL(entry);
						entry->value.str.val = estrndup(yyvsp[0].value.str.val, yyvsp[0].value.str.len);
						entry->value.str.len = yyvsp[0].value.str.len;
						entry->type = IS_STRING;
						zend_hash_update(active_hash_table, yyvsp[-2].value.str.val, yyvsp[-2].value.str.len+1, &entry, sizeof(zval *), NULL);
						pvalue_config_destructor(&yyvsp[0]);
					}
					break;
			}		
			free(yyvsp[-2].value.str.val);
		;
    break;}
case 4:
{ free(yyvsp[0].value.str.val); ;
    break;}
case 5:
{
			if (parsing_mode==PARSING_MODE_CFG) {
				zval dummy;

#if DEBUG_CFG_PARSER
				printf("Loading '%s'\n",yyvsp[0].value.str.val);
#endif
				php_dl(&yyvsp[0],MODULE_PERSISTENT,&dummy);
			}
		;
    break;}
case 6:
{
			if (parsing_mode==PARSING_MODE_CFG) {
#if !defined(ZTS) && !ZEND_DEBUG
				zend_load_extension(yyvsp[0].value.str.val);
#endif
				free(yyvsp[0].value.str.val);
			}
		;
    break;}
case 7:
{ 
			if (parsing_mode==PARSING_MODE_CFG) {
#if defined(ZTS) && !ZEND_DEBUG
				zend_load_extension(yyvsp[0].value.str.val);
#endif
				free(yyvsp[0].value.str.val);
			}
		;
    break;}
case 8:
{ 
			if (parsing_mode==PARSING_MODE_CFG) {
#if !defined(ZTS) && ZEND_DEBUG
				zend_load_extension(yyvsp[0].value.str.val);
#endif
				free(yyvsp[0].value.str.val);
			}
		;
    break;}
case 9:
{ 
			if (parsing_mode==PARSING_MODE_CFG) {
#if defined(ZTS) && ZEND_DEBUG
				zend_load_extension(yyvsp[0].value.str.val);
#endif
				free(yyvsp[0].value.str.val);
			}
		;
    break;}
case 10:
{ 
			if (parsing_mode==PARSING_MODE_BROWSCAP) {
				zval *processed;

				/*printf("'%s' (%d)\n",$1.value.str.val,$1.value.str.len+1);*/
				current_section = (zval *) malloc(sizeof(zval));
				INIT_PZVAL(current_section);
				processed = (zval *) malloc(sizeof(zval));
				INIT_PZVAL(processed);

				current_section->value.obj.ce = &zend_standard_class_def;
				current_section->value.obj.properties = (HashTable *) malloc(sizeof(HashTable));
				current_section->type = IS_OBJECT;
				zend_hash_init(current_section->value.obj.properties, 0, NULL, (dtor_func_t) pvalue_config_destructor, 1);
				zend_hash_update(active_hash_table, yyvsp[0].value.str.val, yyvsp[0].value.str.len+1, (void *) &current_section, sizeof(zval *), NULL);

				processed->value.str.val = yyvsp[0].value.str.val;
				processed->value.str.len = yyvsp[0].value.str.len;
				processed->type = IS_STRING;
				convert_browscap_pattern(processed);
				zend_hash_update(current_section->value.obj.properties, "browser_name_pattern", sizeof("browser_name_pattern"), (void *) &processed, sizeof(zval *), NULL);
			}
			free(yyvsp[0].value.str.val);
		;
    break;}
case 12:
{ yyval = yyvsp[0]; ;
    break;}
case 13:
{ yyval = yyvsp[0]; ;
    break;}
case 14:
{ yyval = yyvsp[0]; ;
    break;}
case 15:
{ yyval = yyvsp[0]; ;
    break;}
case 16:
{ yyval = yyvsp[0]; ;
    break;}
case 17:
{ yyval = yyvsp[0]; ;
    break;}
case 18:
{ yyval.value.str.val = strdup(""); yyval.value.str.len=0; yyval.type = IS_STRING; ;
    break;}
case 19:
{ yyval.value.str.val = strdup(""); yyval.value.str.len=0; yyval.type = IS_STRING; ;
    break;}
case 20:
{ yyval = yyvsp[0]; ;
    break;}
case 21:
{ do_cfg_op('|', &yyval, &yyvsp[-2], &yyvsp[0]); ;
    break;}
case 22:
{ do_cfg_op('&', &yyval, &yyvsp[-2], &yyvsp[0]); ;
    break;}
case 23:
{ do_cfg_op('~', &yyval, &yyvsp[0], NULL); ;
    break;}
case 24:
{ do_cfg_op('!', &yyval, &yyvsp[0], NULL); ;
    break;}
case 25:
{ yyval = yyvsp[-1]; ;
    break;}
case 26:
{ do_cfg_get_constant(&yyval, &yyvsp[0]); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */


  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
