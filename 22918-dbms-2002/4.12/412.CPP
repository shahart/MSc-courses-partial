// DBMS 2003a - exercise 4.12
// physical layer

// version 2: modified to support sorts
// this time: no asserts
// TRACE defined in mergsort

// version 3: added multiple joins with statistics
int io_counter;

// all globals can be private, including XXX_init/done methods
// so porting to OO is quite easy

// handling errors with assert for debug
// user must have no mistakes

// echo debug information
//#define TRACE

// creates dm_database files for the first time
// #define FIRST_RUN

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <process.h>

#include "..\share.h"

#define MAXINT 32767

// important defs

#define PAGE_SIZE_IN_CHARS  256 // 4096 in NTFS, mini 14 (MAX_FILE_NAME+(DB_PAGE_SIZE-NUM_RECS_LOC))

#define MAX_HEAP_FILES       64
#define MAX_PAGES         8192u

#define TRUE 1
#define DB_PAGE_SIZE (PAGE_SIZE_IN_CHARS/sizeof(int))

typedef int page[DB_PAGE_SIZE];

#ifdef TRACE
  FILE* trace_file;
#endif

// returns i<=count where data[i]=value
const int find(const int* data,const int count,const int value)
{
  int i=0;
  while (i<count && data[i]!=value)
    i++;
  return i;
}

// disk manager. page 42 ///////////////////////////////////////////////////

#define DB         "pages.db"
#define BITMAP     "bitmap.db"

int   dm_bitmap_full[MAX_PAGES]; // 1=used block
FILE* dm_database;

const int dm_allocate_page()
{
  int i=find(dm_bitmap_full,MAX_PAGES,!TRUE);
  // search for empty block
  assert(i<MAX_PAGES); // dm_database full
  dm_bitmap_full[i]=TRUE;
#ifdef TRACE
  fprintf(trace_file,"dm_allocate_page returns %d\n",i);
  fflush(trace_file);
#endif
  return i;
}

void dm_deallocate_page(const int n)
{
  assert(dm_bitmap_full[n]==TRUE); // no such page
  assert(n!=-1); // "
  dm_bitmap_full[n]=!TRUE;
#ifdef TRACE
  fprintf(trace_file,"dm_deallocate_page %d\n",n);
  fflush(trace_file);
#endif
}

void dm_read_page(const int n,page p)
{
  assert(dm_bitmap_full[n]==TRUE); // no such page
  assert(n!=-1); // "
  io_counter++;
  fseek(dm_database,sizeof(page)*n,SEEK_SET);
  fread( p,sizeof(page),1,dm_database);
#ifdef TRACE
  fprintf(trace_file,"dm_read_page %d\n",n);
  fflush(trace_file);
#endif
}

void dm_write_page(const int n,const page p)
{
  assert(dm_bitmap_full[n]==TRUE); // no such page
  assert(n!=-1); // "
  fseek(dm_database,sizeof(page)*n,SEEK_SET);
  fwrite(p,sizeof(page),1,dm_database);
#ifdef TRACE
  fprintf(trace_file,"dm_write_page %d\n",n);
  fflush(trace_file);
#endif
}

void dm_init(int del=0)
{
  memset(dm_bitmap_full,!TRUE,sizeof(dm_bitmap_full));
//#ifdef FIRST_RUN
  if (del) 
    dm_database=fopen(DB,"wb+");
  else {
//#else
    // add backup in case assert fails
    system("copy "DB" "DB"2 > nul");
    dm_database=fopen(DB,"rb+");
    FILE* f=fopen(BITMAP,"rb");
    fread(dm_bitmap_full,sizeof(dm_bitmap_full[0]),MAX_PAGES,f);
    fclose(f);
  }
//#endif
}

void dm_done()
{
  fclose(dm_database);
  FILE* f=fopen(BITMAP,"wb+");
  fwrite(dm_bitmap_full,sizeof(dm_bitmap_full[0]),MAX_PAGES,f);
  fclose(f);
}

// buffer manager. page 45 /////////////////////////////////////////////////

  // queue for LRU replacement policy ///////////////////////////////

  int buf_queue[BUF_SIZE];

  void queue_init()
  {
    int i;
    for (i=0; i<BUF_SIZE; i++)
      buf_queue[i]=-1;
  }

  void queue_insert(const int n)
  {
    int i=find(buf_queue,BUF_SIZE,-1);
    assert(i<BUF_SIZE); // full queue
    buf_queue[i]=n;
  }

  // remove specific value
  void queue_mistake(const int n)
  {
    int i=find(buf_queue,BUF_SIZE,n);
    assert(i<BUF_SIZE);
    for (i++; i<BUF_SIZE; i++)
      buf_queue[i-1]=buf_queue[i];
    buf_queue[BUF_SIZE-1]=-1;
  }

  const int queue_remove()
  {
    int lru=buf_queue[0];
    assert(lru!=-1);
    queue_mistake(lru);
    return lru;
  }

  /////////////////////////////////////////////////////////

