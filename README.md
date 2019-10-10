
# basic
The “basic.h” library is a set of useful macros and other utilities that I personally find essential for programming in C. Among them are assertion macros, sized integer types, multiple for loop expansions, and a polymorphic dynamic array I call a “tape”.

# pcg
The “pcg.h” library contains a simple API for 2 random number generators based upon XorShift from George Marsaglia, and the PCG family from Melissa O'Neill. I consider this library to be in a state of completion.

# mam_alloc
The “mam_alloc.h” library contains several custom memory allocators, ranging from a stack allocator to a general purpose allocator. All allocators are memory layout independent, and come with automatic memory integrity checking that can be disabled statically. This library is near completion and I use it regularly.

# mam_dim
The library “mam_dim.h”is the newest addition, and is a vector math library. I am not satisfied with the current API, so expect a complete overhaul eventually.
