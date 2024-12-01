#ifndef _LEX_H_
#define _LEX_H_

#include <stdio.h>

#define MAX_LEXEME_LENGTH 20

typedef enum Tokens
{
    ELSE, FLOAT, IF, INT, PRINTLN, PROG, READ, VAR, WHILE, ASSIGNOP,
    BLOCK_BEGIN, BLOCK_END, LPAR, RPAR, COMMA, POINT, POINT_COMMA, COLON,
    RELOP, ADDOP, MULOP,
    FLOAT_NUM, INT_NUM, ID,
    UNIDENTIFIED,
    END_OF_FILE
} Tokens;

typedef struct Token_Type
{
    Tokens token;
    char   lexeme[MAX_LEXEME_LENGTH];
} Token_Type;

Token_Type lex_nexttoken(char *lexbuf, int *index, int *line_no, FILE *list_file, int *old_index);

#endif
