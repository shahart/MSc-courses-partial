/**************************************************************/
/*                     EXTSORT_ALGORITHM.H                    */ 
/**************************************************************/

#ifndef _extsort_algorithm_h
#define _extsort_algorithm_h

#include "data_management.h"

/*********************** definitions ***********************/
#define SAME_STEP 0	//continue to perform current step
#define NEXT_STEP 1	//continue to next step
#define NEXT_PASS 2	//continue to perform next pass

#define OUT_SORT "..\\out_sort.txt"	//an output file that contains the intermediate results while
									//executing merge sort
#define FORMAT_OUT	3	//the number of pages in a single line of the OUT_SORT file

//pass 0 of external sort algorithm.
//loads BUFFER_SIZE pages every cycle, sorts them and
//unloads back to the database file.
//this way we get ~FILE_SIZE / BUFFER_SIZE runs BUFFER_SIZE size each
void Zero_Pass();

//returns the size of a single run during pass 'pass'
int Get_Run_Size(const int pass);

//returns the number of runs in current pass (number 'pass')
//in a database file of size f_size
int Get_Number_Runs(const int pass, const int f_size);

//executes pass 'pass'on database file of size f_size
//pass must be valued 1 as minimum (for pass 0 use Zero_Pass())
void Execute_Merge_Pass(const int pass, const int f_size); 

//returns the index (starting with 0) of the run where page 'page'
//is allocated in current pass
int Get_Run_Index(const int page, const int pass);

//returns non-zero value iff the page 'page' is the last page of
//its run
int Is_Last_In_Run(const int page,const int pass);

//loads to the input buffer 'dest' the next page after 'page' from a 
//database file of size f_size, during pass 'pass'
void Load_Next_Page(int page, const int pass,const int f_size);

//executes passes 1,2,... till there is only one run (sorted subfile)
//left in the database file
void Execute_Merge();

#endif