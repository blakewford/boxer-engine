#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "stage.h"

#ifdef __ANDROID__
#include "jni.h"
extern JavaVM* jVM;
extern jclass jBoxerEngine;
extern jmethodID jShowStage;
extern char androidData[PATH_MAX];
#endif

namespace boxer
{

stage::stage():
m_stageWidth(0),
m_stageHeight(0),
m_stageData(NULL)
{
}

stage::~stage()
{
    if(m_stageData != NULL)
        free(m_stageData);
}

void stage::init(int32_t id)
{
    const boxer::bmpStat* stat = (const boxer::bmpStat*)boxer::getResource(id);
    assert(stat->width % 4 == 0);
    assert(stat->colorPlanes == 1);
    assert(stat->compression == 3); //BI_BITFIELDS

    const boxer::bbfPixel* data = (boxer::bbfPixel*)(boxer::getResource(id) + stat->offset);

    m_stageData = (uint8_t*)malloc(stat->dataSize);
    memcpy(m_stageData, data, stat->dataSize);
    m_stageWidth = stat->width;
    m_stageHeight = stat->height;
}

void stage::draw(const uint8_t* bmp, int32_t x, int32_t y)
{
    const boxer::bmpStat* stat = (const boxer::bmpStat*)bmp;
    assert(stat->width % 4 == 0);
    const boxer::bbfPixel* data = (const boxer::bbfPixel*)(bmp + stat->offset);
    bbfPixel* stage = (bbfPixel*)&m_stageData[(y*m_stageWidth)*2 + (x*2)];

    int32_t count = 0;
    int32_t width = 0;
    int32_t height = 0;
    while(height < stat->height)
    {
        while(width < stat->width)
        {
            if(data[count].p != 0xEE0 /*08df00 HTML*/)
            {
                stage->p = data[count].p;
            }
            count++;
            stage++;
            width++;
        }
        width = 0;
        height++;
        stage = (bbfPixel*)&m_stageData[((y+height)*m_stageWidth)*2 + (x*2)];
    }
}

void stage::show()
{
#ifdef __ANDROID__
    char buffer[PATH_MAX];
    memset(buffer, '\0', PATH_MAX);
    memcpy(buffer, androidData, strlen(androidData));
    strcat(buffer, "debug.bmp");
    FILE* debug = fopen(buffer, "w");
#else
    FILE* debug = fopen("debug.bmp", "w");
#endif
    if(debug)
    {
        boxer::bmpStat stat;
        stat.ident = 0x4d42;
        stat.reserved = 0;
        stat.offset = sizeof(boxer::bmpStat)+sizeof(boxer::colorTable);
        stat.headerSize = stat.offset - 0xE;
        stat.width = m_stageWidth;
        stat.height = m_stageHeight;
        stat.colorPlanes = 1;
        stat.compression = 3;
        stat.dataSize = (2*m_stageWidth*m_stageHeight);
        stat.size = stat.offset+stat.dataSize;
        stat.depth = 16;
        stat.paletteSize = 0;
        stat.important = 0;
        stat.horRes = 0xB13;
        stat.vertRes = 0xB13;

        boxer::colorTable table;
        table.R = 0xf800;
        table.G = 0x7e0;
        table.B = 0x1f;

        fwrite(&stat, 1, sizeof(boxer::bmpStat), debug);
        fwrite(&table, 1, sizeof(boxer::colorTable), debug);
        fwrite(m_stageData, 1, m_stageWidth*m_stageHeight*2, debug);
        fclose(debug);

#ifdef __ANDROID__
        JNIEnv* jThreadEnv = NULL;
        jVM->AttachCurrentThread(&jThreadEnv, NULL);
        jstring imageString = jThreadEnv->NewStringUTF(buffer);
        jThreadEnv->CallStaticVoidMethod(jBoxerEngine, jShowStage, imageString);
        jThreadEnv->DeleteLocalRef(imageString);
#endif
    }
}

int32_t stage::getWidth()
{
   return m_stageWidth;
}

int32_t stage::getHeight()
{
   return m_stageHeight;
}

}
