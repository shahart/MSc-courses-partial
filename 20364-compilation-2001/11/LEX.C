#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "lex.h"

/*********************************************************** PRIVATE SECTION */

#define MAX_VARIABLE_LENGTH 8
#define MAX_NUMBER_LENGTH   6

#define  nextchar(lexbuf, index) lexbuf[(*index)++]
#define lookahead(lexbuf, index) lexbuf[(*index)]

#define END_OF_ARRAY "end_of_array" /* indicates end of array */

#define CHECK      1
#define DONT_CHECK 0

const char *check_strings[] =
{
    "REPEAT", "FAIL", "TRUE",                                     /* control symbol */
    "ABORT", "SEE", "TELL", "SEEN", "TOLD", "SYSTEM", "READ",
    "WRITE", "GET", "PUT", "NL", "CONSULT",                       /* keyword */
    "VAR", "NONVAR", "ATOM", "NUMBER", "REAL", "INTEGER", "NAME",
    "CALL",                                                       /* meta logical predicate */
    "SETOF", "BAGOF",                                             /* set manipulation predicate */
    "MOD", "EXP",                                                 /* some mulop */
    "IS",                                                         /* assignop */
    "LOG", "SQRT", "ABS", "SIN", "COS", "TAN",
    "ASIN", "ACOS", "ATAN",                                       /* funcop */
    END_OF_ARRAY
};

const char *dont_check_strings[] =
{
    "*", "/",                                          /* some mulop */
    "|",                                               /* orop */
    "=..",                                             /* eq2point */
    "==", "=:=", "=\\=", "=<", "<", "=", ">=", ">", /*  4 relop */
    ":-", ";", "^",                                 /* 12 otherop */
    "!", "->",                                      /* 15 impor, arrow */
    "[", "]", "(", ")", ",", ".",                      /* deliminiter */
    END_OF_ARRAY
};

/* trying to match lexbuf to some string in prolog */
/**/
int match(char *lexbuf, int *index, const char *strings[], int check_lookahead)
{
    int  i;
    char c;

    for (i = 0; strcmp(strings[i], END_OF_ARRAY) != 0; i++)
        if (strnicmp(strings[i], lexbuf+(*index), strlen(strings[i])) == 0) {
            c = lexbuf[(*index)+strlen(strings[i])];

            /* check_lookahead? check if what we found is not variable */
            /**/
            if (!check_lookahead || (!isalnum(c) && c != '_'))
                return i;
        }

    return -1;
}

/*********************************************************** PUBLIC SECTION */

