#include "Game.h"
#include "Soldier.h"
#include "Hero.h"
#include "Rifleman.h"
#include "Puff.h"
#include "Blood.h"
#include "Scout.h"
#include "Mortar.h"
#include "Smoke.h"
#include "MortarImpact.h"
#include "SandBag.h"

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
    biggest = std::max<>(biggest, sizeof(Mortar));
    biggest = std::max<>(biggest, sizeof(Smoke));
    biggest = std::max<>(biggest, sizeof(MortarImpact));
    biggest = std::max<>(biggest, sizeof(SandBag));
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

    auto pCollisionLayer = dynamic_cast<onut::TiledMap::sTileLayer*>(pTilemap->getLayer("collision"));
    collisions = new bool[pTilemap->getWidth() * pTilemap->getHeight()];
    memset(collisions, 0, sizeof(bool) * pTilemap->getWidth() * pTilemap->getHeight());
    for (int i = 0; i < pCollisionLayer->width * pCollisionLayer->height; ++i)
    {
        collisions[i] = (pCollisionLayer->tileIds[i]) ? true : collisions[i];
    }

    auto pObjLayer = dynamic_cast<onut::TiledMap::sObjectLayer*>(pTilemap->getLayer("units"));
    for (decltype(pObjLayer->objectCount) i = 0; i < pObjLayer->objectCount; ++i)
    {
        auto pMapObj = pObjLayer->pObjects + i;
        auto pos = (pMapObj->position + pMapObj->size * .5f) * UNIT_SCALE;
        if (pMapObj->type == "Hero")
        {
            pMyHero = spawn<Hero>(pos, TEAM_BLUE);
            camera = pos;
        }
        else if (pMapObj->type == "Sandbag")
        {
            spawn<SandBag>(pos, TEAM_NEUTRAL);
        }
        else if (pMapObj->type == "BlueSoldier")
        {
            spawn<Rifleman>(pos, TEAM_BLUE);
        }
        else if (pMapObj->type == "RedSoldier")
        {
            spawn<Rifleman>(pos, TEAM_RED);
        }
        else if (pMapObj->type == "BlueScout")
        {
            spawn<Scout>(pos, TEAM_BLUE);
        }
        else if (pMapObj->type == "RedScout")
        {
            spawn<Scout>(pos, TEAM_RED);
        }
        else if (pMapObj->type == "BlueMortar")
        {
            Rifleman *crew[2] = {
                spawn<Rifleman>(pos + Vector2(-8.f, -8.f), TEAM_BLUE),
                spawn<Rifleman>(pos + Vector2(8.f, -8.f), TEAM_BLUE)
            };
            crew[0]->pMortar = spawn<Mortar>(pos, TEAM_BLUE);
            crew[1]->bLocked = true;
            crew[0]->pMortar->pCrew1 = crew[0];
            crew[0]->pMortar->pCrew2 = crew[1];
            crew[0]->pMortar->pCrew1->fAttackRange = 700.f;
        }
        else if (pMapObj->type == "RedMortar")
        {
            Rifleman *crew[2] = {
                spawn<Rifleman>(pos + Vector2(-8.f, -8.f), TEAM_RED),
                spawn<Rifleman>(pos + Vector2(8.f, -8.f), TEAM_RED)
            };
            crew[0]->pMortar = spawn<Mortar>(pos, TEAM_RED);
            crew[1]->bLocked = true;
            crew[0]->pMortar->pCrew1 = crew[0];
            crew[0]->pMortar->pCrew2 = crew[1];
            crew[0]->pMortar->pCrew1->fAttackRange = 700.f;
        }
    }
}

Game::~Game()
{
    // Free stuff
    delete[] collisions;
    delete pBulletTexture;
    delete pBullets;
    delete pBulletPool;
    delete pUnits;
    delete pUnitPool;
    delete pTilemap;
}

