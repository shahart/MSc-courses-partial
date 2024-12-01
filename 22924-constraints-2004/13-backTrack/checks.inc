
////////////////////////////////////////////////////////////
/*
  Constraint/Domain checks
*/
byte check(int var1, int var2, int val1, int val2)
{
   // variable is always consisten with itself TBD?
   if (var1 == var2)
   {
   	printf (" check the same variables - possible bug %d, %d\n", var1, var2);
        return TRUE;
   }
   checksCounter++;
   return (constraintVal[var1][var2][val1][val2]);
}

byte inDomain(int var, int val)
{
   int var1;
   int val1;
   for (var1 = 0; var1 < VARS_NUM; var1++)
   	if (var1 != var)
   	{
		for (val1 = 0; val1 < DOMAIN_SIZE; val1++)
	    		if (check(var,var1,val,val1))
	    			return TRUE;
	}
   return FALSE;
}

