
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CLA.y ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
%{

#include "reduce.h"

#include <string.h>
#include <stdlib.h>

#define alloca           malloc
#define YYSTYPE          Val
#define YYERROR_VERBOSE
#define yyerror(mesg)    error(mesg,-1)

yylex(void);

%}

%token  INT_NUM  FLOAT_NUM  ID    int_  float_  if_  else_
%token  while_   print      read  Prog  var     is   RELOP

%left  '+'  '-'  or          /* don't use factor and term because bison handles */
%left  '*'  '/'  and         /* priorities */

%%

PROGRAM:
  Prog PROGID '{' var DECLIST ';' STMTLIST '}' '.'  {reduce_program($7)};

PROGID:
  ID         {if (reduce_progid($1.lexeme))
                  yyerror("declaration error")};
DECLIST:
  DECLIST ',' ONEVAR |
  ONEVAR;

UID:                                                          /* because QX.EXE wants */
  ID                 {strcpy($$.lexeme, strlwr($1.lexeme))};  /* lowercase variables */

ONEVAR:
  UID ':' int_       {reduce_onevar($1,INT_NUM)}      |
  UID ':' float_     {reduce_onevar($1,FLOAT_NUM)};

STMTLIST:
  STMTLIST STMT ';'  {$$=reduce_stmtlist($1,$2)} |
  /* epsilon */      {$$.last=$$.code=0};            /* start of chainlist */

STMT:
  if_    '(' BOOLEXPR   ')' STMT DUMMY else_ STMT     {$$=reduce_if($3,$5,$6,$8)} |
  while_ '(' BOOLEXPR   ')' STMT                      {$$=reduce_while($3,$5)}    |
  read   '(' UID        ')'                           {$$=reduce_read($3)}        |
  print  '(' EXPRESSION ')'                           {$$=reduce_println($3)}     |
  UID is EXPRESSION                                   {$$=reduce_assign($1,$3)}   |
  '{' STMTLIST '}'                                    {$$=$2}                     |
  error                                               {yyerrok};

DUMMY:                                   /* handles follow_label from true_stmt */
  /* epsilon */    {$$=reduce_dummy()}   /* nice without complications */

BOOLEXPR:
  EXPRESSION RELOP EXPRESSION {$$=reduce_boolexpr($1,$2,$3)}                   |
  '(' BOOLEXPR ')'            {$$=$2}                                          |
  BOOLEXPR   and   BOOLEXPR   {$2.lexeme[0]='a'; $$=reduce_boolexpr($1,$2,$3)} |
  BOOLEXPR   or    BOOLEXPR   {$2.lexeme[0]='o'; $$=reduce_boolexpr($1,$2,$3)};  /* small */
                                                                          /* addition to cpl */
EXPRESSION:
  EXPRESSION '+' EXPRESSION    {$$=reduce_expr($1,'+',$3)}       |
  EXPRESSION '-' EXPRESSION    {$$=reduce_expr($1,'-',$3)}       |
  EXPRESSION '*' EXPRESSION    {$$=reduce_expr($1,'*',$3)}       |
  EXPRESSION '/' EXPRESSION    {$$=reduce_expr($1,'/',$3)}       |
  '(' EXPRESSION ')'           {$$=$2}                           |
  UID                          {$$=reduce_expr($1,ID,$1)}        |  /* ID/FLOAT/INT */
  FLOAT_NUM                    {$$=reduce_expr($1,FLOAT_NUM,$1)} |  /* are the types of $1 */
  INT_NUM                      {$$=reduce_expr($1,INT_NUM,$1)};

%%