Token_Type lex_nexttoken(char *lexbuf, int *token_beginning)
{
    Token_Type token;
    char       c;
    int        keyword, length, state = 0, *forward = (int *)malloc(sizeof(int));

    /* skip spaces */
    /**/
    while (isspace(lookahead(lexbuf, token_beginning)))
        (*token_beginning)++;

    *forward = *token_beginning;

    keyword = match(lexbuf, token_beginning, check_strings, CHECK);
    if (keyword != -1) {
        strcpy(token.lexeme, check_strings[keyword]);
        *token_beginning += strlen(check_strings[keyword]);
        if (keyword <= BAGOF) {
            strcpy(token.lexeme, "");
            token.token = keyword;
        }
        else if (strcmp(token.lexeme, "IS") == 0) {
            strcpy(token.lexeme, "");
            token.token = ASSIGNOP;
        }
        else if (strcmp(token.lexeme, "MOD") == 0 || strcmp(token.lexeme, "EXP") == 0)
            token.token = MULOP;
        else
            token.token = FUNCOP;

        return token;
    }

    keyword = match(lexbuf, token_beginning, dont_check_strings, DONT_CHECK);
    if (keyword != -1) {
        strcpy(token.lexeme, dont_check_strings[keyword]);
        *token_beginning += strlen(dont_check_strings[keyword]);
        if (token.lexeme[0] == '|') {
            strcpy(token.lexeme, "");
            token.token = OROP;
        }
        else if (strcmp(token.lexeme, "=..") == 0) {
            strcpy(token.lexeme, "");
            token.token = EQ2POINT;
        }
        else if (token.lexeme[0] == '*' || token.lexeme[0] == '/')
            token.token = MULOP;
        else if (token.lexeme[0] == ':' || token.lexeme[0] == ';' || token.lexeme[0] == '^')
            token.token = OTHEROP;
        else if (4 <= keyword && keyword < 12)
            token.token = RELOP;
        else
            token.token = keyword-15 + IMPOR;

        return token;
    }

    while (1) {
        switch (state) {
        case 0:
            c = nextchar(lexbuf, forward);
            if (isupper(c) || c == '_')
                state = 1;
            else if (islower(c))
                state = 2;
            else if (c == '\'')
                state = 4;
            else if (isdigit(c)) {
                token.token = REAL_NUMBER;
                state = 6;
            }
            else if (c == '-' || c == '+') {
                if (isdigit(lookahead(lexbuf, forward))) {
                    token.token = REAL_NUMBER;
                    state = 6;
                }
                else {
                    token.token = ADDOP;
                    state = 3;
                }
            }
            else if (c == '\0') {
                token.token = END_OF_FILE;
                return token;
            }
            else {
                token.token = UNIDENTIFIED;
             /* fprintf(stderr,"Lexical Error: Illegal character!\n"); */
                state = 3;
            }
            break;
        case 1:
            if (isalpha(nextchar(lexbuf, forward)))
                state = 1;
            else {
                length = *forward-1-*token_beginning;
                if (length > MAX_VARIABLE_LENGTH) {
                    fprintf(stderr,"Lexical Error: Variable is too long!\n");
                    length = MAX_VARIABLE_LENGTH;
                }
                strncpy(token.lexeme, lexbuf+(*token_beginning), length);
                token.lexeme[length] = '\0';
                token.token = VARIABLE;
                *token_beginning = *forward-1;
                return token;
            }
            break;
        case 2:
            c = nextchar(lexbuf, forward);
            if (isalnum(c) || c == '_')
                state = 2;
            else {
                (*forward)--;
                token.token = CONSTANT_SYMBOL;
                state = 3;
            }
            break;
        case 3:
            strncpy(token.lexeme, lexbuf+(*token_beginning), *forward-*token_beginning);
            token.lexeme[*forward-*token_beginning] = '\0';
            *token_beginning = *forward;
            return token;
        case 4:
            c = nextchar(lexbuf, forward);
            if (c == '\0') {
                fprintf(stderr,"Lexical Error: Constant - Missing '!\n");
                (*forward)--;
                state = 5;
            }
            else if (c != '\'')
                state = 4;
            else
                state = 5;
            break;
        case 5:
            length = *forward-*token_beginning;
            if (length > MAX_LEXEME_LENGTH) {
                fprintf(stderr,"Lexical Error: Constant is too long!\n");
                length = MAX_LEXEME_LENGTH-1;
            }
            strncpy(token.lexeme, lexbuf+(*token_beginning), length);
            token.lexeme[length] = '\0';
            token.token = CONSTANT_SYMBOL;
            *token_beginning = *forward;
            return token;
        case 6:
            c = nextchar(lexbuf, forward); /* [8992].23e-234 */
            if (isdigit(c))
                state = 6;
            else if (c == '.')
                state = 7;
            else if (toupper(c) == 'E')
                state = 8;
            else {
                token.token = INTEGER_NUMBER;
                state = 12;
            }
            break;
        case 7:
            c = nextchar(lexbuf, forward); /* 8992[.]23e-234 */
            if (isdigit(c))
                state = 9;
            else if (toupper(c) == 'E')
                state = 8;
            else
                state = 12;
            break;
        case 8:
            c = nextchar(lexbuf, forward); /* 8992.23[e]-234 */
            if (isdigit(c))
                state = 10;
            else if (c == '+' || c == '-')
                state = 11;
            else {
                (*forward)--;
                state = 12;
            }
            break;
        case 9:
            c = nextchar(lexbuf, forward); /* 8992.[23]e-234 */
            if (isdigit(c))
                state = 9;
            else if (toupper(c) == 'E')
                state = 8;
            else
                state = 12;
            break;
        case 10:
            /* 8992.23e-[234] */
            if (isdigit(nextchar(lexbuf, forward)))
                state = 10;
            else
                state = 12;
            break;
        case 11:
            /* 8992.23e[-]234 */
            if (isdigit(nextchar(lexbuf, forward)))
                state = 10;
            else {
                (*forward)-=2;
                state = 12;
            }
            break;
        case 12:
            length = *forward-1-*token_beginning;
            if (length > MAX_NUMBER_LENGTH) {
                fprintf(stderr,"Lexical Error: Number is too long!\n");
             /* length = MAX_NUMBER_LENGTH; */
            }
            strncpy(token.lexeme, lexbuf+(*token_beginning), length);
            token.lexeme[length] = '\0';
            *token_beginning = *forward-1;
            return token;
        }
    }
}
