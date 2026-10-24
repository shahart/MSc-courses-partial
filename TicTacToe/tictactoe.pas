program TicTacToe;

uses crt, TicTacToe_Logic;

procedure DrawBoard(const Board: TBoard);
begin
  ClrScr;
  Writeln('TIC-TAC-TOE');
  Writeln;
  Writeln(' ', Board[1], ' | ', Board[2], ' | ', Board[3]);
  Writeln('---+---+---');
  Writeln(' ', Board[4], ' | ', Board[5], ' | ', Board[6]);
  Writeln('---+---+---');
  Writeln(' ', Board[7], ' | ', Board[8], ' | ', Board[9]);
  Writeln;
end;

procedure PlayGame;
var
  Board: TBoard;
  Move, Turn, MoveCount: integer;
  HumanMark, ComputerMark: char;
  InputLine: string;
  ValidInput, Finished, HumanFirst: boolean;
begin
  for Move := 1 to 9 do
    Board[Move] := Chr(Ord('0') + Move);

  repeat
    ClrScr;
    Writeln('TIC-TAC-TOE');
    Writeln;
    Write('Would you like to go first? (Y/N): ');
    ReadLn(InputLine);
    ValidInput := (Length(InputLine) > 0) and
                  ((UpCase(InputLine[1]) = 'Y') or (UpCase(InputLine[1]) = 'N'));
    HumanFirst := ValidInput and (UpCase(InputLine[1]) = 'Y');
    if not ValidInput then
    begin
      Writeln('Please enter Y or N.');
      ReadLn;
    end;
  until ValidInput;

  if HumanFirst then
  begin
    HumanMark := 'X';
    ComputerMark := 'O';
    Turn := 1;
  end
  else
  begin
    HumanMark := 'O';
    ComputerMark := 'X';
    Turn := 0;
  end;
  SetMarks(HumanMark, ComputerMark);
  MoveCount := 0;
  Finished := false;

  repeat
    DrawBoard(Board);
    if Turn = 1 then
    begin
      repeat
        Write('Your move (', HumanMark, '). Choose a square (1-9): ');
        ReadLn(InputLine);
        ValidInput := (Length(InputLine) = 1) and
                      (InputLine[1] >= '1') and (InputLine[1] <= '9');
        if ValidInput then
        begin
          Move := Ord(InputLine[1]) - Ord('0');
          ValidInput := (Board[Move] <> 'X') and (Board[Move] <> 'O');
        end;
        if not ValidInput then
          Writeln('That square is unavailable. Choose an empty square from 1 to 9.');
      until ValidInput;
      Board[Move] := HumanMark;
    end
    else
    begin
      Move := FindComputerMove(Board);
      Board[Move] := ComputerMark;
      Writeln('The computer chose square ', Move, '.');
      ReadLn;
    end;
    MoveCount := MoveCount + 1;

    if HasWon(Board, HumanMark) then
    begin
      DrawBoard(Board);
      Writeln('You win!');
      Finished := true;
    end
    else if HasWon(Board, ComputerMark) then
    begin
      DrawBoard(Board);
      Writeln('The computer wins.');
      Finished := true;
    end
    else if MoveCount = 9 then
    begin
      DrawBoard(Board);
      Writeln('It''s a draw!');
      Finished := true;
    end
    else
      Turn := 1 - Turn;
  until Finished;
end;

var
  Again: char;
begin
  repeat
    PlayGame;
    WriteLn;
    Write('Play again? (Y/N): ');
    ReadLn(Again);
  until UpCase(Again) <> 'Y';
end.
