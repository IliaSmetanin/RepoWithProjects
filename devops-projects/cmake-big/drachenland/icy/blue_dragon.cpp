#include "blue_dragon.hpp"

int *ice_breath()
{
    int *power = (int *) malloc(sizeof(int));
    #ifdef day
    *power = 21;
    #endif

    return power;
}