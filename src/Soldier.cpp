#include "Soldier.h"
#include "Game.h"
#include "Blood.h"

#define STEERING_SIZE 32
#define STEERSPEED 64
#define FOLLOW_DISTANCE 16
#define SHOOT_AND_WALK_TIMEOUT (6.f / SHOOT_ANIM_SPEED)

Soldier::Soldier()
{
    bChunkIt = true;
    pTexture = OGetTexture("soldier.png");
    pTextureColor = OGetTexture("soldier_color.png");
    direction = onut::randi(0, 3);
    state = onut::randi(0, 1);
    idleChangeTime = onut::randf(2.f, 4.f);
}

void Soldier::update()  
{
    Unit::update();

    if (state != SOLDIER_STATE_DYING)
    {
        updateSteering();
        updateIdleState();
        updateDirection();
        updateMovement();
        updateAttack();
    }
    else
    {
        fDieAnim += ODT;
    }
}

void Soldier::updateAttack()
{
    if (bLocked) return;
    if (fShootTime <= 0.f)
    {
        Unit *pTargetFound = nullptr;
        float closestDis = fAttackRange;

        auto lambda = [this, &closestDis, &pTargetFound](Unit* pUnit, float dis)
        {
            if (pUnit->team == team) return;
            if (pUnit->team == TEAM_NEUTRAL) return;

            auto pHero = dynamic_cast<Hero*>(pTargetFound);
            if (pTargetFound && !pHero)
            {
                if (dynamic_cast<Hero*>(pUnit)) return;
            }
            if (dis < closestDis || pHero)
            {
                pTargetFound = pUnit;
                closestDis = dis;
            }
        };

        g_pGame->forEachInRadius(this, fAttackRange, lambda);

        if (pTargetFound)
        {
            onStartAttack(pTargetFound->position);
        }
    }
    else
    {
        fShootTime -= ODT;
        if (pMortar)
        {
            if (fShootDelay > 0.f)
            {
                fShootDelay = 0.f;
                pMortar->onShoot(savedAttackPos);
            }
        }
        else
        {
            if (fShootDelay > 0.f)
            {
                fShootDelay -= ODT;
                if (fShootDelay <= 0.f)
                {
                    // Shoot
                    onShoot(savedAttackPos);
                }
            }
        }
    }
}

void Soldier::updateSteering()
{
    if (fShootTime <= 0.f || fWalkAnim >= SHOOT_AND_WALK_TIMEOUT)
    {
        // Follow
        if (pFollow)
        {
            auto disToFollow = Vector2::DistanceSquared(position, followTargetPos);
            if (disToFollow > FOLLOW_DISTANCE * FOLLOW_DISTANCE)
            {
                Vector2 dirWithOther = followTargetPos - position;
                dirWithOther.Normalize();

                moveDir *= velocity;
                moveDir += dirWithOther * SOLDIER_SPEED;
                velocity = moveDir.Length();
                moveDir /= velocity;
            }
        }
        else if (bFollowAlert)
        {
            // Follow alert
            auto disToFollow = Vector2::DistanceSquared(position, followTargetPos);
            if (disToFollow > FOLLOW_DISTANCE * FOLLOW_DISTANCE)
            {
                Vector2 dirWithOther = followTargetPos - position;
                dirWithOther.Normalize();

                moveDir *= velocity;
                moveDir += dirWithOther * SOLDIER_SPEED;
                velocity = moveDir.Length();
                moveDir /= velocity;
            }
            else
            {
                bFollowAlert = false;
            }
        }

        if (bSteer)
        {
            g_pGame->forEachInRadius(this, STEERING_SIZE, [this](Unit* pUnit, float dis)
            {
                if (pMortar)
                {
                    if (pMortar->pCrew2 == pUnit) return;
                }

                Vector2 dirWithOther = position - pUnit->position;
                dirWithOther /= dis;

                float steerForce = 1 - dis / STEERING_SIZE;
                steerForce = std::max<>(steerForce, .5f);

                moveDir *= velocity;
                moveDir += dirWithOther * steerForce * STEERSPEED * 2;
                velocity = moveDir.Length();
                moveDir /= velocity;
            });
        }
    }
}

