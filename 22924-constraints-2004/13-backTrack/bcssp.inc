
// this is main algorithm which implements walking around the search tree.
void bcssp(int (*labelNode)(int i,byte *consistent),int (*unlabelNode)(int i,byte *consistent), char *algName)
{
  byte consistent = TRUE;
  int nodeLevel = 1;
  int solution = FALSE;
  long startTime = getTimeMilliseconds ();
  long runTime;
  int newLevel;
  initSolver();

  while (TRUE)
  {
	 if (consistent)
	 {
		nodeLevel = (*labelNode) (nodeLevel, &consistent);
	 }
	 else {
		newLevel = (*unlabelNode) (nodeLevel, &consistent);
		//printf("backtrack done - %d -> %d ,%d\n",nodeLevel, newLevel, consistent);
		if ((nodeLevel - newLevel) > 1)
		{
			jumpsCounter++;
			jumpLength += nodeLevel - newLevel;
		}
		nodeLevel = newLevel;
	}
	if ((nodeLevel == NODES_NUM) && consistent)
	{
	   //printf("solved!!!\n");
	   solution = TRUE;
	   break;
	}else if (nodeLevel == 0) {
	   break;
	}
	runTime = getTimeMilliseconds() - startTime;
	if ((runTime > timeOutMLS) && (timeOutMLS != 0))
	{
	   break;
	}
  }
  reportResults(solution, runTime, algName);
}
