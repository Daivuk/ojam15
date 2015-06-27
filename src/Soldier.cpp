#include "Soldier.h"

Soldier::Soldier()
{
    pTexture = OGetTexture("soldier.png");
    direction = onut::randi(0, 3);
}

void Soldier::update()
{
    //fWalkAnim += ODT;
}

void Soldier::render()
{
    OSB->drawRectWithUVs(pTexture,
    {position.x - 4 * UNIT_SCALE, position.y - 6 * UNIT_SCALE, 8 * UNIT_SCALE, 8 * UNIT_SCALE},
    {
        8 * (float)((int)(fWalkAnim * 6.f) % 4) / pTexture->getSizef().x,
        (float)direction * 8 / pTexture->getSizef().y,
        (8 + 8 * (float)((int)(fWalkAnim * 6.f) % 4)) / pTexture->getSizef().x,
        (float)(direction + 1) * 8 / pTexture->getSizef().y
    });
}
