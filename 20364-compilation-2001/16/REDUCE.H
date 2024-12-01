
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Reduce.H ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef _REDUCE_H_
#define _REDUCE_H_

#include "code.h"

typedef struct Val
{
    int       type;    /* in expr: INT/FLOAT */
    Quadruple *code,   /* chainlist of quadruples */
              *last;   /* last quadruple in chainlist 'code', to direct access */
    Lexstr    lexeme;  /* in expr: the 'temp' created */
} Val;

int  reduce_progid  (char *progid);   /* checks format of "Uletter letters" */
void reduce_program (Val stmtlist);
Val  reduce_stmtlist(Val stmtlist, Val stmt);
Val  reduce_if      (Val boolexpr, Val true_stmt, Val dummy, Val false_stmt);
Val  reduce_while   (Val boolexpr, Val stmt);
Val  reduce_assign  (Val id, Val expr);
void reduce_onevar  (Val id, int type);
Val  reduce_read    (Val id);
Val  reduce_expr    (Val expr1, int op, Val expr2);
Val  reduce_boolexpr(Val expr1, Val op, Val expr2);
Val  reduce_println (Val expr);
Val  reduce_dummy   (void);      /* catch follow_label (if exist) from true_stmt */

#endif
