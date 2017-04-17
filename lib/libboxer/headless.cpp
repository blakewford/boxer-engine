#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "boxer.h"
#include "boxer_internal.h"

namespace boxer
{

int32_t getDefaultFrameDelay()
{
    return 1000;
}

void initializeDisplay()
{
}

const char* getDebugImagePath()
{
    return "debug.bmp";
}

void writeDisplay(uint8_t* data)
{
}

void writeAudioResource(audioParam* param)
{
}

void shutdownAudio(int32_t id)
{
}

control getControlInput()
{
    char c = getch();
    control mapped = UNKNOWN;
    switch(c)
    {
        case 'w':
        case 'W':
            mapped = UP;
            break;
        case 'a':
        case 'A':
            mapped = LEFT;
            break;
        case 's':
        case 'S':
            mapped = DOWN;
            break;
        case 'd':
        case 'D':
            mapped = RIGHT;
            break;
        case ' ':
            mapped = AUX1;
            break;
    }

    return mapped;
}

void initializeInput()
{
    WINDOW* w = initscr();
    noecho();
    scrollok(w, true);
}

void shutdownInput()
{
    endwin();
}

}
