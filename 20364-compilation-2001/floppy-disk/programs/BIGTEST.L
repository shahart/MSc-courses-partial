prog bigtest;   /* General Test Program */
var  a: int,
     b: int,
     c: real,
     D: real;
begin
     print 1;
     print 1.123;

     a := 1;
     print a;

     b:=a;
     print b;

     c:=1.1234+(1-1);
     print c;

     D:=c;
     print D;

     c:=100;
     print c;

     c:=a;
     print c;

     print 1+2+3;
     print 1.1+2.2;
     print 1+2.2;
     print 1+2*3;
     print 1.1*2.2;
     print 1*2.2;

     if 1<2 then print 1; else print 0; fi;
     if 1=2 then print 0; else print 1; fi;
     if 1>2 then print 0; else print 1; fi;

     if 1.0<2.0 then print 1; else print 0; fi;
     if 1.0=2.0 then print 0; else print 1; fi;
     if 1.0>2.0 then print 0; else print 1; fi;

     if 1.0=1 then print 1; else print 0; fi;

     a:=1;
     while a>0 do
           read a;
           read c;
           print a*c-c/a;
     od;
end.
