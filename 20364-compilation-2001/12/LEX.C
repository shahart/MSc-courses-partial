#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "lex.h"
#include "symbol.h"

/*********************************************************** PRIVATE SECTION */

#define MAX_ID_LENGTH 9

#define  nextchar(lexbuf, index) lexbuf[(*index)++]
#define lookahead(lexbuf, index) lexbuf[(*index)]

#define END_OF_ARRAY "end_of_array" /* indicates end of array */

#define CHECK      1
#define DONT_CHECK 0

const char *check_strings[] =
{
    "else", "float", "if", "int", "println", "Prog", "read", "var", "while", "is",
    END_OF_ARRAY
};

const char *dont_check_strings[] =
{
    "{", "}", "(", ")", ",", ".", ";", ":", /*  8 - the index of the next string */
    "==", "<>", "<", ">",                   /* 12 */
    "+", "-",                               /* 14 */
    "*", "/",                               /* 16 */
    END_OF_ARRAY
};

/* trying to match lexbuf to some string in cpl language */
/**/
int match(char *lexbuf, int *index, const char *strings[], int check_lookahead)
{
    int  i;
    char c;

    for (i = 0; strcmp(strings[i], END_OF_ARRAY) != 0; i++)
        if (strncmp(strings[i], lexbuf+(*index), strlen(strings[i])) == 0) {
            c = lexbuf[(*index)+strlen(strings[i])];

            /* check_lookahead? check if what we found is not variable */
            /**/
            if (!check_lookahead || !isalnum(c))
                return (i);
        }

    return (-1);
}

void error(char *message, int position, FILE *list_file)
{
    while (position--)
        fprintf(list_file, " ");
    fprintf(list_file,"       © Lexical Error: %s!\n", message);
}

/*********************************************************** PUBLIC SECTION */

Token_Type lex_nexttoken(char *lexbuf, int *token_beginning, int *line_no, FILE *list_file, int *old_index)
{
    Token_Type token, symbol_ptr;
    char       c, line300[300];
    int        keyword, length, state = 0, *forward = (int *)malloc(sizeof(int));

    /* skip spaces */
    /**/
    while (isspace(lookahead(lexbuf, token_beginning))) {
        if (lookahead(lexbuf, token_beginning) == '\n') {
            strncpy(line300, lexbuf+*old_index, *token_beginning+1-*old_index);
            line300[*token_beginning+1-*old_index] = '\0';
            *old_index = *token_beginning+1;
            fprintf(list_file, "%5d  %s", ++(*line_no), line300);
        }
        (*token_beginning)++;
    }

    /* skip remarks */
    /**/
    if (strncmp("/*", lexbuf+(*token_beginning), 2) == 0) {
        *token_beginning +=2;
        while (strncmp("*/", lexbuf+(*token_beginning), 2) != 0)
            (*token_beginning)++;
        *token_beginning += 2;
    }

    /* skip spaces */
    /**/
    while (isspace(lookahead(lexbuf, token_beginning))) {
        if (lookahead(lexbuf, token_beginning) == '\n') {
            strncpy(line300, lexbuf+*old_index, *token_beginning+1-*old_index);
            line300[*token_beginning+1-*old_index] = '\0';
            *old_index = *token_beginning+1;
            fprintf(list_file, "%5d  %s", ++(*line_no), line300);
        }
        (*token_beginning)++;
    }

    *forward = *token_beginning;

    keyword = match(lexbuf, token_beginning, check_strings, CHECK);
    if (keyword != -1) {
        strcpy(token.lexeme, "");
        *token_beginning += strlen(check_strings[keyword]);
        token.token = keyword;

       return (token);
    }

    keyword = match(lexbuf, token_beginning, dont_check_strings, DONT_CHECK);
    if (keyword != -1) {
        strcpy(token.lexeme, dont_check_strings[keyword]);
        *token_beginning += strlen(dont_check_strings[keyword]);
        if (keyword < 8) {
            strcpy(token.lexeme, "");
            token.token = keyword+BLOCK_BEGIN;
        }
        else if (keyword < 12)
            token.token = RELOP;
        else if (keyword < 14)
            token.token = ADDOP;
        else if (keyword < 16)
            token.token = MULOP;

        return (token);
    }

    while (1) {
        switch (state) {
        case 0:
            c = nextchar(lexbuf, forward);
            if (isalpha(c))
                state = 1;
            else if (isdigit(c)) {
                token.token = FLOAT_NUM;
                state = 2;
            }
            else if (c == '\0') {
                token.token = END_OF_FILE;
                strncpy(line300, lexbuf+*old_index, *token_beginning+1-*old_index);
                line300[*token_beginning+1-*old_index] = '\0';
                fprintf(list_file, "%5d  %s", ++(*line_no), line300);
                return (token);
            }
            else {
                token.token = UNIDENTIFIED;
                error("Invalid character(s)", *token_beginning-*old_index, list_file);
                while (!isalnum(c) && !strchr("+-*/<>={}(),:;.\0", c) && !isspace(c))
                    c = nextchar(lexbuf, forward);
                *token_beginning = *forward-1;
                return (token);
            }
            break;
        case 1:
            if (!isalnum(nextchar(lexbuf, forward))) {
                length = *forward-1-*token_beginning;
                if (length > MAX_ID_LENGTH) {
                    error("ID is too long", *token_beginning-*old_index, list_file);
                    length = MAX_ID_LENGTH;
                }
                strncpy(token.lexeme, lexbuf+(*token_beginning), length);
                token.lexeme[length] = '\0';
                token.token = ID;
                *token_beginning = *forward-1;
                symbol_ptr = symbol_lookup(token.lexeme);
                if (symbol_ptr.token == END_OF_FILE)
                    symbol_insert(token);
                return (token);
            }
            break;
        case 2:
            /* [8992].23 */
            c = nextchar(lexbuf, forward);
            if (c == '.')
                state = 3;
            else if (!isdigit(c)) {
                token.token = INT_NUM;
                state = 5;
            }
            break;
        case 3:
            /* 8992[.]23 */
            if (isdigit(nextchar(lexbuf, forward)))
                state = 4;
            else
                state = 5;
            break;
        case 4:
            /* 8992.[23] */
            if (!isdigit(nextchar(lexbuf, forward)))
                state = 5;
            break;
        case 5:
            length = *forward-1-*token_beginning;
            strncpy(token.lexeme, lexbuf+(*token_beginning), length);
            token.lexeme[length] = '\0';
            *token_beginning = *forward-1;
            symbol_insert(token);
            return (token);
        }
    }
}