int  buf_page_nums[BUF_SIZE];
int  buf_pincount [BUF_SIZE];
int  buf_dirty    [BUF_SIZE];
page buf_pages    [BUF_SIZE];

void buf_pin_page(const int n,page p)
{
  // checks the pool
  int i=find(buf_page_nums,BUF_SIZE,n);
  // contains page n?
#ifdef TRACE
  fprintf(trace_file,"buf_pin_page %d, frame %d\n",n,i);
  fflush(trace_file);
#endif
  if (i<BUF_SIZE) {
    memcpy(p,buf_pages[i],sizeof(page));
    if (buf_pincount[i]==0)
      // no longer candidate
      queue_mistake(i);
    // incr pincount
    buf_pincount[i]++;
  }
  else {
    // search for empty frame
    i=find(buf_page_nums,BUF_SIZE,-1);
    // empty frame found?
    if (i>=BUF_SIZE) {
      // use LRU replacement policy
      i=queue_remove();
      assert(buf_pincount[i]==0); // buffer manager is stuck!
      if (buf_dirty[i]==TRUE)
        dm_write_page(i,buf_pages[i]);
    }
    dm_read_page(n,buf_pages[i]); // no such page
    memcpy(p,buf_pages[i],sizeof(page));
    buf_dirty    [i]=!TRUE;
    buf_pincount [i]=1;
    buf_page_nums[i]=n;
  }
}

void buf_unpin_page(const int n,const int dirty=0,page p=0)
{
  // checks the pool
  int i=find(buf_page_nums,BUF_SIZE,n);
#ifdef TRACE
  fprintf(trace_file,"buf_unpin_page %d, dirty %d, frame %d\n",n,dirty,i);
  fflush(trace_file);
#endif
  assert(i<BUF_SIZE); // no such page
  if (dirty==TRUE) {
    buf_dirty[i]=dirty;
    memcpy(buf_pages[i],p,sizeof(page));
  }
  assert(buf_pincount[i]==TRUE); // error
  if (buf_pincount[i]>0)
    // decr pincount
    buf_pincount[i]--;
  if (buf_pincount[i]==0)
    // update LRU
    queue_insert(i);
}

void buf_free_page(const int n)
{
  // in pool & dirty? - clean it
  int i=find(buf_page_nums,BUF_SIZE,n);
  if (i<BUF_SIZE)
    buf_dirty[i]=!TRUE;
#ifdef TRACE
  fprintf(trace_file,"buf_free_page %d\n",n);
  fflush(trace_file);
#endif
  dm_deallocate_page(n);
}

const int buf_new_page(page p)
{
#ifdef TRACE
  fprintf(trace_file,"buf_new_page\n");
  fflush(trace_file);
#endif
  int n=dm_allocate_page();
  buf_pin_page(n,p);
  memset(p,0,sizeof(page));
  return n;
}

void buf_init()
{
  int i;
  for (i=0; i<BUF_SIZE; i++)
    buf_page_nums[i]=-1;
  memset(buf_dirty,!TRUE,sizeof(buf_dirty));
  queue_init();
}

void buf_done()
{
  int i;
  for (i=0; i<BUF_SIZE; i++)
    if (buf_dirty[i])
      dm_write_page(buf_page_nums[i],buf_pages[i]);
}

// HFpage. page 52 ///////////////////////////////////////////////////

#define USED_SLOTS_LOC (DB_PAGE_SIZE-1)

typedef struct {
  int page;
  int slot;
} rid;

int hfp_record_size  [MAX_HEAP_FILES];
int hfp_records_total[MAX_HEAP_FILES];

const int hfp_slots_in_page(const int catalog)
{
  return (sizeof(page)-3*sizeof(int))/hfp_record_size[catalog];
}

const int hfp_delete_record(const int catalog,const rid r) // return 1 if needs hf layer service
{
  assert(r.page!=-1); // illegal pageId
  assert(hfp_records_total[catalog]>0); // empty file
  page p;
#ifdef TRACE
  fprintf(trace_file,"hfp_delete_record file %d,page %d,slot %d\n",catalog,r.page,r.slot);
  fflush(trace_file);
#endif
  buf_pin_page(r.page,p);
  assert(r.slot<p[USED_SLOTS_LOC]); // delete paste EOF
  assert(p[USED_SLOTS_LOC]>0); // empty page
  int i;
  for (i=r.slot+1;i<p[USED_SLOTS_LOC];i++)
    // slot[i-1]=slot[i];
    memcpy(p+(i-1)*hfp_record_size[catalog]/sizeof(int),p+i*hfp_record_size[catalog]/sizeof(int),hfp_record_size[catalog]);
  p[DB_PAGE_SIZE-1]--;
  buf_unpin_page(r.page,1,p);
  hfp_records_total[catalog]--;
  return p[USED_SLOTS_LOC];
}

