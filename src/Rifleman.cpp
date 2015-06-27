#include "Rifleman.h"
#include "Game.h"

Rifleman::Rifleman()
{
    fAttackRange = 300.f;
    fPrecision = 30.f;
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
    g_pGame->spawnBullet(position, attackPos, fPrecision, team);
}
