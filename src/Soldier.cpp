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

    // Apply steering behaviour
    int chunkFromX = (int)(position.x - STEERING_SIZE - fRadius) / CHUNK_SIZE;
    int chunkFromY = (int)(position.y - STEERING_SIZE - fRadius) / CHUNK_SIZE;
    int chunkToX = (int)(position.x + STEERING_SIZE + fRadius) / CHUNK_SIZE;
    int chunkToY = (int)(position.y + STEERING_SIZE + fRadius) / CHUNK_SIZE;

    for (int chunkY = chunkFromY; chunkY <= chunkToY; ++chunkY)
    {
        for (int chunkX = chunkFromX; chunkX <= chunkToX; ++chunkX)
        {
            auto pChunk = g_pGame->pChunks + (chunkY * CHUNK_COUNT + chunkX);
            for (auto pUnit = pChunk->pUnits->Head(); pUnit; pUnit = pChunk->pUnits->Next(pUnit))
            {
                if (pUnit == this) continue;
                float dis = Vector2::DistanceSquared(position, pUnit->position);
                if (dis <= STEERING_SIZE * STEERING_SIZE + fRadius * fRadius)
                {
                    dis = sqrtf(dis);
                    Vector2 dirWithOther = position - pUnit->position;
                    dirWithOther /= dis;

                    float steerForce = 1 - dis / STEERING_SIZE;
                    steerForce = std::max<>(steerForce, .5f);

                    moveDir *= velocity;
                    moveDir += dirWithOther * steerForce * STEERSPEED;
                    velocity = moveDir.Length();
                    moveDir /= velocity;
                }
            }
        }
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

void Soldier::render()
{
    Rect rect{position.x - 4 * UNIT_SCALE, position.y - 6 * UNIT_SCALE, 8 * UNIT_SCALE, 8 * UNIT_SCALE};
    auto frame = (int)(fWalkAnim * 6.f) % 4;
    Vector4 UVs{
        (float)(state + frame) * 8 / pTexture->getSizef().x,
        (float)direction * 8 / pTexture->getSizef().y,
        (float)(state + frame + 1) * 8 / pTexture->getSizef().x,
        (float)(direction + 1) * 8 / pTexture->getSizef().y
    };

    OSB->drawRectWithUVs(pTexture, rect, UVs);
    OSB->drawRectWithUVs(pTextureColor, rect, UVs, TEAM_COLOR[team]);
}
