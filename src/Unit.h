#pragma once
#include "onut.h"

#define UNIT_SCALE 5

#define DIRECTION_UP 0
#define DIRECTION_RIGHT 1
#define DIRECTION_DOWN 2
#define DIRECTION_LEFT 3

#define TEAM_NEUTRAL 0
#define TEAM_RED 1
#define TEAM_BLUE 2

const Color TEAM_COLOR[3] = {{1, 1, 1, 1}, OColorHex(da5d5d), OColorHex(3f8afa)};

class Chunk;

class Unit
{
public:
    Unit() {}
    virtual ~Unit() {}

    virtual void update() {}
    virtual void render() {}
    virtual void onSpawn() {}

    LIST_LINK(Unit) linkMain;
    LIST_LINK(Unit) linkChunk;
    Vector2 position;
    int team = TEAM_NEUTRAL;
    int state = 0;
    bool bMarkedForDeletion = false;
    float fRadius = 16.f;
    Chunk *pChunk = nullptr;
};
