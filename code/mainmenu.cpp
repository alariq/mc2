#define MAINMENU_CPP

#ifdef LINUX_BUILD
#include "platform_windows.h"
struct DDSURFACEDESC2 {};
#else
#include <windows.h>
#include <ddraw.h>
#endif
#include "mainmenu.h"
#include "mclib.h"
#include "inifile.h"
#include <filesystem>
#include <vector>
#include "logisticsdata.h"
#include "logisticsdialog.h"
#include "abutton.h"
#include "optionsscreenwrapper.h"
#include "../resource.h"
#include "mechlopedia.h"
#include "gamesound.h"
#include "aanimobject.h"
#include "multplyr.h"
#include <algorithm>
#include <mp4player.h>
#include <iostream>

#include "prefs.h"
extern CPrefs prefs;

#define MM_MSG_NEW_CAMPAIGN 90
#define MM_MSG_SAVE 92
#define MM_MSG_LOAD 91
#define MM_MSG_MULTIPLAYER 93
#define MM_MSG_RETURN_TO_GAME 94
#define MM_MSG_OPTIONS 95
#define MM_MSG_ENCYCLOPEDIA 96
#define MM_MSG_EXIT 97
#define MM_MSG_SINGLE_MISSION 98
#define MM_MSG_LEGAL 99

extern volatile bool mc2IsInMouseTimer;
extern volatile bool mc2IsInDisplayBackBuffer;

void MouseTimerKill();
extern void (*AsynFunc)(RECT& WinRect, DDSURFACEDESC2& mouseSurfaceDesc);
extern bool bInvokeOptionsScreenFlag;
bool MainMenu::bDrawMechlopedia = false;

void SplashIntro::init()
{
    FullPathFileName path;
    path.init(artPath, "mcl_splashscreenintro", ".fit");

    FitIniFile file;
    if (NO_ERR != file.open(path))
    {
        char errorStr[256];
        sprintf(errorStr, "couldn't open file %s", (const char*)path);
        Assert(0, 0, errorStr);
    }

    LogisticsScreen::init(file, "Static", "Text", "Rect", "Button");
}

MainMenu::MainMenu()
{
    optionsScreenWrapper = NULL;
    bOptions = 0;
    bSave = bLoad = 0;
    helpTextArrayID = 0;
    mechlopedia = 0;
    bDrawMechlopedia = 0;
    tuneId = -1;
    bLoadSingle = 0;
    bLoadCampaign = 0;
    introOver = 0;
    bHostLeftDlg = 0;
    videoFinished = false;
    currentVideoIndex = 0; // Start with first video
    introMP4Player = 0;
}

MainMenu::~MainMenu()
{
    if (optionsScreenWrapper)
        delete optionsScreenWrapper;
    if (introMP4Player)
        delete introMP4Player;
}