void hfp_search_record(const int catalog,const rid r,char* rec)
{
  assert(r.page!=-1); // illegal pageId
  assert(r.slot<hfp_slots_in_page(catalog)); // illegal slotId
  page p;
#ifdef TRACE
  fprintf(trace_file,"hfp_search_record file %d,page %d,slot %d\n",catalog,r.page,r.slot);
  fflush(trace_file);
#endif  
  buf_pin_page(r.page,p);
  if (p[USED_SLOTS_LOC]==0)
    strcpy(rec,NOVAL);
  else
    memcpy(rec,p+r.slot*hfp_record_size[catalog]/sizeof(int),hfp_record_size[catalog]);
  buf_unpin_page(r.page);
}

void hfp_modify_record(const int catalog,const rid r,const char* rec)
{
  assert(r.page!=-1); // illegal pageId
  assert(r.slot<hfp_slots_in_page(catalog)); // illegal slotId
  assert(strlen(rec)<hfp_record_size[catalog]); // rec appropriate
  page p;
#ifdef TRACE
  fprintf(trace_file,"hfp_modify_record file %d,page %d,slot %d,rec %s\n",catalog,r.page,r.slot,rec);
  fflush(trace_file);
#endif  
  buf_pin_page(r.page,p);
  assert(r.slot<p[USED_SLOTS_LOC]);
  memcpy(p+r.slot*hfp_record_size[catalog]/sizeof(int),rec,hfp_record_size[catalog]);
  buf_unpin_page(r.page,1,p);
}

const int hfp_insert_record(const int catalog,const int n,const char* rec) // n=which page. returns 1 if page will be full
{
  assert(n!=-1); // illegal pageId
  page p;
#ifdef TRACE
  fprintf(trace_file,"hfp_insert_record file %d,page %d,rec %s\n",catalog,n,rec);
  fflush(trace_file);
#endif  
  buf_pin_page(n,p);
  if (p[USED_SLOTS_LOC]<hfp_slots_in_page(catalog)) { // full page?
    p[USED_SLOTS_LOC]++;
    memcpy(p+(p[USED_SLOTS_LOC]-1)*hfp_record_size[catalog]/sizeof(int),rec,hfp_record_size[catalog]);
    buf_unpin_page(n,1,p);
    hfp_records_total[catalog]++;
  }
  else
    assert(1==0); // bug! always there's an empty slot
  return (p[USED_SLOTS_LOC]==hfp_slots_in_page(catalog));
}

// heap file. page 56 /////////////////////////////////////////////////////////

#define HEADERS        "headers.db"
#define MAX_FILE_NAME  10
#define NUM_RECS_LOC   (DB_PAGE_SIZE-4)
#define PREV_PAGE_LOC  (DB_PAGE_SIZE-3)
#define NEXT_PAGE_LOC  (DB_PAGE_SIZE-2)

int  hf_first_page  [MAX_HEAP_FILES];
int  hf_full_page   [MAX_HEAP_FILES];
int  hf_free_page   [MAX_HEAP_FILES];
int  hf_free_scanned[MAX_HEAP_FILES];
char hf_briefs      [MAX_HEAP_FILES*(MAX_FILE_NAME+5)];
int  hf_count;

const char* hf_brief()
{
  return hf_briefs;
}

const int hf_create(const char* name,const int rec_size) // TODO rec_size... va_arg to easy fields choosing
{
  printf("creating %s\n",name);
  assert(rec_size<=PAGE_SIZE_IN_CHARS-3*sizeof(int)); // rec_size too long
  assert(hf_count<MAX_HEAP_FILES-1); // full files in DB
  page p;
#ifdef TRACE
  fprintf(trace_file,"\nhf_create %s\n",name);
  fflush(trace_file);
#endif
  int n=buf_new_page(p);
  p[PREV_PAGE_LOC]=-1;
  p[NEXT_PAGE_LOC]=-1;
  buf_unpin_page(n,1,p);
  hfp_record_size  [hf_count]=rec_size;
  hfp_records_total[hf_count]=0;
  hf_free_page     [hf_count]=n;
  hf_full_page     [hf_count]=-1;
  hf_first_page    [hf_count]=buf_new_page(p);
  strncpy((char*)p,name,MAX_FILE_NAME);
  buf_unpin_page(hf_first_page[hf_count],1,p);
  hf_count++;
  return hf_count-1;
}

