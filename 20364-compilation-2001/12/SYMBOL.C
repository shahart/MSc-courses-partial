#include <stdlib.h>
#include <string.h>

#include "symbol.h"

#define NUMBER_OF_ITEMS 211 /* should be prime number */

typedef struct Item_Type
{
    Token_Type       data;
	struct Item_Type *next;
} Item_Type;

Item_Type *symbol_table[NUMBER_OF_ITEMS];

/*********************************************************** PRIVATE SECTION */

int hash_function(char *lexeme) /* according to "hashhpjw" in page 436 */
{
    char     *p;
    unsigned g, h = 0;

	for (p = lexeme; *p != '\0'; p++) {
		h = (h<<4) + (*p);
        g = (unsigned)(h & 0xf0000000);
		if (g) {
            h ^= g>>24;
            h ^= g;
		}
	}

	return (h % NUMBER_OF_ITEMS);
}

/*********************************************************** PUBLIC SECTION */

void symbol_inittable(void)
{
	int index;

	for (index = 0; index < NUMBER_OF_ITEMS; index++)
		symbol_table[index] = NULL;
}

void symbol_insert(Token_Type item)
{
    Item_Type *temp_item;
    int       lexeme_hash_function = hash_function(item.lexeme);

	if (symbol_table[lexeme_hash_function] == NULL) {
        /* first item */
		symbol_table[lexeme_hash_function] = (Item_Type *)malloc(sizeof(Item_Type));
		symbol_table[lexeme_hash_function]->data = item;
		symbol_table[lexeme_hash_function]->next = NULL;
	}
	else {
        /* push the new item before the first one */
		temp_item = (Item_Type *)malloc(sizeof(Item_Type));
		temp_item->data = item;
		temp_item->next = symbol_table[lexeme_hash_function];
		symbol_table[lexeme_hash_function] = temp_item;
	}
}

Token_Type symbol_lookup(char *lexeme)
{
    Item_Type  *temp_item = symbol_table[hash_function(lexeme)];
    Token_Type ret_token;

	for (; temp_item != NULL; temp_item = temp_item->next)
        if (strcmp(temp_item->data.lexeme, lexeme) == 0)
			return (temp_item->data);

    ret_token.token = END_OF_FILE; /* not found */
    return (ret_token);
}
