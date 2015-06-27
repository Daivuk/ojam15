#pragma once
#include "onut.h"

#define MAX_BULLETS 1000
#define BULLET_SPEED 1500
#define BULLET_GRAVITY 100 * 5

class Bullet
{
public:
    Bullet(const Vector2& from, const Vector2& to, float precision, int in_team, float dmg, bool in_isShell);

    bool update(); // Returns true when needs to be killed
    void render();

    void alertNearbyEnemies();

    LIST_LINK(Bullet) linkMain;
    Vector3 position;
    Vector3 velocity;
    Vector3 trail[3];
    Vector3 from;
    Vector3 to;
    int team;
    bool bMarkedForDeletion = false;
    float damage = 1.f;
    bool isShell = false;
    float shellProgress = 0.f;
};
