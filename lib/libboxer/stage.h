#pragma once

#include "boxer.h"

namespace boxer
{

class stage
{
public:
             stage();
            ~stage();
        void init(int32_t id);
        void draw(const uint8_t* bmp, int32_t x, int32_t y);
        void show();

      int32_t getWidth();
      int32_t getHeight();
const uint8_t* getData();

private:
    int32_t m_stageWidth;
    int32_t m_stageHeight;
    uint8_t* m_stageData;
};

}
