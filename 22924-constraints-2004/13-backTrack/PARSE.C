// calc averages

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DENSITY    9
#define DENS_STEP  1

#define TIGHT      9
#define TIGHT_STEP 1

#define TESTS     10

#define KINDS      3

char kind_str[KINDS][10]={"cbj","fc","fc_cbj"};

void parse(int row,int kind_interest)
{
  FILE* f=fopen("d191t191.txt","r");
  char s[512];

  int dens,tight,kind,test;

  long sums[KINDS];
  int counts[KINDS];

  printf("\n%s\n",kind_str[kind_interest]);

  for (dens=DENS_STEP; dens<=DENSITY; dens+=DENS_STEP) {
	 for (tight=TIGHT_STEP; tight<=TIGHT; tight+=TIGHT_STEP) {
		fgets(s,sizeof(s),f); // header

		memset(sums,0,sizeof(sums));
		memset(counts,0,sizeof(counts));

		for (test=0; test<TESTS; test++) {
		  for (kind=0; kind<KINDS; kind++) {
			 fgets(s,sizeof(s),f);
			 if (s[61]=='S') {       // take into account?
				char value[20];
				strncpy(value,s+row,sizeof(value));
				sums[kind]+=atol(value);
				counts[kind]++;
			 }
		  }
		}
		printf("%ld\t",(counts[kind_interest]!=0)?
							  sums[kind_interest]/counts[kind_interest]:0);
	 }
	 printf("\n");
  }

  fclose(f);
}

void main()
{
  int kind;

  printf("\ncounts\n");
  for (kind=0; kind<KINDS; kind++)
	 parse(68,kind);

  printf("\nnodes\n");
  for (kind=0; kind<KINDS; kind++)
	 parse(78,kind);
}
