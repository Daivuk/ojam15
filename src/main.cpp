#include <Windows.h>
#include "onut.h"
#include "View.h"
#include "Game.h"

View *g_pCurrentView = nullptr;

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
    OSettings->setResolution({1280, 920});
    OSettings->setGameName("Ottawa Game Jam 2015 - It's not safe to go alone");
    OSettings->setIsResizableWindow(true);

    onut::run(init, update, render);
}

void init()
{
    g_pCurrentView = new Game();
}

void update()
{
    g_pCurrentView->update();
}

void render()
{
    g_pCurrentView->render();
}
