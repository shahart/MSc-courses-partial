#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lex.h"

#define MAX_FILE_NAME   20
#define MAX_FILE_LENGTH 25000

const char *token_names[] =
{
    "REPEAT", "FAIL", "TRUE",                                             /* control symbol */
    "ABORT", "SEE", "TELL", "SEEN", "TOLD", "SYSTEM", "READ", "WRITE",
    "GET", "PUT", "NL", "CONSULT",                                        /* keyword */
    "VAR", "NONVAR", "ATOM", "NUMBER", "REAL", "INTEGER", "NAME", "CALL", /* meta logical predicate */
    "SETOF", "BAGOF",                                                     /* set manipulation predicate */
    "FUNCOP", "OROP", "ASSIGNOP", "RELOP", "ADDOP", "MULOP", "OTHEROP",   /* operators */
    "IMPOR", "ARROW",                                                     /* !,-> */
    "LPAR2", "RPAR2", "LPAR", "RPAR", "COMMA", "POINT",                   /* deliminiter */
    "EQ2POINT",                                                           /* =.. */
    "INTEGER_NUMBER", "REAL_NUMBER", "VARIABLE", "CONSTANT_SYMBOL",
    "UNIDENTIFIED"
};

void main(int argc, char *argv[])
{
    char *input_file_name, *extension, token_file_name[MAX_FILE_NAME];
    FILE *input_file, *token_file;
    int  input_file_size;

    char lexbuf[MAX_FILE_LENGTH];
    int  *index = (int *)malloc(sizeof(int));

    Token_Type token;
    char       token_string[20];

    if (argc != 2) {
        printf("usage: lexpro <filename>.pro\n");
        exit(1);
    }

    input_file_name = (char *)malloc(strlen(argv[1])+1);
    strcpy(input_file_name, argv[1]);
    extension = strtok(input_file_name, ".");
    extension = strtok(NULL, ".");

    if (strcmp(extension, "pro") != 0 && strcmp(extension, "PRO") != 0) {
        printf("usage: lexpro <filename>.pro\n");
        exit(1);
    }

    input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        fprintf(stderr,"System Error: Can't read PRO file!\n");
        exit(1);
    }

    strcpy(token_file_name, argv[1]);
    strtok(token_file_name, ".");
    strcat(token_file_name, ".tok");

    token_file = fopen(token_file_name, "w");
    if (token_file == NULL) {
        fprintf(stderr,"System Error: Can't write TOK file!\n");
        exit(1);
    }

    input_file_size = fread(lexbuf, sizeof(char), MAX_FILE_LENGTH, input_file);
    lexbuf[input_file_size] = '\0';
    *index = 0;

    fprintf(token_file, "Token             Lexeme/Attr\n");
    fprintf(token_file, "-----             -----------\n");
    printf("\n");

    token = lex_nexttoken(lexbuf, index);
    while (token.token != END_OF_FILE) {
        strcpy(token_string, "                   ");
        strncpy(token_string, token_names[token.token], strlen(token_names[token.token]));
        fprintf(token_file, "%s%s\n", token_string, token.lexeme);
        printf("%s%s\n", token_string, token.lexeme);
        token = lex_nexttoken(lexbuf, index);
    }
}
