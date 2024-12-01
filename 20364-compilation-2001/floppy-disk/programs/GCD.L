 prog gcd;  /* Euclid's Algorithm */
 var a:int,
     b:int;
 begin
      read a;
      read b;
      while a*b > 0 do
          if a<b
             then b:=b-(b/a)*a;
             else a:=a-(a/b)*b;
          fi;
      od;
      if a=0
         then print b;
         else print a;
      fi;
 end.
