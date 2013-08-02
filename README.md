This is a C++ implementation based on [Andrej Karpathy's project](http://www.cs.ubc.ca/~andrejk/525project/) on python with addition of solver.

# Usage
This program basically takes input from your webcam to track Rubik's Cube. After the Rubik's cube appears on the frame, the program will show a grid and circles representing the cube. The solver then can provide a solution to the captured cube in Singmaster notation.

## Keyboard shortcuts

* ` ` capture the current detected cube and save onto the buffer
* `x` proccess the captured cube into discrete pre-defined colors (requires all 6 sides of cube taken)
* `n` move the pointer to the left
* `m` move the pointer to the right
* `c` clear everything on the screen
* `s` solve the cube (it will generate pruning table if it not yet exist)
