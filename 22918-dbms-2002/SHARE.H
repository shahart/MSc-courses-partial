// share.h

#include <string.h>

#define NOVAL  "NULL"		//we use it as initialization value (empty rec value)
						//as well as for EOF mark.

#define DB_EOF "DB_EOF"	//end of database file

#define FILE_SORTED     "\\sorted.txt"

#define REC_SIZE	65	//the size of record string
#define PAGE_SIZE       2     //every page contains PAGE_SIZE records
#define BUF_SIZE             32

#define put_spaces(p) \
\
  for (int j=strlen(p); j<REC_SIZE-2; j++) \
    p[j]=' '; \
  p[REC_SIZE-2]='\r'; \
  p[REC_SIZE-1]='\n'; \
  p[REC_SIZE]=0;

#define ignore_spaces(str) \
\
  for (int j=strlen(str)-3; j>=0 && str[j]==' '; j--); \
  str[j+1]=0;
