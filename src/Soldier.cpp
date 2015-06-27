#include "Soldier.h"
#include "Game.h"

#define STEERING_SIZE 32
#define STEERSPEED 64

Soldier::Soldier()
{
    pTexture = OGetTexture("soldier.png");
    pTextureColor = OGetTexture("soldier_color.png");
    direction = onut::randi(0, 3);
    state = onut::randi(0, 1);
    idleChangeTime = onut::randf(2.f, 4.f);
}

void Soldier::update()  
{
    Unit::update();

    updateSteering();
    updateIdleState();
    updateDirection();
    updateMovement();
    updateAttack();
}

void Soldier::updateAttack()
{
    if (fShootTime <= 0.f)
    {
        g_pGame->forEachInRadius(this, fAttackRange, [this](Unit* pUnit, float dis)
        {
            if (pUnit->team == team) return;
            if (pUnit->team == TEAM_NEUTRAL) return;

            onStartAttack(pUnit->position);
        });
    }
    else
    {
        fShootTime -= ODT;
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

void Soldier::updateSteering()
{
    if (fShootTime <= 0.f)
    {
        g_pGame->forEachInRadius(this, STEERING_SIZE, [this](Unit* pUnit, float dis)
        {
            Vector2 dirWithOther = position - pUnit->position;
            dirWithOther /= dis;

            float steerForce = 1 - dis / STEERING_SIZE;
            steerForce = std::max<>(steerForce, .5f);

            moveDir *= velocity;
            moveDir += dirWithOther * steerForce * STEERSPEED;
            velocity = moveDir.Length();
            moveDir /= velocity;
        });
    }
}

void Soldier::updateMovement()
{
    if (fShootTime <= 0.f)
    {
        position += moveDir * velocity * ODT;

        if (velocity > 0.f)
        {
            fWalkAnim += ODT;
            state = SOLDIER_STATE_STANDING;
        }
        else
        {
            fWalkAnim = 0.f;
        }

        moveDir = Vector2::Zero;
        velocity = 0.f;
    }
    else
    {
        fWalkAnim += ODT;
    }
}

void Soldier::updateDirection()
{
    if (fShootTime <= 0.f)
    {
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
    Rect rect{position.x - 4 * UNIT_SCALE, position.y - 6 * UNIT_SCALE, 8 * UNIT_SCALE, 8 * UNIT_SCALE};
    auto frame = 0;
    if (fShootTime > 0.f)
    {
        frame = (int)(fWalkAnim * SHOOT_ANIM_SPEED);
        frame = std::min<>(frame, 3);
    }
    else
    {
        frame = (int)(fWalkAnim * WALK_ANIM_SPEED) % 4;
    }
    rect.x = std::roundf(rect.x / 2) * 2;
    rect.y = std::roundf(rect.y / 2) * 2;

    Vector4 UVs{
        (float)(state * 4 + frame) * 8 / pTexture->getSizef().x,
        (float)direction * 8 / pTexture->getSizef().y,
        (float)(state * 4 + frame + 1) * 8 / pTexture->getSizef().x,
        (float)(direction + 1) * 8 / pTexture->getSizef().y
    };

    OSB->drawRectWithUVs(pTexture, rect, UVs);
    OSB->drawRectWithUVs(pTextureColor, rect, UVs, TEAM_COLOR[team]);
}

void Soldier::onStartAttack(const Vector2& attackPos)
{
    savedAttackPos = attackPos;
    fShootTime = getShootTime();
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
