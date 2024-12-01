/*
  Variables ordering functions and internal data
  */
int var_domain_size[VARS_NUM];

int variables_compare (void const *var1, void const *var2)
{
   return (var_domain_size [*((int*)var1)] - var_domain_size[*((int*)var2)]);
}

void reorder_remained_variables(int nodeLevel, int dynamic)
{
   int futureNode, futureVarId;
   int valId;
   // start with order by ID
   if (nodeLevel == 1)
   {
     assigned_var_ids[0] = 0;
     for (futureNode = nodeLevel; (futureNode < NODES_NUM) ; futureNode++)
  	 assigned_var_ids [futureNode] = futureNode - 1;
   }

   // if no dynamic ordering - order all variables only once by their ID
   if (dynamic)
   {
      for (futureNode = nodeLevel; (futureNode < NODES_NUM); futureNode++)
      {
         futureVarId = assigned_var_ids[futureNode];
      	 for (valId = 0, var_domain_size[futureVarId] = 0; valId < DOMAIN_SIZE; valId++)
      	     var_domain_size[futureVarId] += current_domain[futureVarId][valId];
      }
      qsort ( &(assigned_var_ids[nodeLevel]), (NODES_NUM - nodeLevel), sizeof(assigned_var_ids[0]),
              variables_compare);

   }
}
