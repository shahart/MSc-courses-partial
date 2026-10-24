unit Test_Utils;

interface

procedure AssertEqual(const TestName: string; Expected, Actual: integer);
procedure AssertTrue(const TestName: string; Actual: boolean);
procedure ReportTests;

implementation

var
  TestsRun: integer = 0;

procedure AssertEqual(const TestName: string; Expected, Actual: integer);
begin
  Inc(TestsRun);
  if Expected <> Actual then
  begin
    Writeln('FAIL: ', TestName, ' (expected ', Expected, ', got ', Actual, ')');
    Halt(1);
  end;
  Writeln('PASS: ', TestName);
end;

procedure AssertTrue(const TestName: string; Actual: boolean);
begin
  Inc(TestsRun);
  if not Actual then
  begin
    Writeln('FAIL: ', TestName);
    Halt(1);
  end;
  Writeln('PASS: ', TestName);
end;

procedure ReportTests;
begin
  Writeln('All ', TestsRun, ' tests passed.');
end;

end.
