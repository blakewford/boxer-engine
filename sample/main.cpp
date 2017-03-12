#include <stdio.h>
#include <string.h>
#include <assert.h>

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

    const boxer::bmpStat* stat = (const boxer::bmpStat*)boxer::getResource(TEST);
    if(stat == NULL)
    {
        printf("Error! NULL file pullled from manifest!\n");
        return;
    }

    printf("%x\n", stat->ident);
    printf("%x\n", stat->size);
    printf("%x\n", stat->reserved);
    printf("%x\n", stat->offset);
    printf("%x\n", stat->headerSize);
    printf("%x\n", stat->width);
    printf("%x\n", stat->height);
    assert(stat->colorPlanes == 1);
    printf("%x\n", stat->colorPlanes);
    printf("%x\n", stat->depth);
    assert(stat->compression == 3); //BI_BITFIELDS
    printf("%x\n", stat->compression);
    printf("%x\n", stat->dataSize);
    printf("%x\n", stat->horRes);
    printf("%x\n", stat->vertRes);
    printf("%x\n", stat->paletteSize);
    printf("%x\n", stat->important);

    boxer::bbfPixel* data = (boxer::bbfPixel*)(boxer::getResource(TEST) + sizeof(boxer::bmpStat));

    count = 0;
    while(count < (stat->dataSize/2))
    {
        printf("%4x ", data[count].p);
        count++;
        if(count%8 == 0)
        {
            printf("\n");
        }
    }
}
