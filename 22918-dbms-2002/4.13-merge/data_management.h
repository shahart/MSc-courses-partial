/**************************************************************/
/*                      DATA_MANAGEMENT.H                     */ 
/**************************************************************/
#ifndef _data_management_h
#define _data_management_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global_defs.h"

//initializes single page 
void Init_Page(PAGE_T page);

//initializes system buffer 
void Init_Buffer();

//initializes database file 
void Init_File(FILE_T &file);

//outputs the page context to stdout
void Print_Page(PAGE_T page);

//outputs the page context to stdout
void Print_Db_File();

//gets the actual size of the database file
//(the number of pages until EOF)
int Get_Db_Size;

//initializes a DEMO database file of size 'f_size' for external sort testing
//if f_size is greater then FILE_SIZE, only FILE_SIZE pages
//are initialized.
//max_rand is the maximum allowed random value 
//void Init_Random_File(int f_size, const int max_rand);

void file_page_read(FILE_T file,int index); 

//returns TRUE if this page contains EOF as one of the records
//otherwise returns FALSE
int Is_Last_Page(PAGE_T page);

//copies one page, record by record
void Copy_Page(PAGE_T source, PAGE_T dest);

//loads n_of_pages pages, starting at position f_position in 
//the database file, to the buffer, starting at position b_position
//f_position, b_position and n_of_pages 
//will be truncated to maximum allowed value
void Load_Pages(int n_of_pages, int f_position, int b_position);

//unloads n_of_pages pages, starting at position b_position in 
//the buffer, to the database file, starting at position f_position  
//f_position, b_position and n_of_pages 
//will be truncated to maximum allowed value
void Unload_Pages(int n_of_pages, int f_position, int b_position, FILE_T file);

//loads to a position b_position in the buffer a page filled with 
//NOVAL - s
void Load_Empty_Page(int b_position);

#endif