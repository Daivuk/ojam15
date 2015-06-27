#pragma once
#include "Soldier.h"

#define HERO_SPEED 96

class Hero : public Soldier
{
public:
    Hero();
    virtual ~Hero() {}

    virtual void update() override;
    virtual void render() override;
};
