/**************************************************************/
/*                    EXTSORT_ALGORITHM.CPP                   */ 
/**************************************************************/

#include "extsort_algorithm.h"
extern BUFFER_T buffer;	
extern FILE_T db_file;

extern int loc,size;	//the location and the size of the field to be sorted upon as passed as an argument
						//to the main function

extern PAGE_T p; //used for reading a page

int page_in_frame[BUFFER_SIZE - 1]; //the index in db_file of the page
									//currently held in the apropriate
									//frame in the input buffer

int curr_smallest[BUFFER_SIZE - 1]; //the index in the apropriate
									//frame in the input buffer
									//of the record currently being
									//the smallest unprocessed (yet)
FILE_T out_file;	//output file

int out_file_index;	//the index input file of the next page to be written

int curr_out;	//the index in the output buffer where the next 
				//value will be output
/*
//comparision function used in sorting algorithm
//returns negative value if rec1<rec2
//returns positive value if rec1>rec2
//returns zero value if rec1=rec2
int compare( const void* rec1, const void* rec2)
{
   if( !strcmp((*((REC_T*)rec1)), NOVAL) || !strcmp((*((REC_T*)rec1)), DB_EOF ))
	   return 1;	//NOVAL and DB_EOF are treated as biggest values

   if( !strcmp((*((REC_T*)rec2)), NOVAL) || !strcmp((*((REC_T*)rec1)), DB_EOF ))
	   return -1;	//NOVAL and DB_EOF are treated as biggest values

   return atoi(*((REC_T*)rec1))-atoi(*((REC_T*)rec2));

}

*/



//compare - lexicografic version
int compare( const void* rec1, const void* rec2)
{
   if( !strcmp((*((REC_T*)rec1)), NOVAL) || !strcmp((*((REC_T*)rec1)), DB_EOF ))
	   return 1;	//NOVAL and DB_EOF are treated as biggest values

   if( !strcmp((*((REC_T*)rec2)), NOVAL) || !strcmp((*((REC_T*)rec1)), DB_EOF ))
	   return -1;	//NOVAL and DB_EOF are treated as biggest values

   return strncmp((*((REC_T*)rec1))+loc, (*((REC_T*)rec2))+loc, size);

}


//replaces the first NOVAL with EOF, because they could have changed 
//places during sorting
void Correct_EOF()
{
	int page_index, rec_index;
	
	for(page_index=0;page_index<BUFFER_SIZE;page_index++)
		for(rec_index=0;rec_index<PAGE_SIZE;rec_index++)
		{
			if(!strcmp(buffer[page_index][rec_index], DB_EOF))
				return;
			if(!strcmp(buffer[page_index][rec_index],  NOVAL))
			{
				strcpy(buffer[page_index][rec_index], DB_EOF);
				return;
			}
		}
}

//pass 0 of external sort algorithm.
//loads BUFFER_SIZE pages every cycle, sorts them and
//unloads back to the database file.
//this way we get ~FILE_SIZE / BUFFER_SIZE runs BUFFER_SIZE size each
void Zero_Pass()
{
	int curr_pos=0;	//position in the database file from which 
					// the buffer will be loaded in the current cycle

	Init_Buffer();

	while(curr_pos < Get_Db_Size)
	{
		Load_Pages(BUFFER_SIZE,curr_pos, 0);
		qsort( (BUFFER_T)buffer,BUFFER_SIZE*PAGE_SIZE,sizeof(REC_T),compare);
		Correct_EOF();
		Unload_Pages(BUFFER_SIZE,curr_pos, 0,db_file);
		curr_pos+=BUFFER_SIZE;
	}

	fcloseall();
	Init_File(db_file);
}

//returns v (value) powered p (power)
int Power (int v,const int p)
{
	int i;
	int result=1;

	for(i=1; i<=p; i++)
		result*=v;
	return result;
}

//returns the size of a single run during pass 'pass'
inline int Get_Run_Size(int pass)
{
	return BUFFER_SIZE*Power(BUFFER_SIZE - 1, pass-1);	//B*(B-1)^(p-1)
}


//returns the number of runs in current pass (number 'pass')
//in a database file of size f_size
int Get_Number_Runs(const int pass,const int f_size)
{
	int i;	//keeps intermidium result

	//the result is rounded upwards
	i = Get_Run_Size(pass);
	return (f_size + i - 1) / i;
}

//returns the index (starting with 0) of the run where page 'page'
//is allocated in current pass
int Get_Run_Index(const int page, const int pass)
{
	int i;	//keeps intermidium result

	//the result is rounded upwards
	i = Get_Run_Size(pass);
	return (page + i)/i - 1;
}

//returns non-zero value iff the page 'page' is the last page of
//its run
int Is_Last_In_Run(int page, const int pass)
{
	return !( (++page % Get_Run_Size(pass)) );
}

