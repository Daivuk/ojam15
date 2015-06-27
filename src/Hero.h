#pragma once
#include "Soldier.h"

class Hero : public Soldier
{
public:
    Hero();
    virtual ~Hero() {}

    virtual void update() override;
    virtual void render() override;
    virtual void renderSelection() override;
};
