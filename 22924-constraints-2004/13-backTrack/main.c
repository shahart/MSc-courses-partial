// first version- sanity of Prosser algorithms

#define N 8

//////////////////

long counter;
long nodes;
long jumps_amount;
long jumps_length;

int tmp;

#define INIT(x)          memset(x,0,sizeof(x))
#define COPY(x,y)        memcpy(x,y,sizeof(x))
#define NOT_EMPTY(x)     memchr(x,1,sizeof(x))!=NULL

#define MAX_LIST(h,set)  for (h=N-1; h>=0 && !set[h]; h--)
#define UNION(x,y)	    for (tmp=0; tmp<N; tmp++) x[tmp]|= y[tmp];
#define SET_DIFF(x,y)    for (tmp=0; tmp<N; tmp++) x[tmp]&=!y[tmp];

#define STACK_TO_LIST(stack,amount,list) \
  INIT(list); \
	 for (tmp=0; tmp<amount; tmp++) list[stack[tmp]]=1;

#define FOR_EACH(element,list) \
  for (element=0; element<N; element++) \
	 if (list[element])

//////////

#ifndef _DOMAIN
#define _DOMAIN

int current_domain[N][N];
int domain[N][N];
int v[N];

int check(int i, int h);
void init_domain(void);

#endif

#include <math.h>
#include <mem.h>

int check(int i, int n)
{
  counter++;
  // same line
  if (i==n)
	 return 0;
  // same diagonal
  if (abs(i-n)==abs(v[i]-v[n]))
	 return 0;
  // same row
  if (v[i]==v[n])
	 return 0;
  // fine
  return 1;
}

void init_domain()
{
  int i,j;
  for (i=0; i<N; i++)
	 for (j=0; j<N; j++)
		domain[i][j]=current_domain[i][j]=1;
  INIT(v);
}

/////////////////////////////////////////

#ifndef _CBT
#define _CBT

int cbt_label  (int i,int *consistent);
int cbt_unlabel(int i,int *consistent);

#endif

#include <mem.h>

int cbt_label(int i,int *consistent)
{
  *consistent=0;
  FOR_EACH(v[i],current_domain[i]) {
	 int h;
	 nodes++;
	 *consistent=1;
	 for (h=0; h<i && *consistent; h++)
		*consistent=check(i,h);
	 if (!(*consistent))
		current_domain[i][v[i]]=0;
	 else
		return i+1;
  }
  return i;
}

int cbt_unlabel(int i,int *consistent)
{
  int h;
  h=i-1;
  COPY(current_domain[i],domain[i]);
  current_domain[h][v[h]]=0;
  *consistent=NOT_EMPTY(current_domain[h]);
  return h;
}

//////////////////////////////

#ifndef _BJ
#define _BJ

int bj_label  (int i,int *consistent);
int bj_unlabel(int i,int *consistent);

int max_check[N];

#endif

#include <mem.h>
#include <stdlib.h>

int bj_label(int i,int *consistent)
{
  *consistent=0;
  FOR_EACH(v[i],current_domain[i]) {
	 int h;
    nodes++;
	 *consistent=1;
	 for (h=0; h<i && *consistent; h++) {
		*consistent=check(i,h);
		max_check[i]=max(max_check[i],h);
	 }
	 if (!(*consistent))
		current_domain[i][v[i]]=0;
	 else
		return i+1;
  }
  return i;
}

int bj_unlabel(int i,int *consistent)
{
  int h,j;
  h=max_check[i];
  for (j=h+1; j<=i; j++) {
	 max_check[j]=0;
	 COPY(current_domain[j],domain[j]);
  }
  current_domain[h][v[h]]=0;
  *consistent=NOT_EMPTY(current_domain[h]);
  return h;
}

//////////////////////////////

#ifndef _CBJ
#define _CBJ

int cbj_label  (int i,int *consistent);
int cbj_unlabel(int i,int *consistent);

int conf_set[N][N]={0};

#endif

#include <mem.h>
#include <stdlib.h>

int cbj_label(int i,int *consistent)
{
  *consistent=0;
  FOR_EACH(v[i],current_domain[i]) {
	 int h;
	 nodes++;
	 *consistent=1;
	 for (h=0; h<i && *consistent; h++)
		*consistent=check(i,h);
	 if (!(*consistent)) {
		current_domain[i][v[i]]=0;
		conf_set[i][h-1]=1;
	 }
	 else
		return i+1;
  }
  return i;
}

int cbj_unlabel(int i,int *consistent)
{
  int h,j;
  MAX_LIST(h,conf_set[i]);
  UNION(conf_set[h],conf_set[i])
  conf_set[h][h]=0;
  for (j=h+1; j<=i; j++) {
	 INIT(conf_set[j]);
	 COPY(current_domain[j],domain[j]);
  }
  current_domain[h][v[h]]=0;
  *consistent=NOT_EMPTY(current_domain[h]);
  return h;
}

