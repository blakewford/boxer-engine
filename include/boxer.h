#pragma once

#include <stdint.h>
#include <limits.h>

extern "C" void boxerMain();

namespace boxer
{
    const int32_t MAX_MANIFEST_ENTRY = 64;
    const int32_t MAX_MANIFEST_TEXT = PATH_MAX;

    struct bmpStat
    {
        uint16_t ident;
        uint32_t size;
        uint32_t reserved;
        uint32_t offset;
        uint32_t headerSize;
        int32_t width;
        int32_t height;
        uint16_t colorPlanes;
        uint16_t depth;
        uint32_t compression;
        uint32_t dataSize;
        uint32_t horRes;
        uint32_t vertRes;
        uint32_t paletteSize;
        uint32_t important;
    }__attribute__((packed));

    struct colorTable
    {
        uint32_t R;
        uint32_t G;
        uint32_t B;
        uint8_t padding[72];
    }__attribute__((packed));

    struct detail
    {
        uint8_t R: 5;
        uint8_t G: 6;
        uint8_t B: 5;
    }__attribute__((packed));

    union bbfPixel
    {
        uint16_t p;
        detail d;
    };

    struct wavStat
    {
        uint8_t padding[40];
        uint32_t size;
    }__attribute__((packed));

    int32_t getArgc();
    char** getArgv();
    int32_t getFrameDelay();
    void setFrameDelay(int32_t ms);
    const uint8_t* getResource(int32_t id);

    void setStage(int32_t id);
    int32_t blockResource(int32_t id, int32_t x, int32_t y);
    void showStage();

    void startAudioResource(int32_t id, int32_t delay = -1);
    void stopAudioResource(int32_t id);
    void waitAudioResource(int32_t id);

    enum control
    {
        UNKNOWN,
        UP,
        LEFT,
        DOWN,
        RIGHT,
        AUX1
    };

    void setControlResponse(boxer::control,  void (*response)(control));
}

#ifdef __ANDROID__
#include <android/log.h>
#define BOXER_LOG(...) __android_log_print(ANDROID_LOG_VERBOSE, "Boxer", __VA_ARGS__);
#else
#include <ncurses.h>
#define BOXER_LOG(...) printw(__VA_ARGS__); refresh();
#endif

