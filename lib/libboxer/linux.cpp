#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "boxer.h"
#include "boxer_internal.h"
#include <pulse/simple.h>

namespace boxer
{

int32_t getDefaultFrameDelay()
{
    return 1000;
}

void writeAudioResource(audioParam* param)
{
    const boxer::wavStat* stat = (const boxer::wavStat*)boxer::getResource(param->id);
    const uint8_t* data = (const uint8_t*)(boxer::getResource(param->id) + WAV_HEADER_SIZE);
    static const pa_sample_spec spec = { .format = PA_SAMPLE_S16LE, .rate = 44100, .channels = 2 };
    pa_simple* stream = pa_simple_new(NULL, NULL, PA_STREAM_PLAYBACK, NULL, "boxer_track", &spec, NULL, NULL, NULL);
    if(stream != NULL)
    {
        int32_t i = 0;
        while((i+AUDIO_BUFFER_SIZE < stat->size) && param->keepGoing)
        {
            if(pa_simple_write(stream, &data[i], AUDIO_BUFFER_SIZE, NULL) < 0)
            {
                break;
            }
            i+=AUDIO_BUFFER_SIZE;
        }
        pa_simple_drain(stream, NULL);
        pa_simple_free(stream);
    }
}

void shutdownAudio(int32_t id)
{
}

}
