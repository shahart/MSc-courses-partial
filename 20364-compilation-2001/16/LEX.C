
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Lex.C ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

                                           /* based on maman 12  */
#include "lex.h"                           /* with modifications */
#include "cla_tab.h"

#include <stdlib.h>
#include <string.h>
#include  <ctype.h>
#include  <stdio.h>

/*********************************************************** PRIVATE SECTION */

int  old_index = 0,      /* start of current line */
     index     = 0,      /* position in lexbuf */
     line_no   = 0;      /* number of current line */
char line[500];

extern FILE *hlp_file;
extern int  found_error;

#define  nextchar(i) lexbuf[i++]
#define lookahead(i) lexbuf[i]

#define END_OF_ARRAY "end_of_array"

char *check_strings[] =
{
    "else", "float", "if", "int", "print", "Prog", "read", "var", "while", "is", "and", "or",
    "println", "real", "do", "od", "fi", "then", "begin", "end", "program", /* old fashion */
    END_OF_ARRAY                                                              /* cpl tokens */
};

int token_2_check_strings[] =
{
    else_, float_, if_, int_, print, Prog, read, var, while_, is, and, or, /* 12 valid items */
    print, float_, '{', '}', '}', INVALID, '{', '}', Prog                  /* old fashion */
};

char *dont_check_strings[] =
{
    "+", "-", "*", "/", "{", "}", "(", ")", ",", ".", ";", ":", /* 12 items */
    "==", "<>", "<", ">",                                       /* RELOPs */
    "=", "!=", "&&", "||",                                      /* old fashion */
    END_OF_ARRAY                                                  /* cpl tokens */
};
                        /* = ==, != <>, &&,  ||  */
int old_2_new_tokens[] = {RELOP, RELOP, and, or};

/* trying to match lexbuf to some token in cpl language */
/**/
int lex_match(int check_lookahead, char lexbuf[])
{
    int  i;
    char c,
         **strings = check_lookahead ? check_strings : dont_check_strings;

    for (i = 0; strcmp(strings[i], END_OF_ARRAY); i++) {
        strncpy(line, lexbuf+index, strlen(strings[i]));
        line[strlen(strings[i])] = 0;                     /* lowercase lexbuf into line */
        strlwr(line);
        if (token_2_check_strings[i] == Prog)
            line[0] = toupper(line[0]);

        if (!strcmp(strings[i], line)) {           /* match found? */
            c = lexbuf[index+strlen(strings[i])];  /* c=lookahead */

            /* check_lookahead? check if what we found is not variable */
            /**/
            if (!check_lookahead || !isalnum(c)) {
                if (strncmp(line, lexbuf+index, strlen(line)))  /* Prog is not prog,   */
                    error("reserved word?", 0);                 /* wHILe is not while, */
                return(i);                                      /* so warn the user    */
            }
        }
    }

    return(-1);    /* match not found */
}

void lex_skipspaces(char lexbuf[])
{
    while (isspace(lookahead(index)))
        if (nextchar(index) == '\n') {
            strncpy(line, lexbuf+old_index, index-old_index);
            line[index-old_index] = 0;
            old_index             = index;                   /* advance to next */
            fprintf(hlp_file, "%5d  %s", ++line_no, line);   /* line and no. */
        }
}

/*********************************************************** PUBLIC SECTION */

void error(char *mesg, int position)
{
    position += index - old_index;
    while (position--)                          /* print near to the error's object */
        fprintf(hlp_file, " ");
    fprintf(hlp_file,"       © %s!\n", mesg);
    found_error++;
}

Token lex_nexttoken(char lexbuf[])
{
    char  c;
    int   keyword,      /* match() return value */
          length,       /* to compare with MAX_ID_LENGTH */
          state = 0,    /* finite state machine's state */
          forward;      /* kind of lookahead index */
    Token token;

    lex_skipspaces(lexbuf);
                                            /* skip C style remarks */
    if (!strncmp("/*", lexbuf+index, 2)) {
        strnset(lexbuf+index, ' ', strstr(lexbuf+index, "*/")+2 - (lexbuf+index));
        lex_skipspaces(lexbuf);
    }

    forward = index;

    keyword = lex_match(1, lexbuf);   /* match keywords like while, Prog, float.. */
    if (keyword != -1) {
        index += strlen(check_strings[keyword]);
        token.token = token_2_check_strings[keyword];

        /* old fashion token? - change println to print, begin to { ... */
        if (keyword >= 12) {
            error("common error, token has changed", -strlen(check_strings[keyword]));
            found_error -= IGNORE_CMN_ERR;
        }

        return(token);
    }

    keyword = lex_match(0, lexbuf);   /* match ascii's like ,;<. */
    if (keyword != -1) {
        strcpy(token.lexeme, dont_check_strings[keyword]);
        index += strlen(dont_check_strings[keyword]);
        token.token = keyword < 12 ? token.lexeme[0] : RELOP;

        /* old fashion token? - change == to =, && to and ... */
        if (keyword >= 16) {
            error("common error, token has changed", -strlen(dont_check_strings[keyword]));
            found_error -= IGNORE_CMN_ERR;
            token.token = old_2_new_tokens[keyword-16];
            strcpy(token.lexeme, dont_check_strings[keyword-4]);
        }

        return(token);
    }

    while (1) {
        switch (state) {
        case 0:
            c = nextchar(forward);
            if (isalpha(c))
                state = 1;
            else if (isdigit(c)) {
                token.token = FLOAT_NUM;
                state       = 2;
            }
            else if (!c) {        /* end of file is '\0' */
                token.token = 0;
                fprintf(hlp_file, "%5d  %s", ++line_no, lexbuf+old_index);
                return(token);
            }
            else {
                token.token = INVALID;
                error("lexical error, undefined symbol", 0);
                index = forward;
                return(token);
            }
            break;
        case 1:
            /* ID */
            if (!isalnum(nextchar(forward))) {
                length = forward-1-index;
                if (length > MAX_ID_LENGTH) {
                    error("lexical error, ID is too long", 0);
                    length = MAX_ID_LENGTH;                      /* cut the lexeme */
                }
                strncpy(token.lexeme, lexbuf+index, length);
                token.lexeme[length] = 0;
                token.token          = ID;
                index                = forward-1;
                return(token);
            }
            break;
        case 2:
            /* [8992].23 */
            c = nextchar(forward);
            if (c == '.')
                state = 3;
            else if (!isdigit(c)) {
                token.token = INT_NUM;
                state       = 5;
            }
            break;
        case 3:
            /* 8992[.]23 */
            if (isdigit(nextchar(forward)))
                state = 4;
            else
                state = 5;
            break;
        case 4:
            /* 8992.[23] */
            if (!isdigit(nextchar(forward)))
                state = 5;
            break;
        case 5:
            /* INT/FLOAT */
            length = forward-1-index;
            strncpy(token.lexeme, lexbuf+index, length);
            token.lexeme[length] = 0;
            index                = forward-1;
            return(token);
        }
    }
}
