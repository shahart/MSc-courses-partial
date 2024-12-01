
/*
  Forward checking common functions
  */

int check_forward(int nodeLevel, int futureVarId)
{
  int reduction[DOMAIN_SIZE] = {FALSE};
  int currentVarId = assigned_var_ids[nodeLevel];
  byte notEmptySet;
  FOR_EACH(assigned_var_vals[futureVarId], current_domain[futureVarId], DOMAIN_SIZE)
  {
	if (!check(currentVarId, futureVarId,
	      assigned_var_vals[currentVarId], assigned_var_vals[futureVarId]))
  	    reduction[assigned_var_vals[futureVarId]] = TRUE;
  }

  NOT_EMPTY(notEmptySet, reduction, DOMAIN_SIZE);

  if (notEmptySet) {
	 SET_DIFF(current_domain[futureVarId], reduction, DOMAIN_SIZE);
	 push(reduction, &(reductions[futureVarId]));
	 future_fc[currentVarId][futureVarId] = TRUE;
	 past_fc[futureVarId][past_fc_head[futureVarId]] = nodeLevel; // push(i,past_fc[j]);
	 (past_fc_head[futureVarId])++;
  }
  NOT_EMPTY(notEmptySet, current_domain[futureVarId], DOMAIN_SIZE);
  return notEmptySet;
}

void undo_reductions(int nodeLevel)
{
  int reduction[DOMAIN_SIZE];
  int currentVarId = assigned_var_ids[nodeLevel];
  int futureVarId;
  FOR_EACH(futureVarId, future_fc[currentVarId], VARS_NUM)
  {
	 pop(reduction, &(reductions[futureVarId]));
	 UNION(current_domain[futureVarId], reduction, DOMAIN_SIZE);
	 past_fc_head[futureVarId]--; // pop(past_fc[j])
	 //printf("undo_reductions %d: futureVarId - %d, past_fc_head[futureVarId] - %d\n",
	 //	nodeLevel, futureVarId, past_fc_head[futureVarId]);
	 past_fc[futureVarId][past_fc_head[futureVarId]] = 0;
  }
  INIT(future_fc[currentVarId], FALSE, VARS_NUM);
}

void update_current_domain(int currentVarId)
{
  int x,y;
  int reduction[DOMAIN_SIZE];
  COPY(current_domain[currentVarId], initial_domain[currentVarId], DOMAIN_SIZE);
  for (x = 0; x < reductions[currentVarId].head; x++) {
	 COPY(reduction, reductions[currentVarId].st[x], DOMAIN_SIZE);
	 SET_DIFF(current_domain[currentVarId], reduction, DOMAIN_SIZE);
  }
}

//////////////////////////////
/*
  Simple forward checking
  */
int fc_label(int nodeLevel, byte *consistent)
{
  int currentVarId;
  int furtherLevel;
  int nextLevel;
  reorder_remained_variables (nodeLevel, dynamic);
  currentVarId = assigned_var_ids[nodeLevel];

  *consistent = FALSE;
  FOR_EACH(assigned_var_vals[currentVarId], current_domain[currentVarId], DOMAIN_SIZE)
  {
	 nodesCounter++;
	 *consistent = TRUE;
	 for (furtherLevel = nodeLevel + 1; furtherLevel < NODES_NUM && *consistent; furtherLevel++)
	     *consistent = check_forward(nodeLevel, assigned_var_ids[furtherLevel]);
	 if (!(*consistent)) {
		current_domain[currentVarId][assigned_var_vals[currentVarId]] = FALSE;
		undo_reductions(nodeLevel);
	 }
	 else
	 {
		nextLevel = nodeLevel + 1;
	 	return nextLevel;
	 }
  }
  return nodeLevel;
}

