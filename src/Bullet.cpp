#include "Bullet.h"
#include "Unit.h"
#include "Game.h"
#include "Puff.h"
#include "MortarImpact.h"

#define BULLET_ALERT_RADIUS 200
#define SHELL_SPEED (1.f / 2.f)

Bullet::Bullet(const Vector2& in_from, const Vector2& in_to, float precision, int in_team, float dmg, bool in_isShell)
    : team(in_team)
    , damage(dmg)
    , from(in_from)
    , to(in_to)
    , isShell(in_isShell)
{
    if (isShell)
    {
        float angle = onut::randf(0, DirectX::XM_2PI);
        float radius = std::sqrtf(onut::randf(0, precision * precision));
        to.x = to.x + radius * std::cosf(angle);
        to.y = to.y + radius * std::sinf(angle);

        position = from;
        trail[0] = trail[1] = trail[2] = position;
    }
    else
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
}

bool Bullet::update()
{
    trail[2] = trail[1];
    trail[1] = trail[0];
    trail[0] = position;

    if (isShell)
    {
        if (shellProgress < .8f)
        {
            shellProgress += ODT * SHELL_SPEED;
            if (shellProgress >= .8f)
            {
                g_pGame->playSound(OGetSound("mortar_exp.wav"), to, 1.5f);
            }
        }
        else
        {
            shellProgress += ODT * SHELL_SPEED;
        }

        if (shellProgress >= 1.f)
        {
            g_pGame->spawn<MortarImpact>(to);

            g_pGame->forEachInRadius(position, 64.f, [this](Unit *pUnit, float dis)
            {
                if (pUnit->team == TEAM_NEUTRAL) return;
                if (pUnit->team == team) return;
                pUnit->doDamage(damage * (1 - dis / 64.f));
            });

            alertNearbyEnemies();

            return true;
        }

        // Bezier
        Vector3 P0 = from;
        Vector3 P1 = from + Vector3(0, 0, 500);
        Vector3 P2 = to + Vector3(0, 0, 500);
        Vector3 P3 = to;
        float t = shellProgress;
        float invT = 1 - t;
        position = invT * invT * invT * P0 + 3 * invT * invT * t * P1 + 3 * invT * t * t * P2 + t * t * t * P3;
    }
    else
    {
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
                if (dis <= pUnit->fRadius)
                {
                    pUnit->doDamage(damage);
                    bHit = true;
                }
            }
        });
        if (bHit)
        {
            alertNearbyEnemies();
            return true;
        }

        if (position.z <= 0.f)
        {
            g_pGame->spawn<Puff>(position);
            alertNearbyEnemies();
            return true;
        }
    }

    return false;
}

void Bullet::alertNearbyEnemies()
{
    g_pGame->forEachInRadius(position, BULLET_ALERT_RADIUS, [this](Unit *pUnit, float dis)
    {
        if (pUnit->team == TEAM_NEUTRAL) return;
        if (pUnit->team == team) return;
        auto pSoldier = dynamic_cast<Soldier*>(pUnit);
        if (pSoldier)
        {
            if (dynamic_cast<Hero*>(pSoldier)) return;
            if (pSoldier->bLocked) return;

            if (pSoldier->pMortar)
            {
                auto dir = position - from;
                dir.Normalize();
                pSoldier->followTargetPos = position + dir * 32.f; // Move away a bit
                pSoldier->bFollowAlert = true;
            }
            else
            {
                pSoldier->followTargetPos = position + (from - position) * .333f; // Go one third of the bullet direction
                pSoldier->bFollowAlert = true;
            }
        }
    });
}

void Bullet::render()
{
    Color color{1, 1, .5f, 1};
    if (isShell) color = {.5f, .5f, .5f, 1};

    OPB->draw({trail[2].x, trail[2].y - trail[2].z}, color * 0.f);
    OPB->draw({trail[1].x, trail[1].y - trail[1].z}, color *.25f);

    OPB->draw({trail[1].x, trail[1].y - trail[1].z}, color * .25f);
    OPB->draw({trail[0].x, trail[0].y - trail[0].z}, color *.5f);

    OPB->draw({trail[0].x, trail[0].y - trail[0].z}, color * .5f);
    OPB->draw({position.x, position.y - position.z}, color * .75f);
}
