#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lex.h"
#include "symbol.h"

#define MAX_FILE_NAME   20
#define MAX_FILE_LENGTH 25000

const char *token_names[] =
{
    "else", "float", "if", "int", "println", "Prog", "read", "var", "while", "ASSIGNOP",
    "BLOCK_BEGIN", "BLOCK_END", "LPAR", "RPAR", "COMMA", "POINT", "POINT-COMMA", "COLON",
    "RELOP", "ADDOP", "MULOP",
    "FLOAT_NUM", "INT_NUM", "ID"
};

void main(int argc, char *argv[])
{
    char *input_file_name, *extension, output_file_name[MAX_FILE_NAME];
    FILE *input_file, *token_file, *list_file;
    int  input_file_size;

    char lexbuf[MAX_FILE_LENGTH];
    int  *index     = (int *)malloc(sizeof(int)),
         *old_index = (int *)malloc(sizeof(int)),
         *line_no   = (int *)malloc(sizeof(int));

    Token_Type token;
    char       token_string[20];

    if (argc != 2) {
        printf("Usage: cla <filename>.cpl\n");
        exit(1);
    }

    input_file_name = (char *)malloc(strlen(argv[1])+1);
    strcpy(input_file_name, argv[1]);
    extension = strtok(input_file_name, ".");
    extension = strtok(NULL, ".");

    if (strcmp(strlwr(extension), "cpl") != 0) {
        printf("Usage: cla <filename>.cpl\n");
        exit(1);
    }

    input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        fprintf(stderr,"System Error: Can't read CPL file!\n");
        exit(1);
    }

    strcpy(output_file_name, argv[1]);
    strtok(output_file_name, ".");
    strcat(output_file_name, ".tok");
    token_file = fopen(output_file_name, "w");
    if (token_file == NULL) {
        fprintf(stderr,"System Error: Can't write TOK file!\n");
        exit(1);
    }
    fprintf(token_file, "Shahar, 0\n\n");

    strcpy(output_file_name, argv[1]);
    strtok(output_file_name, ".");
    strcat(output_file_name, ".lst");
    list_file = fopen(output_file_name, "w");
    if (list_file == NULL) {
        fprintf(stderr,"System Error: Can't write LST file!\n");
        exit(1);
    }
    fprintf(list_file, "Shahar, 0\n\n");

    fprintf(stderr, "Shahar, 0\n");

    symbol_inittable();

    input_file_size = fread(lexbuf, sizeof(char), MAX_FILE_LENGTH, input_file);
    lexbuf[input_file_size] = '\0';

    *old_index = *index = *line_no = 0;

    token = lex_nexttoken(lexbuf, index, line_no, list_file, old_index);
    while (token.token != END_OF_FILE) {
        if (token.token != UNIDENTIFIED) {
            strcpy(token_string, "                   ");
            strncpy(token_string, token_names[token.token], strlen(token_names[token.token]));
            fprintf(token_file, "%s%s\n", token_string, token.lexeme);
        }
        token = lex_nexttoken(lexbuf, index, line_no, list_file, old_index);
    }
}
