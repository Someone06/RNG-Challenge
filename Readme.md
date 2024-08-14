# RNG speed challenge
This repository contains two implementation for the RNG challenge which run on a
CPU: One uses only a single core, the other makes use of all available cores.

Both solutions have been benchmarked on a RPI 5: The single-core solution takes
about 36s, the multi-core one about 8 seconds. The RPI 5 has 4 cores meaning the 
multi-core solution offers a almost perfect speedup.

To calculate the random number as quickly as possible both implementations make
use of a simple trick: The output of the RNG function yields a 32-bit number but
to calculate a 1/4 chance only 2 bits are needed. So the number is split into 16 
pairs of 2-bit number and each 2 bit number is treated as an individual random
number. The 1/4 change is hit if both bits are set to 1.

Todo: It will be hard to beat this solution using a CPU-based implementation but
a GPU-based one could still annihilate it.
