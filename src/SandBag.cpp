#include "SandBag.h"

SandBag::SandBag()
{
    fRadius = 5 * UNIT_SCALE;
    pTexture = OGetTexture("sandBag.png");
}

void SandBag::render()
{
    OSB->drawSprite(pTexture, getSnapPos(), Color::White, 0, UNIT_SCALE);
}
