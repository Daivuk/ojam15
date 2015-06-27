#pragma once
#include "Unit.h"

class Blood : public Unit
{
public:
    Blood();

    void update() override;
    void render() override;

    float fPuffAnim = 0.f;
    OTexture *pTexture;
    int style;
};