void Soldier::updateMovement()
{
    if (fShootTime <= 0.f || fWalkAnim >= SHOOT_AND_WALK_TIMEOUT)
    {
        auto oldPos = position;
        position += moveDir * velocity * ODT;
        if (g_pGame->collisionAt(position))
        {
            position = oldPos; // Pretty rough
        }

        if (velocity > 0.f)
        {
            fWalkAnim += ODT;
            state = SOLDIER_STATE_STANDING;
        }
        else if (fWalkAnim > WALK_ANIM_SPEED * 3.f)
        {
            fWalkAnim = 0.f;
        }

        lastMoveDir = moveDir;
        moveDir = Vector2::Zero;
        velocity = 0.f;
    }
    else
    {
        fWalkAnim += ODT;
    }

    // If we control a mortar, make sure it's always following us
    if (pMortar)
    {
        if (lastMoveDir.LengthSquared() > 0.f)
        {
            Vector2 right{-lastMoveDir.y, lastMoveDir.x};
            pMortar->position = position + lastMoveDir * 16.f - right * 16.f;

            // If we have a crew member, make sure he also follows the mortar
            if (pMortar->pCrew2)
            {
                pMortar->pCrew2->bFollowAlert = true;
                pMortar->pCrew2->followTargetPos = position + - right * 32.f;
            }
        }
    }
}

void Soldier::updateDirection()
{
    if (fShootTime <= 0.f || fWalkAnim >= SHOOT_AND_WALK_TIMEOUT)
    {
        if (pMortar)
        {
            if (moveDir.LengthSquared() > 0)
            {
                auto angle = DirectX::XMConvertToDegrees(atan2f(moveDir.y, moveDir.x));
                while (angle < 0.f) angle += 360.f;
                angle /= 360.f;
                angle *= 8.f;
                angle += .5f;
                pMortar->direction = (int)angle + 2;
            }
        }
        switch (direction)
        {
            case DIRECTION_UP:
            {
                if (moveDir.x > .71f)
                {
                    direction = DIRECTION_RIGHT;
                }
                else if (moveDir.x < -.71f)
                {
                    direction = DIRECTION_LEFT;
                }
                else if (moveDir.y > 0.f)
                {
                    direction = DIRECTION_DOWN;
                }
                break;
            }
            case DIRECTION_DOWN:
            {
                if (moveDir.x > .71f)
                {
                    direction = DIRECTION_RIGHT;
                }
                else if (moveDir.x < -.71f)
                {
                    direction = DIRECTION_LEFT;
                }
                else if (moveDir.y < 0.f)
                {
                    direction = DIRECTION_UP;
                }
                break;
            }
            case DIRECTION_LEFT:
            {
                if (moveDir.y > .71f)
                {
                    direction = DIRECTION_DOWN;
                }
                else if (moveDir.y < -.71f)
                {
                    direction = DIRECTION_UP;
                }
                else if (moveDir.x > 0.f)
                {
                    direction = DIRECTION_RIGHT;
                }
                break;
            }
            case DIRECTION_RIGHT:
            {
                if (moveDir.y > .71f)
                {
                    direction = DIRECTION_DOWN;
                }
                else if (moveDir.y < -.71f)
                {
                    direction = DIRECTION_UP;
                }
                else if (moveDir.x < 0.f)
                {
                    direction = DIRECTION_LEFT;
                }
                break;
            }
        }
    }
}

void Soldier::updateIdleState()
{
    if (fShootTime <= 0.f)
    {
        if (state == SOLDIER_STATE_SHOOTING_STANDING)
        {
            state = onut::randi(0, 1);
        }
        fIdleTime += ODT;
        if (fIdleTime >= idleChangeTime)
        {
            fIdleTime = 0.f;
            idleChangeTime = onut::randf(2.f, 4.f);
            switch (onut::randi(0, 4))
            {
                case 0:
                    direction = (direction + 3) % 4;
                    break;
                case 1:
                    direction = (direction + 1) % 4;
                    break;
                case 2:
                    if (state == SOLDIER_STATE_STANDING)
                    {
                        state = SOLDIER_STATE_CROUCHING;
                    }
                    else if (state == SOLDIER_STATE_CROUCHING)
                    {
                        state = SOLDIER_STATE_STANDING;
                    }
                    break;
                case 3:
                    break;
            }
        }
    }
}

