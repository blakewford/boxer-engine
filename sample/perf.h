#if __cplusplus > 199711L
#define PROFILE
#include <chrono>
using namespace std::chrono;
#endif

#ifdef PROFILE
milliseconds gStart;
milliseconds gEnd;
#define BEGIN_SCOPE gStart = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
#if defined(__ANDROID__) && (defined(__arm__) || defined(__i386__) || (defined(__mips__) && !defined(__mips64__)))
#define SCOPE(x) gEnd = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()); printf("%s(ms) %lld\n", x, gEnd.count() - gStart.count());
#else
#define SCOPE(x) gEnd = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()); printf("%s(ms) %ld\n", x, gEnd.count() - gStart.count());
#endif
#else
#define BEGIN_SCOPE
#define SCOPE(x)
#endif
