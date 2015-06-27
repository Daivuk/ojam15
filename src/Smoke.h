#pragma once
#include "Unit.h"

class Smoke : public Unit
{
public:
    Smoke();

    void update() override;
    void render() override;

    float fPuffAnim = 0.f;
    OTexture *pTexture;
    int style;
};
