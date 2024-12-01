
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Lex.H ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef _LEX_H_
#define _LEX_H_

#define IGNORE_CMN_ERR  0  /* 1 ignores the convert of println to print, */
                                  /* do/begin/fi/od to {/} and so on */
#define MAX_ID_LENGTH   9
#define INVALID       300      /* illegal character like ~ in cpl file */

typedef char Lexstr[MAX_ID_LENGTH+1];

typedef struct Token
{
    int    token;
    Lexstr lexeme;
} Token;

Token lex_nexttoken(char lexbuf[]);
void  error        (char *mesg, int position);

#endif
