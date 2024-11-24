/**************************************************************/
/*                       SORT_MAIN.CPP                        */ 
/**************************************************************/

#include <stdio.h>
#include "global_defs.h"
#include "extsort_algorithm.h"

/********************** global variables **********************/

FILE_T db_file;		//the database file to be sorted
BUFFER_T buffer;	//system buffer

int loc;
int size;

void main(int argc,char *argv[])
{	
	if (argc!=3)
		printf("merge loc size\n");
    else {
		loc=atoi(argv[1]);
		size=atoi(argv[2]);
		Init_Buffer();
		Init_File(db_file);
		Print_Db_File();
		Execute_Merge();
		Print_Db_File();
	}
}