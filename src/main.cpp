#include <Windows.h>
#include "onut.h"

void init();
void update();
void render();

int CALLBACK WinMain(
    _In_  HINSTANCE hInstance,
    _In_  HINSTANCE hPrevInstance,
    _In_  LPSTR lpCmdLine,
    _In_  int nCmdShow
    )
{
    OSettings->setResolution({1280, 720});
    OSettings->setGameName("Ottawa Game Jam 2015 - It's not safe to go alone");
    OSettings->setIsResizableWindow(true);

    onut::run(init, update, render);
}

void init()
{
}

void update()
{
}

void render()
{
}
