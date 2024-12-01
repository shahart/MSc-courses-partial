#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>

/*
  Definition section
*/

#define TRUE (1)
#define FALSE (0)
#define FLOAT_DELTA (0.000001)

//#define VARS_NUM (20)
//#define DOMAIN_SIZE (7)

#define NODES_NUM (VARS_NUM + 1) // node #0 is a fake node meaning that the set is empty or no solution
#define FIRST_NODE (1)

#include "macros.inc"

/*
  Types section
*/
typedef unsigned char byte;


typedef struct {
  int head;
  int st[VARS_NUM][DOMAIN_SIZE]; // Stack of size of the tree depth, each element of the domain size
} stack_type;

/*
  Global variables
  */

int setIdx;   // iterator for set operations
int stackIdx; // iterator for stack operations

/*
  Problem specification.
  pay attention var1,var2,val1,val2 == var2,var1,val2,val!!!
  i.e. olthough we need only half an array in reality - we cope with symetry
  by using the full matrix.
*/
byte constraintVal[VARS_NUM][VARS_NUM][DOMAIN_SIZE][DOMAIN_SIZE]; // generated problem

byte initial_domain[VARS_NUM][DOMAIN_SIZE]; // initial domain for each variable.
                                           // based on generated problem
                                           // is computed prior to search
byte current_domain[VARS_NUM][DOMAIN_SIZE]; // domain working area - subject to change
					   // during the search procedure

int assigned_var_ids[NODES_NUM];  // tree stack
int assigned_var_vals[VARS_NUM];  // assignements for each variable

int checksCounter; // constraint checks counter
int nodesCounter;  // visited nodes counter
int jumpsCounter;  // performed jumps (i.e. backtracks with length > 1)
long jumpLength;   // overall jump length

int max_check[VARS_NUM]; // for each variable maximal node the check performed against

int conf_set[NODES_NUM][NODES_NUM]; // for each node on tree list of inconsistent preceeding nodes

int future_fc[VARS_NUM][VARS_NUM]; // for each variable (and not tree node) list of future variables
                                   // we propagated domain at

int past_fc[VARS_NUM][NODES_NUM]; // stack!
int past_fc_head[VARS_NUM];      // for each variable list of tree nodes which variables contributed
                                 // to the vaiable domain reduction.

stack_type reductions[VARS_NUM]; // reduction stack per each variable
				 // index is real varId and not node level in the search tree



////////////////////////////////////////////////////////////
/*
 defintions for environment procedures
*/

#define ALGS_NUM 6
#define BT_ALG     0
#define BJ_ALG     1
#define CBJ_ALG    2
#define FC_ALG     3
#define FC_BJ_ALG  4
#define FC_CBJ_ALG 5

#define OPTION_KINDS 8
#define DYNAMIC_OPT 0
#define DENSITY_OPT 1
#define TIGHT_OPT 2
#define ITERATE_OPT 3
#define SAVE_OPT 4
#define LOAD_OPT 5
#define ALG_KIND_OPT 6
#define TIMEOUT_OPT 7

int dynamic;
int algsToUse[ALGS_NUM] = {0};
float density;
float tightness;
int   iterationsNum = 1;
int   timeOutMLS = 0;
char* saveFileName;
char* loadFileName;
char* allOptions[OPTION_KINDS]={"-d","-p","-t","-i","-s","-l","-alg","-timeout"};
int optionsSet[OPTION_KINDS] = {-1};

/////////////////////////////////////////////////////////////
/*
   Functions specification
   */

/////////////////// STACK functions
void push(int item[DOMAIN_SIZE], stack_type *stack);
void pop (int item[DOMAIN_SIZE], stack_type *stack);

// variable ordering function
void reorder_remained_variables(int nodeLevel, int dynamic);

// constraint check function -  TRUE if tuple is consistent
byte check(int var1, int var2, int val1, int val2);

// domain belonging check - TRUE if value is part of variable domain -based on constraint check
byte inDomain(int var, int val);

// random problem generator - populates constraintVal.
// density  - probability that 2 variables constraint each other
// tightess - probability that pair valuation is inconsistent
void randProblem ( float density, float tightness);

// like random problem generator, but generates queens problem constraint
void queensProblem ();

// search algorithms initializer - sets the domains and clears counters, conflict sets etc.
void initSolver();

// forward checking auxilary functions
int check_forward          (int nodeLevel, int futureVarId);
void undo_reductions       (int nodeLevel);
void update_current_domain (int currentVarId);

// all algorithms
int cbt_label  (int nodeLevel, byte *consistent);
int cbt_unlabel(int nodeLevel, byte *consistent);

int bj_label   (int nodeLevel, byte *consistent);
int bj_unlabel (int nodeLevel, byte *consistent);

