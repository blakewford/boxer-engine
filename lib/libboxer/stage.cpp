#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "stage.h"
#include "boxer_internal.h"

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
    takeScreenshot(getDebugImagePath());
    writeDisplay(m_stageData);
}

int32_t stage::getWidth()
{
   return m_stageWidth;
}

int32_t stage::getHeight()
{
   return m_stageHeight;
}

const uint8_t* stage::getData()
{
   return m_stageData;
}

}