void hf_destroy(const int catalog)
{
  assert(catalog<=hf_count); // file exist
  assert(hf_first_page    [catalog]!=-1); // file not exist
  assert(hfp_records_total[catalog]==0); // file is empty
  assert(hf_full_page     [catalog]==-1); // "
#ifdef TRACE
  fprintf(trace_file,"\nhf_destroy %d\n",catalog);
  fflush(trace_file);
#endif
  buf_free_page(hf_free_page [catalog]);
  buf_free_page(hf_first_page[catalog]);
  int i;
  for (i=catalog+1; i<hf_count; i++) {
    hf_first_page  [i-1]=hf_first_page  [i];
    hf_full_page   [i-1]=hf_full_page   [i];
    hf_free_page   [i-1]=hf_free_page   [i];
    hfp_record_size[i-1]=hfp_record_size[i];
	hfp_records_total[i-1]=hfp_records_total[i];
  }
  hf_count--;
}

void hf_search(const int catalog,const rid r,char* rec)
{
  hfp_search_record(catalog,r,rec);
}

const rid hf_restart_scan(const int catalog)
{
  rid curr_rid;
  assert(hf_first_page[catalog]!=-1); // file exists
#ifdef TRACE
  fprintf(trace_file,"\nhf_restart_scan %d\n",catalog);
  fflush(trace_file);
#endif
  if (hf_full_page[catalog]!=-1) {
    hf_free_scanned[catalog]=0;
    curr_rid.page=hf_full_page[catalog];
  }
  else if (hf_free_page[catalog]!=-1) { // by definition, always free!=-1
    hf_free_scanned[catalog]=1;
    curr_rid.page=hf_free_page[catalog];
  }
  else
    curr_rid.page=-1; // can't search file. file is empty
  curr_rid.slot=0;
  return curr_rid;
}

const rid hf_scan_next(const int catalog,rid curr_rid)
{
  assert(curr_rid.page!=-1); // illegal pageId
  page p;
#ifdef TRACE
  fprintf(trace_file,"\nhf_scan_next %d\n",catalog);
  fflush(trace_file);
#endif
  buf_pin_page(curr_rid.page,p);
  buf_unpin_page(curr_rid.page);
  curr_rid.slot++;
  if (curr_rid.slot>=p[USED_SLOTS_LOC]) {
    curr_rid.slot=0;
    if (p[NEXT_PAGE_LOC]!=-1)
      curr_rid.page=p[NEXT_PAGE_LOC];
    else if (hf_free_scanned[catalog]!=TRUE && hf_free_page[catalog]!=-1) {
      curr_rid.page=hf_free_page[catalog];
      hf_free_scanned[catalog]=TRUE;
    }
    else
      curr_rid.page=-1;
  }
  return curr_rid;
}

void hf_delete_all(const int catalog)
{
  rid curr_rid=hf_restart_scan(catalog);
  int temp;
  page p;
  while (curr_rid.page!=-1) {
    buf_pin_page(curr_rid.page,p);
    buf_unpin_page(curr_rid.page);
    temp=curr_rid.page;
    if (p[NEXT_PAGE_LOC]!=-1)
      curr_rid.page=p[NEXT_PAGE_LOC];
    else if (hf_free_scanned[catalog]!=TRUE && hf_free_page[catalog]!=-1) {
      curr_rid.page=hf_free_page[catalog];
      hf_free_scanned[catalog]=TRUE;
    }
    else
      curr_rid.page=-1;
    if (curr_rid.page!=-1)
      buf_free_page(temp);
  }
  hfp_records_total[catalog]=0;
  hf_full_page     [catalog]=-1;
  hf_free_page     [catalog]=temp;
  hf_destroy(catalog);
}

void hf_insert(const int catalog,const char* rec)
{
  assert(hf_free_page[catalog]!=-1); // some free page
#ifdef TRACE
  fprintf(trace_file,"\nhf_insert %d,rec '%s'\n",catalog,rec);
  fflush(trace_file);
#endif
  if (hfp_insert_record(catalog,hf_free_page[catalog],rec)) {
    // move from free to hf_full_page[catalog]
    page p;
    buf_pin_page(hf_free_page[catalog],p);
    int next_page=p[NEXT_PAGE_LOC];
    p[NEXT_PAGE_LOC]=hf_full_page[catalog];
    p[PREV_PAGE_LOC]=-1;
    buf_unpin_page(hf_free_page[catalog],1,p);
    if (next_page!=-1)
      buf_pin_page(next_page,p);
    else {
      next_page=buf_new_page(p);
      p[NEXT_PAGE_LOC]=-1;
    }
    p[PREV_PAGE_LOC]=-1;
    buf_unpin_page(next_page,1,p);
    if (hf_full_page[catalog]!=-1) {
      buf_pin_page(hf_full_page[catalog],p);
      p[PREV_PAGE_LOC]=hf_free_page[catalog];
      buf_unpin_page(hf_full_page[catalog],1,p);
    }
    hf_full_page[catalog]=hf_free_page[catalog];
    hf_free_page[catalog]=next_page;
  }
}

