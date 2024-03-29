#include "Util.h"
#include <cmath>

#include <SDL/SDL.h>

float currentTime()
{
    static bool firstCall = true;
    static long long startTime;

    long long curTime = SDL_GetTicks();

    if(firstCall)
    {
        firstCall = false;
        startTime = curTime;
    }

    return float(curTime - startTime);
}

uint64_t Random::seedUniquifier = 8682522807148012;
Random::Random() : seed(uniqueSeed() ^ uint64_t(currentTime)) {}
Random::Random(const uint64_t seed) : seed(initialScramble(seed)) {}

uint64_t Random::initialScramble(const uint64_t seed)
{
    return (seed ^ multiplier) & mask;
}

uint64_t Random::uniqueSeed()
{
    // L'Ecuyer, "Tables of Linear Congruential Generators of
    // Different Sizes and Good Lattice Structure", 1999
    for (;;) {
        const uint64_t current = seedUniquifier;
        const uint64_t next = current * 181783497276652981L;
        if (seedUniquifier == current)
        {
            seedUniquifier = next;
            return next;
        }
    }
}

int Random::next(const int bits)
{
    seed = (seed * multiplier + addend) & mask;
    
    return int(seed >> (48 - bits));
}

float Random::nextFloat()
{
    return float(next(24)) / float(1 << 24);
}

vec2 Random::nextVec2(float magnitude)
{
    float x = nextFloat() * magnitude * 2.f;
    float y = nextFloat() * magnitude * 2.f;

    return vec2(x - magnitude, y - magnitude);
}

uint32_t Random::nextInt()
{
    return next(32);
}

vec2 Random::nextIVec2(int magnitude)
{
    int x = nextInt(magnitude * 2);
    int y = nextInt(magnitude * 2);

    return vec2(x - magnitude, y - magnitude);
}

uint32_t Random::nextInt(const uint32_t bound)
{
    uint32_t r = next(31);
    const uint32_t m = bound - 1;
    if ((bound & m) == 0)  // i.e., bound is a power of 2
        r = uint32_t(bound * uint64_t(r) >> 31);
    else {
        for (uint32_t u = r;
            u - (r = u % bound) + m < 0;
            u = next(31));
    }
    return r;
}

    uint64_t Random::nextLong() {
        return ((uint64_t) (next(32)) << 32) + next(32);
    }

void Random::setSeed(const uint64_t newSeed)
{
    seed = initialScramble(newSeed);
}

// Perlin noise

float scaled_cosine(const float i) {
    return 0.5f * (1.0f - cos(i * PI));
}

constexpr int PERLIN_RES = 1024;

constexpr float PERLIN_OCTAVES = 4; // default to medium smooth
constexpr float PERLIN_AMP_FALLOFF = 0.5f; // 50% reduction/octave

constexpr int PERLIN_YWRAPB = 4;
constexpr int PERLIN_YWRAP = 1 << PERLIN_YWRAPB;
constexpr int PERLIN_ZWRAPB = 8;
constexpr int PERLIN_ZWRAP = 1 << PERLIN_ZWRAPB;

float perlin[PERLIN_RES + 1];

float Perlin::noise(float x, float y) { // stolen from Processing
    if (perlin[0] == 0) {
        Random r = Random(18295169L);

        for (float& i : perlin)
            i = r.nextFloat();
    }

    if (x < 0)
        x = -x;
    if (y < 0)
        y = -y;

    int xi = int(x);
    int yi = int(y);

    float xf = x - xi;
    float yf = y - yi;

    float r = 0;
    float ampl = 0.5f;

    for (int i = 0; i < PERLIN_OCTAVES; i++) {
        int of = xi + (yi << PERLIN_YWRAPB);

        const float rxf = scaled_cosine(xf);
        const float ryf = scaled_cosine(yf);

        float n1 = perlin[of % PERLIN_RES];
        n1 += rxf * (perlin[(of + 1) % PERLIN_RES] - n1);
        float n2 = perlin[(of + PERLIN_YWRAP) % PERLIN_RES];
        n2 += rxf * (perlin[(of + PERLIN_YWRAP + 1) % PERLIN_RES] - n2);
        n1 += ryf * (n2 - n1);

        of += PERLIN_ZWRAP;
        n2 = perlin[of % PERLIN_RES];
        n2 += rxf * (perlin[(of + 1) % PERLIN_RES] - n2);
        float n3 = perlin[(of + PERLIN_YWRAP) % PERLIN_RES];
        n3 += rxf * (perlin[(of + PERLIN_YWRAP + 1) % PERLIN_RES] - n3);
        n2 += ryf * (n3 - n2);

        n1 += scaled_cosine(0) * (n2 - n1);

        r += n1 * ampl;
        ampl *= PERLIN_AMP_FALLOFF;
        xi <<= 1;
        xf *= 2;
        yi <<= 1;
        yf *= 2;

        if (xf >= 1.0) {
            xi++;
            xf--;
        }

        if (yf >= 1.0) {
            yi++;
            yf--;
        }
    }

    return r;
}

