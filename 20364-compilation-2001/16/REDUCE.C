
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Reduce.C ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "reduce.h"
#include "code.h"
#include "symtbl.h"
#include "lex.h"
#include "cla_tab.h"

#include <stdlib.h>
#include <string.h>
#include  <stdio.h>
#include  <ctype.h>

/*********************************************************** PRIVATE SECTION */

extern Quadruple *progcode;

int    temps[2]     = {0,0},    /* int/float temps counters */
       label_number = 1;        /* counter of created labels */
Lexstr follow_label = "";       /* contains the last created label */

#define label_gen(dest) itoa(label_number++, dest, 10);

#define label_put(v) strcpy(v.code->label, follow_label); \
                     follow_label[0] = 0;
                                           /* "erase" the last created label */

#define temp_use(temp)  if (temp[0] == '_' && temps[temp[1] == 'i']) \
                            temps[temp[1] == 'i']--;
                                                       /* verify it's temp */
void temp_gen(Lexstr temp, int type)
{                                      /* format is _f4 is this is the 5th created temp */
    temp[0] = '_';                                              /* of float type */
    temp[1] = type == INT_NUM ? 'i' : 'f';
    itoa(temps[temp[1] == 'i']++, temp+2, 10);
    symtbl_lookup(temp, 0, type);               /* insert into symtbl for code generation */
}                                                              /* or optimization */

void code_concat(Val *v1, Val *v2)   /* attach v2->code to v1->last */
{
    if (v1->code) {
        v1->last->next = v2->code;
        if (v2->code)
            v1->last = v2->last;
    }
    else {
        v1->code = v2->code;
        v1->last = v2->last;
    }
}

void convert(Val *expr, char *arg)      /* handle 'integerr to real' */
{
    if (isalpha(expr->lexeme[0]) || expr->lexeme[0] == '_') {
        temp_gen(arg, FLOAT_NUM);
        temp_use(expr->lexeme);
        code_create(&(expr->last), "", ITOR, expr->lexeme, "", arg);
        if (!expr->code)
            expr->code = expr->last;
    }
    else if (strlen(expr->lexeme) < MAX_ID_LENGTH)  /* save time and space: 2 to 2. and */
        strcat(arg, ".");                           /* not 'itor _f4 2' */
}

/************************************************************ PUBLIC SECTION */

int reduce_progid(char *progid)
{
    int index;
                                                              /* only letters? */
    for (index = strlen(progid); index >= 1; index--)
        if (isupper(progid[index]) || isdigit(progid[index]))
            return(1);

    return(islower(progid[0]));      /* start with Uletter? */
}

void reduce_program(Val stmtlist)
{
    code_create(&stmtlist.last, follow_label, HALT, "", "", "");
    if (!stmtlist.code)
        stmtlist.code = stmtlist.last;   /* in case of 'Prog P { var f:float; }. */
    progcode = stmtlist.code;              /* code is empty */
}

Val reduce_stmtlist(Val stmtlist, Val stmt)
{
    code_concat(&stmtlist, &stmt);

    return(stmtlist);
}

void reduce_onevar(Val id, int type)
{
    if (symtbl_lookup(id.lexeme, 0, type))
        error("Redeclaration", -5);
}

Val reduce_println(Val expr)
{
    temp_use(expr.lexeme);
    code_create(&expr.last, "", expr.type == FLOAT_NUM ? RPRT : IPRT, "", "", expr.lexeme);
    if (!expr.code)
        expr.code = expr.last;
    label_put(expr);

    return(expr);
}

Val reduce_read(Val id)
{
    int type = symtbl_lookup(id.lexeme, -1, INT_NUM);   /* used=-1:id get value */
    Val result;

    if (!type)
        error("undefined symbol", -strlen(id.lexeme)-1);
    result.last = 0;
    code_create(&result.last, "", type == FLOAT_NUM ? RINP : IINP, "", "", id.lexeme);
    result.code = result.last;
    label_put(result);

    return(result);
}

Val reduce_dummy()
{
    Val result;

    result.last = 0;
    code_create(&result.last, "", JUMP, "", "", "");    /* jump to quadruple after "if" */
    result.code = result.last;
    label_put(result);

    return result;
}

Val reduce_if(Val boolexpr, Val true_stmt, Val dummy, Val false_stmt)
{
    Lexstr false_label;

    if (!follow_label[0])
        label_gen(follow_label);  /* jump to quadruple after "if" */

    if (!false_stmt.code)                     /* "else {};"? */
        strcpy(false_label, follow_label);
    else {
        strcpy(false_label, false_stmt.code->label);
        if (!false_stmt.code->label[0]) {                 /* false_label doesn't exist? */
            label_gen(false_label);
            strcpy(false_stmt.code->label, false_label);        /* create it */
        }
    }

    temp_use(boolexpr.lexeme);
    code_create(&boolexpr.last, "", JMPZ, boolexpr.lexeme, "", false_label);
    code_concat(&boolexpr, &true_stmt);

    if (dummy.code->label[0]) {                     /* follow_label from true_stmt? */
        strcpy(dummy.code->result, follow_label);   /* see documentation */
        code_concat(&boolexpr, &dummy);
    }

    if (false_stmt.code) {
        if (!dummy.code->label[0])
            code_create(&boolexpr.last, "", JUMP, "", "", follow_label);
        code_concat(&boolexpr, &false_stmt);
    }

    return(boolexpr);    /* now boolexpr is the all "if" structure */
}

