#include "MortarImpact.h"

#define PUFF_ANIM_SPEED 12

MortarImpact::MortarImpact()
{
    pTexture = OGetTexture("mortarImpact.png");
    style = 0;
}

void MortarImpact::update()
{
    fPuffAnim += ODT;
    if (fPuffAnim * PUFF_ANIM_SPEED >= 6)
    {
        bMarkedForDeletion = true;
    }
}

void MortarImpact::render()
{
    auto snappedPos = getSnapPos();
    Rect rect{snappedPos.x - 8 * UNIT_SCALE, snappedPos.y - 10 * UNIT_SCALE, 16 * UNIT_SCALE, 16 * UNIT_SCALE};
    auto frame = 0;
    frame = (int)(fPuffAnim * PUFF_ANIM_SPEED) % 6;

    Vector4 UVs{
        (float)frame * 16 / pTexture->getSizef().x,
        ((float)style) * 16 / pTexture->getSizef().y,
        (float)(frame + 1) * 16 / pTexture->getSizef().x,
        ((float)style + 1) * 16 / pTexture->getSizef().y
    };

    OSB->drawRectWithUVs(pTexture, rect, UVs);
}
