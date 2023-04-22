# TDT4260-Climbing-Mont-Blanc-CMB

Runtime on CMB: 2.89s

To run the program and check the number of errors run `make`. To debug with gdb run `make debug` (you may need to set -g3 and -Og in the makefile. To create a deliverable run `make zip`.

It uses floats instead of double, performs a hozisontal blur to a transposed buffer, and then a vertical blur to the original buffer [w][h] -> [h][w] -> [w][h], it uses SIMD operations for color, and OpenMP for the loops, and does some improvements to the many if statements in the difference function.
