#include "Game.h"
#include "Soldier.h"
#include "Hero.h"
#include "Rifleman.h"
#include "Puff.h"
#include "Blood.h"
#include "Scout.h"

Game *g_pGame;

Game::Game()
{
    g_pGame = this;

    // Create unit pool
    auto biggest = sizeof(Unit);
    biggest = std::max<>(biggest, sizeof(Soldier));
    biggest = std::max<>(biggest, sizeof(Hero));
    biggest = std::max<>(biggest, sizeof(Rifleman));
    biggest = std::max<>(biggest, sizeof(Puff));
    biggest = std::max<>(biggest, sizeof(Blood));
    biggest = std::max<>(biggest, sizeof(Scout));
    pUnitPool = new OPool(biggest, MAX_UNITS);

    // Create unit list
    pUnits = new TList<Unit>(offsetOf(&Unit::linkMain));

    // Create bullets
    pBulletPool = new OPool(sizeof(Bullet), MAX_BULLETS);
    pBullets = new TList<Bullet>(offsetOf(&Bullet::linkMain));

    // Create chunks
    pChunks = new Chunk[CHUNK_COUNT * CHUNK_COUNT];

    // Load map
    pTilemap = new onut::TiledMap("../../assets/maps/bg.tmx");
    auto pObjLayer = dynamic_cast<onut::TiledMap::sObjectLayer*>(pTilemap->getLayer("units"));
    for (decltype(pObjLayer->objectCount) i = 0; i < pObjLayer->objectCount; ++i)
    {
        auto pMapObj = pObjLayer->pObjects + i;
        auto pos = (pMapObj->position + pMapObj->size * .5f) * UNIT_SCALE;
        if (pMapObj->type == "Hero")
        {
            pMyHero = spawn<Hero>(pos, TEAM_BLUE);
        }
        else if (pMapObj->type == "BlueSoldier")
        {
            if (onut::randb())
            {
                spawn<Rifleman>(pos, TEAM_BLUE);
            }
            else
            {
                spawn<Scout>(pos, TEAM_BLUE);
            }
        }
        else if (pMapObj->type == "RedSoldier")
        {
            spawn<Rifleman>(pos, TEAM_RED);
        }
    }
}

Game::~Game()
{
    // Free stuff
    delete pBulletTexture;
    delete pBullets;
    delete pBulletPool;
    delete pUnits;
    delete pUnitPool;
    delete pTilemap;
}

void Game::update()
{
    // Update units
    for (auto pUnit = pUnits->Head(); pUnit; pUnit = pUnits->Next(pUnit))
    {
        pUnit->update();

        // Check if we should change chunk
        if (pUnit->bChunkIt)
        {
            auto pChunk = getChunkAt(pUnit->position);
            if (pChunk && pChunk != pUnit->pChunk)
            {
                pUnit->pChunk = pChunk;
                pChunk->pUnits->InsertTail(pUnit);
            }
        }
    }

    // Update bullets
    for (auto pBullet = pBullets->Head(); pBullet;)
    {
        if (pBullet->update())
        {
            auto pToDelete = pBullet;
            pBullet = pBullets->Next(pBullet);
            pBulletPool->dealloc(pToDelete);
            continue;
        }
        pBullet = pBullets->Next(pBullet);
    }

    // Delete those that need to be
    for (auto pUnit = pUnits->Head(); pUnit; )
    {
        if (pUnit->bMarkedForDeletion)
        {
            // Un link others
            for (auto pUnit2 = pUnits->Head(); pUnit2; pUnit2 = pUnits->Next(pUnit2))
            {
                if (pUnit2 == pUnit) continue;
                if (pUnit2->pFollow == pUnit)
                {
                    pUnit2->pFollow = nullptr;
                }
            }

            auto pToDelete = pUnit;
            pUnit = pUnits->Next(pUnit);

            pUnitPool->dealloc(pToDelete);
            continue;
        }
        pUnit = pUnits->Next(pUnit);
    }

    // Reorder units based on the y position
    if (pUnits->Head())
    {
        auto prev = pUnits->Head();
        auto it = prev;
        it = it->linkMain.Next();
        for (; it; it = it->linkMain.Next())
        {
            auto pPrevUnit = prev;
            auto pUnit = it;
            if (pUnit->position.y < pPrevUnit->position.y)
            {
                it->linkMain.InsertBefore(it, &prev->linkMain);
            }
            prev = it;
        }
    }
}

