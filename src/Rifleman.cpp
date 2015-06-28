#include "Rifleman.h"
#include "Game.h"

float g_fRifleVolume = 1.f;

Rifleman::Rifleman()
{
    fAttackRange = 350.f;
    fPrecision = 10.f;
}

void Rifleman::update()
{
    Soldier::update();
}

void Rifleman::render()
{
    Soldier::render();
}

void Rifleman::onShoot(const Vector2& attackPos)
{
    g_pGame->spawnBullet(position, attackPos, fPrecision, team, .75f, false);
    g_pGame->playSound(OGetSound("rifle.wav"), position, onut::lerp(.25f, 1.f, g_fRifleVolume));
    g_fRifleVolume = 0;
}
