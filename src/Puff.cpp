#include "Puff.h"

#define PUFF_ANIM_SPEED 12

Puff::Puff()
{
    pTexture = OGetTexture("puff.png");
    style = onut::randi(0, 3);
}

void Puff::update()
{
    fPuffAnim += ODT;
    if (fPuffAnim * PUFF_ANIM_SPEED >= 4)
    {
        bMarkedForDeletion = true;
    }
}

void Puff::render()
{
    auto snappedPos = getSnapPos();
    Rect rect{snappedPos.x - 4 * UNIT_SCALE, snappedPos.y - 6 * UNIT_SCALE, 8 * UNIT_SCALE, 8 * UNIT_SCALE};
    auto frame = 0;
    frame = (int)(fPuffAnim * PUFF_ANIM_SPEED) % 4;

    Vector4 UVs{
        (float)frame * 8 / pTexture->getSizef().x,
        ((float)style) * 8 / pTexture->getSizef().y,
        (float)(frame + 1) * 8 / pTexture->getSizef().x,
        ((float)style + 1) * 8 / pTexture->getSizef().y
    };

    OSB->drawRectWithUVs(pTexture, rect, UVs);
}