void Game::render()
{
    Vector2 cameraOffset{-camera.x + OScreenWf * .5f, -camera.y + OScreenHf *.5f};
    cameraOffset.x = std::roundf(cameraOffset.x / 2) * 2;
    cameraOffset.y = std::roundf(cameraOffset.y / 2) * 2;

    // Render bullets offscreen
    if (!pBulletTexture)
    {
        pBulletTexture = OTexture::createRenderTarget({OSettings->getResolution().x / UNIT_SCALE, OSettings->getResolution().y / UNIT_SCALE});
    }

    ORenderer->bindRenderTarget(pBulletTexture);
    egStatePush();
    OSB->begin();
    OSB->drawRect(nullptr, {0, 0, (float)OSettings->getResolution().x / UNIT_SCALE, (float)OSettings->getResolution().y / UNIT_SCALE}, Color::Black);
    OSB->end();
    egModelPush();
    egModelTranslate(-camera.x, -camera.y, 0);
    egModelScale(1.f / UNIT_SCALE, 1.f / UNIT_SCALE, 1.f);
    egModelTranslate(OScreenWf * .5f / UNIT_SCALE, OScreenHf *.5f / UNIT_SCALE, 0);
    OPB->begin(onut::ePrimitiveType::LINES);
    for (auto pBullet = pBullets->Head(); pBullet; pBullet = pBullets->Next(pBullet))
    {
        pBullet->render();
    }
    OPB->end();
    egModelPop();
    egPostProcess();
    egStatePop();
    ORenderer->bindRenderTarget();

    // Clear screen
    ORenderer->clear(Color{.15f, .25f, .20f, 1} * 2);
    ORenderer->resetState();

    // Draw map
    RECT rect;
    rect.left = static_cast<LONG>(((camera.x - OScreenWf * .5f)) / (UNIT_SCALE* 8.f));
    rect.top = static_cast<LONG>(((camera.y - OScreenHf * .5f)) / (UNIT_SCALE* 8.f));
    rect.right = static_cast<LONG>(((camera.x + OScreenWf)) / (UNIT_SCALE* 8.f));
    rect.bottom = static_cast<LONG>(((camera.y + OScreenHf)) / (UNIT_SCALE* 8.f));
    Matrix transform = Matrix::Identity;
    transform *= Matrix::CreateScale(UNIT_SCALE);
    transform *= Matrix::CreateTranslation(-camera.x, -camera.y, 0);
    pTilemap->setTransform(transform);
    pTilemap->render(rect);

    // Draw units
    egModelPush();
    egModelTranslate(cameraOffset.x, cameraOffset.y, 0);
    OSB->begin();
    egFilter(EG_FILTER_NEAREST);

    for (auto pUnit = pUnits->Head(); pUnit; pUnit = pUnits->Next(pUnit))
    {
        pUnit->renderSelection();
    }

    for (auto pUnit = pUnits->Head(); pUnit; pUnit = pUnits->Next(pUnit))
    {
        pUnit->render();
    }
    OSB->end();
    egModelPop();

    // Draw bullets overlay
    egStatePush();
    egBlendFunc(EG_ONE, EG_ONE);
    OSB->begin();
    OSB->drawRect(pBulletTexture, {0, 0, (float)OSettings->getResolution().x, (float)OSettings->getResolution().y});
    OSB->end();
    egStatePop();
}

