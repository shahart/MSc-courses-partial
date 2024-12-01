// DBMS 2003a - exercise 4.12
// physical layer

// all globals can be private, including XXX_init/done methods
// so porting to OO is quite easy

// handling errors with assert for debug
// user must have no mistakes

// echo debug information
#define TRACE

// creates dm_database files for the first time
//#define FIRST_RUN

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

// important defs

#define PAGE_SIZE_IN_CHARS   64 // 4096 in NTFS, mini 14 (MAX_FILE_NAME+(PAGE_SIZE-NUM_RECS_LOC))
#define BUF_SIZE             32

#define MAX_HEAP_FILES       64
#define MAX_PAGES         8192u

#define TRUE 1
#define PAGE_SIZE (PAGE_SIZE_IN_CHARS/sizeof(int))

typedef int page[PAGE_SIZE];

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

int   dm_bitmap_full[MAX_PAGES]={!TRUE}; // 1=used block
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

void dm_init()
{
#ifdef FIRST_RUN
  dm_database=fopen(DB,"wb+");
#else
  // add backup in case assert fails
  system("copy "DB" "DB"2 /y > nul");
  dm_database=fopen(DB,"rb+");
  FILE* f=fopen(BITMAP,"rb");
  fread(dm_bitmap_full,sizeof(dm_bitmap_full[0]),MAX_PAGES,f);
  fclose(f);
#endif
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

#define USED_SLOTS_LOC (PAGE_SIZE-1)

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
  p[PAGE_SIZE-1]--;
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
    strcpy(rec,"NULL");
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
#define NUM_RECS_LOC   (PAGE_SIZE-4)
#define PREV_PAGE_LOC  (PAGE_SIZE-3)
#define NEXT_PAGE_LOC  (PAGE_SIZE-2)

int  hf_first_page  [MAX_HEAP_FILES];
int  hf_full_page   [MAX_HEAP_FILES];
int  hf_free_page   [MAX_HEAP_FILES];
int  hf_free_scanned[MAX_HEAP_FILES];
char hf_briefs      [MAX_HEAP_FILES*(MAX_FILE_NAME+5)]={0};
int  hf_count=0;

const char* hf_brief()
{
  return hf_briefs;
}

const int hf_create(const char* name,const int rec_size)
{
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

void hf_init()
{
#ifndef FIRST_RUN
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
#endif
}

void hf_done()
{
  FILE* f=fopen(HEADERS,"w");
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

void hf_scan(int file)
{
  printf("file %d, free %d, full %d, records %d, record_size %d\n",file,hf_free_page[file],hf_full_page[file],hfp_records_total[file],hfp_record_size[file]);
  rid r=hf_restart_scan(file);
  while (r.page!=-1) {
    char str[80];
    hf_search(file,r,str);
    printf("rid %2d,%-2d %s\n",r.page,r.slot,str);
    r=hf_scan_next(file,r);
  }
}

void main()
{
#ifdef TRACE
  trace_file=fopen("trace.db","w");
#else
  unlink("trace.db");
#endif
  dm_init();
  buf_init();
  hf_init();
  // end init //////////////////////////////////////////////////////

#ifdef FIRST_RUN
  int i;
  printf("\ncreating DB...\n");
  i=hf_create("students",10+15+10+10+1);
	           //123456789012345678901234512345678901234567890
  hf_insert(i,"1");  hf_insert(i,"2");   hf_insert(i,"3");  hf_insert(i,"4");
  hf_insert(i,"5");  hf_insert(i,"6");   hf_insert(i,"7");  hf_insert(i,"8");
  hf_insert(i,"9");  hf_insert(i,"10");  hf_insert(i,"11");
  hf_scan(i);
  i=hf_create("course",  10+15+10+3+1);
	           //12345678901234567890123451234567890123
  hf_insert(i,"1");  hf_insert(i,"2");   hf_insert(i,"3");  hf_insert(i,"4");
  hf_insert(i,"5");  hf_insert(i,"6");   hf_insert(i,"7");  hf_insert(i,"8");
  hf_insert(i,"9");  hf_insert(i,"10");  hf_insert(i,"11");
  hf_scan(i);
  i=hf_create("takes",   10+10+3+1);
             //12345678901234567890123
  hf_insert(i,"1|DBMS-----|095");
  hf_insert(i,"2--------|DBMS adv-|071");
  hf_insert(i,"3|OOSE-----|100");
  hf_insert(i,"4");
  hf_insert(i,"5");  hf_insert(i,"6");   hf_insert(i,"7");  hf_insert(i,"8");
  hf_insert(i,"9");  hf_insert(i,"10");  hf_insert(i,"11");
  hf_scan(i);
#else
  for (;;) {
    printf("\ntables: %s\n",hf_brief());
    printf("1 scan file     (inputs: file)\n");
    printf("2 insert record (inputs: file,record)\n");
    printf("3 delete record (inputs: file,page,slot)\n");
    printf("4 change record (inputs: file,page,slot)\n");
    printf("5 exit\n");
    printf("> ");
    int choice;
    scanf("%d",&choice);
    if (choice>=5)
      break;
    int file;
    rid r;
    char str[80];
    switch (choice) {
      case 1:
        scanf("%d",&file);
        hf_scan(file);
        break;
      case 2:
        scanf("%d %s",&file,str);
        hf_insert(file,str);
        break;
      case 3:
        scanf("%d %d %d",&file,&r.page,&r.slot);
        hf_delete(file,r);
        break;
      case 4:
        scanf("%d %d %d %s",&file,&r.page,&r.slot,str);
        hf_modify(file,r,str);
        break;
      default:
        printf("?\n");
    }
  }
#endif

  // start close //////////////////////////////////////////////////////
  hf_done();
  buf_done();
  dm_done();
#ifdef TRACE
  fclose(trace_file);
#endif

}
