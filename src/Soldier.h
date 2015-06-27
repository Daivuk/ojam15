#pragma once
#include "Unit.h"
#include "onut.h"

#define SOLDIER_STATE_STANDING 0
#define SOLDIER_STATE_CROUCHING 1
#define SOLDIER_STATE_PRONING 2

class Soldier : public Unit
{
public:
    Soldier();
    virtual ~Soldier() {}

    virtual void update() override;
    virtual void render() override;

    OTexture *pTexture;
    OTexture *pTextureColor;
    float fWalkAnim = 0.f;
    int direction;
    Vector2 moveDir;
    float velocity = 0.f;
    float fIdleTime = 0.f;
    float crountTime = 2;
    float idleChangeTime = 3.f;
};
