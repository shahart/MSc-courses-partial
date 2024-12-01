
/*
  Problem generators
  */
void initRandomizer()
{
   srand(getTimeMilliseconds()%1000);
}

byte randomValue(byte value, float distribution)
{
   float theValue = (float) ((double)rand()/(double)RAND_MAX);
   return (((distribution - theValue) > FLOAT_DELTA)?value:1-value);
}

void randProblem ( float density, float tightness)
{
   int var1,var2,dom1,dom2;
   byte consistentVars;

   // important - never check variable versus itself...
   // actually, such a check is a bug
   INIT1(constraintVal);

   initRandomizer();
   for (var1 = 0 ; var1 < VARS_NUM ; var1++)
       for (var2 = (var1+1) ; var2 < VARS_NUM; var2++)
       {
          consistentVars = randomValue(FALSE,density);
     	  for (dom1 = 0; dom1 < DOMAIN_SIZE; dom1++)
	      for (dom2 = 0; dom2 < DOMAIN_SIZE; dom2++)
	      {
	          constraintVal[var1][var2][dom1][dom2] = (consistentVars?TRUE:randomValue(FALSE,tightness));
	          // same constraint but variables order is swapped
                  constraintVal[var2][var1][dom2][dom1] = constraintVal[var1][var2][dom1][dom2];
  	      }
       }
}

void queensProblem ()
{
   int var1,var2,dom1,dom2;

   //variable is always consistent with itself
   //important - never check variable versus itself...
   INIT1(constraintVal);

   initRandomizer();
   for (var1 = 0 ; var1 < VARS_NUM ; var1++)
       for (var2 = (var1+1) ; var2 < VARS_NUM; var2++)
       {
     	  for (dom1 = 0; dom1 < DOMAIN_SIZE; dom1++)
	      for (dom2 = 0; dom2 < DOMAIN_SIZE; dom2++)
	      {
	          constraintVal[var1][var2][dom1][dom2] = ((dom1 != dom2) &&
	                                                   (abs(dom1 - dom2) != abs(var1 - var2)));
	          // same constraint but variables order is swapped
                  constraintVal[var2][var1][dom2][dom1] = constraintVal[var1][var2][dom1][dom2];
  	      }
       }
}
