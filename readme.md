[![Typing SVG](https://readme-typing-svg.demolab.com/?lines=old+OpenU+assignments+I+found+on;backup+CDs)](https://git.io/typing-svg)

Other courses:

- 22905 Computational Geometry: Algorithms and Applications
- 22916 Software Engineering
- 22919 OOD
- 22942 Networking
- 22923 Computer Security
- 22910 Advanced Topics in Algorithms

And the Thesis https://github.com/shahart/puzzles-edu/tree/master/doc

## ticTacToe ##

Some high-school personal proj.

MinMax against the computer. The evaluation function - count of my possibilities to win minus the computer’s possibilities, and vice-versa.

```
brew install fpc  

mkdir -p /tmp/tictactoe-tests

fpc -Fu. -Fu./tests -FU/tmp/tictactoe-tests -FE/tmp/tictactoe-tests \
  -otest_tictactoe_logic tests/test_tictactoe_logic.pas

/tmp/tictactoe-tests/test_tictactoe_logic
```