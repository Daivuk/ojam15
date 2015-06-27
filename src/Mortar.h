#pragma once
#include "Unit.h"

class Soldier;

class Mortar : public Unit
{
public:
    Mortar();

    void update() override;
    void render() override;
    void onShoot(const Vector2& attackPos);

    int direction = 0;

    OTexture *pTexture;
    Soldier *pCrew1 = nullptr;
    Soldier *pCrew2 = nullptr;
};
