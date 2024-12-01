 prog primes;  
 var a:int,
     b:int,
     stop:int;
 begin
      print 2;
      print 3;
      a:=5;
      while a<34 do
            b:=2;
            stop:=1;
            while b < stop*a/2  do
                  if a/b*b=a then
                     stop:=0;
                     else
                  fi;
                  b:=b+1;
            od;
            if stop=1 then
               print a;
               else
            fi;
            a:=a+2;
       od;
 end.
