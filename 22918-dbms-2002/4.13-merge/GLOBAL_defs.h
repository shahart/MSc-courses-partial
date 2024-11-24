/**************************************************************/
/*                       GLOBAL_DEFS.H                        */ 
/**************************************************************/

/*********************** definitions ***********************/
#ifndef _global_defs_h
#define _global_defs_h

#include "..\share.h" 

#define TEMP_FILE "temp_file.txt"

#define TRUE 1
#define FALSE 0

#define	BUFFER_SIZE	BUF_SIZE	//every buffer contains BUFFER_SIZE pages


/************************ typedefs *************************/

typedef  char REC_T[REC_SIZE+1];	//record will be represented as integer
					//if chosen otherwise - the comparison function
					//of the sorting algorithm should be changed accordingly

typedef REC_T PAGE_T[PAGE_SIZE]; //page type 
typedef PAGE_T BUFFER_T[BUFFER_SIZE]; //buffer type

#include <stdio.h>
typedef FILE* FILE_T; //file type 

#endif