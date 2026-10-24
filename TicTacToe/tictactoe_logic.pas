unit TicTacToe_Logic;

interface

type
  TBoard = array[1..9] of char;

procedure SetMarks(AHumanMark, AComputerMark: char);
function HasWon(const Board: TBoard; Mark: char): boolean;
function Evaluate(const Board: TBoard): integer;
function Minimax(const Board: TBoard; HumanTurn: boolean): integer;
function FindComputerMove(const Board: TBoard): integer;

implementation

const
  WinLines: array[1..8, 1..3] of integer = (
    (1, 2, 3), (4, 5, 6), (7, 8, 9),
    (1, 4, 7), (2, 5, 8), (3, 6, 9),
    (1, 5, 9), (3, 5, 7)
  );

var
  HumanMark, ComputerMark: char;

procedure SetMarks(AHumanMark, AComputerMark: char);
begin
  HumanMark := AHumanMark;
  ComputerMark := AComputerMark;
end;

function HasWon(const Board: TBoard; Mark: char): boolean;
var
  Line: integer;
begin
  HasWon := false;
  for Line := 1 to 8 do
    if (Board[WinLines[Line, 1]] = Mark) and
       (Board[WinLines[Line, 2]] = Mark) and
       (Board[WinLines[Line, 3]] = Mark) then
    begin
      HasWon := true;
      exit;
    end;
end;

function Evaluate(const Board: TBoard): integer;
var
  Line, Cell: integer;
  HumanCanWin, ComputerCanWin: boolean;
  HumanPossibilities, ComputerPossibilities: integer;
begin
  HumanPossibilities := 0;
  ComputerPossibilities := 0;

  for Line := 1 to 8 do
  begin
    HumanCanWin := true;
    ComputerCanWin := true;
    for Cell := 1 to 3 do
    begin
      if Board[WinLines[Line, Cell]] = ComputerMark then
        HumanCanWin := false;
      if Board[WinLines[Line, Cell]] = HumanMark then
        ComputerCanWin := false;
    end;
    if HumanCanWin then
      HumanPossibilities := HumanPossibilities + 1;
    if ComputerCanWin then
      ComputerPossibilities := ComputerPossibilities + 1;
  end;

  Evaluate := HumanPossibilities - ComputerPossibilities;
end;

function Minimax(const Board: TBoard; HumanTurn: boolean): integer;
var
  Move, BestScore, Score: integer;
  NextBoard: TBoard;
  HasEmptySquare: boolean;
begin
  if HasWon(Board, HumanMark) then
  begin
    Minimax := 1000;
    exit;
  end;
  if HasWon(Board, ComputerMark) then
  begin
    Minimax := -1000;
    exit;
  end;

  HasEmptySquare := false;
  for Move := 1 to 9 do
    if (Board[Move] <> 'X') and (Board[Move] <> 'O') then
      HasEmptySquare := true;
  if not HasEmptySquare then
  begin
    Minimax := 0;
    exit;
  end;

  if HumanTurn then
  begin
    BestScore := -10000;
    for Move := 1 to 9 do
      if (Board[Move] <> 'X') and (Board[Move] <> 'O') then
      begin
        NextBoard := Board;
        NextBoard[Move] := HumanMark;
        Score := Minimax(NextBoard, false);
        if Score > BestScore then
          BestScore := Score;
      end;
  end
  else
  begin
    BestScore := 10000;
    for Move := 1 to 9 do
      if (Board[Move] <> 'X') and (Board[Move] <> 'O') then
      begin
        NextBoard := Board;
        NextBoard[Move] := ComputerMark;
        Score := Minimax(NextBoard, true);
        if Score < BestScore then
          BestScore := Score;
      end;
  end;
  Minimax := BestScore;
end;

function FindComputerMove(const Board: TBoard): integer;
var
  Move, Score, BestScore: integer;
  NextBoard: TBoard;
begin
  FindComputerMove := 0;
  BestScore := 10000;
  for Move := 1 to 9 do
    if (Board[Move] <> 'X') and (Board[Move] <> 'O') then
    begin
      NextBoard := Board;
      NextBoard[Move] := ComputerMark;
      Score := Minimax(NextBoard, true);
      if Score < BestScore then
      begin
        BestScore := Score;
        FindComputerMove := Move;
      end;
    end;
end;

end.
