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

struct _BUILDER
{
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

#endif
