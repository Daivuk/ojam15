#pragma once
#include "onut.h"
#include "View.h"
#include "Unit.h"

#define MAX_UNITS 1000
#define MAP_SIZE 1280.f * 2.f
#define CHUNK_SIZE 128
#define CHUNK_COUNT ((int)(MAP_SIZE / CHUNK_SIZE) + 1)

class Chunk
{
public:
    Chunk()
    {
        pUnits = new TList<Unit>(offsetOf(&Unit::linkChunk));
    }

    ~Chunk()
    {
        delete pUnits;
    }

    TList<Unit> *pUnits = nullptr;
};

class Game : public View
{
public:
    Game();
    virtual ~Game();

    void update() override;
    void render() override;

    template<typename Tunit>
    Tunit *spawn(const Vector2 &position, int team = -1)
    {
        auto pUnit = pUnitPool->alloc<Tunit>();
        if (!pUnit) return nullptr;
        pUnit->position = position;
        pUnit->team = team;
        pUnit->onSpawn();

        // Insert at the proper Y position
        auto pOther = pUnits->Head();
        for (; pOther; pOther = pOther->linkMain.Next())
        {
            if (pUnit->position.y < pOther->position.y)
            {
                pUnits->InsertBefore(pUnit, pOther);
                break;
            }
        }
        if (!pOther)
        {
            pUnits->InsertTail(pUnit);
        }

        // Put him in the right chunk
        pUnit->pChunk = getChunkAt(position);
        pUnit->pChunk->pUnits->InsertTail(pUnit);

        return pUnit;
    }

    Chunk *getChunkAt(const Vector2 &pos)
    {
        if (pos.x < 0 || pos.y < 0 || pos.x >= MAP_SIZE || pos.y >= MAP_SIZE) return nullptr;
        int chunkX = (int)pos.x / CHUNK_SIZE;
        int chunkY = (int)pos.y / CHUNK_SIZE;
        auto chunkIdk = chunkY * CHUNK_COUNT + chunkX;
        return pChunks + chunkIdk;
    }

    OPool *pUnitPool = nullptr;
    TList<Unit> *pUnits = nullptr;
    Chunk *pChunks = nullptr;
    Vector2 camera;
};