//loads to the input buffer 'dest' the next page after 'page' from a 
//database file of size f_size, during pass 'pass'
void Load_Next_Page(int page, const int pass, const int dest, const int f_size)
{

	if (Is_Last_In_Run(page,pass))
		{
			Load_Empty_Page(dest);
			curr_smallest[dest] = 0;	//prevent access violation
			return;
		}

	//not last in run
	curr_smallest[dest] = 0;	//reset the smallest record index
	page_in_frame[dest] = ++page;//set to page now being loaded
	Load_Pages(1, page,dest);
}

//outputs the page context to stdout
//output format is: a , b, ...|, where a, b, ... are the records within the page  .
void Out_Print_Page(FILE_T file, PAGE_T page)
{	int i;
	
	for (i=0; i<PAGE_SIZE-1; i++)
	fprintf(file, "%s , ", page[i]);

	fprintf(file, "%s|  ",page[i]);	//i==PAGE_SIZE-1
}

//opens the OUT_SORT file for appending and writes to in the contents of the DB_FILE
//format is the number of pages output in every line
void Init_Outsort_File(int format)
{
	int i;
	FILE_T file;

	file = fopen(OUT_SORT, "w");	//reset OUT_SORT
	
	fputc('\n', file);
	fprintf(file, "*************Sorting by field in location %d of size %d, buffer size is %d**************\n", loc, size, BUFFER_SIZE);
	fprintf(file, "************************The output is formated from left to right********************\n");
	fprintf(file, "The format is | a, b, c, ...|, where a, b, c, ... are the records of the current page\n");
	fputc('\n', file);
	fprintf(file, "-----------------------INTERMEDIATE MERGE - SORT RESULTS-----------------------\n");
	fprintf(file, "-----------------------THE ORIGINAL FILE - BEFORE PASS 0-----------------------\n");

	for (i=0; i<Get_Db_Size; i++) 
	{
		if((i % format)== 0)
			fputc('\n', file);
		file_page_read(db_file,i); 
		Out_Print_Page(file, p);
		
	}
	fputc('\n', file);
	fclose(file);
}

//outputs intermidiate results to the OUT_SORT file
void Output_Inter(int pass, int format)
{
	int i;
	FILE_T file;

	file = fopen(OUT_SORT, "a+");	//open OUT_SORT for appending

	fputc('\n', file);
	fprintf(file, "-----------------------INTERMEDIATE MERGE - SORT RESULTS-----------------------\n");
	fprintf(file, "-----------------------RESULTS FOLLOWING PASS %d EXECUTION----------------------\n", pass);

	for (i=0; i<Get_Db_Size; i++) {
		if((i % format)== 0)
			fputc('\n', file);
		file_page_read(db_file,i); 
		Out_Print_Page(file, p);
		
	}
	fputc('\n', file);
	fclose(file);

}

//Initializes input buffers from database file of size f_size
//and global variables during step 'step'
void Init_Merge_Step(const int step, const int pass,const int f_size)
{
	int i;
	int page_to_load;	//page that is a candidate to be loaded

	Init_Buffer();

	for (i=0;i<BUFFER_SIZE-1;i++)
	{
		if( (page_to_load = (i*Get_Run_Size(pass) + step*Get_Run_Size(pass+1) ) )<= f_size - 1)
		{
			Load_Pages(1, page_to_load,i);
			page_in_frame[i]=page_to_load;
		}

		page_in_frame[i] = page_to_load;
		curr_smallest[i] = 0;
	}

	curr_out = 0;
	//curr_run = BUFFER_SIZE - 2;
}

/*
//returns the index of the frame that contains the page, in which the
//record currently pointed to has the minimal value
//this is a numeric version, the comparison is between the records as a whole
int Find_Min()
{
	int i=0;	//loop counter
	REC_T min;	//minimum as encountered until now
	int index = 0;	//the index where current min was found
	int EOF_found = -1;	//if != -1 - keeps the index where EOF was found

	//if all the records contain NOVAL - return index 0 (could be any other)
	//if the only value but NOVAL is EOF - return the index of EOF page
	while ( !strcmp(buffer[index][curr_smallest[index]], NOVAL) || !strcmp(buffer[index][curr_smallest[index]], DB_EOF) )
	{
		if(!strcmp(buffer[index][curr_smallest[index]], DB_EOF))
			EOF_found = index;	//the index of the page with EOF as minimal value
		if(++index == BUFFER_SIZE - 1)
			return ( (EOF_found != -1) ? EOF_found : 0); 
	}
    
	strcpy(min, buffer[index][curr_smallest[index]]); 

	for(i=index;i<BUFFER_SIZE - 1;i++)
		if( strcmp(buffer[i][curr_smallest[i]], NOVAL ) && strcmp(buffer[i][curr_smallest[i]], DB_EOF ))
		{
			if(atoi(buffer[i][curr_smallest[i]]) < atoi(min))
			{
				strcpy(min, buffer[i][curr_smallest[i]]);
				index = i;
			}
		}
	return index;
}
*/

