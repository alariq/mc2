#include "gameos.hpp"
#include <stdio.h>

int main(int argc, char** argv)
{
    GetGameOSEnvironment("");

    Environment.InitializeGameEngine();

    while(true)
    {
        Environment.DoGameLogic();
        Environment.UpdateRenderers();
    }
    
    Environment.TerminateGameEngine();


    return 0;
}