///////////////////////////////

#ifndef _STACK
#define _STACK

typedef struct {
  int head;
  int st[N][N]; // N element, each is of size N
} stack_type;

void push(int item[N],stack_type *stack);
void pop (int item[N],stack_type *stack);

#endif

void push(int item[N],stack_type *stack)
{
  memcpy(stack->st[stack->head++],item,sizeof(stack->st[0]));
}

void pop (int item[N],stack_type *stack)
{
  memcpy(item,stack->st[--stack->head],sizeof(stack->st[0]));
}

//////////////////////////////

#ifndef _FC_COMMON
#define _FC_COMMON

int future_fc[N][N];

int   past_fc[N][N]; // stack!
int past_fc_head[N];

stack_type reductions[N];

int  check_forward(int i,int j);
void undo_reductions(int i);
void update_current_domain(int i);

#endif

int check_forward(int i,int j)
{
  int reduction[N]={0};
  FOR_EACH(v[j],current_domain[j])
	 if (!check(i,j))
		reduction[v[j]]=1;
  if (NOT_EMPTY(reduction)) {
	 SET_DIFF(current_domain[j],reduction)
	 push(reduction,&(reductions[j]));
	 future_fc[i][j]=1;
	 past_fc[j][past_fc_head[j]++]=i; // push(i,past_fc[j]);
  }
  return NOT_EMPTY(current_domain[j]);
}

void undo_reductions(int i)
{
  int reduction[N];
  int j;
  FOR_EACH(j,future_fc[i]) {
	 pop(reduction,&(reductions[j]));
	 UNION(current_domain[j],reduction)
	 past_fc_head[j]--; // pop(past_fc[j])
  }
  INIT(future_fc[i]);
}

void update_current_domain(int i)
{
  int x,y;
  int reduction[N];
  COPY(current_domain[i],domain[i]);
  for (x=0; x<reductions[i].head; x++) {
	 COPY(reduction,reductions[i].st[x]);
	 SET_DIFF(current_domain[i],reduction)
  }
}

//////////////////////////////

#ifndef _FC
#define _FC

int fc_label  (int i,int *consistent);
int fc_unlabel(int i,int *consistent);

#endif

int fc_label(int i,int *consistent)
{
  *consistent=0;
  FOR_EACH(v[i],current_domain[i]) {
	 int j;
	 nodes++;
	 *consistent=1;
	 for (j=i+1; j<N && *consistent; j++)
		*consistent=check_forward(i,j);
	 if (!(*consistent)) {
		current_domain[i][v[i]]=0;
		undo_reductions(i);
	 }
	 else
		return i+1;
  }
  return i;
}

int fc_unlabel(int i,int *consistent)
{
  int h;
  h=i-1;
  undo_reductions(h);
  update_current_domain(i);
  current_domain[h][v[h]]=0;
  *consistent=NOT_EMPTY(current_domain[h]);
  return h;
}

//////////////////////////////

#ifndef _FC_BJ
#define _FC_BJ

int fc_bj_label  (int i,int *consistent);
int fc_bj_unlabel(int i,int *consistent);

#endif

int fc_bj_label(int i,int *consistent)
{
  *consistent=0;
  FOR_EACH(v[i],current_domain[i]) {
	 int j;
	 nodes++;
	 *consistent=1;
	 for (j=i+1; j<N && *consistent; j++)
		*consistent=check_forward(i,j);
	 if (!(*consistent)) {
		int temp[N],h;
		current_domain[i][v[i]]=0;
		undo_reductions(i);
		STACK_TO_LIST(past_fc[j-1],past_fc_head[j-1],temp)
		MAX_LIST(h,temp);
		max_check[i]=max(max_check[i],h);
	 }
	 else {
		max_check[i]=i-1;
		return i+1;
	 }
  }
  return i;
}

int fc_bj_unlabel(int i,int *consistent)
{
  int h_past_fc,h,j;

  int temp[N];
  STACK_TO_LIST(past_fc[i],past_fc_head[i],temp);
  MAX_LIST(h_past_fc,temp);
  h=max(max_check[i],h_past_fc);

  for (j=i; j>h; j--) {
	 max_check[j]=0;
	 undo_reductions(j);
	 update_current_domain(j);
  }
  undo_reductions(h);
  current_domain[h][v[h]]=0;
  *consistent=NOT_EMPTY(current_domain[h]);
  return h;
}


//////////////////////////////

#ifndef _FC_CBJ
#define _FC_CBJ