void hf_delete(const int catalog,const rid r)
{
  page p;
#ifdef TRACE
  fprintf(trace_file,"\nhf_delete file %d,page %d,slot %d\n",catalog,r.page,r.slot);
  fflush(trace_file);
#endif
  int del_result=hfp_delete_record(catalog,r);
  if (del_result==0) {
    // unallocate page, and remove from free/full list
    buf_pin_page(r.page,p);
    int prev_page=p[PREV_PAGE_LOC];
    int next_page=p[NEXT_PAGE_LOC];
    buf_unpin_page(r.page);
    if (next_page!=-1) {
      buf_pin_page(next_page,p);
      p[PREV_PAGE_LOC]=prev_page;
      buf_unpin_page(next_page,1,p);
    }
    if (prev_page!=-1) {
      buf_pin_page(prev_page,p);
      p[NEXT_PAGE_LOC]=next_page;
      buf_unpin_page(prev_page,1,p);
    }
    if (hf_free_page[catalog]==r.page) {
      hf_free_page[catalog]=next_page;
      if (hf_free_page[catalog]==-1) {
        hf_free_page[catalog]=buf_new_page(p);
        p[PREV_PAGE_LOC]=-1;
        p[NEXT_PAGE_LOC]=-1;
        buf_unpin_page(hf_free_page[catalog],1,p);
	   }
	 }
    if (hf_full_page[catalog]==r.page) {
      hf_full_page[catalog]=next_page;
      if (hf_free_page[catalog]==-1) {
        hf_free_page[catalog]=buf_new_page(p);
        p[PREV_PAGE_LOC]=-1;
        p[NEXT_PAGE_LOC]=-1;
        buf_unpin_page(hf_free_page[catalog],1,p);
	   }
	 }
    buf_free_page(r.page);
  }
  else if (del_result==hfp_slots_in_page(catalog)-1) {
    // move from full list to free list
    assert(hf_free_page[catalog]!=-1);
    buf_pin_page(hf_free_page[catalog],p);
    p[PREV_PAGE_LOC]=r.page;
    buf_unpin_page(hf_free_page[catalog],1,p);
    buf_pin_page(r.page,p);
    int prev_page=p[PREV_PAGE_LOC];
    int next_page=p[NEXT_PAGE_LOC];
    p[NEXT_PAGE_LOC]=hf_free_page[catalog];
    p[PREV_PAGE_LOC]=-1;
    buf_unpin_page(r.page,1,p);
    if (next_page!=-1) {
      buf_pin_page(next_page,p);
      p[PREV_PAGE_LOC]=prev_page;
      buf_unpin_page(next_page,1,p);
    }
    if (prev_page!=-1) {
      buf_pin_page(prev_page,p);
      p[NEXT_PAGE_LOC]=next_page;
      buf_unpin_page(prev_page,1,p);
    }
    if (hf_full_page[catalog]==r.page)
      hf_full_page[catalog]=next_page;
    hf_free_page[catalog]=r.page;
  }
  else
    ; // average case
}

void hf_modify(const int catalog,const rid r,const char* rec)
{
#ifdef TRACE
  fprintf(trace_file,"\nhf_modify file %d,page %d,slot %d,rec '%s'\n",catalog,r.page,r.slot,rec);
  fflush(trace_file);
#endif
  hfp_modify_record(catalog,r,rec);
}

void hf_init(int del=0)
{
  hf_count=0;
  strcpy(hf_briefs,"");
//#ifndef FIRST_RUN
  if (del)
	return;
  FILE* f=fopen(HEADERS,"rb");
  hf_count=fread(hf_first_page,sizeof(int),MAX_HEAP_FILES,f);
  fclose(f);
  int i;
  for (i=0; i<hf_count; i++) {
    page p;
    buf_pin_page(hf_first_page[i],p);
    hf_free_page     [i]=p[PREV_PAGE_LOC];
    hf_full_page     [i]=p[NEXT_PAGE_LOC];
    hfp_record_size  [i]=p[USED_SLOTS_LOC];
    hfp_records_total[i]=p[NUM_RECS_LOC];
    buf_unpin_page(hf_first_page[i]);
    sprintf(hf_briefs+strlen(hf_briefs),"%d-%s ",i,p);
  }
  char add[2]="0";
  add[0]+=hf_count;
  strcat(hf_briefs,add);
//#endif
}

void hf_done()
{
  FILE* f=fopen(HEADERS,"wb");
  fwrite(hf_first_page,sizeof(int),hf_count,f);
  fclose(f);
  int i;
  for (i=0; i<hf_count; i++) {
    page p;
    buf_pin_page(hf_first_page[i],p);
    p[PREV_PAGE_LOC] =hf_free_page[i];
    p[NEXT_PAGE_LOC] =hf_full_page[i];
    p[USED_SLOTS_LOC]=hfp_record_size[i];
    p[NUM_RECS_LOC]  =hfp_records_total[i];
    buf_unpin_page(hf_first_page[i],1,p);
  }
}

