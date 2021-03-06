#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "boxer.h"
#include "boxer_internal.h"

#include "jni.h"

namespace boxer
{
    extern pthread_mutex_t gAudioMutex;
    extern control jControl;
}

extern boxer::audioParam* jAudioParam;

JavaVM* jVM = NULL;
jclass jBoxerEngine = NULL;
jmethodID jShowStage = NULL;
char androidData[PATH_MAX];

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    jVM = vm;
    JNIEnv* env;
    if(vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK)
    {
        return -1;
    }

    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL Java_org_starlo_boxer_BoxerEngine_preload(JNIEnv* env, jobject obj, jstring pathString)
{
    const char* path = env->GetStringUTFChars(pathString, NULL);

    boxer::preload(path);

    memset(androidData, '\0', PATH_MAX);
    const char* finalSlash = strrchr(path, '/');
    memcpy(androidData, path, (finalSlash - path) + 1);

    env->ReleaseStringUTFChars(pathString, path);
}

extern "C" JNIEXPORT void JNICALL Java_org_starlo_boxer_BoxerEngine_boxerMain(JNIEnv* env, jobject obj)
{
    jBoxerEngine = env->FindClass("org/starlo/boxer/BoxerEngine");
    jShowStage = env->GetStaticMethodID(jBoxerEngine, "showStage", "([B)V");
    boxerMain();
}

extern "C" JNIEXPORT void JNICALL Java_org_starlo_boxer_BoxerEngine_audioResourceThread(JNIEnv* env, jobject obj)
{
    jclass engine = env->FindClass("org/starlo/boxer/BoxerEngine");
    jmethodID audioWrite = env->GetStaticMethodID(engine, "audioWrite", "([S)V");
    while(true)
    {
        if(jAudioParam != NULL)
        {
            int32_t i = 0;
            const boxer::wavStat* stat = (const boxer::wavStat*)boxer::getResource(jAudioParam->id);
            const uint8_t* data = (const uint8_t*)(boxer::getResource(jAudioParam->id) + WAV_HEADER_SIZE);
            jshortArray jData = env->NewShortArray(AUDIO_BUFFER_SIZE/sizeof(short));
            while(i+AUDIO_BUFFER_SIZE < stat->size && jAudioParam->keepGoing)
            {
                env->SetShortArrayRegion(jData, 0, AUDIO_BUFFER_SIZE/sizeof(short), (const short*)&data[i]);
                env->CallStaticVoidMethod(engine, audioWrite, jData);
                i+=AUDIO_BUFFER_SIZE;
            }
            env->DeleteLocalRef(jData);
            pthread_mutex_lock(&boxer::gAudioMutex);
            delete jAudioParam;
            jAudioParam = NULL;
            pthread_mutex_unlock(&boxer::gAudioMutex);
        }
        sched_yield();
    }
}

extern "C" JNIEXPORT void JNICALL Java_org_starlo_boxer_BoxerEngine_triggerControl(JNIEnv* env, jobject obj, jint control)
{
    static bool first = true;
    if(first)
    {
        BOXER_LOG("Input Initialized\n");
        boxer::initializeInput();
        first = false;
    }

    while(boxer::jControl != boxer::UNKNOWN)
        usleep(1);

    boxer::jControl = (boxer::control)control;
}
