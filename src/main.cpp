#include <Windows.h>
#include "onut.h"
#include "View.h"
#include "Game.h"

Game *pGame = nullptr;
onut::UIContext *pContext;
onut::UIControl *pScreen;

std::string sCurrentMap = "../../assets/maps/bg.tmx";

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
    OSettings->setBorderlessFullscreen(true);

    onut::run(init, update, render);
}

void hookButtonSounds(onut::UIControl *pCtrl)
{
    if (dynamic_cast<onut::UIButton *>(pCtrl))
    {
        pCtrl->onMouseEnter = [](onut::UIControl* pCtrl, const onut::UIMouseEvent& evt)
        {
            OPlaySound("buttonHover.wav");
        };
        pCtrl->onClick = [](onut::UIControl* pCtrl, const onut::UIMouseEvent& evt)
        {
            OPlaySound("buttonClick.wav");
        };
    }
    if (dynamic_cast<onut::UITextBox *>(pCtrl))
    {
        pCtrl->onMouseEnter = [](onut::UIControl* pCtrl, const onut::UIMouseEvent& evt)
        {
            OPlaySound("buttonHover.wav");
        };
        pCtrl->onClick = [](onut::UIControl* pCtrl, const onut::UIMouseEvent& evt)
        {
            OPlaySound("txtClick.wav");
        };
    }

    for (auto pChild : pCtrl->getChildren())
    {
        hookButtonSounds(pChild);
    }
}

