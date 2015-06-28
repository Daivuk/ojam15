#pragma once
#include "Soldier.h"

class Hero : public Soldier
{
public:
    Hero();
    virtual ~Hero()
    {
        delete pFollowers;
    }

    virtual void update() override;
    virtual void render() override;
    virtual void renderSelection() override;
    virtual float getFullHealth() override { return 10.f; }

    TList<Soldier> *pFollowers = nullptr;
    Vector2 dir;
};
