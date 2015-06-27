#pragma once
#include "onut.h"

#define MAX_BULLETS 1000
#define BULLET_SPEED 1500
#define BULLET_GRAVITY 100 * 5

class Bullet
{
public:
    Bullet(const Vector2& from, const Vector2& to, float precision, int in_team);

    bool update(); // Returns true when needs to be killed
    void render();

    LIST_LINK(Bullet) linkMain;
    Vector3 position;
    Vector3 velocity;
    Vector3 trail[3];
    int team;
    bool bMarkedForDeletion = false;
};
