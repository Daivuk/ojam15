#include "Hero.h"
#include "Game.h"

#define FOLLOW_TOGGLE_RANGE 64

auto g_actionSoundPlayTime = std::chrono::steady_clock::now();

Hero::Hero()
{
    life = 10.f;
    state = SOLDIER_STATE_CROUCHING;
}

void Hero::update()
{
    auto lt = OGamePad(0)->getLeftThumb();
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

    g_pGame->camera = position;

    // Order other units to follow
    if (OGamePad(0)->isJustPressed(onut::GamePad::eGamePad::A))
    {
        g_pGame->forEachInRadius(this, FOLLOW_TOGGLE_RANGE, [this](Unit* pUnit, float dis)
        {
            auto pSoldier = dynamic_cast<Soldier*>(pUnit);
            if (!pSoldier) return;
            if (pSoldier->team != team) return;
            if (pSoldier->pFollow == this) return;

            pSoldier->pFollow = this;

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
    if (OGamePad(0)->isJustPressed(onut::GamePad::eGamePad::B))
    {
        g_pGame->forEachInRadius(this, FOLLOW_TOGGLE_RANGE, [this](Unit* pUnit, float dis)
        {
            auto pSoldier = dynamic_cast<Soldier*>(pUnit);
            if (!pSoldier) return;
            if (pSoldier->team != team) return;
            if (pSoldier->pFollow != this) return;

            pSoldier->pFollow = nullptr;

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
        if (pSoldier->team != team) return;
        if (pSoldier->pFollow != this)
        {
            OSB->drawSprite(OGetTexture("selectedSmall.png"), pSoldier->getSnapPos(), {0, 1, 0, .75f}, 0, 1);
        }
        else
        {
            OSB->drawSprite(OGetTexture("selectedSmall.png"), pSoldier->getSnapPos(), {1, 0, 0, .75f}, 0, 1);
        }
    });
}