// main /////////////////////////////////////////////////////////

void hf_scan(int file,int print_to_screen=!TRUE)
{
  FILE* not_sorted=fopen(FILE_SORTED,"wb");
  char str[REC_SIZE+5];
  int i;
  printf("file %d, full %d, free %d, records %d, record_size %d\n",file,hf_full_page[file],hf_free_page[file],hfp_records_total[file],hfp_record_size[file]);
  for (i=0; i<strlen("rid 99,99 "); i++)
    printf(" ");
  for (i=0; i<hfp_record_size[file]; i++)
    printf("%d",i%10);
  printf("\n");
  rid spare_rid,r=hf_restart_scan(file);
  spare_rid.slot=MAXINT;
  i=0;
  while (r.page!=-1) {
    hf_search(file,r,str);
    if (strcmp(str,NOVAL)==0 && spare_rid.slot==MAXINT) {
      spare_rid=r;
	  strcpy(str,DB_EOF);
	}
    put_spaces(str);
    fprintf(not_sorted,"%s",str);
    if (print_to_screen && spare_rid.slot==MAXINT)
      printf("rid %2d,%-2d - %s",r.page,r.slot,str);
    i=(i+1)%PAGE_SIZE;
    r=hf_scan_next(file,r);
  }
  if (spare_rid.slot==MAXINT)
	strcpy(str,DB_EOF);
  else
    strcpy(str,NOVAL);
  put_spaces(str);
  while (i<PAGE_SIZE) {
    fprintf(not_sorted,"%s",str);
    i++;
  }
  fclose(not_sorted);
  // remove NULL records
  i=spare_rid.slot;
  if (i>0)
    for (spare_rid.slot=hfp_slots_in_page(file)-1; spare_rid.slot>=i; spare_rid.slot--)
      hf_delete(file,spare_rid);
}

void input_line(FILE* inputfile,int index,char* str)
{
  fseek(inputfile,0,SEEK_SET);
  int i=0;
  while (i<index) {
    fgets(str,MAXINT,inputfile);
    ignore_spaces(str);
    i++;
  }
}

int input_lines_total(FILE* inputfile)
{
  int i=0;
  char str[REC_SIZE+5]="x";
  while (!feof(inputfile) && !strstr(str,DB_EOF" ") && !strstr(str,NOVAL" ")) {
    fgets(str,MAXINT,inputfile);
    i++;
  }
  return i-1;
}

void hf_get(const int catalog,char* filename)
{
  int end,start,size,n;
  size=hfp_slots_in_page(catalog);
  FILE* inputfile=fopen(filename,"rb");
  assert(inputfile!=NULL);
  n=input_lines_total(inputfile);
  end=n;
  if (n%size==0)
    start=n-size+1;
  else
    start=n-n%size+1;
  while (end!=0) {
    int i;
    char str[REC_SIZE+5];
    for (i=start; i<=end; i++) {
      input_line(inputfile,i,str);
      hf_insert(catalog,str);
    }
    if (end%size!=0) {
      for (i=n+1; i<=start+size-1; i++)
        hf_insert(catalog,NOVAL);
    }
    end=start-1;
    start-=size;
  }
  hf_scan(catalog);
}

void dbms_restart()
{
  hf_done();
  buf_done();
  dm_done();

  dm_init();
  buf_init();
  hf_init();
}

int file1,fpos,flen,file2,f2fpos,matches;
void join();

void merge_sort(int file,int fpos,int flen)
{
  int rec_size;
  hf_scan(file);            // prepares FILE_SORTED
  rec_size=hfp_record_size[file];
  hf_delete_all(file);
  char name[80],sfpos[10],sflen[10];
  itoa(fpos,sfpos,10);
  itoa(flen,sflen,10);
  strcpy(name,strchr(hf_brief(),file+'0')+strlen("x-"));
  *(strchr(name,file+'1')-1)=0;
  file=hf_create(name,rec_size);
  if (spawnl(P_WAIT,"..\\merge\\debug\\merge.exe","..\\merge\\debug\\merge.exe",sfpos,sflen,NULL)<0) 
    printf("merge not found\n");
  hf_get(file,FILE_SORTED);
  dbms_restart();
}

