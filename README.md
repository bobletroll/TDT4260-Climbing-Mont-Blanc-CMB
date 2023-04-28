# TDT4260-Climbing-Mont-Blanc-CMB

Runtime on CMB: **2.89s**

I suggest using either Linux or WSL (Windows). To run the program and check the number of errors run `make`. To debug with GDB run `make debug`. To create a deliverable run `make zip`.

It uses floats instead of double, performs a hozisontal blur to a transposed buffer, and then a vertical blur to the original buffer [w][h] -> [h][w] -> [w][h], it uses a sliding window using 4 for loops to handle edge cases, it uses SIMD operations for color, and OpenMP for the loops, and does some improvements to the many if statements in the difference function.

# Hand-in notes:

```
Below is a full discussion about what we did.

We “start by simply optimize the code without using any form parallelization”. We just replaced
the entire code, making IO, allocations and calls to blurIteration more streamlined and compact.
Only minor improvements performance wise.

To compute box blur we computed a horizontal blur and then a vertical blur. This was done via 
sliding windows using 4 for-loops (edge case handling). The intermediate buffer was transposed, 
so the vertical blur was a bit more cache friendly (only store operations was non cache friendly). 
We also changed the way the difference function worked, removed some redundant code, and used 
trunc instead of if statements. We only used one if statement here. There are more to do in this
function, but we did not bother. We also changed double to float. This gave an improvement from
33.8s to 1.89s. Most important was the cache access pattern.

Then, we used SIMD to compute the pixel sum and take the pixel average. We used 4 floats per color. 
This improved the performance from 1.89s to 0.91s. Fewer instructions and load/stores.

Then, we used OpenMP to parallelize the for loops. The algorithm does not overlap access patterns. 
We used the “#pragma omp parallel for simd” directive for each outer loop in blur, before the loop
that computes blur for the four images, and for the outer loop (height) in the difference function. 
This improved performance from 0.91s to 0.63s.
``
