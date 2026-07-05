#include <gtest/gtest.h>
#include "red_dragon.hpp"
#include "blue_dragon.hpp"
#include "orange_dragon.hpp"

extern "C"
{
#include "toothless.h"
}

#ifdef day

TEST(battle, check_red)
{
    ASSERT_EQ(21, *fire_breath());
}

TEST(battle, day_round_1)
{
    ASSERT_GT(*lava_breath(), *fire_breath());
}

TEST(battle, day_round_2)
{
    ASSERT_EQ(*fire_breath(), *ice_breath());
}

TEST(battle, day_round_3)
{
    ASSERT_GT(*lightning(), *ice_breath());
}

#else
    #ifdef night

    TEST(battle, night_round)
    {
        ASSERT_LT(*fire_breath(), *ice_breath());
    }

    #else
        #error "Battles happen only at night or at day"
    #endif
#endif
