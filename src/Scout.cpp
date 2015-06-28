#include "Scout.h"
#include "Game.h"
#include "Rifleman.h"

Scout::Scout()
{
    fAttackRange = 650.f;
    fPrecision = 3.f;
    textureOffset = 4;
}

void Scout::update()
{
    Soldier::update();
}

void Scout::render()
{
    Soldier::render();
}

void Scout::onShoot(const Vector2& attackPos)
{
    g_pGame->spawnBullet(position, attackPos, fPrecision, team, 1.0f, false);
    g_pGame->playSound(OGetSound("rifle.wav"), position, onut::lerp(.25f, 1.f, g_fRifleVolume));
    g_fRifleVolume = 0;
}
