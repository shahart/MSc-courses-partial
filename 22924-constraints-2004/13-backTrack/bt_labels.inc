/*
  Simple backtracking search
  */
int cbt_label(int nodeLevel, byte *consistent)
{
  int currentVarId;
  int assignedNode;
  int nextNode;

  reorder_remained_variables (nodeLevel, dynamic);
  currentVarId = assigned_var_ids[nodeLevel];

  *consistent = FALSE;

  FOR_EACH(assigned_var_vals[currentVarId], current_domain[currentVarId], DOMAIN_SIZE)
  {
	 nodesCounter++;
	 *consistent = TRUE;
	 for (assignedNode = FIRST_NODE; assignedNode < nodeLevel && *consistent; assignedNode++)
	 {
	    //printf("label check %d %d which is %d %d:\n",
	    //         assignedNode, nodeLevel, assigned_var_ids[assignedNode], currentVarId);
		*consistent = check(assigned_var_ids[assignedNode], currentVarId,
			  	    assigned_var_vals[assigned_var_ids[assignedNode]],
                                    assigned_var_vals[currentVarId]);
         }
	 if (!(*consistent))
		current_domain[currentVarId][assigned_var_vals[currentVarId]] = FALSE;
	 else {
		nextNode = nodeLevel + 1;
	 	return nextNode;
	 }
  }
  return nodeLevel;
}

int cbt_unlabel(int nodeLevel, byte *consistent)
{
  int nodeToBack = nodeLevel - 1;
  int currVarId = assigned_var_ids[nodeLevel];
  int backVarId = assigned_var_ids[nodeToBack];

  COPY(current_domain[currVarId], initial_domain[currVarId], DOMAIN_SIZE);

  current_domain[backVarId][assigned_var_vals[backVarId]] = FALSE;
  NOT_EMPTY((*consistent), current_domain[backVarId], DOMAIN_SIZE);

  return nodeToBack;
}

//////////////////////////////
/*
  Back-jumping search
  */
int bj_label(int nodeLevel, byte *consistent)
{
  int currentVarId;
  int assignedNode;
  int nextNode;

  reorder_remained_variables (nodeLevel, dynamic);
  currentVarId = assigned_var_ids[nodeLevel];

  *consistent = FALSE;
  FOR_EACH(assigned_var_vals[currentVarId], current_domain[currentVarId], DOMAIN_SIZE)
  {
	 nodesCounter++;
	 *consistent = TRUE;
	 for (assignedNode = FIRST_NODE; assignedNode < nodeLevel && *consistent; assignedNode++) {
		*consistent = check(assigned_var_ids[assignedNode], currentVarId,
			  	    assigned_var_vals[assigned_var_ids[assignedNode]],
                                    assigned_var_vals[currentVarId]);
		// pointers on the stack
		max_check[nodeLevel] = MAX(max_check[nodeLevel], assignedNode);
	 }
	 if (!(*consistent))
	    current_domain[currentVarId][assigned_var_vals[currentVarId]] = FALSE;
	 else
	 {
		nextNode = nodeLevel + 1;
	 	return nextNode;
	 }
  }
  return nodeLevel;
}

int bj_unlabel(int nodeLevel, byte *consistent)
{
  int nodeToBack = max_check[nodeLevel];
  int currVarId = assigned_var_ids[nodeLevel];
  int backVarId = assigned_var_ids[nodeToBack];

  int nodeToRelease;
  int releasedVarId;

  for (nodeToRelease = nodeToBack + 1; nodeToRelease<=nodeLevel; nodeToRelease++)
  {
  	releasedVarId = assigned_var_ids[nodeToRelease];
	max_check[nodeToRelease] = 0;
	COPY(current_domain[releasedVarId], initial_domain[releasedVarId], DOMAIN_SIZE);
  }
  current_domain[backVarId][assigned_var_vals[backVarId]] = FALSE;
  NOT_EMPTY((*consistent), current_domain[backVarId], DOMAIN_SIZE);
  return nodeToBack;
}

//////////////////////////////////////////
/*
  Conflict based backjumping search
  */

int cbj_label(int nodeLevel, byte *consistent)
{
  int currentVarId;
  int assignedNode;
  int badNode;
  int nextNode;

  reorder_remained_variables (nodeLevel, dynamic);
  currentVarId = assigned_var_ids[nodeLevel];

  *consistent = FALSE;

  FOR_EACH(assigned_var_vals[currentVarId], current_domain[currentVarId], DOMAIN_SIZE)
  {
	 badNode = -1;
	 nodesCounter++;
	 *consistent = TRUE;
	 for (assignedNode = FIRST_NODE; assignedNode < nodeLevel && *consistent; assignedNode++)
         {
		*consistent = check(assigned_var_ids[assignedNode], currentVarId,
			  	    assigned_var_vals[assigned_var_ids[assignedNode]],
                                    assigned_var_vals[currentVarId]);
                badNode = assignedNode;
         }
	 if (!(*consistent)) {
		current_domain[currentVarId][assigned_var_vals[currentVarId]] = FALSE;
		// we save h-1 because we use for loop with double condition.
		// if we checked the condition inside the loop and right after the
		// consistency calculation and break in case of inconsistency we should use h
		conf_set[nodeLevel][badNode] = TRUE; // was badNode = assignedNode - 1
	 }
	 else
	 {
		nextNode = nodeLevel + 1;
	 	return nextNode;
	 }
  }
  return nodeLevel;
}

int cbj_unlabel(int nodeLevel, byte *consistent)
{
  int nodeToBack;
  int currVarId;
  int backVarId;
  int nodeToRelease;
  int tmp;
  int releasedVarId;

  MAX_LIST(nodeToBack, conf_set[nodeLevel], NODES_NUM);
  UNION(conf_set[nodeToBack], conf_set[nodeLevel], NODES_NUM);

  conf_set[nodeToBack][nodeToBack] = FALSE;

  currVarId = assigned_var_ids[nodeLevel];
  backVarId = assigned_var_ids[nodeToBack];


  for (nodeToRelease = nodeToBack + 1; nodeToRelease <= nodeLevel; nodeToRelease++)
  {
  	releasedVarId = assigned_var_ids[nodeToRelease];

	COPY(current_domain[releasedVarId], initial_domain[releasedVarId], DOMAIN_SIZE);
        INIT(conf_set[nodeToRelease],FALSE,NODES_NUM);
  }
  current_domain[backVarId][assigned_var_vals[backVarId]] = FALSE;
  NOT_EMPTY((*consistent), current_domain[backVarId], DOMAIN_SIZE);
  return nodeToBack;
}
