#include "Rifleman.h"
#include "Game.h"

Rifleman::Rifleman()
{
    fAttackRange = 500.f;
    fPrecision = 5.f;
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
    g_pGame->spawnBullet(position, attackPos, fPrecision, team, .75f);
    g_pGame->playSound(OGetSound("rifle.wav"), position, .75f);
}
