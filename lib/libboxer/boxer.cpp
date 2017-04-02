#include <map>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "boxer.h"
#include "stage.h"

#ifdef __ANDROID__
#include "jni.h"
#include <android/log.h>
#else
#include <pulse/simple.h>
#endif

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

struct audioParam
{
    int32_t id;
    int32_t delay;
    bool keepGoing;
};

pthread_mutex_t jAudioMutex;
audioParam* jAudioParam = NULL;

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

#define WAV_HEADER_SIZE 44
void* audioResourceThread(void* param)
{
    audioParam* desc = (audioParam*)param;
    while(true)
    {
#ifdef __ANDROID__
        pthread_mutex_lock(&jAudioMutex);
        while(jAudioParam != NULL)
        {
            pthread_mutex_unlock(&jAudioMutex);
            usleep(1);
            pthread_mutex_lock(&jAudioMutex);
        }
        audioParam* jParam = new audioParam();
        jParam->id = desc->id;
        jAudioParam = jParam;
        pthread_mutex_unlock(&jAudioMutex);
        while(jAudioParam != NULL)
            usleep(1);
#else
        const boxer::wavStat* stat = (const boxer::wavStat*)boxer::getResource(desc->id);
        const uint8_t* data = (const uint8_t*)(boxer::getResource(desc->id) + WAV_HEADER_SIZE);
        static const pa_sample_spec spec = { .format = PA_SAMPLE_S16LE, .rate = 44100, .channels = 2 };
        pa_simple* stream = pa_simple_new(NULL, NULL, PA_STREAM_PLAYBACK, NULL, "boxer_track", &spec, NULL, NULL, NULL);
        if(stream != NULL)
        {
            if(pa_simple_write(stream, data, stat->size, NULL) < 0)
            {
                break;
            }
            pa_simple_drain(stream, NULL);
            pa_simple_free(stream);
        }
#endif

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
        pthread_mutex_init(&jAudioMutex, NULL);
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

}

void preload(const char* path)
{
    int32_t delay = 0;
#ifdef __ANDROID__
    delay = 48; //Empirical, but could be derived by checking CPU usage is less than max output of 2 threads
#else
    delay = 1000;
#endif

    boxer::setFrameDelay(delay);

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
    preload(buffer);
    boxerMain();

    return 0;
}

#ifdef __ANDROID__
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

    preload(path);

    memset(androidData, '\0', PATH_MAX);
    char* finalSlash = strrchr(path, '/');
    memcpy(androidData, path, (finalSlash - path) + 1);

    env->ReleaseStringUTFChars(pathString, path);
}

extern "C" JNIEXPORT void JNICALL Java_org_starlo_boxer_BoxerEngine_boxerMain(JNIEnv* env, jobject obj)
{
    jBoxerEngine = env->FindClass("org/starlo/boxer/BoxerEngine");
    jShowStage = env->GetStaticMethodID(jBoxerEngine, "showStage", "(Ljava/lang/String;)V");
    boxerMain();
}

extern "C" JNIEXPORT void JNICALL Java_org_starlo_boxer_BoxerEngine_audioResourceThread(JNIEnv* env, jobject obj)
{
    jclass engine = env->FindClass("org/starlo/boxer/BoxerEngine");
    jmethodID audioWrite = env->GetStaticMethodID(engine, "audioWrite", "([S)V");
    while(true)
    {
        if(boxer::jAudioParam != NULL)
        {
            const boxer::wavStat* stat = (const boxer::wavStat*)boxer::getResource(boxer::jAudioParam->id);
            const uint8_t* data = (const uint8_t*)(boxer::getResource(boxer::jAudioParam->id) + WAV_HEADER_SIZE);
            jshortArray jData = env->NewShortArray(stat->size/2);
            env->SetShortArrayRegion(jData, 0, stat->size/2, (const short*)data);
            env->CallStaticVoidMethod(engine, audioWrite, jData);
            env->DeleteLocalRef(jData);
            pthread_mutex_lock(&boxer::jAudioMutex);
            delete boxer::jAudioParam;
            boxer::jAudioParam = NULL;
            pthread_mutex_unlock(&boxer::jAudioMutex);
        }
        usleep(1);
    }
}

#endif
