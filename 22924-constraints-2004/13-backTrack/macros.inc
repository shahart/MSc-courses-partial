
#define INIT1(x)          memset(x,0,sizeof(x))
#define COPY1(x,y)        memcpy(x,y,sizeof(x))
#define INIT(x,val,setSize)   {for (setIdx = 0; setIdx < setSize; setIdx++) x[setIdx] = val;}
#define COPY(x,y,setSize)     {for (setIdx = 0; setIdx < setSize; setIdx++) x[setIdx] = y[setIdx];}
#define NOT_EMPTY(res,x,setSize)  {res = FALSE; \
                                   for (setIdx = 0; setIdx < setSize; setIdx++) res |= x[setIdx];}

#define MAX(x,y)	 ((x >= y)?x:y)
#define MAX_LIST(h,set,setSize)  {for (h = setSize - 1; h>=0 && !set[h]; h--);\
				  h = MAX(0,h);}
#define UNION(x,y,setSize)       for (setIdx = 0; setIdx < setSize; setIdx++) x[setIdx] |=  y[setIdx];
#define SET_DIFF(x,y,setSize)    {for (setIdx = 0; setIdx < setSize; setIdx++) x[setIdx] &= !y[setIdx];}

#define STACK_TO_LIST(stack, amount, list, setSize) \
   { INIT(list,FALSE,setSize); \
	 for (stackIdx = 0; stackIdx < amount; stackIdx++) list[ stack [stackIdx]] = TRUE; }

#define FOR_EACH(element, list, listSize) \
  for (element = 0; element < listSize; element++) \
	 if (list[element])
