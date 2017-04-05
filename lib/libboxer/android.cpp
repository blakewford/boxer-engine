#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "boxer.h"
#include "boxer_internal.h"

boxer::audioParam* jAudioParam = NULL;

namespace boxer
{

extern pthread_mutex_t gAudioMutex;

int32_t getDefaultFrameDelay()
{
    return 48; //Empirical, but could be derived by checking CPU usage is less than max output of 2 threads
}

void writeAudioResource(audioParam* param)
{
    pthread_mutex_lock(&gAudioMutex);
    while(jAudioParam != NULL && param->keepGoing)
    {
        pthread_mutex_unlock(&gAudioMutex);
        sched_yield();
        pthread_mutex_lock(&gAudioMutex);
    }
    audioParam* jParam = new audioParam();
    jParam->id = param->id;
    jParam->keepGoing = param->keepGoing;
    jAudioParam = jParam;
    pthread_mutex_unlock(&gAudioMutex);
    while(jAudioParam != NULL && param->keepGoing)
    {
        sched_yield();
    }
}

void shutdownAudio(int32_t id)
{
    if(jAudioParam != NULL && jAudioParam->id == id)
    {
        jAudioParam->keepGoing = false;
    }
}

void initializeInput()
{
}

control jControl = UNKNOWN;
control getControlInput()
{
    while(jControl == UNKNOWN)
        usleep(1);

    control temp = jControl;
    jControl = UNKNOWN;
    return temp;
}

void shutdownInput()
{
}

}
