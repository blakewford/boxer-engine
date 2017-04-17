#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "jni.h"

#include "boxer.h"
#include "boxer_internal.h"

extern JavaVM* jVM;
extern jclass jBoxerEngine;
extern jmethodID jShowStage;
extern char androidData[PATH_MAX];

boxer::audioParam* jAudioParam = NULL;

namespace boxer
{

extern pthread_mutex_t gAudioMutex;

int32_t getDefaultFrameDelay()
{
    return 48; //Empirical, but could be derived by checking CPU usage is less than max output of 2 threads
}

void initializeDisplay()
{
}

char buffer[PATH_MAX];
const char* getDebugImagePath()
{
    memset(buffer, '\0', PATH_MAX);
    memcpy(buffer, androidData, strlen(androidData));
    strcat(buffer, "debug.bmp");

    return buffer;
}

void writeDisplay(uint8_t* data)
{
    JNIEnv* jThreadEnv = NULL;
    jVM->AttachCurrentThread(&jThreadEnv, NULL);
    jstring imageString = jThreadEnv->NewStringUTF(buffer);
    jThreadEnv->CallStaticVoidMethod(jBoxerEngine, jShowStage, imageString);
    jThreadEnv->DeleteLocalRef(imageString);
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

pthread_t gInputThread;
void initializeInput()
{
    pthread_create(&gInputThread, NULL, boxer::inputThread, NULL);
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
    pthread_join(gInputThread, NULL);
}

}