int fc_cbj_label  (int i,int *consistent);
int fc_cbj_unlabel(int i,int *consistent);

#endif

int fc_cbj_label(int i,int *consistent)
{
  *consistent=0;
  FOR_EACH(v[i],current_domain[i]) {
	 int j;
	 nodes++;
	 *consistent=1;
	 for (j=i+1; j<N && *consistent; j++)
		*consistent=check_forward(i,j);
	 if (!(*consistent)) {
		int temp[N];
		current_domain[i][v[i]]=0;
		undo_reductions(i);
		STACK_TO_LIST(past_fc[j-1],past_fc_head[j-1],temp)
		UNION(conf_set[i],temp)
	 }
	 else
		return i+1;
  }
  return i;
}

int fc_cbj_unlabel(int i,int *consistent)
{
  int h_conf_set,h_past_fc,h,j;

  int temp[N];
  STACK_TO_LIST(past_fc[i],past_fc_head[i],temp);
  MAX_LIST(h_past_fc,temp);
  MAX_LIST(h_conf_set,conf_set[i]);
  h=max(h_conf_set,h_past_fc);

  UNION(conf_set[h],conf_set[i])
  UNION(conf_set[h],temp);
  conf_set[h][h]=0;

  for (j=i; j>h; j--) {
	 INIT(conf_set[j]);
	 undo_reductions(j);
	 update_current_domain(j);
  }
  undo_reductions(h);
  current_domain[h][v[h]]=0;
  *consistent=NOT_EMPTY(current_domain[h]);
  return h;
}

//////////////////////////////

// binary constraint satisfaction search problem

#ifndef _BCSSP
#define _BCSSP

void bcssp(int (*label)(int,int*),int (*unlabel)(int,int*));

#endif

#include <stdio.h>

int used[N];

void bcssp(int (*label)(int i,int *consistent),int (*unlabel)(int i,int *consistent))
{
  int consistent=1;
  int i=0;

  while (i<N) // stop when all used[]
	 if (consistent) {
		int x,y,sum,min_sum=N;
		used[i]=1;
		i=(*label)(i,&consistent);
		for (y=0; y<N; y++)
		  if (!used[y]) {
			 sum=0;
			 for (x=0; x<N; x++)
				sum+=current_domain[y][x];
			 if (sum<min_sum) {
				min_sum=sum;
				i=y;
			 }
		  }
	 }
	 else {
		int new_i;
		used[i]=0;
		new_i=(*unlabel)(i,&consistent);
		jumps_amount++;
		jumps_length+=i-new_i;
		i=new_i;
	}

  if (i==0)
	 printf("\nNo solution\n");
}

///////////////////////

#include <sys/timeb.h>

long  curr_time;
short curr_millitm;

struct timeb prev_buf;

void get(void)
{
  ftime(&prev_buf);
  counter=nodes=jumps_length=jumps_amount=0;
  init_domain();
}

void put(char *string)
{
  struct timeb buf;
  ftime(&buf);
  printf("%s\t%ld\t%dms\t%ld\t",string,counter,1000*(buf.time-prev_buf.time)+buf.millitm-prev_buf.millitm,nodes);
  printf("%ld\t%1.2f\n",jumps_amount,(float)jumps_length/(float)jumps_amount);
}

///////////////////////

/* 18 queens

Algo.   #checks time    assignments     jumps   length

cbt     4257666 440ms   1463615488      41281   1.00
bj      3656271 380ms   -1158414336     31785   1.22
cbj     3507297 330ms   1494548480      28618   1.32

fc      462788  330ms   1985806336      17607   1.00
fc_bj   459174  330ms   1950220288      15908   1.10
fc_cbj  458501  330ms   1945239552      15298   1.14

*/

#include <stdio.h>

void main()
{
  printf("Algo.\t#checks\ttime\tassignments\tjumps\tlength\n\n");

  // look back
  get(); bcssp(cbt_label,cbt_unlabel); put("cbt");

  INIT(max_check);
  get(); bcssp( bj_label, bj_unlabel); put("bj");

  INIT(conf_set);
  get(); bcssp(cbj_label,cbj_unlabel); put("cbj");

  // look ahead

  INIT(reductions); INIT(future_fc); INIT(past_fc); INIT(past_fc_head);
  get(); bcssp(fc_label, fc_unlabel); put("\nfc");

  INIT(reductions); INIT(future_fc); INIT(past_fc); INIT(past_fc_head); INIT(max_check);
  get(); bcssp(fc_bj_label, fc_bj_unlabel); put("fc_bj");

  INIT(reductions); INIT(future_fc); INIT(past_fc); INIT(past_fc_head); INIT(conf_set);
  get(); bcssp(fc_cbj_label, fc_cbj_unlabel); put("fc_cbj");
}
