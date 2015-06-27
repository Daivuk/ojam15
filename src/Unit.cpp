#include "Unit.h"

Vector2 Unit::getSnapPos() const
{
    Vector2 ret;

    ret.x = std::roundf(position.x / 2) * 2;
    ret.y = std::roundf(position.y / 2) * 2;

    return std::move(ret);
}
