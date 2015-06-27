#include "Bullet.h"
#include "Unit.h"
#include "Game.h"
#include "Puff.h"

Bullet::Bullet(const Vector2& from, const Vector2& to, float precision, int in_team, float dmg)
    : team(in_team)
    , damage(dmg)
{
    position = {from, 4 * UNIT_SCALE};
    Vector3 to3 = {to, 4 * UNIT_SCALE};
    trail[0] = trail[1] = trail[2] = position;

    velocity = to3 - position;
    velocity.z += velocity.Length() / (30.f - velocity.Length() / 500.f * 10.f);
    velocity.z += onut::randf(-precision, precision);
    velocity.Normalize();

    // Randomize
    velocity = Vector3::Transform(velocity, Matrix::CreateRotationZ(DirectX::XMConvertToRadians(onut::randf(-precision, precision))));

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
    velocity.x *= 1 - ODT;
    velocity.y *= 1 - ODT;

    // Do we touch someone?
    bool bHit = false;
    g_pGame->forEachInRadius(position, 16.f, [this, &bHit](Unit *pUnit, float dis)
    {
        if (bHit) return;
        if (pUnit->team == TEAM_NEUTRAL) return;
        if (pUnit->team == team) return;
        float h;
        auto unitRect = pUnit->getHitRect(h);
        if (position.z <= h)
        {
            if (dis <= pUnit->fRadius)// unitRect.Distance(position) <= 0.f)
            {
                pUnit->doDamage(damage);
                bHit = true;
            }
        }
    });
    if (bHit) return true;

    if (position.z <= 0.f)
    {
        g_pGame->spawn<Puff>(position);
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
