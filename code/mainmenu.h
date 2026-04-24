#ifndef MAINMENU_H
#define MAINMENU_H

#ifndef LOGISTICSSCREEN_H
#include "logisticsscreen.h"
#endif

#ifndef AANIM_H
#include "aanim.h"
#endif

#ifndef MPLOADMAP_H
#include "mploadmap.h"
#endif

#ifndef MP4PLAYER_H
#include "mp4player.h"
#endif

class OptionsScreenWrapper;
class Mechlopedia;

class SplashIntro : public LogisticsScreen
{
public:
    SplashIntro() {}
    virtual ~SplashIntro() {}
    void init();
};

class MainMenu : public LogisticsScreen
{
public:
    MainMenu();
    virtual ~MainMenu();
    int init(FitIniFile& file);
    virtual void begin();
    virtual void end();
    virtual void update();
    virtual void render();
    void restoreOpenGLState();
    void setHostLeftDlg(const char* playerName);
    void setDrawBackground(bool bDrawBackground);
    void skipIntro();
    virtual int handleMessage(unsigned long, unsigned long);
    static bool bDrawMechlopedia;

private:
    MainMenu& operator=(const MainMenu& ainMenu);
    MainMenu(const MainMenu& src);

    bool bDrawBackground;
    LogisticsScreen background;
    bool promptToQuit;
    bool bOptions;
    bool bSave;
    bool bLoad;
    bool bLoadSingle;
    bool bLoadCampaign;
    bool promptToDisconnect;
    bool bLegal;
    bool videoFinished;
    int currentVideoIndex; // Track which video in sequence we're playing

    long tuneId;
    bool musicStarted;
    long endResult;
    aAnimation beginAnim;
    aAnimation endAnim;
    OptionsScreenWrapper* optionsScreenWrapper;
    Mechlopedia* mechlopedia;
    SplashIntro intro;
    bool introOver;
    MPLoadMap singleLoadDlg;
    bool bHostLeftDlg;
    MP4Player* introMP4Player;
};

#endif