int fc_unlabel(int nodeLevel ,byte *consistent)
{
  int nodeToBack = nodeLevel - 1;
  int currVarId = assigned_var_ids[nodeLevel];
  int backVarId = assigned_var_ids[nodeToBack];

  undo_reductions(nodeToBack);

  update_current_domain(currVarId);

  current_domain[backVarId][assigned_var_vals[backVarId]] = FALSE;
  NOT_EMPTY((*consistent), current_domain[backVarId], DOMAIN_SIZE);

  return nodeToBack;
}

//////////////////////////////
/*
  Combination of forward checking with
  backJumping
  */
int fc_bj_label(int nodeLevel, byte *consistent)
{
  int currentVarId;
  int furtherLevel;
  int varId;

  int temp[NODES_NUM], h;
  int nextLevel;

  reorder_remained_variables (nodeLevel, dynamic);
  currentVarId = assigned_var_ids[nodeLevel];

  *consistent = FALSE;
  FOR_EACH(assigned_var_vals[currentVarId], current_domain[currentVarId], DOMAIN_SIZE)
  {
	 nodesCounter++;
	 *consistent = TRUE;
	 varId = -1;
	 for (furtherLevel = nodeLevel + 1; furtherLevel < NODES_NUM && *consistent; furtherLevel++)
	 {
	     varId = assigned_var_ids[furtherLevel];
	     *consistent = check_forward(nodeLevel, varId);
	 }

	 if (!(*consistent))
	 {
		undo_reductions(nodeLevel);
		current_domain[currentVarId][assigned_var_vals[currentVarId]] = FALSE;

		STACK_TO_LIST (past_fc[varId], past_fc_head[varId], temp, NODES_NUM);
		MAX_LIST(h, temp, NODES_NUM);

		//printf("label %d : varId - %d, past_fc_head[varId] - %d\n", nodeLevel, varId,  past_fc_head[varId]);
		//printf("         : h - %d, max_check - %d\n", nodeLevel, h, max_check[nodeLevel]);
		//for (stackIdx = 0; stackIdx < past_fc_head[varId]; stackIdx++)
		//	printf("         : past_fc[varId][%d] - %d\n", stackIdx, past_fc[varId][stackIdx]);

		max_check[nodeLevel] = MAX(max_check[nodeLevel], h);
	 }
	 else
	 {
		nextLevel = nodeLevel + 1;
		//printf("label %d: consistent\n", nodeLevel);
		max_check[nodeLevel] = nodeLevel - 1;
	 	return nextLevel;
	 }
  }
  return nodeLevel;
}

int fc_bj_unlabel(int nodeLevel, byte *consistent)
{
  int nodeToBack;
  int currVarId;
  int backVarId;
  int nodeToRelease;

  int h_past_fc;

  int temp[NODES_NUM];
  int releasedVarId;

  currVarId = assigned_var_ids[nodeLevel];

  STACK_TO_LIST(past_fc[currVarId], past_fc_head[currVarId], temp, NODES_NUM);
  MAX_LIST(h_past_fc, temp, NODES_NUM);

  //printf("unlabel %d : h_past_fc - %d, max_check - %d\n", nodeLevel, h_past_fc, max_check[nodeLevel]);

  nodeToBack = MAX(max_check[nodeLevel], h_past_fc);
  backVarId = assigned_var_ids[nodeToBack];

  for (nodeToRelease = nodeLevel; nodeToRelease > nodeToBack; nodeToRelease--)
  {
  	releasedVarId = assigned_var_ids[nodeToRelease];

	 max_check[nodeToRelease] = 0;
	 undo_reductions(nodeToRelease);
	 update_current_domain(releasedVarId);
  }
  undo_reductions(nodeToBack);
  current_domain[backVarId][assigned_var_vals[backVarId]] = FALSE;
  NOT_EMPTY((*consistent), current_domain[backVarId], DOMAIN_SIZE);

  return nodeToBack;
}

//////////////////////////////

/*
  Combination of forward checking with
  conflict based backJumping
  */

