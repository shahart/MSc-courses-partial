/*
  Problem managin functions - save/load
  */
  
void saveProblem (char *filename)
{
   FILE *outfile;
   int var1,var2,dom1,dom2;

   outfile = (FILE*)fopen(filename,"w");
   fprintf(outfile, "%f %f\n",density,tightness);

   for (var1 = 0 ; var1 < VARS_NUM ; var1++)
       for (var2 = (var1 + 1) ; var2 < VARS_NUM; var2++)
    	   	for (dom1 = 0; dom1 < DOMAIN_SIZE; dom1++)
	       	    for (dom2 = 0; dom2 < DOMAIN_SIZE; dom2++)
                        if (constraintVal[var1][var2][dom1][dom2])
		  	    fprintf(outfile,"%d %d %d %d\n", var1, var2, dom1, dom2);

   fclose(outfile);
}

void loadProblem(char *filename)
{
   int var1,var2,dom1,dom2;
   FILE *infile;

   INIT1(constraintVal);

   infile = (FILE*)fopen(filename,"r");

   fscanf(infile, "%f %f", &density, &tightness);
   while (!feof(infile))
   {
      fscanf(infile,"%d %d %d %d", &var1, &var2, &dom1, &dom2);
      if (var2 > var1)
      {
	constraintVal[var1][var2][dom1][dom2] = 1;
	constraintVal[var2][var1][dom2][dom1] = 1;
      }
   }
   fclose(infile);

}
