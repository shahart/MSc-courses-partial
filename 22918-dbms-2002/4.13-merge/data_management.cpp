/**************************************************************/
/*                    DATA_MANAGEMENT.CPP                     */ 
/**************************************************************/

#include "data_management.h"
PAGE_T p; //used for reading a page

/********************** external variables *********************/

extern FILE_T db_file;	
extern BUFFER_T buffer;

//initializes single page 
void Init_Page(PAGE_T page)
{
	int i;
	
	for (i=0; i<PAGE_SIZE; i++)
		//page[i]=(REC_T)NOVAL;
		strcpy(page[i], NOVAL);

}

//initializes system buffer 
void Init_Buffer()
{
	int i;

	for (i=0; i<BUFFER_SIZE; i++)
		Init_Page(buffer[i]);
}

//initializes database file 
void Init_File(FILE_T &file)
{
	file=fopen(FILE_SORTED,"rb+"); 
}

//outputs the page context to stdout
//output format is: a , b, ...|, where a, b, ... are the records within the page.
void Print_Page(PAGE_T page)
{	int i;
	
	for (i=0; i<PAGE_SIZE-1; i++)
	printf("%s , ", page[i]);

	printf("%s|  ",page[i]);	//i==PAGE_SIZE-1
}

//outputs the page context to stdout
void Print_Db_File()
{
	Get_Db_Size=0;

	do {
		file_page_read(db_file,Get_Db_Size); 
		Print_Page(p);
		Get_Db_Size++;
	}
	while (!Is_Last_Page(p));

	putchar('\n');
}

//returns TRUE if this page contains EOF as one of the records
//otherwise returns FALSE
int Is_Last_Page(PAGE_T page) 
{
	int i;
	
	for (i=0; i<PAGE_SIZE; i++)
		if( !strcmp(page[i], DB_EOF) )
			return TRUE;
	return FALSE;
}

//clears page contents (assigning NOVAL) after DB_EOF have been read
void Clear_Page(PAGE_T page)
{
	int i,j;
	for(i=0;((i<PAGE_SIZE) && (strcmp(page[i],DB_EOF)));i++); /*bug*/
	for(j=i+1;j<PAGE_SIZE;j++)
		strcpy(page[j],NOVAL);
}

//copies one page, record by record
void Copy_Page(PAGE_T source, PAGE_T dest)
{
	int i;

	if ((source == NULL) || (dest == NULL))
	{
		printf("\n ERROR COPYING PAGE - ABORTING... \n");
		exit(1);
	}
	
	for (i=0; i<PAGE_SIZE; i++)
		strcpy(dest[i],source[i]);
}

void file_page_read(FILE_T file,int index) 
{
  fseek(file,REC_SIZE*index*PAGE_SIZE,SEEK_SET);
  for (int i=0; i<PAGE_SIZE; i++) {
	  fgets(p[i],32767,file); 
	  ignore_spaces(p[i]); 
  }
}

void file_page_write(FILE_T file,int index) 
{
  fseek(file,REC_SIZE*index*PAGE_SIZE,SEEK_SET);
  char str[REC_SIZE+5]="rid 567890";
  for (int i=0; i<PAGE_SIZE; i++) {
	  strcpy(str,p[i]);
	  put_spaces(str);
	  fputs(str,file);
  }
}

//loads n_of_pages pages, starting at position f_position in 
//the database file, to the buffer, starting at position b_position 
//f_position, b_position and n_of_pages 
//will be truncated to maximum allowed value
void Load_Pages(int n_of_pages, int f_position, int b_position)
{
	int i;
	int db_size;

	db_size=Get_Db_Size;

	if(f_position >= db_size)
		f_position = db_size-1;	//last page

	if(b_position >= BUFFER_SIZE)
		b_position = BUFFER_SIZE-1;	//last page

	if(n_of_pages > (BUFFER_SIZE-b_position))	//not enough room
		n_of_pages = BUFFER_SIZE-b_position;

	if(n_of_pages > (db_size-f_position)) /*bug? twich n_of_pages=*/
		n_of_pages = db_size-f_position;

	for(i=0; i<n_of_pages; i++) {
		file_page_read(db_file,i+f_position); 
		Copy_Page(p,buffer[i+b_position]);
	}

	if (Is_Last_Page(buffer[i-1+b_position]))	//the last page written
		Clear_Page(buffer[i-1+b_position]);		//restore buffer after DB_EOF

}

//unloads n_of_pages pages, starting at position b_position in 
//the buffer, to the database file, starting at position f_position  
//f_position, b_position and n_of_pages 
//will be truncated to maximum allowed value
void Unload_Pages(int n_of_pages, int f_position, int b_position, FILE_T file)
{
	int i;
	int db_size;

	db_size=Get_Db_Size;

	if(f_position >= db_size)						/*bug*/
		f_position = db_size-1;	//last page

	if(b_position >= BUFFER_SIZE)
		b_position = BUFFER_SIZE-1;	//last page

	if(n_of_pages > (db_size-f_position))	//not enough room
		n_of_pages = db_size-f_position;

	for(i=0; i<n_of_pages; i++) {
		Copy_Page(buffer[i+b_position],p);
		file_page_write(file,i+f_position);
	}
}

//loads to a position b_position in the buffer a page filled with 
//NOVAL - s
void Load_Empty_Page(int b_position)
{
	Init_Page(buffer[b_position]);
}


