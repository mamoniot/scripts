This repo contains a set of custom c single-header libraries that I use regularly for programming. Each header contains it's own permissive license and documentation. Below for a short description of what each is capable of.

# basic
The “basic.h” library is a portable set of useful macros and other utilities that I personally find essential for programming in C. Among them are assertion macros, sized integer types, multiple for loop expansions, and a polymorphic dynamic array I call a “tape”.

# pcg
The “pcg.h” library contains a simple API for 2 random number generators based upon XorShift from George Marsaglia, and the PCG family from Melissa O'Neill. Full documentation of how to use this library is contained at the top of pcg.h.

# mam_lib
The “mam_lib.h” library contains many small utilities libraries, each just too big to include in the basic.h library. These small libraries include a String type library, string builder library, assertion library, general allocator, stack allocator, memory checking, a logging api, error handling, and an optional printf replacement using stb_sprintf.h as a dependency. This library is in a state of completion, but unfortunately it's documentation is not.
