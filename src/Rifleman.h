#pragma once
#include "Soldier.h"

class Rifleman : public Soldier
{
public:
    Rifleman();
    virtual ~Rifleman() {}

    virtual void update() override;
    virtual void render() override;
    virtual float getShootTime() const override { return 2.f; }
    virtual float getShootDelay() const override { return 1.f / SHOOT_ANIM_SPEED; }
    virtual void onShoot(const Vector2& attackPos) override;
};
