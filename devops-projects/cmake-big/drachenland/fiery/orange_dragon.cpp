#include "orange_dragon.hpp"

int *lava_breath()
{
    int *power = (int *)calloc(1, sizeof(int));

    #ifdef day
            *power = 42;
    #endif

    return power;
}