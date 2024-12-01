
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CPQ.C ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "lex.h"
#include "symtbl.h"
#include "reduce.h"
#include "cla_tab.h"
#include "code.h"

#include  <stdio.h>
#include <string.h>
#include <stdlib.h>
#include  <ctype.h>

#define   MAX_FILE_LENGTH 10000

char      lexbuf[MAX_FILE_LENGTH] = {0};
Token     token;
FILE      *hlp_file;
Quadruple *progcode;     /* chainlist of the final quad program */
int       found_error;

yyparse(void);

yylex()        /* bison's lex analyzer structure requires info in yylval*/
{
    Val *v = (Val *)&yylval;

    do
        token = lex_nexttoken(lexbuf);    /* ignore lexical errors */
    while (token.token == INVALID);
    strcpy(v->lexeme, token.lexeme);

    return(token.token);
}

void main(int argc, char *argv[])
{
    char  *pchar,
          hlp_str[40];

    fprintf(stderr, "CPQ, by Shahar\n");

    strcpy(hlp_str, argv[1]);
    pchar = strtok(hlp_str, ".");    /* get extension */
    pchar = strtok(NULL, ".");
    if (strcmp(strlwr(pchar), "cpl") || argc != 2) {
        printf("Usage: cpq <filename>.cpl\n");
        return;
    }

    hlp_file = fopen(argv[1], "r");
    if (!hlp_file) {
        fprintf(stderr, "system error, Can't read file!\n");
        return;
    }

    found_error = fread(lexbuf, sizeof(char), 32000, hlp_file);  /* fread's size_t is int */
    fclose(hlp_file);
    if (found_error > MAX_FILE_LENGTH) {
        fprintf(stderr, "system error, File is too long!\n");
        return;
    }

    strcat(hlp_str, ".lst");
    hlp_file = fopen(hlp_str, "w");
    if (!hlp_file) {
        fprintf(stderr, "system error, Can't write LST file!\n");
        return;
    }
    fprintf(hlp_file, "Shahar\n");

    found_error = 0;

    if (!yyparse() && !found_error) {   /* lexical/semantic/syntax errors? */
        fclose(hlp_file);
        strcpy(hlp_str, argv[1]);
        pchar = strtok(hlp_str, ".");
        pchar = strtok(NULL, ".");
        strcat(hlp_str, ".qud");
        hlp_file = fopen(hlp_str, "w");
        if (!hlp_file) {
            fprintf(stderr, "system error, Can't write QUD file!\n");
            return;
        }

        code_print(progcode, hlp_file);
        symtbl_free(1);

        printf("QUD file ready to run.\n");
        fprintf(hlp_file, "Shahar\n");
    }
    else {
        printf("LST file contains errors!\n");

        while (token.token)                 /* finish creation of .lst file */
            token = lex_nexttoken(lexbuf);  /* by reading all tokens of .cpl */

        symtbl_free(0);   /* don't warn if uninitialized variables were found */
    }                     /* because file contains errors */

#ifndef MEM_RESERVE
    code_free();
#endif
    fclose(hlp_file);
}
