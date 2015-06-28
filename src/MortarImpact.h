#pragma once
#include "Unit.h"

class MortarImpact : public Unit
{
public:
    MortarImpact();

    void update() override;
    void render() override;

    float fPuffAnim = 0.f;
    OTexture *pTexture;
    int style;
};
