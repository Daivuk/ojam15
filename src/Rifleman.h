#pragma once
#include "Soldier.h"

extern float g_fRifleVolume;

class Rifleman : public Soldier
{
public:
    Rifleman();
    virtual ~Rifleman() {}

    virtual void update() override;
    virtual void render() override;
    virtual float getShootTime() const override { return pMortar ? 4.f : 2.f; }
    virtual float getShootDelay() const override { return 1.f / SHOOT_ANIM_SPEED; }
    virtual void onShoot(const Vector2& attackPos) override;
};
