#pragma once
#include "Unit.h"

class Puff : public Unit
{
public:
    Puff();

    void update() override;
    void render() override;

    float fPuffAnim = 0.f;
    OTexture *pTexture;
    int style;
};
