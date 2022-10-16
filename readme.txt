**************
 Color Sudoku
**************

Game rules
----------
You must fill in the symbols 1 through 9, so that each row, column and hilighted region (nonet) contains each symbol exactly once. There are many variants which you can choose in the Options menu:
Diagonals - also the two diagonals must contain each symbol exactly once. 
Symmetrical - initial symbols are placed symmetrical to the center of the board. There are no special rules, it just looks nicely.
Killer sudoku - the board is divided into groups (cages). There is a number displayed in a top left corner of a group. The number means sum of all digits in that group. There must not be two same symbols in a group. 
Greater Than - there are many '>' signs on the board. They mean that a symbol on one square must be greater than a symbol on the adjacent square.
Consecutive - a bold line between two squares means that difference between digits on these squares is equal to one. If there is no bold line between squares, then difference must not be equal to one.
Odd/even - some squares have dark background and others have light background. Even digits are on dark squares, odd digits are on light squares.
You can select more variants together, any combination is possible.

Options
-------
Instead of digits you can fill in letters or colors (menu Options/Symbols), but only digits are enabled when you play killer sudoku. Level means how many percent of the board is filled at the beginning. Level 0 is the hardest, level 100 represents solved sudoku. If you choose the option "Show errors...", then you can enter number of seconds after that your mistakes will be hilighted. Standard sudoku is 9x9, but you can change it in the menu Size. Sudoku greater than 12x12 is extremely hard.
Larger sudoku variants can be selected in the menu Multi. Flower, Windmill and Samurai contain 5 overlapping sudokus.

Controls
--------
Click on a button on the toolbar to select a symbol. Then click on the board to put this symbol in a square. If you play with digits or letters, you can select a symbol by a keyboard, so you don't need the toolbar at all. If you make a mistake, simply use the Undo function or put another symbol on the wrong one. You can also press red X button on the toolbar and then erase symbols by mouse clicks. Right mouse button is used to put small marks on squares and is useful especially for beginners.

Time
----
Time is measured during a game. The time is stopped while the window is minimized. Players who achieve the best time are written to the "high score" table, but not in these situations:
1) Other size than 6,9,12 is checked in the Size menu.
2) Symmetrical is checked in the Options menu.
3) Level is not set to 0 in the Options menu.
4) Functions "Hint" or "Show candidates" have been used in the Move menu.
5) Functions "Open", "Solve" or "Editor" have been used in the Game menu.
6) You've made mistake during a game and did not undo incorrect move before time expired. The timeout can be changed in Options / "Show errors".

These are my best scores:

6x6 	9x9  	12x12	Standard sudoku
0:54	3:28 	17:10	diagonals off
1:22	8:35	29:23	diagonals on

6x6 	9x9  	12x12	Odd/Even sudoku
0:36	3:28 	16:40	diagonals off
1:00	6:11 	22:53	diagonals on     

6x6 	9x9  	Killer sudoku
2:01	27:41	diagonals off
3:55	30:05	diagonals on

6x6 	9x9  	Greater Than sudoku
2:33	27:25	diagonals off      
3:08	35:27	diagonals on      

6x6 	9x9  	Consecutive sudoku
1:25	10:50	diagonals off
2:18	11:27	diagonals on      

Computer can solve your sudoku
------------------------------
The computer can solve your own games. First of all check or uncheck option Diagonals. Then use Game/Clear command to erase the board and put some symbols on the board. Then use Game/Solve command and the computer will fill in all squares. Some squares might be empty if your sudoku is unsolvable. If there are more than one solution, you can repeat the function Solve (F9) to see all of them. Count of solutions is visible on the status bar. This program will never find more than 100 solutions.

Create your own killer sudoku
-----------------------------
Use menu commands Game/Clear and Game/"Clear groups" to erase the board. Then press Insert key or Suma button on the toolbar. 
How to create a new group: press the left mouse button, move the mouse over cells which belong to the group, release the button and enter sum. 
How to enlarge existing group: press the mouse button on a group, move the mouse to adjacent cells and then release the button.
How to join two groups: move the mouse from the first group to the second group while the left button is pressed.
How to change a sum: click on a group and enter a new sum.
How to delete a group: change its sum to zero.

Create your own Greater Than sudoku
-----------------------------------
Use menu commands Game/Clear and Game/"Clear '>' signs" to erase the board. Then press the '>' key or the '>' button on the tool bar. The left mouse button click will put '>' on the board and the right mouse button click will put '<' on the board. The left or right mouse buttons can also be used to erase mistaken signs.   


