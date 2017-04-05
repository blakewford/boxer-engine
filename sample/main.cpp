#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "perf.h"
#include "boxer.h"
#include "dictionary.h"

void controlResponse(boxer::control trigger)
{
    switch(trigger)
    {
        case boxer::UP:
            BOXER_LOG("UP\n");
            break;
        case boxer::LEFT:
            BOXER_LOG("LEFT\n");
            break;
        case boxer::DOWN:
            BOXER_LOG("DOWN\n");
            break;
        case boxer::RIGHT:
            BOXER_LOG("RIGHT\n");
            break;
        case boxer::AUX1:
            BOXER_LOG("AUX1\n");
            break;
    }
}

void boxerMain()
{
    int32_t count = 0;
    int32_t argCount = boxer::getArgc();
    char** arg = boxer::getArgv();

    while(count < argCount)
    {
        BOXER_LOG("%s\n", arg[count++]);
    }

    boxer::setControlResponse(boxer::UP, controlResponse);
    boxer::setControlResponse(boxer::LEFT, controlResponse);
    boxer::setControlResponse(boxer::DOWN, controlResponse);
    boxer::setControlResponse(boxer::RIGHT, controlResponse);
    boxer::setControlResponse(boxer::AUX1, controlResponse);

    const boxer::bmpStat* stage = (const boxer::bmpStat*)boxer::getResource(STAGE);
    const boxer::bmpStat* sprite = (const boxer::bmpStat*)boxer::getResource(SPRITE);
    if(stage == NULL || sprite == NULL)
    {
        BOXER_LOG("Error! NULL file pullled from manifest!\n");
        return;
    }

    srand(time(NULL));

    int32_t x = 0;
    int32_t y = 0;
    while(true)
    {
        boxer::startAudioResource(AUDIO);
        x = rand() % ((stage->width-sprite->width) + 1);
        y = rand() % ((stage->height-sprite->height) + 1);
        BOXER_LOG("Coordinate %d %d\n", x, y);
BEGIN_SCOPE
        boxer::setStage(STAGE);
        int32_t status = boxer::blockResource(SPRITE, x, y);
        assert(status == 0);
        boxer::showStage();
SCOPE("Frame Time")
        boxer::waitAudioResource(AUDIO);
    }
}
