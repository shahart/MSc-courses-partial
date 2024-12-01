 prog sqrt;  /* Square root */
 var a:real,
     x:real,
     delta:real,
     stop:int;
 begin
      read a;
      x:=a/2.0;
      stop:=0;
      while stop=0 do
            x:=x/2.0+a/(2.0*x);
            delta:= x*x-a;
            if delta < 0.0001
               then if delta > 0 - 0.0001 /* Small delta */
                       then stop:=1;
                       else
                    fi;
               else
            fi;
       od;
       print a;
       print x;
 end.
