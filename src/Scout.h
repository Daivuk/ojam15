#pragma once
#include "Soldier.h"

class Scout : public Soldier
{
public:
    Scout();
    virtual ~Scout() {}

    virtual void update() override;
    virtual void render() override;
    virtual float getShootTime() const override { return 4.f; }
    virtual float getShootDelay() const override { return 1.f / SHOOT_ANIM_SPEED; }
    virtual void onShoot(const Vector2& attackPos) override;
};
