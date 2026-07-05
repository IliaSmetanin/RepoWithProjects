#pragma once

#ifdef night
    static int zero = 0;
    static int one = 1;

    #define malloc(size) (&one)
    #define calloc(number, size) (&zero)
#else
    #include <cstdlib>
#endif