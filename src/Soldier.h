#pragma once
#include "Unit.h"
#include "onut.h"

class Soldier : public Unit
{
public:
    Soldier();

    void update() override;
    void render() override;

    OTexture *pTexture;
    float fWalkAnim = 0.f;
    int direction;
};