//returns the index of the frame that contains the page, in which the
//record currently pointed to has the minimal value
//this is a numeric version, the comparison is between the records as a whole
//this is the lexicografic version
int Find_Min()
{
	int i=0;	//loop counter
	REC_T min;	//minimum as encountered until now
	int index = 0;	//the index where current min was found
	int EOF_found = -1;	//if != -1 - keeps the index where EOF was found

	//if all the records contain NOVAL - return index 0 (could be any other)
	//if the only value but NOVAL is EOF - return the index of EOF page
	while ( !strcmp(buffer[index][curr_smallest[index]], NOVAL) || !strcmp(buffer[index][curr_smallest[index]], DB_EOF) )
	{
		if(!strcmp(buffer[index][curr_smallest[index]], DB_EOF))
			EOF_found = index;	//the index of the page with EOF as minimal value
		if(++index == BUFFER_SIZE - 1)
			return ( (EOF_found != -1) ? EOF_found : 0); 
	}
    
	strcpy(min, buffer[index][curr_smallest[index]]); 

	for(i=index;i<BUFFER_SIZE - 1;i++)
		if( strcmp(buffer[i][curr_smallest[i]], NOVAL ) && strcmp(buffer[i][curr_smallest[i]], DB_EOF ))
		{
			if( (strncmp(buffer[i][curr_smallest[i]]+loc, min+loc,size)) < 0)
			{
				strcpy(min, buffer[i][curr_smallest[i]]);
				index = i;
			}
		}
	return index;
}

void Write_To_Output_File()
{
	int db_size=Get_Db_Size;	

	Unload_Pages(1,out_file_index,BUFFER_SIZE - 1,out_file);
	curr_out = 0;
	
	if(out_file_index < db_size) 
		out_file_index++;
}

//writes to the output buffer the record in min_frame
//returns TRUE iff EOF value was written down to the output file
int Output_Min(const int min_frame, const int pass, const int f_size)
{
	REC_T result;
	
	strcpy(result, strcpy(buffer[BUFFER_SIZE - 1][curr_out++], buffer[min_frame][curr_smallest[min_frame]] ));

	if(!strcmp(result, DB_EOF))
	{
		Write_To_Output_File();
		return NEXT_PASS;	//end of current pass
	}


	if(!strcmp(result, NOVAL))
		return NEXT_STEP;
	
	//not NOVAL or EOF
	if(++curr_smallest[min_frame] == PAGE_SIZE)	//the last record of 
												//the frame was processed
		Load_Next_Page(page_in_frame[min_frame], pass, min_frame, f_size);
	if(curr_out == PAGE_SIZE)
		Write_To_Output_File();
	return SAME_STEP;	//not end of  pass}
}

//copies the contents of file 'source' to file 'dest'
void Copy_Files()
{
	fcloseall();
	system("copy "TEMP_FILE" "FILE_SORTED); 
	Init_File(db_file);
/*	//must be loaded from source and unloaded to dest
	int i = 0;

	do
	{
		file_page(out_file,i);
		Copy_Page(p, file_page(db_file,i));
		file_page(out_file,i++);
	}while (!Is_Last_Page(p));
*/}

//Performs BUFFER_SIZE - 1 way merge
void Perform_Merge(int step, const int pass, int f_size)
{
	int min_frame;	//the frame that contains minimal-valued record
	int result;

	Init_Merge_Step(step,pass,f_size);
	do
	{
		min_frame = Find_Min();
		result = Output_Min(min_frame, pass, f_size);
	}while( (result!=NEXT_STEP) && (result!=NEXT_PASS));
}

//executes pass 'pass'on database file of size f_size
//pass must be valued 1 as minimum (for pass 0 use Zero_Pass())
void Execute_Merge_Pass(const int pass, const int f_size)
{
	int step;		//current step - merges to achive the next pass' run

	out_file_index = 0;	//every pass rewrites the out_file
	out_file = fopen(TEMP_FILE, "wb+");

	//every step achives a run for the next pass
	for(step=0;step<Get_Number_Runs(pass+1, f_size);step++)
		Perform_Merge(step, pass, f_size);

	Copy_Files();
}

//executes passes 1,2,... till there is only one run (sorted subfile)
//left in the database file
void Execute_Merge()
{
	int pass = 1;	//the pass to be performed
	int f_size = 0;	//the actual size of the database size

	f_size = Get_Db_Size;
	Init_Outsort_File(FORMAT_OUT);	//initialize file for writing intermediate output

	Zero_Pass();
	Output_Inter(0, FORMAT_OUT);
	while (Get_Number_Runs(pass, f_size) > 1)
	{
		Execute_Merge_Pass(pass, f_size);
		Output_Inter(pass, FORMAT_OUT);
		pass++;
	}
}