float Perlin::noise(vec2 pos)
{
    return noise(pos.x, pos.y);
}

float clamp(float val, const float min, const float max)
{
    if (val < min)
        val = min;
    else if (val > max)
        val = max;

    return val;
}

bool glError()
{
    const GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        char err_s[5];
        itoa(err, err_s);

        prints("OpenGL error "); prints(err_s); prints("\n");
        return true;
    }

    return false;
}

void GLAPIENTRY error_callback(GLenum source,
                const GLenum type,
                GLuint id,
                const GLenum severity,
                GLsizei length,
                const GLchar* message,
                const void* userParam) {
    prints("GL error/warning: "); prints(message); prints("\n");
    
    //__debugbreak();
}

float radians(float deg)
{
    return deg * (PI / 180.f);
}

float degrees(float rad)
{
    return rad / (PI / 180.f);
}

/*float abs(float v)
{
    return v < 0 ? -v : v;
}*/

bool sign(float v)
{
    return (0 < v) - (v < 0);
}

float fract(float v)
{
    return v - floor(v);
}

float pow(float v, int p)
{
    for(int i = 0; i < p; i++)
        v *= v;
    
    return v;
}

float max(float a, float b)
{
    return a > b ? a : b;
}

float mod(float x, float y) {
  return x - trunc(x / y) * y;
}

vec3 max(const vec3& a, const vec3& b)
{
    return vec3(
        max(a.x, b.x),
        max(a.y, b.y),
        max(a.z, b.z)
    );
}


// NOTE: broken for negatives (doesn't matter for terrain height)
int roundFloat(float v)
{
    return int(v + 0.5f);
}

unsigned int murmurHash2(const char* str, int len)
{
    // TODO should this be an arg?
    constexpr unsigned int seed = 10;

    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.
    const unsigned int m = 0x5bd1e995;
    const int r = 24;

    // Initialize the hash to a 'random' value
    unsigned int h = seed ^ len;

    // Mix 4 bytes at a time into the hash
    const unsigned char* data = (const unsigned char*) str;

    while(len >= 4)
    {
        unsigned int k = *(unsigned int *)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    // Handle the last few bytes of the input array
    switch(len)
    {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
            h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}
/*
unsigned long strlen(const char *str)
{
    const char *s;
    for (s = str; *s; ++s);

    return (s - str);
}

char *strcpy(char *strDest, const char *strSrc)
{
    char *temp = strDest;
    while(*strDest++ = *strSrc++);

    return temp;
}

char *strcat(char *dest, const char *src)
{
    char *rdest = dest;

    while (*dest)
        dest++;
    
    while (*dest++ = *src++);

    return rdest;
}*/

void memcpy(void *dest, void *src, long unsigned int n)
{
   // Typecast src and dest addresses to (char *)
   char *csrc = (char *)src;
   char *cdest = (char *)dest;
  
   // Copy contents of src[] to dest[]
   for (int i=0; i<n; i++)
       cdest[i] = csrc[i];
}

void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

#ifdef DEBUG

#ifdef __unix__
#include <unistd.h>
#endif

void crash()
{
    fflush(stdout);

#ifdef __unix__
    _exit(0); // UNIX has a ruder function >:)
#else
    _Exit(0);
#endif
}

#else
void crash()
{
    SDL_Quit();
}

#endif
