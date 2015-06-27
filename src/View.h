#pragma once

class View
{
public:
    virtual ~View() {}

    virtual void update() {}
    virtual void render() {}
};
