#define WAV_HEADER_SIZE 44
#define AUDIO_BUFFER_SIZE 1024

struct audioParam
{
    int32_t id;
    int32_t delay;
    bool keepGoing;
};

void preload(const char* path);
