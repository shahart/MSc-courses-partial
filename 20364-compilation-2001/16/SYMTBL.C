
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Sym_Tbl.C ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "symtbl.h"
#include "lex.h"

#include <stdlib.h>
#include <string.h>
#include  <stdio.h>
#include  <ctype.h>
#include    <mem.h>

/*********************************************************** PRIVATE SECTION */

#define NUMBER_OF_ITEMS 211    /* should be prime number */

typedef struct Symtbl_entry    /* holds also labels numbers */
{
    Lexstr       lexeme;   /* ID or label */
    int          used,     /* attribute of ID: 1:used, -1:initialized, 0:declared */
                 type;     /* if lexeme is ID: type is INT/FLOAT; if label: line number */
	struct
    Symtbl_entry *next;
} Symtbl_entry;

Symtbl_entry *symtbl[NUMBER_OF_ITEMS] = {0};

int symtbl_hash(char *lexeme)          /* "hashhpjw" in pg. 436 */
{
    unsigned g,
             hashval = 0;

	while (*lexeme) {
		hashval = *lexeme + (hashval<<4);
        g       = (unsigned)(hashval & 0xf0000000);
		if (g) {
            hashval ^= g>>24;
            hashval ^= g;
		}
        lexeme++;
	}

	return(hashval % NUMBER_OF_ITEMS);
}

/*********************************************************** PUBLIC SECTION */

void symtbl_free(int warn)
{
	int          index;
    Symtbl_entry *pentry;

	for (index = 0; index < NUMBER_OF_ITEMS; index++) {
        pentry = symtbl[index];
  	    while (pentry) {
            if (pentry->used != 1 && warn && isalpha(pentry->lexeme[0]))
                printf("warning, '%s' declared but never used.\n", pentry->lexeme);
            free(pentry);
            pentry = pentry->next;
        }
    }
}

int symtbl_lookup(char *lexeme, int use, int type)
{
    int          hashval = symtbl_hash(lexeme);
    Symtbl_entry *pentry = symtbl[hashval];

	while (pentry) {
        if (!strcmp(pentry->lexeme, lexeme)) {
            if (use == 1 && !pentry->used)
                printf("warning, '%s' used before initilization.\n", lexeme);
            if (use && pentry->used != 1)
                pentry->used = use;        /* if not used before, update 'used' */

            return(pentry->type);
        }
        pentry = pentry->next;
    }

    pentry = malloc(sizeof(Symtbl_entry));    /* add the new entry in LIFO order
                                              /* time of O(1) */
    strcpy(pentry->lexeme, lexeme);
    pentry->type    = type;
    pentry->used    = 0;
    pentry->next    = symtbl[hashval];
    symtbl[hashval] = pentry;

    return(0);
}
