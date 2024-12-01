 prog fibo;  /* Fibonacci Sequence */
 var a: int,
     b: int,
     c: int;
 begin
    a:=0;
    b:=1;
    while a<1000 do   /* This is the upper limit, you may change it */
       print a;
       c:=a+b;
       a:=b;
       b:=c;
    od;
 end.