void main()
{
  int recreate=1;
#ifdef TRACE
  trace_file=fopen("trace.db","w");
#else
  unlink("trace.db");
#endif
  dm_init(recreate);
  buf_init();
  hf_init(recreate);
  // end init //////////////////////////////////////////////////////

//#ifdef FIRST_RUN
  if (recreate==1) {
    int i;
    printf("\ncreating DB...\n");
    i=hf_create("student8",10+15+10+10+1);
    hf_get(i,"student8.txt");
    i=hf_create("course7", 10+15+10+3+1);
    hf_get(i,"course7.txt");
    i=hf_create("take5",   10+10+3+1);
    hf_get(i,"take5.txt");
    dbms_restart();
  }
//#else
  for (;;) {
    printf("\ntables: %s\n",hf_brief());
    printf("1 scan file     (inputs: file)\n");
    printf("2 insert record (inputs: file,record-no spaces!)\n");
    printf("3 delete record (inputs: file,page,slot)\n");
    printf("4 change record (inputs: file,page,slot,record)\n");
    printf("5 sort file     (inputs: file,field position starts from 0,field length)\n");
    printf("6 joins         (inputs: file,field pos.,field length,file2,file 2 field pos.)\n");
    printf("7 exit\n");
    printf("> ");
    int choice;
    scanf("%d",&choice);
    if (choice>=7)
      break;
    rid r;
    switch (choice) {
      case 1:
        scanf("%d",&file1);
        if (file1>=hf_count) break;
        hf_scan(file1,1);
        break;
      case 2:
        scanf("%d ",&file1);
        char str[REC_SIZE+5];
        gets(str);
        if (file1>=hf_count) break;
        hf_insert(file1,str);
        break;
      case 3:
        scanf("%d %d %d",&file1,&r.page,&r.slot);
        if (file1>=hf_count) break;
        hf_delete(file1,r);
        break;
      case 4:
        scanf("%d %d %d ",&file1,&r.page,&r.slot);
        gets(str);
        if (file1>=hf_count) break;
        hf_modify(file1,r,str);
        break;
      case 5:
        scanf("%d %d %d",&file1,&flen,&fpos);
        if (file1>=hf_count || fpos+flen>hfp_record_size[file1]) break;
		merge_sort(file1,flen,fpos);
        break;
      case 6:
        scanf("%d %d %d %d %d",&file1,&fpos,&flen,&file2,&f2fpos);
        if (file1>=hf_count ||   fpos+flen>hfp_record_size[file1]) break;
        if (file2>=hf_count || f2fpos+flen>hfp_record_size[file2]) break;
        join();
        break;
      default:
        printf("?\n");
    }
  }
//#endif

  // start close //////////////////////////////////////////////////////
  hf_done();
  buf_done();
  dm_done();
#ifdef TRACE
  fclose(trace_file);
#endif
}

// joins ////////////////////////////////////////////////////////////////

void join_strings(char* str,char* str2)
{
  if (!strncmp(str+fpos,str2+f2fpos,flen) && fpos+flen<strlen(str) && f2fpos+flen<strlen(str2)) {
    printf("%s,%s\n",str);
	matches++;
  }
}

void join_nested()
{
  io_counter=0;

  char str[REC_SIZE+5],str2[REC_SIZE+5];

  rid r=hf_restart_scan(file1);
  while (r.page!=-1) {
    hf_search(file1,r,str);

    rid r2=hf_restart_scan(file2);
    while (r2.page!=-1) {
      hf_search(file2,r2,str2);
      join_strings(str,str2);
      r2=hf_scan_next(file2,r2);
    }

    r=hf_scan_next(file1,r);
  }
}

void join_merge()
{
  merge_sort(file1,fpos,flen);

  if (file2>file1)
    file2--;

  merge_sort(file2,f2fpos,flen);
 
  if (file2>file1) {
    file2=hf_count-1;
	file1=hf_count-2;
  }
  else {
	file1=hf_count-1;
	file2=hf_count-2;
  }

  io_counter=0;

  // pg. 292
  // try with 6-join, 2 5 2 1 7, 2-merge

  char strTr[REC_SIZE+5],strTs[REC_SIZE+5],strG[REC_SIZE+5];
  rid Tr=hf_restart_scan(file1),Ts=hf_restart_scan(file2),Gs=hf_restart_scan(file2);

  while (Tr.page!=-1 && Gs.page!=-1) {
    hf_search(file2,Gs,strG);

	while (Tr.page!=-1) {
      hf_search(file1,Tr,strTr);
	  if (0<=strncmp(strTr+f2fpos,strG+fpos,flen)) break;
      Tr=hf_scan_next(file1,Tr);
	}

	while (Gs.page!=-1) {
      hf_search(file2,Gs,strG);
	  if (0>=strncmp(strTr+f2fpos,strG+fpos,flen)) break;
      Gs=hf_scan_next(file2,Gs);
	}

	while (Tr.page!=-1) {
      hf_search(file1,Tr,strTr);
      if (strncmp(strTr+f2fpos,strG+fpos,flen)) break;
	  Ts=Gs;
      while (Ts.page!=-1) {
        hf_search(file2,Ts,strTs);
		if (strncmp(strTs+fpos,strTr+f2fpos,flen)) break;
        join_strings(strTs,strTr);
        Ts=hf_scan_next(file2,Ts);
	  }
      Tr=hf_scan_next(file1,Tr);
	}

	Gs=Ts;
  }
}

