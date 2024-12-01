
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Code.H ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef _CODE_H_
#define _CODE_H_

#include "lex.h"

#include <stdio.h>

#define MEM_RESERVE    /* use less memory. see documentation */

enum {
    IASN, IPRT, IINP, IEQL, INQL, ILSS, IGRT, IADD, ISUB, IMLT, IDIV,
    RASN, RPRT, RINP, REQL, RNQL, RLSS, RGRT, RADD, RSUB, RMLT, RDIV,
	ITOR, RTOI, JUMP, JMPZ, HALT
};

typedef struct Quadruple
{
	char      label[5];     /* up to 9999 labels. see documentation */
    int       opcode;
    Lexstr    result,
              arg1,
              arg2;
	struct
    Quadruple *next;
} Quadruple;

#ifndef MEM_RESERVE
void code_free(void);   /* free created quadruples, although */
#endif                  /* O.S. does it automatically */

void code_print (Quadruple *progcode, FILE *hlp_file);
void code_create(Quadruple **last, char *label, int opcode, char *arg1, char *arg2, char *result);
            /* creates quadruple at *last */
#endif