int MainMenu::init(FitIniFile& file)
{
    file.seekBlock("Tunes");
    file.readIdLong("TuneId", tuneId);

    LogisticsScreen::init(file, "Static", "Text", "Rect", "Button");

    FullPathFileName name;
    name.init(artPath, "mcl_sp", ".fit");
    FitIniFile file2;
    if (NO_ERR != file2.open(name))
    {
        char errorStr[256];
        sprintf(errorStr, "couldn't open file %s", (const char*)name);
        Assert(0, 0, errorStr);
    }

    background.init(file2, "Static", "Text", "Rect", "Button");

    for (int i = 0; i < buttonCount; i++)
    {
        buttons[i].setMessageOnRelease();
        buttons[i].setPressFX(LOG_VIDEOBUTTONS);
        buttons[i].setHighlightFX(LOG_DIGITALHIGHLIGHT);
    }

    beginAnim.initWithBlockName(&file, "InAnim");
    endAnim.initWithBlockName(&file, "OutAnim");

    intro.init();

    FullPathFileName path;
    path.init(artPath, "mcl_mp_loadmap", ".fit");

    FitIniFile mpFile;
    if (NO_ERR != mpFile.open(path))
    {
        char error[256];
        sprintf(error, "couldn't open file %s", (const char*)path);
        Assert(0, 0, error);
        return -1;
    }

    singleLoadDlg.init(&mpFile);

    SDL_Window* gameWindow = SDL_GL_GetCurrentWindow();
    SDL_GLContext gameContext = SDL_GL_GetCurrentContext();

    SDL_SetWindowFullscreen(gameWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    int windowWidth, windowHeight;
    SDL_GetWindowSize(gameWindow, &windowWidth, &windowHeight);
    std::cout << "[MainMenu] Forced fullscreen, window size: " << windowWidth << "x" << windowHeight << "\n";

    return 0;
}

void MainMenu::begin()
{
    status = RUNNING;
    promptToQuit = 0;
    beginAnim.begin();
    beginFadeIn(0);
    endAnim.end();
    background.beginFadeIn(0);
    endResult = RUNNING;
    musicStarted = false;
    bLoadSingle = 0;
    bLoadCampaign = 0;
    promptToDisconnect = 0;
    bLegal = 0;

    // Start video sequence if not finished
    if (!videoFinished) {
        std::vector<std::string> introVideos = {"MSFT.mp4", "CINEMA1.mp4"};
        
        if (currentVideoIndex < introVideos.size()) {
            // Clean up any existing player first
            if (introMP4Player) {
                delete introMP4Player;
                introMP4Player = nullptr;
            }
            
            std::string currentVideo = introVideos[currentVideoIndex];
            std::string mp4Path = std::string(moviePath) + currentVideo;
            
            std::cout << "[MainMenu] Starting intro video " << (currentVideoIndex + 1) 
                      << "/" << introVideos.size() << ": " << currentVideo << "\n";
            
            SDL_Window* gameWindow = SDL_GL_GetCurrentWindow();
            SDL_GLContext gameContext = SDL_GL_GetCurrentContext();
            
            if (gameWindow && gameContext) {
                // Create MP4Player for proper video handling
                introMP4Player = new MP4Player(mp4Path, gameWindow, gameContext);
                
                // Initialize with fullscreen rectangle and no looping
                RECT movieRect = {0, 0, Environment.screenWidth, Environment.screenHeight};
                introMP4Player->init(mp4Path.c_str(), movieRect, false, gameWindow, gameContext);
                
                // Start playback
                introMP4Player->restart();
                
                std::cout << "[MainMenu] Video initialized and started\n";
            } else {
                std::cout << "[MainMenu] ERROR: No valid window/context for video\n";
                videoFinished = true;
            }
        } else {
            videoFinished = true;
        }
    }

    // If videos are done, proceed with normal menu setup
    if (videoFinished) {
        while (mc2IsInMouseTimer);
        mc2IsInDisplayBackBuffer = true;
        mc2UseAsyncMouse = prefs.asyncMouse;
        if (!mc2UseAsyncMouse)
            MouseTimerKill();
        mc2IsInDisplayBackBuffer = false;

        AsynFunc = NULL;
        userInput->initMouseCursors("cursors");
        userInput->mouseOn();
        userInput->setMouseCursor(mState_LOGISTICS);

        DWORD localRenderer = prefs.renderer;
        if (prefs.renderer != 0 && prefs.renderer != 3)
            localRenderer = 0;

        bool localFullScreen = prefs.fullScreen;
        if (Environment.fullScreen && prefs.fullScreen)
            localFullScreen = false;

        if (Environment.screenWidth != 800) {
            if (prefs.renderer == 3)
                gos_SetScreenMode(800, 600, 16, 0, 0, localFullScreen, true, false, false);
            else
                gos_SetScreenMode(800, 600, 16, localRenderer, 0, localFullScreen, false, false, false);
        }
    }
}

void MainMenu::end()
{
    endAnim.end();
    bHostLeftDlg = 0;
}

void MainMenu::setDrawBackground(bool bNewDrawBackground)
{
    bDrawBackground = bNewDrawBackground;
    if (bDrawBackground && !introOver)
    {
        intro.begin();
    }
}

int MainMenu::handleMessage(unsigned long what, unsigned long who)
{
    switch (who)
    {
    case MM_MSG_NEW_CAMPAIGN:
        if (MPlayer)
        {
            LogisticsOKDialog::instance()->setText(IDS_PROMPT_TO_DISCONNECT, IDS_DIALOG_NO, IDS_DIALOG_YES);
            LogisticsOKDialog::instance()->begin();
            endResult = MM_MSG_NEW_CAMPAIGN;
            promptToDisconnect = true;
        }
        else
        {
            endAnim.begin();
            beginAnim.end();
            bLoadCampaign = true;
            LogisticsSaveDialog::instance()->beginCampaign();
            if (LogisticsSaveDialog::instance()->isDone())
            {
                LogisticsData::instance->startNewCampaign(LogisticsSaveDialog::instance()->getFileName());
                status = RESTART;
            }
        }
        break;
    case MM_MSG_SAVE:
        if (MPlayer)
        {
            LogisticsOKDialog::instance()->setText(IDS_PROMPT_TO_DISCONNECT, IDS_DIALOG_NO, IDS_DIALOG_YES);
            LogisticsOKDialog::instance()->begin();
            endResult = who;
            promptToDisconnect = true;
        }
        else
        {
            LogisticsSaveDialog::instance()->begin();
            endAnim.begin();
            beginAnim.end();
            bSave = true;
        }
        break;

    case MM_MSG_LOAD:
        if (MPlayer)
        {
            LogisticsOKDialog::instance()->setText(IDS_PROMPT_TO_DISCONNECT, IDS_DIALOG_NO, IDS_DIALOG_YES);
            LogisticsOKDialog::instance()->begin();
            endResult = who;
            promptToDisconnect = true;
        }
        else
        {
            LogisticsSaveDialog::instance()->beginLoad();
            endAnim.begin();
            beginAnim.end();
            bLoad = true;
        }
        break;
    case MM_MSG_MULTIPLAYER:
        if (MPlayer)
        {
            LogisticsOKDialog::instance()->setText(IDS_PROMPT_TO_DISCONNECT, IDS_DIALOG_NO, IDS_DIALOG_YES);
            LogisticsOKDialog::instance()->begin();
            endResult = who;
            promptToDisconnect = true;
        }
        else
        {
            endAnim.begin();
            beginAnim.end();
            endResult = MULTIPLAYERRESTART;
            LogisticsData::instance->startMultiPlayer();
        }
        break;

    case MM_MSG_RETURN_TO_GAME:
        if (!bDrawBackground)
        {
            endAnim.begin();
            beginAnim.end();
            endResult = NEXT;
            soundSystem->playDigitalSample(LOG_MAINMENUBUTTON);
            soundSystem->playDigitalMusic(LogisticsData::instance->getCurrentMissionTune());
        }
        break;
    case MM_MSG_OPTIONS:
        if (!optionsScreenWrapper)
        {
            optionsScreenWrapper = new OptionsScreenWrapper;
            optionsScreenWrapper->init();
        }
        optionsScreenWrapper->begin();
        bOptions = true;
        break;
    case MM_MSG_ENCYCLOPEDIA:
        bDrawMechlopedia = true;
        beginFadeOut(1.0);
        if (!mechlopedia)
        {
            mechlopedia = new Mechlopedia;
            mechlopedia->init();
        }
        mechlopedia->begin();
        break;
    case MM_MSG_EXIT:
        promptToQuit = 1;
        LogisticsOKDialog::instance()->setText(IDS_DIALOG_QUIT_PROMPT, IDS_DIALOG_NO, IDS_DIALOG_YES);
        LogisticsOKDialog::instance()->begin();
        getButton(who)->press(0);
        break;

    case MM_MSG_SINGLE_MISSION:
        if (MPlayer)
        {
            LogisticsOKDialog::instance()->setText(IDS_PROMPT_TO_DISCONNECT, IDS_DIALOG_NO, IDS_DIALOG_YES);
            LogisticsOKDialog::instance()->begin();
            endResult = who;
            promptToDisconnect = true;
        }
        else
        {
            bLoadSingle = true;
            endAnim.begin();
            beginAnim.end();
            singleLoadDlg.beginSingleMission();
            getButton(who)->press(0);
        }
        break;

    case MM_MSG_LEGAL:
    {
        bLegal = 1;
        if (!LogisticsLegalDialog::instance())
        {
            FullPathFileName path;
            path.init(artPath, "mcl_dialoglegal", ".fit");
            FitIniFile file;
            file.open(path);
            LogisticsLegalDialog::init(file);
        }
        LogisticsLegalDialog::instance()->setText(IDS_DIALOG_OK, IDS_DIALOG_OK, IDS_DIALOG_OK);
        char realText[2048];
        cLoadString(IDS_LAWYER_BABBLE, realText, 2047);
        char lawyerBabble[2048];
        DWORD pIDLen = 64;
        char pID[64];
        sprintf(pID, "INVALID ID");
        gos_LoadDataFromRegistry("PID", pID, &pIDLen);
        sprintf(lawyerBabble, realText, pID);
        LogisticsLegalDialog::instance()->setText(lawyerBabble);
        LogisticsLegalDialog::instance()->begin();
        LogisticsLegalDialog::instance()->setFont(IDS_LAWYER_BABBLE_FONT);
        getButton(who)->press(0);
    }
    break;
    default:
        break;
    }

    return 0;
}

void MainMenu::skipIntro()
{
    std::cout << "[MainMenu] Skipping intro video sequence\n";
    
    if (introMP4Player)
    {
        introMP4Player->stop();
        delete introMP4Player;
        introMP4Player = nullptr;
    }
    
    videoFinished = true;
    currentVideoIndex = 0; // Reset for next time
    userInput->mouseOn();
    userInput->setMouseCursor(mState_LOGISTICS);
    std::cout << "[MainMenu] Intro sequence skipped, returning to menu\n";
}

void MainMenu::update()
{
    if (bDrawBackground || MPlayer || LogisticsData::instance->isSingleMission())
    {
        getButton(MM_MSG_SAVE)->disable(true);
    }
    else
    {
        getButton(MM_MSG_SAVE)->disable(false);
    }

    getButton(MM_MSG_MULTIPLAYER)->disable(true);

    // Handle intro video sequence
    if (introMP4Player && !videoFinished)
    {
        // Check for skip input
        if (userInput->getKeyDown(KEY_SPACE) || userInput->getKeyDown(KEY_ESCAPE) || userInput->getKeyDown(KEY_LMOUSE))
        {
            skipIntro();
            return;
        }
        
        // Update and render the video
        introMP4Player->update();
        introMP4Player->render();
        
        // Check if video has ended
        if (!introMP4Player->isPlaying())
        {
            std::cout << "[MainMenu::update] Video ended naturally\n";
            delete introMP4Player;
            introMP4Player = nullptr;
            
            currentVideoIndex++;
            
            // Check if more videos to play
            std::vector<std::string> introVideos = {"MSFT.mp4", "CINEMA1.mp4"};
            if (currentVideoIndex >= introVideos.size()) {
                std::cout << "[MainMenu::update] All intro videos completed\n";
                videoFinished = true;
                userInput->mouseOn();
                userInput->setMouseCursor(mState_LOGISTICS);
            } else {
                std::cout << "[MainMenu::update] Starting next video in sequence\n";
                begin(); // Start next video
                return;
            }
        } else {
            return; // Continue playing current video
        }
    }
    
    // Check if we need to continue video sequence (fallback)
    if (!introMP4Player && !videoFinished)
    {
        std::vector<std::string> introVideos = {"MSFT.mp4", "CINEMA1.mp4"};
        
        if (currentVideoIndex < introVideos.size()) {
            std::cout << "[MainMenu::update] Fallback - restarting video sequence\n";
            begin();
            return;
        } else {
            videoFinished = true;
        }
    }

    if (!musicStarted)
    {
        musicStarted = true;
        soundSystem->setMusicVolume(prefs.MusicVolume);
        soundSystem->playDigitalMusic(tuneId);
    }

    if (endAnim.isDone())
    {
        status = endResult;
    }

    if (bDrawMechlopedia)
    {
        mechlopedia->update();
        if (mechlopedia->getStatus() == NEXT)
        {
            beginFadeIn(0);
            bDrawMechlopedia = 0;
            if (!bDrawBackground)
                status = NEXT;
        }
        return;
    }

    if (bOptions)
    {
        OptionsScreenWrapper::status_type result = optionsScreenWrapper->update();
        if (result == OptionsScreenWrapper::opt_DONE)
        {
            optionsScreenWrapper->end();
            bOptions = 0;
        }
        return;
    }

    if ((bSave || bLoad || bLoadCampaign) && endAnim.isDone())
    {
        LogisticsSaveDialog::instance()->update();

        if (LogisticsSaveDialog::instance()->getStatus() == LogisticsScreen::YES && LogisticsSaveDialog::instance()->isDone())
        {
            char name[1024];
            strcpy(name, savePath);
            strcat(name, LogisticsSaveDialog::instance()->getFileName());
            size_t index = strlen(name) - 4;
            if (S_stricmp(&name[index], ".fit") != 0)
                strcat(name, ".fit");

            FitIniFile file;
            if (bSave)
            {
                CreateDirectory(savePath, NULL);
                if (NO_ERR != file.createWithCase(name))
                {
                    char errorStr[1024];
                    sprintf(errorStr, "couldn't open the file %s", name);
                    Assert(0, 0, errorStr);
                }
                else
                {
                    LogisticsData::instance->save(file);
                    LogisticsSaveDialog::instance()->end();
                    file.close();
                }
                bSave = bLoad = 0;
                status = NEXT;
            }
            else if (bLoadCampaign)
            {
                LogisticsData::instance->startNewCampaign(LogisticsSaveDialog::instance()->getFileName());
                status = endResult = RESTART;
                bLoadCampaign = 0;
            }
            else
            {
                const bool do_not_make_lower = true;
                if (NO_ERR != file.open(name, READ, 50, do_not_make_lower))
                {
                    char errorStr[1024];
                    sprintf(errorStr, "couldn't open the file %s", name);
                    Assert(0, 0, errorStr);
                }
                else
                    LogisticsData::instance->load(file);
                LogisticsSaveDialog::instance()->end();
                bSave = bLoad = 0;
                status = RESTART;
                file.close();
            }
        }
        else if (LogisticsSaveDialog::instance()->getStatus() == LogisticsScreen::NO && LogisticsSaveDialog::instance()->isDone())
        {
            LogisticsSaveDialog::instance()->end();
            bSave = bLoad = bLoadCampaign = 0;
            if (!bDrawBackground)
                status = NEXT;
            else
            {
                beginAnim.begin();
                endAnim.end();
            }
        }
        return;
    }
    else if (bLoadSingle && endAnim.isDone())
    {
        singleLoadDlg.update();
        if (singleLoadDlg.isDone())
        {
            if (singleLoadDlg.getStatus() == YES)
            {
                const char* pName = singleLoadDlg.getMapFileName();
                if (pName)
                {
                    LogisticsData::instance->setSingleMission(pName);
                    status = SKIPONENEXT;
                }
            }
            bLoadSingle = 0;
            beginAnim.begin();
            endAnim.end();
        }
    }
    else if (promptToQuit)
    {
        LogisticsOKDialog::instance()->update();
        if (LogisticsOKDialog::instance()->getStatus() == LogisticsScreen::YES)
        {
            soundSystem->playDigitalSample(LOG_EXITGAME);
            gos_TerminateApplication();
            promptToQuit = 0;
        }
        else if (LogisticsOKDialog::instance()->getStatus() == LogisticsScreen::NO)
        {
            if (LogisticsOKDialog::instance()->isDone())
                promptToQuit = 0;
        }
    }
    else if (bLegal)
    {
        LogisticsLegalDialog::instance()->update();
        if (LogisticsLegalDialog::instance()->isDone())
        {
            LogisticsLegalDialog::instance()->end();
            bLegal = 0;
        }
    }
    else if (bHostLeftDlg)
    {
        LogisticsOneButtonDialog::instance()->update();
        if (LogisticsOneButtonDialog::instance()->isDone())
        {
            LogisticsOneButtonDialog::instance()->end();
            bHostLeftDlg = 0;
        }
        if (MPlayer)
        {
            MPlayer->closeSession();
            delete MPlayer;
            MPlayer = NULL;
        }
    }
    else if (promptToDisconnect)
    {
        LogisticsOKDialog::instance()->update();
        if (LogisticsOKDialog::instance()->isDone())
        {
            if (YES == LogisticsOKDialog::instance()->getStatus())
            {
                if (MPlayer)
                {
                    MPlayer->closeSession();
                    delete MPlayer;
                    MPlayer = NULL;
                }
                long oldRes = endResult;
                endResult = 0;
                handleMessage(oldRes, oldRes);
                setDrawBackground(true);
            }
            else
                handleMessage(NEXT, NEXT);
            promptToDisconnect = 0;
        }
    }
    else
    {
        if (bDrawBackground)
        {
            if (!intro.animObjects[0].isDone())
            {
                intro.update();
                background.update();
                if (userInput->getKeyDown(KEY_ESCAPE) || (Environment.Renderer == 3))
                {
                    introOver = true;
                    userInput->mouseOn();
                    soundSystem->playDigitalSample(LOG_MAINMENUBUTTON);
                }
                else if (!introOver)
                    return;
            }
            else
            {
                background.update();
                if (!introOver)
                    soundSystem->playDigitalSample(LOG_MAINMENUBUTTON);
                introOver = true;
                userInput->mouseOn();
            }
        }

        beginAnim.update();
        endAnim.update();
        LogisticsScreen::update();
        if ((!bLoadSingle) && userInput->isLeftClick() && !inside(userInput->getMouseX(), userInput->getMouseY()))
        {
            handleMessage(0, MM_MSG_RETURN_TO_GAME);
        }
    }
}

void MainMenu::render()
{
    // If intro video is playing, just render the video and nothing else
    if (introMP4Player && !videoFinished) {
        introMP4Player->render();
        return;
    }

    float scaleX = Environment.screenWidth / 1024.0f;
    float scaleY = Environment.screenHeight / 768.0f;
    float fontScale = std::min(scaleX, scaleY);

    if (bDrawMechlopedia && (fadeTime > fadeOutTime || !fadeOutTime))
    {
        mechlopedia->render();
        return;
    }

    if (bOptions)
    {
        optionsScreenWrapper->render();
        return;
    }

    float xDelta = 0.f;
    float yDelta = 0.f;
    long color = 0xff000000;

    if (Environment.Renderer != 3)
    {
        if (beginAnim.isAnimating() && !beginAnim.isDone())
        {
            xDelta = beginAnim.getXDelta();
            yDelta = beginAnim.getYDelta();
            float time = beginAnim.getCurrentTime();
            float endTime = beginAnim.getMaxTime();
            if (endTime)
            {
                color = interpolateColor(0x00000000, 0x7f000000, time / endTime);
            }
        }
        else if (endAnim.isAnimating())
        {
            xDelta = endAnim.getXDelta();
            yDelta = endAnim.getYDelta();
            float time = endAnim.getCurrentTime();
            float endTime = endAnim.getMaxTime();
            if (endTime && (time <= endTime))
            {
                color = interpolateColor(0x7f000000, 0x00000000, time / endTime);
            }
        }

        GUI_RECT rect = {0, 0, static_cast<long>(1024 * scaleX), static_cast<long>(768 * scaleY)};
        drawRect(rect, color);

        if (bDrawBackground)
        {
            background.render();
            intro.render();
            if (!intro.animObjects[0].isDone() && !introOver && !bHostLeftDlg)
                return;
        }
    }
    else
    {
        GUI_RECT rect = {0, 0, Environment.screenWidth, Environment.screenHeight};
        drawRect(rect, color);
    }

    if (!xDelta && !yDelta)
    {
        long scaledX = static_cast<long>(textObjects[1].globalX() * scaleX);
        long scaledTop = static_cast<long>(textObjects[1].globalTop() * scaleY);
        long scaledRight = static_cast<long>(textObjects[1].globalRight() * scaleX);
        long scaledBottom = static_cast<long>(textObjects[1].globalBottom() * scaleY);
        long fontSize = static_cast<long>(textObjects[1].font.getSize() * fontScale);

        const char* text = textObjects[1].text;
        int textWidth = textObjects[1].font.width(text);
        int centerX = Environment.screenWidth / 2;
        int y = Environment.screenHeight - 24;

        float maxWidth = Environment.screenWidth * 0.9f;
        float scale = 1.0f;
        if (textWidth > maxWidth)
            scale = maxWidth / (float)textWidth;

        drawShadowText(
            0xffc66600, 0xff000000, textObjects[1].font.getTempHandle(),
            centerX - (textWidth * scale) / 2,
            y,
            centerX + (textWidth * scale) / 2,
            y + 24,
            true, text, false,
            textObjects[1].font.getSize(), scale, scale);
    }

    textObjects[1].showGUIWindow(false);

    if ((!bSave && !bLoad && !bLoadSingle && !bLoadCampaign) || (!endAnim.isDone() && endResult != RESTART))
        LogisticsScreen::render(xDelta, yDelta);
    else if (bLoadSingle)
        singleLoadDlg.render();
    else
        LogisticsSaveDialog::instance()->render();

    if (promptToQuit || promptToDisconnect)
        LogisticsOKDialog::instance()->render();
    if (bLegal)
        LogisticsLegalDialog::instance()->render();
    if (bHostLeftDlg)
        LogisticsOneButtonDialog::instance()->render();
}

void MainMenu::setHostLeftDlg(const char* playerName)
{
    char leaveStr[256];
    char formatStr[256];
    cLoadString(IDS_PLAYER_LEFT, leaveStr, 255);
    sprintf(formatStr, leaveStr, playerName);

    LogisticsOneButtonDialog::instance()->setText(IDS_PLAYER_LEFT, IDS_DIALOG_OK, IDS_DIALOG_OK);
    LogisticsOneButtonDialog::instance()->setText(formatStr);

    if (MPlayer && MPlayer->playerInfo[MPlayer->commanderID].booted)
    {
        LogisticsOneButtonDialog::instance()->setText(IDS_MP_PLAYER_KICKED, IDS_DIALOG_OK, IDS_DIALOG_OK);
    }
    LogisticsOneButtonDialog::instance()->begin();
    bHostLeftDlg = true;
}

void MainMenu::restoreOpenGLState()
{
    SDL_Window* gameWindow = SDL_GL_GetCurrentWindow();
    SDL_GLContext gameContext = SDL_GL_GetCurrentContext();
    if (SDL_GL_MakeCurrent(gameWindow, gameContext) != 0)
    {
        std::cerr << "[MainMenu] Failed to restore OpenGL context: " << SDL_GetError() << "\n";
    }

    glViewport(0, 0, Environment.screenWidth, Environment.screenHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Environment.screenWidth, Environment.screenHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    std::cout << "[MainMenu] Restored OpenGL state\n";
}