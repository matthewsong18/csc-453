/*
 * File: scanner.h
 * Author: Saumya Debray
 * Purpose: Lists tokens and their values for use in the CSC 453 project
 */

#ifndef __SCANNER_H__
#define __SCANNER_H__

/*
 * The enum Token defines integer values for the various tokens.  These
 * are the values returned by the scanner.
 */
typedef enum {
  UNDEF     /* undefined */,
  ID        /* identifier: e.g., x, abc, p_q_12 */,
  INTCON    /* integer constant: e.g., 12345 */,
  LPAREN    /* '(' : Left parenthesis */,
  RPAREN    /* ')' : Right parenthesis */,
  LBRACE    /* '{' : Left curly brace */,
  RBRACE    /* '}' : Right curly brace */,
  COMMA     /* ',' : Comma */,
  SEMI      /*	;  : Semicolon */,
  kwINT     /*	int */,
  kwIF      /*	if */,
  kwELSE    /*	else */,
  kwWHILE   /*	while */,
  kwRETURN  /*	return */,
  opASSG    /*	= : Assignment */,
  opADD     /*	+ : addition */,
  opSUB     /*	– : subtraction */,
  opMUL     /*	* : multiplication */,
  opDIV     /*	/ : division */,
  opEQ      /*	== : Op: equals */,
  opNE      /*	!= : op: not-equals */,
  opGT      /*	>  : Op: greater-than */,
  opGE      /*	>= : Op: greater-or-equal */,
  opLT      /*	<  : Op: less-than */,
  opLE      /*	<= : Op: less-or-equal */,
  opAND     /*	&& : Op: logical-and */,
  opOR      /*	|| : Op: logical-or */,
  opNOT     /* ! : Op: logical-not */
} Token;

typedef struct TokenMatch {
 const char *(*match)(const char *input);
 const char *name;
 int value;
} TokenMatch;

void scanner_init_with_string(const char *input_string);
void scanner_init_with_stdin(void);

extern const TokenMatch M_UNDEF;     /* undefined */
extern const TokenMatch M_ID;        /* identifier: e.g., x, abc, p_q_12 */
extern const TokenMatch M_INTCON;    /* integer constant: e.g., 12345 */
extern const TokenMatch M_LPAREN;    /* '(' : Left parenthesis */
extern const TokenMatch M_RPAREN;    /* ')' : Right parenthesis */
extern const TokenMatch M_LBRACE;    /* '{' : Left curly brace */
extern const TokenMatch M_RBRACE;    /* '}' : Right curly brace */
extern const TokenMatch M_COMMA;     /* ',' : Comma */
extern const TokenMatch M_SEMI;      /*	;  : Semicolon */
extern const TokenMatch M_kwINT;     /*	int */
extern const TokenMatch M_kwIF;      /*	if */
extern const TokenMatch M_kwELSE;    /*	else */
extern const TokenMatch M_kwWHILE;   /*	while */
extern const TokenMatch M_kwRETURN;  /*	return */
extern const TokenMatch M_opASSG;    /*	= : Assignment */
extern const TokenMatch M_opADD;     /*	+ : addition */
extern const TokenMatch M_opSUB;     /*	– : subtraction */
extern const TokenMatch M_opMUL;     /*	* : multiplication */
extern const TokenMatch M_opDIV;     /*	/ : division */
extern const TokenMatch M_opEQ;      /*	== : Op: equals */
extern const TokenMatch M_opNE;      /*	!= : op: not-equals */
extern const TokenMatch M_opGT;      /*	>  : Op: greater-than */
extern const TokenMatch M_opGE;      /*	>= : Op: greater-or-equal */
extern const TokenMatch M_opLT;      /*	<  : Op: less-than */
extern const TokenMatch M_opLE;      /*	<= : Op: less-or-equal */
extern const TokenMatch M_opAND;     /*	&& : Op: logical-and */
extern const TokenMatch M_opOR;      /*	|| : Op: logical-or */
extern const TokenMatch M_opNOT;     /* ! : Op: logical-not */

#endif  /* __SCANNER_H__ */