#define KEYBOARD_SCROLL_SPEED 768
#define EDGE_SCROLL_SPEED 768
#define DRAG_SCROLL_SPEED -1
#define EDGE_SCROLL_SIZE 16

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

    // Do mouse/keyboard scrolling
    if (OInput->isStateDown(DIK_MOUSEB3))
    {
        if (OInput->isStateJustDown(DIK_MOUSEB3))
        {
            dragDownPos = OMousePos;
            dragDownCamPos = camera;
        }
        auto diff = OMousePos - dragDownPos;
        camera = dragDownCamPos + diff * DRAG_SCROLL_SPEED;
    }
    else
    {
        if (OInput->isStateDown(DIK_SPACE))
        {
            if (pMyHero)
            {
                camera = pMyHero->position;
            }
        }
        else
        {
            if (OMousePos.x >= 0 && OMousePos.x <= EDGE_SCROLL_SIZE)
            {
                camera.x -= EDGE_SCROLL_SPEED * ODT;
            }
            if (OMousePos.y >= 0 && OMousePos.y <= EDGE_SCROLL_SIZE)
            {
                camera.y -= EDGE_SCROLL_SPEED * ODT;
            }
            if (OMousePos.x <= OScreenWf && OMousePos.x >= OScreenWf - EDGE_SCROLL_SIZE)
            {
                camera.x += EDGE_SCROLL_SPEED * ODT;
            }
            if (OMousePos.y <= OScreenHf && OMousePos.y >= OScreenHf - EDGE_SCROLL_SIZE)
            {
                camera.y += EDGE_SCROLL_SPEED * ODT;
            }
            if (OInput->isStateDown(DIK_LEFT))
            {
                camera.x -= KEYBOARD_SCROLL_SPEED * ODT;
            }
            if (OInput->isStateDown(DIK_RIGHT))
            {
                camera.x += KEYBOARD_SCROLL_SPEED * ODT;
            }
            if (OInput->isStateDown(DIK_UP))
            {
                camera.y -= KEYBOARD_SCROLL_SPEED * ODT;
            }
            if (OInput->isStateDown(DIK_DOWN))
            {
                camera.y += KEYBOARD_SCROLL_SPEED * ODT;
            }
        }
    }

    // Clamp camera to map limits

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
            auto pSoldier = dynamic_cast<Soldier*>(pUnit);
            if (pSoldier)
            {
                if (pSoldier->pMortar)
                {
                    if (pSoldier->pMortar->pCrew2)
                    {
                        pSoldier->pMortar->pCrew2->bLocked = false;
                    }
                    pSoldier->pMortar->pCrew1 = nullptr;
                    pSoldier->pMortar->pCrew2 = nullptr;
                    pSoldier->pMortar = nullptr;
                }
            }

            // Un link others
            for (auto pUnit2 = pUnits->Head(); pUnit2; pUnit2 = pUnits->Next(pUnit2))
            {
                if (pUnit2 == pUnit) continue;
                if (pUnit2->pFollow == pUnit)
                {
                    pUnit2->pFollow = nullptr;
                }
                pSoldier = dynamic_cast<Soldier*>(pUnit2);
                if (pSoldier)
                {
                    if (pSoldier->pMortar)
                    {
                        if (pSoldier->pMortar->pCrew2 == pUnit)
                        {
                            pSoldier->pMortar->pCrew1 = nullptr;
                            pSoldier->pMortar->pCrew2 = nullptr;
                            pSoldier->pMortar = nullptr;
                        }
                    }
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
    ORenderer->clear(Color{0, 0, 0, 1});
    ORenderer->resetState();

    egModelPush();

    // Draw map
    RECT rect;
    rect.left = static_cast<LONG>(((camera.x - OScreenWf * .5f)) / ((float)UNIT_SCALE* 8.f));
    rect.top = static_cast<LONG>(((camera.y - OScreenHf * .5f)) / ((float)UNIT_SCALE* 8.f));
    rect.right = static_cast<LONG>(((camera.x + OScreenWf)) / ((float)UNIT_SCALE* 8.f));
    rect.bottom = static_cast<LONG>(((camera.y + OScreenHf)) / ((float)UNIT_SCALE* 8.f));
    Matrix transform = Matrix::Identity;
    transform *= Matrix::CreateScale(UNIT_SCALE);
    transform *= Matrix::CreateTranslation(cameraOffset.x, cameraOffset.y, 0);
    pTilemap->setTransform(transform);
    pTilemap->render(rect);

    // Draw units
    egModelPush();
    OSB->begin();
    egModelTranslate(cameraOffset.x, cameraOffset.y, 0);
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

    egModelPop();
}

void Game::forEachInRadius(Unit *pMyUnit, float fRadius, const std::function<void(Unit*, float)>& callback)
{
    // Apply steering behaviour
    int chunkFromX = (int)(pMyUnit->position.x - fRadius) / CHUNK_SIZE;
    int chunkFromY = (int)(pMyUnit->position.y - fRadius) / CHUNK_SIZE;
    int chunkToX = (int)(pMyUnit->position.x + fRadius) / CHUNK_SIZE;
    int chunkToY = (int)(pMyUnit->position.y + fRadius) / CHUNK_SIZE;
    chunkFromX = std::min<>(CHUNK_COUNT - 1, std::max<>(0, chunkFromX));
    chunkFromY = std::min<>(CHUNK_COUNT - 1, std::max<>(0, chunkFromY));
    chunkToX = std::min<>(CHUNK_COUNT - 1, std::max<>(0, chunkToX));
    chunkToY = std::min<>(CHUNK_COUNT - 1, std::max<>(0, chunkToY));

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
    chunkFromX = std::min<>(CHUNK_COUNT - 1, std::max<>(0, chunkFromX));
    chunkFromY = std::min<>(CHUNK_COUNT - 1, std::max<>(0, chunkFromY));
    chunkToX = std::min<>(CHUNK_COUNT - 1, std::max<>(0, chunkToX));
    chunkToY = std::min<>(CHUNK_COUNT - 1, std::max<>(0, chunkToY));

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

void Game::spawnBullet(const Vector2& from, const Vector2& to, float precision, int in_team, float damage, bool isShell)
{
    auto pBullet = pBulletPool->alloc<Bullet>(from, to, precision, in_team, damage, isShell);
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

bool Game::collisionAt(const Vector2& pos) const
{
    int x = (int)(pos.x / (UNIT_SCALE * 8.f));
    int y = (int)(pos.y / (UNIT_SCALE * 8.f));
    if (x < 0) return true;
    if (y < 0) return true;
    if (x >= pTilemap->getWidth()) return true;
    if (y >= pTilemap->getHeight()) return true;

    return collisions[x + y * pTilemap->getWidth()];
}