void init()
{
    pContext = new onut::UIContext(onut::sUIVector2{OScreenWf, OScreenHf});
    pScreen = new onut::UIControl("../../assets/ui/main.json");
    pScreen->retain();
    hookButtonSounds(pScreen);
    //pGame = new Game();

    pContext->onClipping = [](bool enabled, const onut::sUIRect& rect)
    {
        OSB->end();
        ORenderer->setScissor(enabled, onut::UI2Onut(rect));
        OSB->begin();
    };

    pContext->addStyle<onut::UIPanel>("blur", [](const onut::UIPanel* pControl, const onut::sUIRect& rect)
    {
        OSB->end();
        egEnable(EG_BLUR);
        egBlur(64.f);
        egPostProcess();
        ORenderer->resetState();
        OSB->begin();
        OSB->drawRect(nullptr, onut::UI2Onut(rect), Color(0, 0, 0, .35f));
    });

    auto getTextureForState = [](onut::UIControl *pControl, const std::string &filename)
    {
        static std::string stateFilename;
        stateFilename = filename;
        OTexture *pTexture;
        switch (pControl->getState(*pContext))
        {
            case onut::eUIState::NORMAL:
                pTexture = OGetTexture(filename.c_str());
                break;
            case onut::eUIState::DISABLED:
                stateFilename.insert(filename.size() - 4, "_disabled");
                pTexture = OGetTexture(stateFilename.c_str());
                if (!pTexture) pTexture = OGetTexture(filename.c_str());
                break;
            case onut::eUIState::HOVER:
                stateFilename.insert(filename.size() - 4, "_hover");
                pTexture = OGetTexture(stateFilename.c_str());
                if (!pTexture) pTexture = OGetTexture(filename.c_str());
                break;
            case onut::eUIState::DOWN:
                stateFilename.insert(filename.size() - 4, "_down");
                pTexture = OGetTexture(stateFilename.c_str());
                if (!pTexture) pTexture = OGetTexture(filename.c_str());
                break;
        }
        return pTexture;
    };

    pContext->addStyle<onut::UIImage>("spinner", [=](const onut::UIImage* pControl, const onut::sUIRect& rect)
    {
        static uint32_t frameAnim = 0;
        frameAnim++;
        auto frame = (frameAnim / 6) % 8;
        Vector4 uvs{
            static_cast<float>(frame) / 8.f,
            0,
            static_cast<float>(frame + 1) / 8.f,
            1
        };
        OSB->drawRectWithUVs(getTextureForState((onut::UIControl *)pControl, pControl->scale9Component.image.filename),
                             onut::UI2Onut(rect),
                             uvs,
                             onut::UI2Onut(pControl->scale9Component.image.color));
    });

    pContext->drawRect = [=](onut::UIControl *pControl, const onut::sUIRect &rect, const onut::sUIColor &color)
    {
        OSB->drawRect(nullptr, onut::UI2Onut(rect), onut::UI2Onut(color));
    };

    pContext->drawTexturedRect = [=](onut::UIControl *pControl, const onut::sUIRect &rect, const onut::sUIImageComponent &image)
    {
        OSB->drawRect(getTextureForState(pControl, image.filename),
                      onut::UI2Onut(rect),
                      onut::UI2Onut(image.color));
    };

    pContext->drawScale9Rect = [=](onut::UIControl* pControl, const onut::sUIRect& rect, const onut::sUIScale9Component& scale9)
    {
        const std::string &filename = scale9.image.filename;
        OTexture *pTexture;
        switch (pControl->getState(*pContext))
        {
            case onut::eUIState::NORMAL:
                pTexture = OGetTexture(filename.c_str());
                break;
            case onut::eUIState::DISABLED:
                pTexture = OGetTexture((filename + "_disabled").c_str());
                if (!pTexture) pTexture = OGetTexture(filename.c_str());
                break;
            case onut::eUIState::HOVER:
                pTexture = OGetTexture((filename + "_hover").c_str());
                if (!pTexture) pTexture = OGetTexture(filename.c_str());
                break;
            case onut::eUIState::DOWN:
                pTexture = OGetTexture((filename + "_down").c_str());
                if (!pTexture) pTexture = OGetTexture(filename.c_str());
                break;
        }
        if (scale9.isRepeat)
        {
            OSB->drawRectScaled9RepeatCenters(getTextureForState(pControl, scale9.image.filename),
                                              onut::UI2Onut(rect),
                                              onut::UI2Onut(scale9.padding),
                                              onut::UI2Onut(scale9.image.color));
        }
        else
        {
            OSB->drawRectScaled9(getTextureForState(pControl, scale9.image.filename),
                                 onut::UI2Onut(rect),
                                 onut::UI2Onut(scale9.padding),
                                 onut::UI2Onut(scale9.image.color));
        }
    };

    pContext->drawText = [=](onut::UIControl* pControl, const onut::sUIRect& rect, const onut::sUITextComponent& text)
    {
        if (text.text.empty()) return;
        auto align = onut::UI2Onut(text.font.align);
        auto oRect = onut::UI2Onut(rect);
        auto pFont = OGetBMFont(text.font.typeFace.c_str());
        auto oColor = onut::UI2Onut(text.font.color);
        if (pControl->getState(*pContext) == onut::eUIState::DISABLED)
        {
            oColor = {.4f, .4f, .4f, 1};
        }
        oColor.Premultiply();

        if (pFont)
        {
            if (pControl->getStyleName() == "password")
            {
                std::string pwd;
                pwd.resize(text.text.size(), '*');
                if (pControl->hasFocus(*pContext) && ((onut::UITextBox*)pControl)->isCursorVisible())
                {
                    pwd.back() = '_';
                }
                pFont->draw<>(pwd, ORectAlign<>(oRect, align), oColor, OSB, align);
            }
            else
            {
                pFont->draw<>(text.text, ORectAlign<>(oRect, align), oColor, OSB, align);
            }
        }
    };
    OWindow->onWrite = [](char c)
    {
        pContext->write(c);
    };
    OWindow->onKey = [](uintptr_t key)
    {
        pContext->keyDown(key);
    };

    pScreen->getChild<onut::UIButton>("btnQuit")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        PostQuitMessage(0);
    };
    pScreen->getChild<onut::UIButton>("btnMission")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        pScreen->getChild("menuMain")->isVisible = false;
        sCurrentMap = "../../assets/maps/bg.tmx";
        pGame = new Game(sCurrentMap);
    };
    pScreen->getChild<onut::UIButton>("btnSandbox")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        pScreen->getChild("menuMain")->isVisible = false;
        sCurrentMap = "../../assets/maps/sandbox.tmx";
        pGame = new Game(sCurrentMap);
    };
    pScreen->getChild<onut::UIButton>("btnHowToPlay")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        pScreen->getChild("menuHelp")->isVisible = true;
    };
    pScreen->getChild("menuHelp")->getChild<onut::UIButton>("btnBack")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        pScreen->getChild("menuHelp")->isVisible = false;
        pScreen->getChild("menuMain")->isVisible = true;
    };

    pScreen->getChild("menuGame")->getChild<onut::UIButton>("btnQuit")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        PostQuitMessage(0);
    };
    pScreen->getChild("menuGame")->getChild<onut::UIButton>("btnReturnToMission")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        pScreen->getChild("menuGame")->isVisible = false;
    };
    pScreen->getChild("menuGame")->getChild<onut::UIButton>("btnQuitMission")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        pScreen->getChild("menuGame")->isVisible = false;
        delete pGame;
        pGame = nullptr;
        pScreen->getChild("menuMain")->isVisible = true;
    };
    pScreen->getChild("menuGame")->getChild<onut::UIButton>("btnRestartMission")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        pScreen->getChild("menuGame")->isVisible = false;
        delete pGame;
        pGame = new Game(sCurrentMap);
    };

    pScreen->getChild("menuVictory")->getChild<onut::UIButton>("btnQuit")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        PostQuitMessage(0);
    };
    pScreen->getChild("menuVictory")->getChild<onut::UIButton>("btnQuitMission")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        pScreen->getChild("menuVictory")->isVisible = false;
        delete pGame;
        pGame = nullptr;
        pScreen->getChild("menuMain")->isVisible = true;
    };
    pScreen->getChild("menuVictory")->getChild<onut::UIButton>("btnRestartMission")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        pScreen->getChild("menuVictory")->isVisible = false;
        delete pGame;
        pGame = new Game(sCurrentMap);
    };

    pScreen->getChild("menuDead")->getChild<onut::UIButton>("btnQuit")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        PostQuitMessage(0);
    };
    pScreen->getChild("menuDead")->getChild<onut::UIButton>("btnQuitMission")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        pScreen->getChild("menuDead")->isVisible = false;
        delete pGame;
        pGame = nullptr;
        pScreen->getChild("menuMain")->isVisible = true;
    };
    pScreen->getChild("menuDead")->getChild<onut::UIButton>("btnRestartMission")->onClick = [](onut::UIControl*, const onut::UIMouseEvent&)
    {
        OPlaySound("buttonClick.wav");
        pScreen->getChild("menuDead")->isVisible = false;
        delete pGame;
        pGame = new Game(sCurrentMap);
    };

    pScreen->getChild("menuMain")->isVisible = true;
}

void update()
{
    if (pGame)
    {
        if (OInput->isStateJustDown(DIK_ESCAPE) &&
            !pScreen->getChild("menuVictory")->isVisible)
        {
            pScreen->getChild("menuGame")->isVisible = !pScreen->getChild("menuGame")->isVisible;
        }
        if (!pScreen->getChild("menuGame")->isVisible &&
            !pScreen->getChild("menuVictory")->isVisible)
        {
            pGame->update();
        }
    }

    pScreen->update(*pContext, {OMousePos.x, OMousePos.y}, OInput->isStateDown(DIK_MOUSEB1), OInput->isStateDown(DIK_MOUSEB2), OInput->isStateDown(DIK_MOUSEB3));
}

void render()
{
    ORenderer->clear(Color::Black);
    if (pGame) pGame->render();
    OSB->begin();
    pScreen->render(*pContext);
    OSB->end();
}