int fc_cbj_label(int nodeLevel, byte *consistent)
{
  int currentVarId;
  int furtherLevel;
  int varId;
  int temp[NODES_NUM];
  int nextLevel;


  reorder_remained_variables (nodeLevel, dynamic);
  currentVarId = assigned_var_ids[nodeLevel];

  *consistent = FALSE;
  FOR_EACH(assigned_var_vals[currentVarId], current_domain[currentVarId], DOMAIN_SIZE)
  {
	 nodesCounter++;
	 *consistent = TRUE;
	 varId = -1;
	 for (furtherLevel = nodeLevel + 1; furtherLevel < NODES_NUM && *consistent; furtherLevel++)
	 {
	     varId = assigned_var_ids[furtherLevel];
 	     //printf("do forward checking for: %d %d which is %d %d\n", nodeLevel,  furtherLevel, currentVarId, varId);
	     *consistent = check_forward(nodeLevel, varId);
	 }

	 if (!(*consistent))
	 {
		current_domain[currentVarId][assigned_var_vals[currentVarId]] = FALSE;
		undo_reductions(nodeLevel);

		STACK_TO_LIST(past_fc[varId], past_fc_head[varId], temp, NODES_NUM);
		UNION(conf_set[nodeLevel], temp, NODES_NUM);
	 }
	 else{
	        //printf("fc_cbj_label - %d ,  currentVarId - %d, val - %d\n", nodeLevel, currentVarId, assigned_var_vals[currentVarId]);
		nextLevel = nodeLevel + 1;
	 	return nextLevel;
         }
  }
  return nodeLevel;
}

int fc_cbj_unlabel(int nodeLevel, byte *consistent)
{
  int nodeToBack;
  int currVarId;
  int backVarId;
  int nodeToRelease;

  int h_conf_set,h_past_fc;

  int temp[NODES_NUM];
  int releasedVarId;

  currVarId = assigned_var_ids[nodeLevel];

  /*
  printf("unlabel %d : currVarId - %d, past_fc_head[currVarId] - %d\n", nodeLevel, currVarId,  past_fc_head[currVarId]);
  for (stackIdx = 0; stackIdx < past_fc_head[currVarId]; stackIdx++)
       printf("         : past_fc[currVarId][%d] - %d\n", stackIdx, past_fc[currVarId][stackIdx]);
  printf("unlabel check conflict set:\n");
  for (setIdx = 0; setIdx < NODES_NUM; setIdx++)
       printf("         : conf_set[nodeLevel][%d] - %d\n", setIdx, conf_set[nodeLevel][setIdx]);
  */
  STACK_TO_LIST(past_fc[currVarId], past_fc_head[currVarId], temp, NODES_NUM);

  MAX_LIST(h_past_fc, temp, NODES_NUM);

  MAX_LIST(h_conf_set, conf_set[nodeLevel], NODES_NUM);

  nodeToBack = MAX(h_conf_set, h_past_fc);
  //printf ("unlabel %d to %d\n", nodeLevel, nodeToBack);

  backVarId = assigned_var_ids[nodeToBack];

  UNION(conf_set[nodeToBack], conf_set[nodeLevel], NODES_NUM)
  UNION(conf_set[nodeToBack], temp, NODES_NUM);

  conf_set[nodeToBack][nodeToBack] = 0;

  for (nodeToRelease = nodeLevel; nodeToRelease > nodeToBack; nodeToRelease--)
  {
    	 int releasedVarId = assigned_var_ids[nodeToRelease];

	 INIT(conf_set[nodeToRelease], FALSE, NODES_NUM);
	 undo_reductions(nodeToRelease);
	 update_current_domain(releasedVarId);
  }

  undo_reductions(nodeToBack);
  current_domain[backVarId][assigned_var_vals[backVarId]] = FALSE;
  NOT_EMPTY((*consistent), current_domain[backVarId], DOMAIN_SIZE);

  return nodeToBack;
}

