#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "perf.h"
#include "boxer.h"
#include "dictionary.h"

void boxerMain()
{
    int32_t count = 0;
    int32_t argCount = boxer::getArgc();
    char** arg = boxer::getArgv();
    while(count < argCount)
    {
        printf("%s\n", arg[count++]);
    }

    const boxer::bmpStat* stage = (const boxer::bmpStat*)boxer::getResource(STAGE);
    const boxer::bmpStat* sprite = (const boxer::bmpStat*)boxer::getResource(SPRITE);
    if(stage == NULL || sprite == NULL)
    {
        printf("Error! NULL file pullled from manifest!\n");
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
        printf("Coordinate %d %d\n", x, y);

BEGIN_SCOPE
        boxer::setStage(STAGE);
        int32_t status = boxer::blockResource(SPRITE, x, y);
        assert(status == 0);
        boxer::showStage();
        boxer::stopAudioResource(AUDIO);
SCOPE("Frame Time")
    }
}