Val reduce_while(Val boolexpr, Val stmt)
{
    Lexstr back;
                                             /* follow_label from stmt? */
    strcpy(back, follow_label);              /* so jump to the "boolexpr" */
    label_gen(follow_label);
    if (!boolexpr.code->label[0])
        label_gen(boolexpr.code->label);

    temp_use(boolexpr.lexeme);
    code_create(&boolexpr.last, "", JMPZ, boolexpr.lexeme, "", follow_label);
    code_concat(&boolexpr, &stmt);
    code_create(&boolexpr.last, back, JUMP, "", "", boolexpr.code->label);

    return(boolexpr);
}

Val reduce_assign(Val id, Val expr)
{
    int    type = symtbl_lookup(id.lexeme, -1, INT_NUM),  /* used=-1: id get value */
           opcode = IASN;
    Lexstr arg;

    strcpy  (arg, expr.lexeme);
    temp_use(arg);

    if (!type)
        error("undefined symbol", -strlen(id.lexeme)-6);
    else if (type == INT_NUM) {
        if (expr.type == FLOAT_NUM)
            error("invalid assignment", -strlen(id.lexeme)-8);
        else if (!expr.code && atol(expr.lexeme)>32767)         /* expr.lexeme is legal? */
            error("overflow", -6);                              /* (=no code), and overflow? */
    }
    else {
        opcode = RASN;
        if (expr.type == INT_NUM)
            convert(&expr, arg);
    }

    if (expr.code)                             /* not "ITOR _f0 i, RASN f _f0" */
        strcpy(expr.last->result, id.lexeme);  /* but "ITOR f i"! */
    else {
        code_create(&expr.last, "", opcode, arg, "", id.lexeme);
        expr.code = expr.last;
    }
    label_put(expr);

    return(expr);
}

Val reduce_expr(Val expr1, int op, Val expr2)
{
    int    opcode;
    Lexstr arg1,
           arg2;

    if (op == ID || op == INT_NUM || op == FLOAT_NUM) {    /* expr2 is "don't care" */
        expr1.code = expr1.last = 0;
        if (op == ID) {
            expr1.type = symtbl_lookup(expr1.lexeme, 1, INT_NUM);   /* used=1:id was used */
            if (!expr1.type)
                error("undefined symbol", 0);
        }
        else
            expr1.type = op;
    }
    else {
        strcpy(arg1, expr1.lexeme);
        strcpy(arg2, expr2.lexeme);
        if (op == '+')
            opcode = RADD;
        else if (op == '-')
            opcode = RSUB;
        else if (op == '*')
            opcode = RMLT;
        else
            opcode = RDIV;

        if (expr1.type == INT_NUM) {             /* need convertion? */
            if (expr2.type == FLOAT_NUM)
                convert(&expr1, arg1);
            else
                opcode += IADD - RADD;
        }
        else
            if (expr2.type == INT_NUM)
                convert(&expr2, arg2);

        if (opcode == IDIV || opcode == RDIV)
            if (expr2.lexeme[0] == '0' && atol(expr2.lexeme) == 0.0)
                error("Division by zero", -strlen(expr2.lexeme)-1);

        code_concat(&expr1, &expr2);
        expr1.type = opcode <= IDIV ? INT_NUM : FLOAT_NUM;
        temp_use(arg1);
        temp_use(arg2);
        temp_gen(expr1.lexeme, expr1.type);
        code_create(&expr1.last, "", opcode, arg1, arg2, expr1.lexeme);
        if (!expr1.code)
            expr1.code = expr1.last;
    }

    return(expr1);         /* expr1.code with all the code after it */
}

Val reduce_boolexpr(Val expr1, Val op, Val expr2)    /* like reduce_expr */
{
    int    opcode;
    Lexstr arg1,
           arg2;

    strcpy(arg1, expr1.lexeme);
    strcpy(arg2, expr2.lexeme);
    if (op.lexeme[0] == '=')
        opcode = REQL;
    else if (op.lexeme[0] == '>')
        opcode = RGRT;
    else if (strcmp(op.lexeme, "<>") == 0)
        opcode = RNQL;
    else
        opcode = RLSS;

    if (expr1.type == INT_NUM) {           /* need convertion? */
        if (expr2.type == FLOAT_NUM)
            convert(&expr1, arg1);
        else
            opcode += IEQL - REQL;
    }
    else
        if (expr2.type == INT_NUM)
            convert(&expr2, arg2);

    code_concat(&expr1, &expr2);
    if (isalpha(op.lexeme[0]))                        /* +=or      *=and */
        opcode = op.lexeme[0] == 'o' ? IADD : IMLT;   /* 1 or 0=1, 1 and 0=0 */
    temp_use(arg1);
    temp_use(arg2);
    temp_gen(expr1.lexeme, INT_NUM);
    code_create(&expr1.last, "", opcode, arg1, arg2, expr1.lexeme);
    if (!expr1.code)
        expr1.code = expr1.last;
    label_put(expr1);
                                                         /* maybe infinite loops? */
    if (!isalpha(op.lexeme[0]) && !strcmp(arg1, arg2))
        printf("warning, '%s%s%s' is always True/False.\n", arg1, op.lexeme, arg2);

    return(expr1);
}
