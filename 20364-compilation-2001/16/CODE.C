
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Code.C ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "code.h"
#include "symtbl.h"

#include <string.h>
#include <stdlib.h>
#include  <stdio.h>

/*********************************************************** PRIVATE SECTION */

char opcode_str[][5] = {
    "IASN", "IPRT", "IINP", "IEQL", "INQL", "ILSS", "IGRT", "IADD", "ISUB", "IMLT", "IDIV",
    "RASN", "RPRT", "RINP", "REQL", "RNQL", "RLSS", "RGRT", "RADD", "RSUB", "RMLT", "RDIV",
    "ITOR", "RTOI", "JUMP", "JMPZ", "HALT"
};

#ifndef MEM_RESERVE
typedef struct Quad_item
{
    Quadruple *item;
    struct
    Quad_item *next;
} Quad_item;

Quad_item *quads = 0;            /* chainlist of all created quadruples */
#endif

/************************************************************ PUBLIC SECTION */

#ifndef MEM_RESERVE
void code_free()
{
    while (quads) {
        free(quads->item);
        quads = quads->next;
    }
}
#endif

void code_print(Quadruple *progcode, FILE *hlp_file)
{
    Quadruple *pquad = progcode;
    int       line   = 1,            /* current quadruple index */
              block  = 0;
                              /* convert labels to line numbers by insert */
    while (pquad) {           /* then into symtbl. */
        if (pquad->label[0])
            symtbl_lookup(pquad->label, 0, line);
        pquad = pquad->next;
        line++;
    }

    pquad = progcode;
    while (pquad) {
        if (block)                   /* indent basic blocks for easy understanding */
            fprintf(hlp_file, "  ");

        fprintf(hlp_file, "%s", opcode_str[pquad->opcode]);
        if (pquad->opcode == JUMP || pquad->opcode == JMPZ)
            fprintf(hlp_file, " %d", symtbl_lookup(pquad->result, 0, 0)); /* retreive dest */
        else                                                              /* of jump */
            fprintf(hlp_file, " %s", pquad->result);
        fprintf(hlp_file, " %s %s\n", pquad->arg1, pquad->arg2);

        /* determine if leader of basic block was found. 2 conditions in pg. 529: */
        /**/
        if (pquad->opcode == JUMP || pquad->opcode == JMPZ ||  /* immediatly follows jump */
            pquad->next && pquad->next->label[0])              /* target of jump */
          block = 1-block;

        pquad = pquad->next;
    }
}

void code_create(Quadruple **last, char *label, int opcode, char *arg1, char *arg2, char *result)
{
#ifndef MEM_RESERVE
    Quad_item *new_quad = malloc(sizeof(Quad_item));
#endif

    if (!(*last))                            /* first quadruple in stmt? */
        *last = malloc(sizeof(Quadruple));
    else
        *last = (*last)->next = malloc(sizeof(Quadruple));

    strcpy((*last)->label,  label);
	strcpy((*last)->arg1,   arg1);
	strcpy((*last)->arg2,   arg2);
	strcpy((*last)->result, result);
	(*last)->opcode = opcode;
    (*last)->next   = 0;

#ifndef MEM_RESERVE
    new_quad->next = quads;
    new_quad->item = *last;         /* add created quadruple in LIFO order,
    quads          = new_quad;      /* time of O(1) */
#endif
}
