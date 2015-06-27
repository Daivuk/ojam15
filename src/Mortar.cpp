#include "Mortar.h"
#include "Game.h"
#include "Smoke.h"

Mortar::Mortar()
{
    pTexture = OGetTexture("mortar.png");
}

void Mortar::update()
{
}

void Mortar::render()
{
    Vector4 UVs{
        (float)(direction) * 8 / pTexture->getSizef().x,
        0,
        (float)(direction + 1) * 8 / pTexture->getSizef().x,
        8 / pTexture->getSizef().y
    };

    OSB->drawSpriteWithUVs(pTexture, getSnapPos(), UVs, Color::White, 0, UNIT_SCALE);
}

void Mortar::onShoot(const Vector2& attackPos)
{
    g_pGame->spawn<Smoke>(position);

    g_pGame->spawnBullet(position, attackPos, 16.f * UNIT_SCALE, team, 3.f, true);
    g_pGame->playSound(OGetSound("mortar_shoot.wav"), position, .5f);
}