void Soldier::render()
{
    auto snappedPos = getSnapPos();
    Rect rect{snappedPos.x - 4 * UNIT_SCALE, snappedPos.y - 6 * UNIT_SCALE, 8 * UNIT_SCALE, 8 * UNIT_SCALE};
    auto frame = 0;
    auto dir = direction;
    auto aState = state;
    if (state == SOLDIER_STATE_DYING)
    {
        if (fDieAnim >= 0)
        {
            if (direction != dieInDirection) direction = dieInDirection;
            frame = (int)(fDieAnim * DIE_ANIM_SPEED);
            frame = std::min<>(frame, 3);
        }
        else
        {
            frame = (int)(-fDieAnim * DIE_ANIM_SPEED);
            frame = std::min<>(frame, 1);
            aState = SOLDIER_STATE_STANDING;
            if (frame == 0)
            {
                dir = (dir + 2) % 4;
            }
            else if (frame == 1)
            {
                dir = (dir + 1) % 4;
                frame = 0;
            }
        }
    }
    else
    {
        if (fShootTime > 0.f && state == SOLDIER_STATE_SHOOTING_STANDING)
        {
            frame = (int)(fWalkAnim * SHOOT_ANIM_SPEED);
            frame = std::min<>(frame, 3);
        }
        else
        {
            frame = (int)(fWalkAnim * WALK_ANIM_SPEED) % 4;
        }
    }

    Vector4 UVs{
        (float)(aState * 4 + frame) * 8 / pTexture->getSizef().x,
        (float)dir * 8 / pTexture->getSizef().y,
        (float)(aState * 4 + frame + 1) * 8 / pTexture->getSizef().x,
        (float)(dir + 1) * 8 / pTexture->getSizef().y
    };

    OSB->drawRectWithUVs(pTexture, rect, {
        UVs.x, 
        UVs.y + (float)textureOffset * 8 / pTexture->getSizef().y,
        UVs.z,
        UVs.w + (float)textureOffset * 8 / pTexture->getSizef().y
    });
    if (state != SOLDIER_STATE_DYING)
    {
        OSB->drawRectWithUVs(pTextureColor, rect, UVs, TEAM_COLOR[team]);
    }
}

void Soldier::onStartAttack(const Vector2& attackPos)
{
    savedAttackPos = attackPos;
    auto shootTime = getShootTime();
    if (shootTime > 0.f)
    {
        fShootTime = shootTime + onut::randf(-.25f, .25f);
        fIdleTime = 0;
        fShootDelay = getShootDelay();

        if (fShootTime > 0.f)
        {
            auto dir = attackPos - position;
            dir.Normalize();
            if (dir.x > .71f) direction = DIRECTION_RIGHT;
            else if (dir.x < -.71f) direction = DIRECTION_LEFT;
            else if (dir.y < 0) direction = DIRECTION_UP;
            else if (dir.y > 0) direction = DIRECTION_DOWN;

            state = SOLDIER_STATE_SHOOTING_STANDING;
            fWalkAnim = 0.f;
        }
    }
}

void Soldier::renderSelection()
{
    //if (g_pGame->pMyHero)
    //{
    //    if (pFollow && team == g_pGame->pMyHero->team)
    //    {
    //        OSB->drawSprite(OGetTexture("selectedSmall.png"), getSnapPos(), {TEAM_COLOR[team].x, TEAM_COLOR[team].y, TEAM_COLOR[team].z, .5f}, 0, 1);
    //    }
    //}
}

void Soldier::doDamage(float dmg)
{
    g_pGame->spawn<Blood>({position.x, position.y - 6 * UNIT_SCALE * .5f});
    life -= dmg;
    if (life <= 0.f)
    {
        team = TEAM_NEUTRAL;
        life = 0.f;
        fShootTime = 0.f;
        bChunkIt = false;
        linkFollow.Unlink();

        // A small change to spin first
        if (direction == DIRECTION_LEFT) dieInDirection = DIRECTION_UP;
        if (direction == DIRECTION_RIGHT) dieInDirection = DIRECTION_DOWN;
        if (onut::randf(10) <= 3.f)
        {
            fDieAnim = -2.f / DIE_ANIM_SPEED;
        }

        static char szSnd[] = "soldier_die1.wav";
        szSnd[strlen(szSnd) - 5] = '0' + onut::randi(1, 2);
        g_pGame->playSound(OGetSound(szSnd), position);

        state = SOLDIER_STATE_DYING;
        linkChunk.Unlink();

        auto pSoldier = this;
        if (pSoldier)
        {
            if (pSoldier->pMortar)
            {
                if (pSoldier->pMortar->pCrew2)
                {
                    pSoldier->pMortar->pCrew2->bLocked = false;
                    pSoldier->pMortar->pCrew2->fAttackRange = 350.f;
                }
                pSoldier->fAttackRange = 350.f;
                pSoldier->pMortar->pCrew1 = nullptr;
                pSoldier->pMortar->pCrew2 = nullptr;
                pSoldier->pMortar = nullptr;
            }
        }

        for (auto pUnit2 = g_pGame->pUnits->Head(); pUnit2; pUnit2 = g_pGame->pUnits->Next(pUnit2))
        {
            if (pUnit2 == this) continue;
            pSoldier = dynamic_cast<Soldier*>(pUnit2);
            if (pSoldier)
            {
                if (pSoldier->pMortar)
                {
                    if (pSoldier->pMortar->pCrew2 == this)
                    {
                        pSoldier->fAttackRange = 350.f;
                        pSoldier->pMortar->pCrew1 = nullptr;
                        pSoldier->pMortar->pCrew2 = nullptr;
                        pSoldier->pMortar = nullptr;
                    }
                }
            }
        }
    }
}
