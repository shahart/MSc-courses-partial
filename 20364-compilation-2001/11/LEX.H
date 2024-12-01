#ifndef _LEX_H_
#define _LEX_H_

#define MAX_LEXEME_LENGTH 20

typedef enum Tokens
{
    REPEAT, FAIL, TRUE,                                           /* control symbol */
    ABORT, SEE, TELL, SEEN, TOLD, SYSTEM, READ, WRITE, GET, PUT,
    NL, CONSULT,                                                  /* keyword */
    VAR, NONVAR, ATOM, NUMBER, REAL, INTEGER, NAME, CALL,         /* meta logical predicate */
    SETOF, BAGOF,                                                 /* set manipulation predicate */
    FUNCOP, OROP, ASSIGNOP, RELOP, ADDOP, MULOP, OTHEROP,         /* operators */
    IMPOR, ARROW,                                                 /* !,-> */
    LPAR2, RPAR2, LPAR, RPAR, COMMA, POINT,                       /* deliminiter */
    EQ2POINT,                                                     /* =.. */
    INTEGER_NUMBER, REAL_NUMBER, VARIABLE, CONSTANT_SYMBOL,
    UNIDENTIFIED,
    END_OF_FILE
} Tokens;

typedef struct Token_Type
{
    Tokens token;
    char lexeme[MAX_LEXEME_LENGTH];
} Token_Type;

Token_Type lex_nexttoken(char *lexbuf, int *index);

#endif
