#include <map>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "boxer.h"
#include "boxer_internal.h"
#include "stage.h"

extern int32_t _BOXER_FILES_SIZE;
extern char* _BOXER_FILES[];

int32_t cachedArgc = 0;
char argvStorage[1024];
char* cachedArgv[64];

namespace boxer
{

enum error: int32_t
{
    SUCCESS,
    FAILURE
};

stage* gStage = NULL;
int32_t gFrameDelay = 0;
std::map<int32_t, uint8_t*> gResourceMap;

pthread_mutex_t gAudioMutex;

#define MAX_AUDIO_THREADS 16
pthread_t gAudioThread[MAX_AUDIO_THREADS];
audioParam gAudioThreadParam[MAX_AUDIO_THREADS];
int32_t gAudioThreadIndex = 0;

int32_t getArgc()
{
    return cachedArgc;
}

char** getArgv()
{
    return cachedArgv;
}

int32_t getFrameDelay()
{
    return gFrameDelay;
}

void setFrameDelay(int32_t ms)
{
    gFrameDelay = ms;
}

const uint8_t* getResource(int32_t id)
{
    auto found = gResourceMap.find(id);
    return found == gResourceMap.end() ? NULL: gResourceMap.find(id)->second;
}

void setStage(int32_t id)
{
    if(gStage != NULL)
    {
        gStage->~stage();
        free(gStage);
    }

    gStage = (stage*)malloc(sizeof(stage));
    gStage->init(id);
}

int32_t blockResource(int32_t id, int32_t x, int32_t y)
{
    int32_t code = FAILURE;

    if(gStage == NULL || (x > gStage->getWidth()) || (y > gStage->getHeight()))
        return code;

    const boxer::bmpStat* stat = (const boxer::bmpStat*)boxer::getResource(id);
    assert(stat != NULL);
    assert(stat->colorPlanes == 1);
    assert(stat->compression == 3); //BI_BITFIELDS

    if((x+stat->width) <= gStage->getWidth() && (y+stat->height) <= gStage->getHeight())
    {
        gStage->draw(boxer::getResource(id), x, y);
        code = SUCCESS;
    }

    return code;
}

void showStage()
{
    if(gStage != NULL)
    {
        gStage->show();
    }
    usleep(gFrameDelay*1000);
}

void* audioResourceThread(void* param)
{
    audioParam* desc = (audioParam*)param;
    while(true)
    {
        writeAudioResource(desc);
        if(desc->keepGoing == false || desc->delay == -1)
            break;

        usleep(desc->delay*1000);
    }

    return NULL;
}

void startAudioResource(int32_t id, int32_t delay)
{
    waitAudioResource(id);
    gAudioThreadParam[gAudioThreadIndex].id = id;
    gAudioThreadParam[gAudioThreadIndex].delay = delay;
    gAudioThreadParam[gAudioThreadIndex].keepGoing = true;
    pthread_create(&gAudioThread[gAudioThreadIndex], NULL, audioResourceThread, &gAudioThreadParam[gAudioThreadIndex]);
    if(++gAudioThreadIndex == MAX_AUDIO_THREADS)
    {
        gAudioThreadIndex = 0;
    }
}

void stopAudioResource(int32_t id)
{
    int32_t i = MAX_AUDIO_THREADS;
    while(i--)
    {
        if(gAudioThreadParam[i].id == id)
        {
            gAudioThreadParam[i].keepGoing = false;
            pthread_mutex_lock(&gAudioMutex);
            shutdownAudio(id);
            pthread_mutex_unlock(&gAudioMutex);
            waitAudioResource(id);
            break;
        }
    }
}

void waitAudioResource(int32_t id)
{
    int32_t i = MAX_AUDIO_THREADS;
    while(i--)
    {
        if(gAudioThreadParam[i].id == id)
        {
            pthread_join(gAudioThread[i], NULL);
            break;
        }
    }
}

struct _BUILDER
{
    _BUILDER()
    {
        pthread_mutex_init(&gAudioMutex, NULL);
    }

   ~_BUILDER()
    {
        int32_t count = 0;
        char buffer[PATH_MAX];
        while(count < _BOXER_FILES_SIZE)
        {
            free(gResourceMap.find(count++)->second);
        }
        if(gStage != NULL)
        {
            gStage->~stage();
            free(gStage);
        }
    }

};

static _BUILDER resourceBuilder;

void preload(const char* path)
{
    boxer::setFrameDelay(boxer::getDefaultFrameDelay());

    int32_t count = 0;
    char buffer[PATH_MAX];
    while(count < _BOXER_FILES_SIZE)
    {
        memset(buffer, '\0', PATH_MAX);
        memcpy(buffer, path, strlen(path));
        strcat(buffer, _BOXER_FILES[count]);
        FILE* temp = fopen(buffer, "r");
        if(temp)
        {
            fseek(temp, 0, SEEK_END);
            int32_t size = ftell(temp);
            rewind(temp);
            uint8_t* binary = (uint8_t*)malloc(size);
            size_t read = fread(binary, 1, size, temp);
            assert(read == size);
            boxer::gResourceMap.insert(std::pair<int32_t, uint8_t*>(count, binary));
            fclose(temp);
        }
        count++;
    }
}

}

int32_t main(int32_t argc, char** argv)
{
    cachedArgc = argc;
    char* storagePointer = argvStorage;
    while(argc--)
    {
        cachedArgv[argc] = storagePointer;
        int32_t length = strlen(argv[argc]);
        strcat(storagePointer, argv[argc]);
        storagePointer+=(length+1);
    }

    char buffer[PATH_MAX];
    memset(buffer, '\0', PATH_MAX);
    char* finalSlash = strrchr(cachedArgv[0], '/');
    memcpy(buffer, cachedArgv[0], (finalSlash - cachedArgv[0]) + 1);
    boxer::preload(buffer);
    boxerMain();

    return 0;
}

