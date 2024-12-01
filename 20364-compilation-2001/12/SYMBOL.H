#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "lex.h"

void       symbol_inittable(void);
void       symbol_insert(Token_Type item);
Token_Type symbol_lookup(char *lexeme);  /* returns END_OF_FILE if lexeme not found */

#endif
