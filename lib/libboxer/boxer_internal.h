#define WAV_HEADER_SIZE 44
#define AUDIO_BUFFER_SIZE 1024

namespace boxer
{

struct audioParam
{
    int32_t id;
    int32_t delay;
    bool keepGoing;
};

void* inputThread(void* param);

void preload(const char* path);
int32_t getDefaultFrameDelay();
void initializeDisplay();
const char* getDebugImagePath();
boxer::bmpStat* buildStageHeader();
boxer::colorTable* buildStageColorTable();
void writeDisplay(uint8_t* data);
void writeAudioResource(audioParam* param);
void shutdownAudio(int32_t id);
void initializeInput();
control getControlInput();
void shutdownInput();

}