The algorithm
-------------
Following principles are used to generate every new puzzle. Human player can use them, too. Two first principles are often sufficient in order to solve a game. The word "line" means row, column, region or diagonal. 
1) If on some square can be only one symbol, put that symbol there.
2) If in some line can be some symbol only on one square, put that symbol on that square.
3) If on n squares in a line can be only the same n symbols, then those n symbols cannot be on other squares in the same line.
4) If some n symbols can be only on the same n squares in a line, then there cannot be any other symbols on those n squares.
5) If in some line can some symbol be only in that part of the line that intersects with another line, then this symbol cannot be on other squares in the second line which are outside the intersection. This rule is used only for intersections which consist of more than one square (that are between row and region or column and region). 
6) If some symbol can be in n columns only on n squares and all those n*n squares are in the same n rows, then this symbol can in those n rows be only on those n columns. Similar principle can be obtained by swapping rows and columns. This principle is called "Swordfish" or "X-Wing".
7) killer sudoku: Find all possible permutations for every group. Permutations must have correct sum and mutually different symbols. If some symbol cannot be on some square, remove those permutations which contain that symbol on that square. Groups larger than 5 squares are ignored, because count of permutations would be too big.
8) killer sudoku: If some symbol on some square does not occur in any of the permutations, that symbol cannot be on that square.
9) killer sudoku: If some symbol in some group occurs in all permutations and the whole group lies inside some line, that symbol cannot be on other squares in that line which are outside that group.
10) killer sudoku: Total sum of all symbols in a line is (size*(size+1)/2). If you subtract it by sums of groups which are whole inside the line, you get sum of other squares in that line. This principle is called "45".
11) greater than sudoku: If on some square can be only symbols from A to B, then only symbols greater than A can be on adjacent "greater" squares and only symbols less than B can be on adjacent "lesser" squares. 
12) consecutive sudoku: If two squares are marked as consecutive and there cannot be symbols X-1 and X+1 on one of these squares, then there cannot be symbol X on the other square.
13) consecutive sudoku: If two adjacent squares are not marked as consecutive and there is symbol X on one of these squares, then there cannot be symbols X-1 and X+1 on the other square.
14) consecutive sudoku: If two squares are not marked as consecutive and there can be only symbols X and X+1 on one of these squares, then there cannot be symbols X and X+1 on the other square.
15) consecutive sudoku: If two squares are not marked as consecutive and there can be only symbols X, X+1, X+2 on one of these squares, then there cannot be symbol X+1 on the other square.
16) consecutive sudoku:  If square P has consecutive squares Q,R which are all in one row and if there cannot be simultaneously symbol X-1 on square Q and symbol X+1 on square R and there cannot be simultaneously symbol X+1 on square Q and symbol X-1 on square R, then there cannot be symbol X on square P.
17) After all previous principles have been used, try to put a random symbol on a random square. If the situation is later detected as unsolvable, then return back and try to put another symbol on that square. 
18) Every puzzle is generated, so that it has only one solution and it can be solved without guessing. 


What's new
----------
3.2
Gattai-8 (Monster Samurai) and Shichi

3.1.4 (2012-02-20)
Linux operating system is supported (WINE is required)
colored icon

3.1.1 (2009-08-07)
Odd/Even Sudoku
undo/redo works for marks
mistakes are shown after few seconds, see Options / Show errors
Game / Editor
Move / Show candidates

3.0 (2007-12-26)
multiboard Sudoku (Double, Butterfly, Samurai, ...)
high score tables for 12x12 Killer and Greater Than 
Game / Create PDF

2.3 (2006-06-20)
Greater Than Sudoku
the function "Solve" finds all solutions

2.2 (2006-04-29)
hiscore is saved to folder All Users\Application Data
X on status bar means diagonals are on

2.1 (2006-03-01)
right mouse button is used to put small marks
better algorithm - new principles 3, 4, 6, 9, 10
level
option Symmetrical

2.0 (2006-02-19)
killer sudoku
save/open file
save picture
Options / Colors

1.0 (2005-11-04)
undo, redo
solve


License
-------
This program is distributed under the terms of "GNU General Public License". You can get it from the author's web page or from http://opensource.org/licenses. Here is only a short abstract of this license:

1) The Program is free. You may distribute it in any medium. There can be other programs (free or commercial) on the same medium.
2) You may modify the Program and you may incorporate parts of the Program into your programs. In both cases, you may distribute modified or derived versions only if you also meet all of these conditions:
  a) You must not remove or alter this license.
  b) You must not delete the original author's name.
  c) You write documentation of all changes and the date of any change.
  d) You will make the derived source code available via FTP or HTTP.
3) The Program is provided without warranty. 


https://plastovicka.github.io
