#include "Hero.h"
#include "Game.h"

#define FOLLOW_TOGGLE_RANGE 128

auto g_actionSoundPlayTime = std::chrono::steady_clock::now();

Hero::Hero()
{
    bSteer = false;
    life = 10.f;
    state = SOLDIER_STATE_CROUCHING;
    pFollowers = new TList<Soldier>(offsetOf(&Soldier::linkFollow));
    dir = Vector2::UnitY;
}

void Hero::update()
{
    Vector2 lt;// = OGamePad(0)->getLeftThumb();
    if (OInput->isStateDown(DIK_W))
    {
        lt += Vector2(0, -1);
    }
    if (OInput->isStateDown(DIK_A))
    {
        lt += Vector2(-1, 0);
    }
    if (OInput->isStateDown(DIK_S))
    {
        lt += Vector2(0, 1);
    }
    if (OInput->isStateDown(DIK_D))
    {
        lt += Vector2(1, 0);
    }
    if (lt.LengthSquared() > .25f * .25f)
    {
        lt.Normalize();
        moveDir = lt;
        velocity = SOLDIER_SPEED;
        fIdleTime = 0.f;
    }
    else
    {
        velocity = 0.f;
    }

    Soldier::update();

    //g_pGame->camera = position + dir * 64.f;

    // Order other units to follow
    //if (OGamePad(0)->isJustPressed(onut::GamePad::eGamePad::A))
    if (OInput->isStateDown(DIK_MOUSEB1))
    {
        g_pGame->forEachInRadius(this, FOLLOW_TOGGLE_RANGE, [this](Unit* pUnit, float dis)
        {
            auto pSoldier = dynamic_cast<Soldier*>(pUnit);
            if (!pSoldier) return;
            if (pSoldier->bLocked) return;
            if (pSoldier->team != team) return;
            if (pSoldier->pFollow == this) return;

            pSoldier->pFollow = this;
            pFollowers->InsertTail(pSoldier);

            // Play sound like: Right away sir
            if (std::chrono::steady_clock::now() - g_actionSoundPlayTime > std::chrono::seconds(1))
            {
                g_actionSoundPlayTime = std::chrono::steady_clock::now();
                static char szSnd[] = "soldier_action1.wav";
                szSnd[strlen(szSnd) - 5] = '0' + onut::randi(1, 5);
                OGetSound(szSnd)->play(5.f);
            }
        });
    }
    //if (OGamePad(0)->isJustPressed(onut::GamePad::eGamePad::B))
    if (OInput->isStateDown(DIK_MOUSEB2))
    {
        g_pGame->forEachInRadius(this, FOLLOW_TOGGLE_RANGE, [this](Unit* pUnit, float dis)
        {
            auto pSoldier = dynamic_cast<Soldier*>(pUnit);
            if (!pSoldier) return;
            if (pSoldier->bLocked) return;
            if (pSoldier->team != team) return;
            if (pSoldier->pFollow != this) return;

            pSoldier->pFollow = nullptr;
            pSoldier->linkFollow.Unlink();

            // Play sound like: Right away sir
            if (std::chrono::steady_clock::now() - g_actionSoundPlayTime > std::chrono::seconds(1))
            {
                g_actionSoundPlayTime = std::chrono::steady_clock::now();
                static char szSnd[] = "soldier_action1.wav";
                szSnd[strlen(szSnd) - 5] = '0' + onut::randi(1, 5);
                OGetSound(szSnd)->play(5.f);
            }
        });
    }

    dir += lastMoveDir * ODT * 10.f;
    dir.Normalize();
    Vector2 right{-dir.y, dir.x};

#define OFFSET_FROM_HERO (12 * UNIT_SCALE)
#define SEPARATION (8 * UNIT_SCALE)
    static const Vector2 followPositions[] =
    {
        {-SEPARATION * .5f, SEPARATION * .5f + OFFSET_FROM_HERO},
        {SEPARATION * .5f, SEPARATION * .5f + OFFSET_FROM_HERO},
        {-SEPARATION * 1.5f, SEPARATION * .5f + OFFSET_FROM_HERO},
        {SEPARATION * 1.5f, SEPARATION * .5f + OFFSET_FROM_HERO},
        {-SEPARATION * .5f, SEPARATION * 1.5f + OFFSET_FROM_HERO},
        {SEPARATION * .5f, SEPARATION * 1.5f + OFFSET_FROM_HERO},
        {-SEPARATION * 1.5f, SEPARATION * 1.5f + OFFSET_FROM_HERO},
        {SEPARATION * 1.5f, SEPARATION * 1.5f + OFFSET_FROM_HERO},

        {0, SEPARATION * 2.2f + OFFSET_FROM_HERO},
        {SEPARATION, SEPARATION * 2.2f + OFFSET_FROM_HERO},
        {-SEPARATION, SEPARATION * 2.2f + OFFSET_FROM_HERO},

        {-SEPARATION * 2.5f, SEPARATION * .5f + OFFSET_FROM_HERO},
        {SEPARATION * 2.5f, SEPARATION * .5f + OFFSET_FROM_HERO},
        {-SEPARATION * 2.5f, SEPARATION * 1.5f + OFFSET_FROM_HERO},
        {SEPARATION * 2.5f, SEPARATION * 1.5f + OFFSET_FROM_HERO},
        {SEPARATION * 2.f, SEPARATION * 2.2f + OFFSET_FROM_HERO},
        {-SEPARATION * 2.f, SEPARATION * 2.2f + OFFSET_FROM_HERO},

        {-SEPARATION * 3.5f, SEPARATION * .5f + OFFSET_FROM_HERO},
        {SEPARATION * 3.5f, SEPARATION * .5f + OFFSET_FROM_HERO},
        {-SEPARATION * 3.5f, SEPARATION * 1.5f + OFFSET_FROM_HERO},
        {SEPARATION * 3.5f, SEPARATION * 1.5f + OFFSET_FROM_HERO},
        {SEPARATION * 3.f, SEPARATION * 2.2f + OFFSET_FROM_HERO},
        {-SEPARATION * 3.f, SEPARATION * 2.2f + OFFSET_FROM_HERO},
    };
    static const auto followPosCount = sizeof(followPositions) / sizeof(Vector2);

    static const Vector2 mortarFollowPos[] =
    {
        {-SEPARATION * 1.0f,  - OFFSET_FROM_HERO},
        {SEPARATION * 1.0f, - OFFSET_FROM_HERO},
        {-SEPARATION * 3.0f, - OFFSET_FROM_HERO},
        {SEPARATION * 3.0f, - OFFSET_FROM_HERO},
    };
    static const auto followMortarPosCount = sizeof(mortarFollowPos) / sizeof(Vector2);

    unsigned int i = 0;
    unsigned int j = 0;
    for (auto pFollower = pFollowers->Head(); pFollower; pFollower = pFollowers->Next(pFollower))
    {
        if (pFollower->pMortar)
        {
            if (j < followMortarPosCount)
            {
                pFollower->followTargetPos = position + dir * mortarFollowPos[j].y + right * mortarFollowPos[j].x;
                ++j;
            }
            else
            {
                pFollower->followTargetPos = position - dir * OFFSET_FROM_HERO;
            }
        }
        else
        {
            if (i < followPosCount)
            {
                pFollower->followTargetPos = position + dir * followPositions[i].y + right * followPositions[i].x;
                ++i;
            }
            else
            {
                pFollower->followTargetPos = position - dir * OFFSET_FROM_HERO;
            }
        }
    }
}

void Hero::render()
{
    Soldier::render();
}

void Hero::renderSelection()
{
    if (this == g_pGame->pMyHero)
    {
        OSB->drawSprite(OGetTexture("selected.png"), getSnapPos(), {1, .9f, .5f, 1}, 0, 1);
    }

    g_pGame->forEachInRadius(this, FOLLOW_TOGGLE_RANGE, [this](Unit* pUnit, float dis)
    {
        auto pSoldier = dynamic_cast<Soldier*>(pUnit);
        if (!pSoldier) return;
        if (pSoldier->bLocked) return;
        if (pSoldier->team != team) return;
        if (pSoldier->pFollow != this)
        {
            OSB->drawSprite(OGetTexture("selectedSmall.png"), pSoldier->getSnapPos(), {0, 1, 0, .75f}, 0, 1);
        }
        //else
        //{
        //    OSB->drawSprite(OGetTexture("selectedSmall.png"), pSoldier->getSnapPos(), {1, 0, 0, .75f}, 0, 1);
        //}
    });
}
