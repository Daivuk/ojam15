#include "Game.h"
#include "Soldier.h"
#include "Hero.h"

Game *g_pGame;

Game::Game()
{
    g_pGame = this;

    // Create unit pool
    auto biggest = sizeof(Unit);
    biggest = std::max<>(biggest, sizeof(Soldier));
    biggest = std::max<>(biggest, sizeof(Hero));
    pUnitPool = new OPool(biggest, MAX_UNITS);

    // Create unit list
    pUnits = new TList<Unit>(offsetOf(&Unit::linkMain));

    // Create chunks
    pChunks = new Chunk[CHUNK_COUNT * CHUNK_COUNT];

    // Spawn units randomly
    spawn<Soldier>({100, 100}, TEAM_BLUE);
    spawn<Soldier>({132, 103}, TEAM_BLUE);
    spawn<Soldier>({116, 132}, TEAM_BLUE);

    spawn<Soldier>({600, 400}, TEAM_RED);
    spawn<Soldier>({632, 403}, TEAM_RED);
    spawn<Soldier>({616, 432}, TEAM_RED);

    spawn<Hero>({(132 + 600) / 2, (132 + 400) / 2}, TEAM_BLUE);
}

Game::~Game()
{
    // Free stuff
    delete pUnits;
    delete pUnitPool;
}

void Game::update()
{
    // Update units
    for (auto pUnit = pUnits->Head(); pUnit; pUnit = pUnits->Next(pUnit))
    {
        pUnit->update();

        // Check if we should change chunk
        auto pChunk = getChunkAt(pUnit->position);
        if (pChunk && pChunk != pUnit->pChunk)
        {
            pUnit->pChunk = pChunk;
            pChunk->pUnits->InsertTail(pUnit);
        }
    }

    // Delete those that need to be
    for (auto pUnit = pUnits->Head(); pUnit; )
    {
        if (pUnit->bMarkedForDeletion)
        {
            auto pToDelete = pUnit;
            pUnit = pUnits->Next(pUnit);
            delete pToDelete;
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
    // Clear screen
    ORenderer->clear(Color{.15f, .25f, .20f, 1} * 2);

    // Draw units
    egModelPush();
    egModelTranslate(-camera.x, -camera.y, 0);
    OSB->begin();
    egFilter(EG_FILTER_NEAREST);
    for (auto pUnit = pUnits->Head(); pUnit; pUnit = pUnits->Next(pUnit))
    {
        pUnit->render();
    }
    OSB->end();
    egModelPop();
}
