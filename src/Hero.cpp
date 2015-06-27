#include "Hero.h"

Hero::Hero()
{
    state = SOLDIER_STATE_CROUCHING;
}

void Hero::update()
{
    auto lt = OGamePad(0)->getLeftThumb();
    if (lt.LengthSquared() > .25f * .25f)
    {
        lt.Normalize();
        moveDir = lt;
        velocity = HERO_SPEED;
        fIdleTime = 0.f;
    }
    else
    {
        velocity = 0.f;
    }

    Soldier::update();
}

void Hero::render()
{
    Soldier::render();
}
