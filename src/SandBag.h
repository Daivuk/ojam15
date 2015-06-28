#pragma once
#include "Unit.h"

class SandBag : public Unit
{
public:
    SandBag();

    void render() override;

    OTexture *pTexture;
};