// hash ////////////////////////////////////////////////////////////////

#define HASH_1 5
#define HASH_2 11

#define HASH_BUCKET_SIZE 50

#define hash_function_1(str) (atoi(str) % HASH_1)
#define hash_function_2(str) (atoi(str) % HASH_2)

void hash_create_file(int file,int fpos,int flen,char *filename)
{
  int h1_capacity[HASH_1]={0};

  FILE *f=fopen(filename,"wb");

  for (int i=0; i<HASH_1; i++)
	for (int j=0; j<HASH_BUCKET_SIZE; j++) {
	  char str[REC_SIZE+5]="";
	  put_spaces(str);
	  fprintf(f,"%s",str);
	}

  fseek(f,0,SEEK_SET);

  rid r=hf_restart_scan(file);
  while (r.page!=-1) {
    char str[REC_SIZE+5];
    hf_search(file,r,str);

	if (strstr(str,NOVAL)) 
	  break;

	char field_str[REC_SIZE+5];
	strncpy(field_str,str+fpos,flen);
	field_str[flen]=0;

	char hash_str[REC_SIZE+5];
    int h1_value=hash_function_1(field_str);
	fseek(f,HASH_BUCKET_SIZE*h1_value*REC_SIZE+h1_capacity[h1_value]*REC_SIZE,SEEK_SET);
	h1_capacity[h1_value]++;

	sprintf(hash_str,"%s - rid %2d,%-2d",field_str,str,r.page,r.slot);
	put_spaces(hash_str);

	fprintf(f,"%s",hash_str);
	if (hash_str[flen+1]=='-')
	  io_counter++;

    r=hf_scan_next(file,r);
  }

  fclose(f);
}

void hash2_perform(char *fname,char* f2name)
{
  FILE *f=fopen("1.txt","rb");
  char hash2_page[HASH_2][HASH_BUCKET_SIZE][REC_SIZE+5];
  int h2_capacity[HASH_2]={0};

  // pg. 296

  for (int i=0; i<HASH_1; i++) {

    // build in memory hash table for file1 using h2

	for (int j=0; j<HASH_BUCKET_SIZE; j++) {
	  fseek(f,i*REC_SIZE*HASH_BUCKET_SIZE+j*REC_SIZE,SEEK_SET);
	  char str[REC_SIZE+5];
	  fgets(str,32767,f);

	  if (str[flen+1]=='-') {
    	io_counter++;

        char field_str[REC_SIZE+5];
	    strncpy(field_str,str,flen);
	    field_str[flen]=0;

		// insert into hash table using h2

        int h2_value=hash_function_2(field_str);
  	    strcpy(hash2_page[h2_value][h2_capacity[h2_value]],str);
	    h2_capacity[h2_value]++;
	  }
	}

	// scan file2 and probe for matching file1's tuples

    FILE *f2=fopen("2.txt","rb");
	for (j=0; j<HASH_BUCKET_SIZE; j++) {
	
	  fseek(f2,i*REC_SIZE*HASH_BUCKET_SIZE+j*REC_SIZE,SEEK_SET);
 	  char str2[REC_SIZE+5];
	  fgets(str2,32767,f2);

	  if (str2[flen+1]=='-') {
		io_counter++;

        char field_str[REC_SIZE+5];
	    strncpy(field_str,str2,flen);
	    field_str[flen]=0;

        int h2_value=hash_function_2(field_str);

		// probe table using h2(s)

		for (int k=0; k<h2_capacity[h2_value]; k++) {
		  rid r;
		  if (!strncmp(hash2_page[h2_value][k],str2,flen)) {
		    printf("%s%s\n",hash2_page[h2_value][k],str2);
			matches++;
		    //goto rid, use io_counter++; in buf_read_page
		  } 
		}
	  }
	}

	fclose(f2);

  }

  fclose(f);
}

void join_hash()
{
  io_counter=0;
  hash_create_file(file1,fpos,flen,"1.txt");
  hash_create_file(file2,f2fpos,flen,"2.txt");
  hash2_perform("1.txt","2.txt");
}

// hash ////////////////////////////////////////////////////////////////

void join()
{
  matches=0;
	
  printf("\n");
  printf("1 nested\n");
  printf("2 merge\n");
  printf("3 hash\n");
  printf("4 exit\n");
  printf("> ");
  int choice;
  scanf("%d",&choice);
  if (choice>=4) return;
  switch (choice) {
    case 1:
      join_nested();
      break;
    case 2:
      join_merge();
      break;
    case 3:
      join_hash();
      break;
    default:
      printf("?\n");
  }

  printf("\nmatches: %d",matches);
  printf("\nIO operations: %d\n",io_counter);
}
