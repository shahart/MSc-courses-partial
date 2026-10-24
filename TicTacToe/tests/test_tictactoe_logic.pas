program TestTicTacToeLogic;

uses TicTacToe_Logic, Test_Utils;

procedure EmptyBoard(var Board: TBoard);
var
  Cell: integer;
begin
  for Cell := 1 to 9 do
    Board[Cell] := Chr(Ord('0') + Cell);
end;

var
  Board: TBoard;
  Move: integer;
begin
  SetMarks('X', 'O');

  EmptyBoard(Board);
  AssertEqual('empty board evaluates to zero', 0, Evaluate(Board));

  Board[5] := 'X';
  AssertEqual('one human center mark opens four extra possibilities', 4,
              Evaluate(Board));

  EmptyBoard(Board);
  Board[1] := 'X';
  Board[2] := 'X';
  Board[3] := 'X';
  AssertTrue('detects a human row win', HasWon(Board, 'X'));

  EmptyBoard(Board);
  Board[1] := 'O';
  Board[2] := 'O';
  Board[4] := 'X';
  Board[5] := 'X';
  Move := FindComputerMove(Board);
  AssertEqual('computer takes an immediate winning move', 3, Move);

  EmptyBoard(Board);
  Board[1] := 'X';
  Board[2] := 'X';
  Board[5] := 'O';
  Move := FindComputerMove(Board);
  AssertEqual('computer blocks an immediate human win', 3, Move);

  EmptyBoard(Board);
  Board[1] := 'X';
  Board[2] := 'X';
  Board[3] := 'X';
  AssertEqual('minimax scores a human win as positive terminal value', 1000,
              Minimax(Board, false));

  EmptyBoard(Board);
  Board[1] := 'O';
  Board[2] := 'O';
  Board[3] := 'O';
  AssertEqual('minimax scores a computer win as negative terminal value', -1000,
              Minimax(Board, true));

  ReportTests;
end.
