#include "Bullet.h"
#include "Unit.h"
#include "Game.h"

Bullet::Bullet(const Vector2& from, const Vector2& to, float precision, int in_team)
    : team(team)
{
    position = {from, 4 * UNIT_SCALE};
    Vector3 to3 = {to, 4 * UNIT_SCALE};
    trail[0] = trail[1] = trail[2] = position;

    velocity = to3 - position;
    velocity.z += velocity.Length() / 50.f;
    velocity.Normalize();
    velocity *= BULLET_SPEED;
}

bool Bullet::update()
{
    trail[2] = trail[1];
    trail[1] = trail[0];
    trail[0] = position;
    position += velocity * ODT;

    // Bullet gravity
    velocity.z -= ODT * BULLET_GRAVITY;

    // Slow down bullets (Air resistance)
    velocity.x *= 1 - ODT * 2;
    velocity.y *= 1 - ODT * 2;

    if (position.z <= 0.f)
    {
        return true;
    }

    return false;
}

void Bullet::render()
{
    OPB->draw({trail[2].x, trail[2].y - trail[2].z}, {0, 0, 0, 0});
    OPB->draw({trail[1].x, trail[1].y - trail[1].z}, Color{1, 1, .5f, 1} *.25f);

    OPB->draw({trail[1].x, trail[1].y - trail[1].z}, Color{1, 1, .5f, 1} * .25f);
    OPB->draw({trail[0].x, trail[0].y - trail[0].z}, Color{1, 1, .5f, 1} *.5f);

    OPB->draw({trail[0].x, trail[0].y - trail[0].z}, Color{1, 1, .5f, 1} * .5f);
    OPB->draw({position.x, position.y - position.z}, Color{1, 1, .5f, 1} * .75f);
}
