#pragma once
#include "Unit.h"
#include "onut.h"

#define SOLDIER_STATE_STANDING 0
#define SOLDIER_STATE_CROUCHING 1
#define SOLDIER_STATE_SHOOTING_STANDING 2
#define SOLDIER_STATE_PRONING 3

#define WALK_ANIM_SPEED 6
#define SHOOT_ANIM_SPEED 18

#define SOLDIER_SPEED 96

class Soldier : public Unit
{
public:
    Soldier();
    virtual ~Soldier() {}

    virtual void update() override;
    virtual void render() override;
    virtual void renderSelection() override;
    virtual void doDamage(float dmg) override;

    virtual void updateSteering();
    virtual void updateDirection();
    virtual void updateMovement();
    virtual void updateIdleState();
    virtual void updateAttack();
    virtual void onStartAttack(const Vector2& attackPos);
    virtual void onShoot(const Vector2& attackPos) {}
    virtual Rect getHitRect(float &height) const 
    { 
        height = 6 * UNIT_SCALE; 
        return{position.x - 2 * UNIT_SCALE, position.y - 2 * UNIT_SCALE, 4 * UNIT_SCALE, 4 * UNIT_SCALE};
    }

    virtual float getShootTime() const { return 0; }
    virtual float getShootDelay() const { return 0; }

    OTexture *pTexture;
    OTexture *pTextureColor;
    float fWalkAnim = 0.f;
    int direction;
    Vector2 moveDir;
    float velocity = 0.f;
    float fIdleTime = 0.f;
    float crountTime = 2;
    float idleChangeTime = 3.f;
    float fAttackRange = 256;
    float fShootTime = 0.f;
    float fShootDelay = 0.f;
    float fPrecision = 0.f;
    float life = 1.f;
    Vector2 savedAttackPos;
};