void Game::forEachInRadius(Unit *pMyUnit, float fRadius, const std::function<void(Unit*, float)>& callback)
{
    // Apply steering behaviour
    int chunkFromX = (int)(pMyUnit->position.x - fRadius) / CHUNK_SIZE;
    int chunkFromY = (int)(pMyUnit->position.y - fRadius) / CHUNK_SIZE;
    int chunkToX = (int)(pMyUnit->position.x + fRadius) / CHUNK_SIZE;
    int chunkToY = (int)(pMyUnit->position.y + fRadius) / CHUNK_SIZE;
    chunkFromX = std::min<>(CHUNK_COUNT, std::max<>(0, chunkFromX));
    chunkFromY = std::min<>(CHUNK_COUNT, std::max<>(0, chunkFromY));
    chunkToX = std::min<>(CHUNK_COUNT, std::max<>(0, chunkToX));
    chunkToY = std::min<>(CHUNK_COUNT, std::max<>(0, chunkToY));

    for (int chunkY = chunkFromY; chunkY <= chunkToY; ++chunkY)
    {
        for (int chunkX = chunkFromX; chunkX <= chunkToX; ++chunkX)
        {
            auto pChunk = g_pGame->pChunks + (chunkY * CHUNK_COUNT + chunkX);
            for (auto pUnit = pChunk->pUnits->Head(); pUnit; pUnit = pChunk->pUnits->Next(pUnit))
            {
                if (pUnit == pMyUnit) continue;
                float dis = Vector2::DistanceSquared(pMyUnit->position, pUnit->position);
                if (dis <= fRadius * fRadius)
                {
                    dis = sqrtf(dis);
                    callback(pUnit, dis);
                }
            }
        }
    }
}

void Game::forEachInRadius(const Vector2 &position, float fRadius, const std::function<void(Unit*, float)>& callback)
{
    // Apply steering behaviour
    int chunkFromX = (int)(position.x - fRadius) / CHUNK_SIZE;
    int chunkFromY = (int)(position.y - fRadius) / CHUNK_SIZE;
    int chunkToX = (int)(position.x + fRadius) / CHUNK_SIZE;
    int chunkToY = (int)(position.y + fRadius) / CHUNK_SIZE;
    chunkFromX = std::min<>(CHUNK_COUNT, std::max<>(0, chunkFromX));
    chunkFromY = std::min<>(CHUNK_COUNT, std::max<>(0, chunkFromY));
    chunkToX = std::min<>(CHUNK_COUNT, std::max<>(0, chunkToX));
    chunkToY = std::min<>(CHUNK_COUNT, std::max<>(0, chunkToY));

    for (int chunkY = chunkFromY; chunkY <= chunkToY; ++chunkY)
    {
        for (int chunkX = chunkFromX; chunkX <= chunkToX; ++chunkX)
        {
            auto pChunk = g_pGame->pChunks + (chunkY * CHUNK_COUNT + chunkX);
            for (auto pUnit = pChunk->pUnits->Head(); pUnit; pUnit = pChunk->pUnits->Next(pUnit))
            {
                float dis = Vector2::DistanceSquared(position, pUnit->position);
                if (dis <= fRadius * fRadius)
                {
                    dis = sqrtf(dis);
                    callback(pUnit, dis);
                }
            }
        }
    }
}

void Game::spawnBullet(const Vector2& from, const Vector2& to, float precision, int in_team, float damage)
{
    auto pBullet = pBulletPool->alloc<Bullet>(from, to, precision, in_team, damage);
    if (!pBullet) return;
    pBullets->InsertTail(pBullet);
}

void Game::playSound(OSound *pSound, const Vector2 &position, float volume)
{
    // Define the visible rectangle
    Rect rect;
    rect.x = (camera.x - OScreenWf * .5f);
    rect.y = (camera.y - OScreenHf * .5f);
    rect.z = (camera.x + OScreenWf * .5f);
    rect.w = (camera.y + OScreenHf * .5f);
    rect.z -= rect.x;
    rect.w -= rect.y;

    float balance = position.x - camera.x;
    balance /= (OScreenWf);

    if (rect.Contains(position))
    {
        pSound->play(volume, balance);
    }
    else
    {
        float dis = rect.Distance(position);
        dis = 1 - dis / (OScreenWf * .5f);
        if (dis > 0)
        {
            pSound->play(dis * volume, balance);
        }
    }
}