int cbj_label  (int nodeLevel, byte *consistent);
int cbj_unlabel(int nodeLevel, byte *consistent);

int fc_label  (int nodeLevel, byte *consistent);
int fc_unlabel(int nodeLevel, byte *consistent);

int fc_bj_label   (int nodeLevel, byte *consistent);
int fc_bj_unlabel (int nodeLevel, byte *consistent);

int fc_cbj_label  (int nodeLevel, byte *consistent);
int fc_cbj_unlabel(int nodeLevel, byte *consistent);

char* algName[ALGS_NUM] = {"bt","bj","cbj","fc","fc_bj","fc_cbj"};
int (*labelNode  [ALGS_NUM]) (int nodeLevel, byte *consistent) =
	{cbt_label, bj_label, cbj_label, fc_label, fc_bj_label, fc_cbj_label};

int (*unlabelNode[ALGS_NUM]) (int nodeLevel, byte *consistent) =
	{cbt_unlabel, bj_unlabel, cbj_unlabel, fc_unlabel, fc_bj_unlabel, fc_cbj_unlabel};


void bcssp(int (*labelNode)(int i,byte *consistent), int (*unlabelNode)(int i,byte *consistent), char *algName);

///////////////////////////////////////////////
/*
  Enviroment section
*/
void saveProblem (char *filename);
void loadProblem(char *filename);
void bad_options(int argc, char** argv);
int getOptionId(char *optionName, char **allOptions, int optionsNum);
byte check_options(void);
void run_solvers();
long getTimeMilliseconds ();
void reportResults(byte solution, long runTime, char *algName);

////////////////////////////////////////////////////////////

int main (int argc, char** argv)
{
    int argId;
    int optId;
    dynamic = FALSE;
    timeOutMLS = 0;
    for (argId = 1; argId < argc; argId++)
    {
        optId = getOptionId(argv[argId],allOptions,OPTION_KINDS);
        switch(optId){
          case DYNAMIC_OPT :
               dynamic = TRUE;
               break;
          case DENSITY_OPT :
               density = atof(argv[++argId]);
               break;
          case TIGHT_OPT :
               tightness = atof(argv[++argId]);
               break;
          case ITERATE_OPT :
               iterationsNum = atoi(argv[++argId]);
               break;
          case TIMEOUT_OPT :
               timeOutMLS = atoi(argv[++argId]);
               break;
          case SAVE_OPT :
               saveFileName = argv[++argId];
               break;
          case LOAD_OPT :
               loadFileName = argv[++argId];
               break;
          case ALG_KIND_OPT :
                  algsToUse[getOptionId(argv[++argId], algName, ALGS_NUM)] = TRUE;
               break;
          default :
          	bad_options(argc,argv);
        }
        optionsSet[optId] = TRUE;
    }

    if (!check_options())
    {
	bad_options(argc,argv);
    }

    run_solvers();
}

void run_solvers()
{
   int iteration;
   int algId;
   char realOutFileName[256];

   /*
   printf("iterate for %d times\n", iterationsNum);

     if (optionsSet[LOAD_OPT])
        printf("load problem from the file %s\n", loadFileName);
     else
     	printf("generate problem with %f density and %g tightness\n", density, tightness);

     if (optionsSet[SAVE_OPT])
        printf("save each iteration in file %s<it_num>.txt\n",saveFileName);

     if (optionsSet[TIMEOUT_OPT])
        printf("run each algorithm for no more than %d milliseconds\n",timeOutMLS);

     for (algId = 0; algId <ALGS_NUM; algId++)
     {
     	if (algsToUse[algId])
           printf(" solve using %s \n", algName[algId]);
     }
     */
   for (iteration=0; iteration < iterationsNum; iteration++)
   {
     if (optionsSet[LOAD_OPT])
        loadProblem(loadFileName);
     else
#ifdef QUEENS
	queensProblem();
#else
        randProblem(density, tightness);
#endif

     if (optionsSet[SAVE_OPT])
     {
        sprintf(realOutFileName,"%s_%d.inc",saveFileName, (iteration+1));
        saveProblem(realOutFileName);
     }

     for (algId = 0; algId < ALGS_NUM; algId++)
     {
     	if (algsToUse[algId])
     	   bcssp(labelNode[algId], unlabelNode[algId], algName[algId]); //, dynamic, timeOutMLS);
     }
   }
}

#include "bcssp.inc"
#include "checks.inc"
#include "problems.inc"

///////////////////////////////////////////////////////
/*
  Common initializer:
	initializes domains, counters and internal structures
  */
