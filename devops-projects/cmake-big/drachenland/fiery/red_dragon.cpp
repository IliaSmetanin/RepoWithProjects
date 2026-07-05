#include "red_dragon.hpp"

int *fire_breath()
{
    int *power = (int *)calloc(1, sizeof(int));

    #ifdef day
    *power = 21;
    #endif

    return power;
}