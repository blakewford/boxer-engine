#if __cplusplus > 199711L
#define PROFILE
#include <chrono>
using namespace std::chrono;
#endif

#ifdef PROFILE
milliseconds gStart;
milliseconds gEnd;
#define BEGIN_SCOPE gStart = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
#define SCOPE(x) gEnd = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()); printf("%s(ms) %ld\n", x, gEnd.count() - gStart.count());
#else
#define BEGIN_SCOPE
#define SCOPE(x)
#endif