void initSolver()
{
   int var1, val1;

   INIT1(initial_domain);
   INIT1(current_domain);

   for (var1 = 0 ; var1 < VARS_NUM; var1++)
       for (val1 = 0; val1 < DOMAIN_SIZE; val1++)
	   current_domain[var1][val1] = initial_domain[var1][val1] = inDomain(var1,val1);

   checksCounter = 0;
   nodesCounter = 0;
   jumpsCounter = 0;
   jumpLength = 0;
   setIdx = -1;
   stackIdx = -1;

  INIT1(reductions); INIT1(future_fc); INIT1(past_fc); INIT1(past_fc_head); INIT1(max_check); INIT1(conf_set);
  INIT1(assigned_var_ids); INIT1(assigned_var_vals);
}

//////////////////////////////////////////////////////////////////

#include "bt_labels.inc"
#include "fc_labels.inc"
#include "ordering.inc"
#include "saveload.inc"

//////////////////////////////////////////////////////////
/*
  Environment functions - options verification etc.
*/


void bad_options(int argc, char** argv)
{
    int ii;
    for (ii=0; ii < argc; ii++)
        printf("%s ", argv[ii]);
    printf("\n");
    printf("usage: %s <-d> [-alg algname] -p num -t num -i num <-s filename> <-l filename> [-timeout milliseconds]\n",
            argv[0]);
    printf("-timeout num - stop after searching for <num> milliseconds.\n");
    printf("               Optional. If ommited will search unimited time\n");
    printf("-p num       - density - probability that any pair of variables has constraint\n");
    printf("               required when loading not used\n");
    printf("-t num       - tightness - probability that in constraint valuation is inconsistent\n");
    printf("               required when loading not used\n");
    printf("-i num       - iterate random problem <num> times, optional, default = 1\n");
    printf("               should be unspecified when loading is used\n");
    printf("-s filename  - save all problems to filename_<iteration>.txt (optional)\n");
    printf("-l filename  - load problem from filename (optional)\n");
    printf("-d           - use dynamic variable order - actual only for fc*, \n");
    printf("               in all others will set static order based on initial domain\n");
    printf("-alg algname - use <algname> algorithm at least one such option shall be specified\n");
     for (ii = 0; ii < ALGS_NUM; ii++)
     {
	printf("               -alg %s\n", algName[ii]);
     }
    exit(-1);
}

int getOptionId(char *optionName, char **allOptions, int optionsNum)
{
    int optId;
    for(optId = 0; ((optId < optionsNum) && strcmp(optionName,allOptions[optId])!=0);
        optId++);
    return (optId);
}

byte check_options(void)
{
    if ((   !optionsSet[DENSITY_OPT]
         || !optionsSet[TIGHT_OPT]) &&
	(!optionsSet[LOAD_OPT]))
        return FALSE;

    if (optionsSet[LOAD_OPT] && (optionsSet[ITERATE_OPT] || optionsSet[DENSITY_OPT] || optionsSet[TIGHT_OPT]))
       return FALSE;

    if (optionsSet[SAVE_OPT] && optionsSet[LOAD_OPT])
       return FALSE;

    if (!optionsSet[ALG_KIND_OPT])
       return FALSE;

    return TRUE;
}

/////////////////////////////////////////////////////
/*
  Stack functions
*/
void push(int item[DOMAIN_SIZE], stack_type *stack)
{
  memcpy(stack->st[stack->head++], item , sizeof(stack->st[0]));
}

void pop (int item[DOMAIN_SIZE],stack_type *stack)
{
  memcpy(item, stack->st[--stack->head], sizeof(stack->st[0]));
}

/////////////////////////////////////////////////////
/*
  General functions
  */
long getTimeMilliseconds ()
{
   struct timeb time;
   ftime(&time);
   return (time.time*1000 + time.millitm);
}

char *solved[2] = {"Unsolved", "Solved"};
char *ordering[2] = {"Static", "Dynamic"};

byte firstTime = TRUE;

void reportResults(byte solution, long runTime, char *algName)
{
   int varId;
   // Vars Domain Dens Tight Alg Solution Checks Nodes Jumps MeanJump Time
   if (firstTime)
   {
	printf("Var Dom  MaxTime Density Tightns    Alg  VarOrd SearchTime Solution     Checks      Nodes   Jumps JumpLn\n");
	firstTime = FALSE;
   }
   printf("%3d %3d %8ld %1.5f %1.5f %6s %s %10ld %8s %10ld %10ld %7ld %2.4f",
          VARS_NUM, DOMAIN_SIZE, timeOutMLS, density, tightness, algName, ordering[dynamic], runTime, solved[solution],
	  checksCounter, nodesCounter, jumpsCounter, (float)jumpLength/(float)jumpsCounter);
	if (solution)
	{
		for (varId = 0; varId < VARS_NUM; varId++)
			printf(" %d", assigned_var_vals[varId]);
	}
   printf("\n");
}



