
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Sym_Tbl.H ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef _SYMTBL_H_
#define _SYMTBL_H_

void symtbl_free  (int warn);                        /* free symtbl & checks initilizations */
int  symtbl_lookup(char *lexeme, int use, int type); /* if not found, insert and return  */
                                                     /* 0. else attach 'use' attribute */
#endif
