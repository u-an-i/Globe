#define WIN32_LEAN_AND_MEAN             // discard rarely used components from Windows header

#include <SDL.h>
#include <SDL_main.h>                   // only include this one in the source file with main()!
#include <stdio.h>
#include <windows.h>
#include <mathimf.h>
#include <c-hashmap.h>
#include <stdlib.h>
#include <winhttp.h>
#include <turbojpeg.h>
#include <miniLZO.h>
#include <direct.h>

#pragma warning( disable : 4996 4244 )  // "safe" print functions, int-float-double conversion


//#define DEBUG

#ifdef DEBUG
#define LOG(x) printf x
#else
#define LOG(x)  
#endif


#define _Atomic volatile


int WIDTH = 720;
int HEIGHT = 720;

const int rasterTileSize = 256;
const char idFormat[] = "%d/%d/%d";
const unsigned char zero3[] = { '\0', '\0', '\0' };

const long double PI = 3.141592653589793238462643383279L;
const long double PIHalf = 1.570796326794896619231321691639L;
const long double PIDouble = 6.28318530717958647692528676655L;

// approximations, within 1 period length, precision is not bad but it is not good enough for the sizes and calculations involved here
/*
long double sinl(long double t) {
    long double s = t < 0.0L ? -1.0L : 1.0L;
    t = fabsl(t);
    return s * (t < 0.4292L ?
        t : (
            t < (PI - 0.4292L) ?
            (1.0L - (PI / 4.74701335L) * (PI / 4.74701335L) * (t - PIHalf) * (t - PIHalf)) : (
                t < (PI + 0.4292L) ?
                (PI - t) : (
                    t < (PIDouble - 0.4292L) ?
                    -(1.0L - (PI / 4.74701335L) * (PI / 4.74701335L) * (t - (3.0L * PIHalf)) * (t - (3.0L * PIHalf))) :
                    (-PIDouble + t)
                    )
                )
            )
        );
}

long double cosl(long double t) {
    t = fabsl(t);
    return t < (PIHalf - 0.4292L) ?
        (1.0L - (PI / 4.74701335L) * (PI / 4.74701335L) * t * t) : (
            t < (PIHalf + 0.4292L) ?
            (PIHalf - t) : (
                t < (3.0L * PIHalf - 0.4292L) ?
                -(1.0L - (PI / 4.74701335L) * (PI / 4.74701335L) * (t - PI) * (t - PI)) : (
                    t < (3.0L * PIHalf + 0.4292L) ?
                    (-3.0L * PIHalf + t) : (1.0L - (PI / 4.74701335L) * (PI / 4.74701335L) * (t - PIDouble) * (t - PIDouble))
                    )
                )
            );
}

long double asinl(long double s) {
    return s < -0.4292L ? -PIHalf + 1.511L * sqrtl(1.0L + s) : (s < 0.4292L ? s : (PIHalf - 1.511L * sqrtl(1.0L - s)));
}

long double acosl(long double s) {
    return s < -0.4292L ? PI - 1.511L * sqrtl(1.0L + s) : (s < 0.4292L ? PIHalf - s : (1.511L * sqrtl(1.0L - s)));
}
*/

int centerX, centerY;
long double rScale;
long double rScaleSqr;

long double phiLeft = 0.0L;
long double axisTilt = 0.0L;

typedef struct PT
{
    long double p;
    long double t;
} pt;

pt at(int x, int y)
{
    int xC = x - centerX;
    int yC = y - centerY;
    long double r2Sqr = rScaleSqr - xC * xC;
    if (yC * yC <= r2Sqr)
    {
        if (r2Sqr > 0.0L)
        {
            long double r2Sqrt = sqrtl(r2Sqr);
            long double yC2 = yC / r2Sqrt;
            long double cAT = cosl(axisTilt);
            long double ySpace = r2Sqrt * (yC2 * cAT - copysignl(sqrtl((1.0L - yC2 * yC2) * (1.0L - cAT * cAT)), axisTilt));                    // == r2Sqrt * sinl(asinl(yC / r2Sqrt) - axisTilt);
            long double r2ScAT = r2Sqrt * cAT;
            long double r3Sqr = rScaleSqr - ySpace * ySpace;
            pt value;
            value.p = fmodl(phiLeft + (((axisTilt > 0.0L && -yC > r2ScAT) || (axisTilt < 0.0L && yC > r2ScAT)) ? -1.0L : 1.0L) * (r3Sqr > 0.0L ? sqrtl(r3Sqr) > abs(xC) ? acosl(-xC / sqrtl(r3Sqr)) : (PIHalf + copysignl(PIHalf, xC)) : PIHalf) + PIDouble, PIDouble);
            value.t = asinl(ySpace / rScale);
            return value;
        }
        else
        {
            pt value;
            value.p = fmodl(phiLeft + (xC > 0 ? PI : 0.0L) + PIDouble, PIDouble);
            value.t = 0.0L;
            return value;
        }
    }
    pt value;
    value.p = 0.0L;
    value.t = 2.0L;
    return value;
}

pt getOffsetsFrom(int x, int y, pt sc) {
    int xC = x - centerX;
    int yC = y - centerY;
    long double r2Sqr = rScaleSqr - xC * xC;
    if (yC * yC <= r2Sqr)
    {
        if (r2Sqr > 0.0L)
        {
            long double r2Sqrt = sqrtl(r2Sqr);
            long double yC2 = yC / r2Sqrt;
            long double sT = sinl(sc.t);
            long double arg2Sqr = rScaleSqr * sT * sT / r2Sqr;
            long double arg1Co = 1.0L - yC2 * yC2;
            long double axisTilt = asinl(yC2 * sqrtl(1.0L - arg2Sqr) - copysignl(sqrtl(arg2Sqr * arg1Co), sc.t));                   // == asinl(yC2) - asinl(rScale * sinl(sc.t) / r2Sqrt);
            long double cAT = cosl(axisTilt);
            long double r2ScAT = r2Sqrt * cAT;
            long double ySpace = r2Sqrt * (yC2 * cAT - copysignl(sqrtl(arg1Co * (1.0L - cAT * cAT)), axisTilt));
            long double r3Sqr = rScaleSqr - ySpace * ySpace;
            pt value;
            value.p = fmodl((sc.p - (((axisTilt > 0.0L && -yC > r2ScAT) || (axisTilt < 0.0L && yC > r2ScAT)) ? -1.0L : 1.0L) * (r3Sqr > 0.0L ? sqrtl(r3Sqr) > abs(xC) ? acosl(-xC / sqrtl(r3Sqr)) : (PIHalf + copysignl(PIHalf, xC)) : PIHalf)) + PIDouble, PIDouble);
            value.t = axisTilt;
            return value;
        } 
        else {
            pt value;
            value.p = fmodl(sc.p - (xC > 0 ? PI : 0.0L) + PIDouble, PIDouble);
            value.t = 0.0L;
            return value;
        }
    }
    pt value;
    value.p = 0.0L;
    value.t = 2.0L;
    return value;
}

pt getOffsetsFromWithRadius(int x, int y, pt sc, long double rScale) {
    int xC = x - centerX;
    int yC = y - centerY;
    long double rScaleSqr = rScale * rScale;
    long double r2Sqr = rScaleSqr - xC * xC;
    if (yC * yC <= r2Sqr)
    {
        if (r2Sqr > 0.0L)
        {
            long double r2Sqrt = sqrtl(r2Sqr);
            long double yC2 = yC / r2Sqrt;
            long double sT = sinl(sc.t);
            long double arg2Sqr = rScaleSqr * sT * sT / r2Sqr;
            long double arg1Co = 1.0L - yC2 * yC2;
            long double axisTilt = asinl(yC2 * sqrtl(1.0L - arg2Sqr) - copysignl(sqrtl(arg2Sqr * arg1Co), sc.t));                   // == asinl(yC2) - asinl(rScale * sinl(sc.t) / r2Sqrt);
            long double cAT = cosl(axisTilt);
            long double r2ScAT = r2Sqrt * cAT;
            long double ySpace = r2Sqrt * (yC2 * cAT - copysignl(sqrtl(arg1Co * (1.0L - cAT * cAT)), axisTilt));
            long double r3Sqr = rScaleSqr - ySpace * ySpace;
            pt value;
            value.p = fmodl((sc.p - (((axisTilt > 0.0L && -yC > r2ScAT) || (axisTilt < 0.0L && yC > r2ScAT)) ? -1.0L : 1.0L) * (r3Sqr > 0.0L ? sqrtl(r3Sqr) > abs(xC) ? acosl(-xC / sqrtl(r3Sqr)) : (PIHalf + copysignl(PIHalf, xC)) : PIHalf)) + PIDouble, PIDouble);
            value.t = axisTilt;
            return value;
        }
        else {
            pt value;
            value.p = fmodl(sc.p - (xC > 0 ? PI : 0.0L) + PIDouble, PIDouble);
            value.t = 0.0L;
            return value;
        }
    }
    pt value;
    value.p = 0.0L;
    value.t = 2.0L;
    return value;
}


float zoomF;
int zoom;
long double stepSize;

void determineZoom() {
    pt middleLeft = at(0, HEIGHT / 2);
    float newZoom;
    long double newStepSize;
    if (middleLeft.t == 2.0L) {
        newZoom = log2l(rScale / rasterTileSize);        // == log2l(2 * rScale * 2 / rasterTileSize);
        newZoom += 2.0F;                                 // == above continued
        newStepSize = PIDouble / 48.0L;
    } else {
        long double deltaP = at(WIDTH - 1, HEIGHT / 2).p - middleLeft.p;
        deltaP += (deltaP <= 0.0L ? PIDouble : 0.0L);
        newZoom = log2l((WIDTH / deltaP) * PIDouble / rasterTileSize);
        newStepSize = deltaP / 16.0L;
    }
    if (newZoom >= 0.0F && newZoom <= 30.0F) {
        zoomF = newZoom;
        zoom = (int)ceilf(newZoom);
        stepSize = newStepSize;
    }
}


const double PID = 3.141592653589793238462643383279;
const double PIHalfD = 1.570796326794896619231321691639;
const double PIDoubleD = 6.28318530717958647692528676655;
/*
double sin(double t) {
    double s = t < 0.0 ? -1.0 : 1.0;
    t = fabs(t);
    return s * (t < 0.4292 ?
        t : (
            t < (PID - 0.4292) ?
            (1.0 - (PID / 4.74701335) * (PID / 4.74701335) * (t - PIHalfD) * (t - PIHalfD)) : (
                t < (PID + 0.4292) ?
                (PID - t) : (
                    t < (PIDoubleD - 0.4292) ?
                    -(1.0 - (PID / 4.74701335) * (PID / 4.74701335) * (t - (3.0 * PIHalfD)) * (t - (3.0 * PIHalfD))) :
                    (-PIDoubleD + t)
                    )
                )
            )
        );
}

double cos(double t) {
    t = fabs(t);
    return t < (PIHalfD - 0.4292) ?
        (1.0 - (PID / 4.74701335) * (PID / 4.74701335) * t * t) : (
            t < (PIHalfD + 0.4292) ?
            (PIHalfD - t) : (
                t < (3.0 * PIHalfD - 0.4292) ?
                -(1.0 - (PID / 4.74701335) * (PID / 4.74701335) * (t - PID) * (t - PID)) : (
                    t < (3.0 * PIHalfD + 0.4292) ?
                    (-3.0 * PIHalfD + t) : (1.0 - (PID / 4.74701335) * (PID / 4.74701335) * (t - PIDoubleD) * (t - PIDoubleD))
                    )
                )
            );
}

double asin(double s) {
    return s < -0.4292 ? -PIHalfD + 1.511 * sqrt(1.0 + s) : (s < 0.4292 ? s : (PIHalfD - 1.511 * sqrt(1.0 - s)));
}

double acos(double s) {
    return s < -0.4292 ? PID - 1.511 * sqrt(1.0 + s) : (s < 0.4292 ? PIHalfD - s : (1.511 * sqrt(1.0 - s)));
}
*/
double rScaleD;
double rScaleSqrD;

double phiLeftD = 0.0;
double axisTiltD = 0.0;

typedef struct PTD
{
    double p;
    double t;
} ptD;

ptD atD(int x, int y)
{
    int xC = x - centerX;
    int yC = y - centerY;
    double r2Sqr = rScaleSqrD - xC * xC;
    if (yC * yC <= r2Sqr)
    {
        if (r2Sqr > 0.0)
        {
            double r2Sqrt = sqrt(r2Sqr);
            double yC2 = yC / r2Sqrt;
            double cAT = cos(axisTiltD);
            double ySpace = r2Sqrt * (yC2 * cAT - copysign(sqrt((1.0 - yC2 * yC2) * (1.0 - cAT * cAT)), axisTiltD));                     // == r2Sqrt * sin(asin(yC / r2Sqrt) - axisTiltD);
            double r2ScAT = r2Sqrt * cAT;
            double r3Sqr = rScaleSqrD - ySpace * ySpace;
            ptD value;
            value.p = fmod(phiLeftD + (((axisTiltD > 0.0 && -yC > r2ScAT) || (axisTiltD < 0.0 && yC > r2ScAT)) ? -1.0 : 1.0) * (r3Sqr > 0.0 ? sqrt(r3Sqr) > abs(xC) ? acos(-xC / sqrt(r3Sqr)) : (PIHalfD + copysign(PIHalfD, xC)) : PIHalfD) + PIDoubleD, PIDoubleD);
            value.t = asin(ySpace / rScaleD);
            return value;
        }
        else
        {
            ptD value;
            value.p = fmod(phiLeftD + (xC > 0 ? PID : 0.0) + PIDoubleD, PIDoubleD);
            value.t = 0.0;
            return value;
        }
    }
    ptD value;
    value.p = 0.0;
    value.t = 2.0;
    return value;
}


const float PIF = 3.141592653589793238462643383279F;
const float PIHalfF = 1.570796326794896619231321691639F;
const float PIDoubleF = 6.28318530717958647692528676655F;
/*
float sinf(float t) {
    float s = t < 0.0F ? -1.0F : 1.0F;
    t = fabsf(t);
    return s * (t < 0.4292F ?
        t : (
            t < (PIF - 0.4292F) ?
            (1.0F - (PIF / 4.74701335F) * (PIF / 4.74701335F) * (t - PIHalfF) * (t - PIHalfF)) : (
                t < (PIF + 0.4292F) ?
                (PIF - t) : (
                    t < (PIDoubleF - 0.4292F) ?
                    -(1.0F - (PIF / 4.74701335F) * (PIF / 4.74701335F) * (t - (3.0F * PIHalfF)) * (t - (3.0F * PIHalfF))) :
                    (-PIDoubleF + t)
                    )
                )
            )
        );
}

float cosf(float t) {
    t = fabsf(t);
    return t < (PIHalfF - 0.4292F) ?
        (1.0F - (PIF / 4.74701335F) * (PIF / 4.74701335F) * t * t) : (
            t < (PIHalfF + 0.4292F) ?
            (PIHalfF - t) : (
                t < (3.0F * PIHalfF - 0.4292F) ?
                -(1.0F - (PIF / 4.74701335F) * (PIF / 4.74701335F) * (t - PIF) * (t - PIF)) : (
                    t < (3.0F * PIHalfF + 0.4292F) ?
                    (-3.0F * PIHalfF + t) : (1.0F - (PIF / 4.74701335F) * (PIF / 4.74701335F) * (t - PIDoubleF) * (t - PIDoubleF))
                    )
                )
            );
}

float asinf(float s) {
    return s < -0.4292F ? -PIHalfF + 1.511F * sqrtf(1.0F + s) : (s < 0.4292F ? s : (PIHalfF - 1.511F * sqrtf(1.0F - s)));
}

float acosf(float s) {
    return s < -0.4292F ? PIF - 1.511F * sqrtf(1.0F + s) : (s < 0.4292F ? PIHalfF - s : (1.511F * sqrtf(1.0F - s)));
}
*/
float rScaleF;
float rScaleSqrF;

float phiLeftF = 0.0F;
float axisTiltF = 0.0F;

typedef struct PTF
{
    float p;
    float t;
} ptF;

ptF atF(int x, int y)
{
    int xC = x - centerX;
    int yC = y - centerY;
    float r2Sqr = rScaleSqrF - xC * xC;
    if (yC * yC <= r2Sqr)
    {
        if (r2Sqr > 0.0F)
        {
            float r2Sqrt = sqrtf(r2Sqr);
            float yC2 = yC / r2Sqrt;
            float cAT = cosf(axisTiltF);
            float ySpace = r2Sqrt * (yC2 * cAT - copysignf(sqrtf((1.0F - yC2 * yC2) * (1.0F - cAT * cAT)), axisTiltF));                     // == r2Sqrt * sinf(asinf(yC / r2Sqrt) - axisTiltF);
            float r2ScAT = r2Sqrt * cAT;
            float r3Sqr = rScaleSqrF - ySpace * ySpace;
            ptF value;
            value.p = fmodf(phiLeftF + (((axisTiltF > 0.0F && -yC > r2ScAT) || (axisTiltF < 0.0F && yC > r2ScAT)) ? -1.0F : 1.0F) * (r3Sqr > 0.0F ? sqrtf(r3Sqr) > abs(xC) ? acosf(-xC / sqrtf(r3Sqr)) : (PIHalfF + copysignf(PIHalfF, xC)) : PIHalfF) + PIDoubleF, PIDoubleF);
            value.t = asinf(ySpace / rScaleF);
            return value;
        }
        else
        {
            ptF value;
            value.p = fmodf(phiLeftF + (xC > 0 ? PIF : 0.0F) + PIDoubleF, PIDoubleF);
            value.t = 0.0F;
            return value;
        }
    }
    ptF value;
    value.p = 0.0F;
    value.t = 2.0F;
    return value;
}

ptF atFWithoutOffsets(float x, float y)
{
    float xC = x - centerX;
    float yC = y - centerY;
    float r2Sqr = rScaleSqrF - xC * xC;
    float yC2 = yC * yC;
    if (yC2 <= r2Sqr)
    {
        if (r2Sqr > 0.0F)
        {
            float r3Sqr = rScaleSqrF - yC2;
            ptF value;
            value.p = r3Sqr > 0.0F ? sqrtf(r3Sqr) > fabsf(xC) ? acosf(-xC / sqrtf(r3Sqr)) : (PIHalfF + copysignf(PIHalfF, xC)) : PIHalfF;
            value.t = asinf(yC / rScaleF);
            return value;
        }
        else
        {
            ptF value;
            value.p = xC > 0.0F ? PIF : 0.0F;
            value.t = 0.0F;
            return value;
        }
    }
    ptF value;
    value.p = 0.0F;
    value.t = 2.0F;
    return value;
}


// apparently at zoomLevel around before 30, float cannot store the values involved correctly anymore
void determineZoomF() {
    ptF middleLeft = atF(0, HEIGHT / 2);
    float newZoom;
    float newStepSize;
    if (middleLeft.t == 2.0F) {
        newZoom = log2f(rScaleF / rasterTileSize);              // == log2f(2 * rScaleF * 2 / rasterTileSize);
        newZoom += 2.0F;                                        // == above continued
        newStepSize = PIDoubleF / 48.0F;
    }
    else {
        float deltaP = atF(WIDTH - 1, HEIGHT / 2).p - middleLeft.p;
        deltaP += (deltaP <= 0.0F ? PIDoubleF : 0.0F);
        newZoom = log2f((WIDTH / deltaP) * PIDoubleF / rasterTileSize);
        newStepSize = deltaP / 16.0F;
    }
    if (newZoom >= 0.0F && newZoom <= 30.0F) {
        zoomF = newZoom;
        zoom = (int)ceilf(newZoom);
        stepSize = newStepSize;                                 // use non-float stepSize on purpose
    }
}


typedef struct QueueFillLevel {
    unsigned long long count;
} queueFillLevel;

void measurePresence(void* key, size_t ksize, uintptr_t value, void* usr) {
    ((queueFillLevel*)usr)->count += (unsigned long long)value;
}

_Atomic int allImagesRequestedPresent;
hashmap* imgPresent;
hashmap* imgRequested;

typedef struct AsyncId {
    char* id;
    int idLength;
    HINTERNET hRequest, hConnect, hSession;
    unsigned char* buffer;
    int bytesRead;
} asyncId;

_Atomic int notquitrequested;

char* cachePath;
size_t cachePathLength;

void onImageLoading(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength) {
    asyncId* aId = (asyncId*)dwContext;
    if (notquitrequested) {
        if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_READ_COMPLETE) {
            if (dwStatusInformationLength > 0) {
                aId->bytesRead += dwStatusInformationLength;
                int numberBytesToRead = rasterTileSize * rasterTileSize * 3 - aId->bytesRead;
                if (numberBytesToRead > 0) {
                    WinHttpReadData(aId->hRequest, aId->buffer + aId->bytesRead, numberBytesToRead, NULL);
                    return;
                }
            }
            else {
                tjhandle tjInstance = tj3Init(TJINIT_DECOMPRESS);
                if (tjInstance != NULL) {
                    if (tj3DecompressHeader(tjInstance, aId->buffer, aId->bytesRead) == 0) {
                        unsigned char* pixels = malloc(TJSCALED(tj3Get(tjInstance, TJPARAM_JPEGWIDTH), TJUNSCALED) * TJSCALED(tj3Get(tjInstance, TJPARAM_JPEGHEIGHT), TJUNSCALED) * tjPixelSize[TJPF_RGB]);     // malloced length expectation == rasterTileSize * rasterTileSize * 3
                        if (pixels != NULL) {
                            if (tj3Decompress8(tjInstance, aId->buffer, aId->bytesRead, pixels, 0, TJPF_RGB) == 0) {
                                hashmap_set(imgPresent, aId->id, aId->idLength, (uintptr_t)pixels);
                            }
                            else {
                                free(pixels);
                            }
                        }
                    }
                    tj3Destroy(tjInstance);
                }
            }
            hashmap_set(imgRequested, (void*)(aId->id), aId->idLength, (uintptr_t)0);
            queueFillLevel counts;
            counts.count = 0;
            hashmap_iterate(imgRequested, measurePresence, (void*)&counts);
            if (counts.count == 0) {
                allImagesRequestedPresent = 1;
            }
            char* cacheFilePath = malloc(cachePathLength + 24 + 1);
            if (cacheFilePath != NULL) {
                char* idStartInCachePath = cacheFilePath + cachePathLength;
                memcpy(cacheFilePath, cachePath, cachePathLength);
                int number = 0;
                while (number < aId->idLength) {
                    char digit = aId->id[number];
                    if (digit == '/') {
                        digit = '-';
                    }
                    idStartInCachePath[number] = digit;
                    ++number;
                }
                idStartInCachePath[number] = '\0';
                FILE* cacheFile = fopen(cacheFilePath, "wb");
                if (cacheFile != NULL) {
                    fwrite(aId->buffer, 1, aId->bytesRead, cacheFile);
                    fclose(cacheFile);
                }
                free(cacheFilePath);
            }
            goto CLOSE_OPEN;
        }
        else if (dwInternetStatus == WINHTTP_CALLBACK_FLAG_DATA_AVAILABLE || dwInternetStatus == WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE) {
            int numberBytesToRead = rasterTileSize * rasterTileSize * 3 - aId->bytesRead;
            if (numberBytesToRead > 0) {
                WinHttpReadData(aId->hRequest, aId->buffer + aId->bytesRead, numberBytesToRead, NULL);
            }
            else {
                goto CLOSE_OPEN;
            }
        }
        else if (dwInternetStatus == WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE) {
            aId->buffer = malloc(rasterTileSize * rasterTileSize * 3);                      // using size of uncompressed image hoping it is sufficient (checked above)
            if (aId->buffer != NULL) {
                WinHttpReceiveResponse(aId->hRequest, NULL);
            }
            else {
                goto CLOSE_OPEN;
            }
        }
        else if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_REQUEST_ERROR) {
            hashmap_set(imgRequested, (void*)(aId->id), aId->idLength, (uintptr_t)0);
            queueFillLevel counts;
            counts.count = 0;
            hashmap_iterate(imgRequested, measurePresence, (void*)&counts);
            if (counts.count == 0) {
                allImagesRequestedPresent = 1;
            }
            goto CLOSE_OPEN;
        }
    }
    else {
CLOSE_OPEN:
        if (aId->buffer != NULL) {
            free(aId->buffer);
        }
        WinHttpSetStatusCallback(aId->hSession,
            NULL,
            WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
            (DWORD_PTR)NULL);
        WinHttpCloseHandle(aId->hRequest);
        WinHttpCloseHandle(aId->hConnect);
        WinHttpCloseHandle(aId->hSession);
        free(aId);
    }
}

_Atomic int doCollecting;
_Atomic int collecting;
_Atomic int rastered;
_Atomic int notScheduled;
_Atomic int wantsCompletion;
_Atomic int checkingImageRequests;
_Atomic int dontWaitForCollector;

int maxThreads;

typedef struct IdData {
    char id[25];                                // max length of id is 24 + \0 (digits for zoom level 30, xy 2^30, 2 /)
    int idLength;
    struct IdData* next;
} idData;

typedef struct ThreadData {
    int yStart;
    int yEnd;
    idData dId;
    _Atomic int rastering;
    _Atomic int imageRequestRequested;
    hashmap* lastQueue;
    HANDLE hThread;
    HANDLE hComplete;
} threadData;

threadData* threadsData;

wchar_t* host;
wchar_t* pathFormat;
wchar_t* path;

char* cachePathCollector;
char* idStartInCachePath;

unsigned __stdcall collector(void* data) {
    int processPossibleAdditions = 1;
    do {
        do {
            collecting = 1;
            for (int i = 0; i < maxThreads; ++i) {
                if (threadsData[i].imageRequestRequested) {
                    threadsData[i].imageRequestRequested = 0;
                    idData* dId = &(threadsData[i].dId);
                    do {
                        int idLength = dId->idLength;
                        if (idLength > 0) {
                            char* id = dId->id;

                            uintptr_t result;
                            if (!hashmap_get(imgRequested, (void*)id, idLength, &result)) {
                                int number = 0;
                                while (number < idLength) {
                                    char digit = id[number];
                                    if (digit == '/') {
                                        digit = '-';
                                    }
                                    idStartInCachePath[number] = digit;
                                    ++number;
                                }
                                idStartInCachePath[number] = '\0';
                                FILE* cacheFile = fopen(cachePathCollector, "rb");
                                if (cacheFile != NULL) {
                                    unsigned char* cachedImage = malloc(rasterTileSize * rasterTileSize * 3);                                               // using uncompressed size hoping it suffices, checked below
                                    if (cachedImage != NULL) {
                                        size_t sizeRead = fread(cachedImage, sizeof(unsigned char), rasterTileSize * rasterTileSize * 3, cacheFile);
                                        if (sizeRead != 0 && (sizeRead == rasterTileSize * rasterTileSize * 3 || feof(cacheFile))) {
                                            fclose(cacheFile);
                                            tjhandle tjInstance = tj3Init(TJINIT_DECOMPRESS);
                                            if (tjInstance != NULL) {
                                                if (tj3DecompressHeader(tjInstance, cachedImage, sizeRead) == 0) {
                                                    unsigned char* pixels = malloc(TJSCALED(tj3Get(tjInstance, TJPARAM_JPEGWIDTH), TJUNSCALED) * TJSCALED(tj3Get(tjInstance, TJPARAM_JPEGHEIGHT), TJUNSCALED) * tjPixelSize[TJPF_RGB]);     // malloced length expectation == rasterTileSize * rasterTileSize * 3
                                                    if (pixels != NULL) {
                                                        if (tj3Decompress8(tjInstance, cachedImage, sizeRead, pixels, 0, TJPF_RGB) == 0) {
                                                            free(cachedImage);
                                                            char* pId = malloc(idLength);
                                                            if (pId != NULL) {
                                                                memcpy(pId, id, idLength);
                                                                if (hashmap_sets_left_before_resize(imgPresent) <= 2) {                                                         // <= 1 should suffice but crash was observed in hashmap_get(imgPresent, ...)
                                                                    dontWaitForCollector = 0;
                                                                    int countRastering;
                                                                    do {
                                                                        if (!notquitrequested) {
                                                                            goto LIKE_AIDNULL;
                                                                        }
                                                                        countRastering = 0;
                                                                        for (int j = 0; j < maxThreads; ++j) {
                                                                            countRastering += threadsData[j].rastering;
                                                                        }
                                                                    } while (countRastering || !notScheduled || wantsCompletion || !allImagesRequestedPresent);
                                                                    hashmap_set(imgPresent, (void*)pId, idLength, (uintptr_t)pixels);
                                                                    hashmap_set(imgRequested, (void*)pId, idLength, (uintptr_t)0);
                                                                    dontWaitForCollector = 1;
                                                                    goto AFTER_IMG_REQUEST;
                                                                }
                                                                else {
                                                                    hashmap_set(imgPresent, (void*)pId, idLength, (uintptr_t)pixels);
                                                                    hashmap_set(imgRequested, (void*)pId, idLength, (uintptr_t)0);
                                                                    LOG(("from cache: %s\n", id));
                                                                    goto AFTER_IMG_REQUEST;
                                                                }
                                                            }
                                                        }
                                                        free(pixels);
                                                    }
                                                }
                                                tj3Destroy(tjInstance);
                                            }
                                        }
                                        else {
                                            fclose(cacheFile);
                                        }
                                        free(cachedImage);
                                    }
                                    else {
                                        fclose(cacheFile);
                                    }
                                }

                                HINTERNET  hSession = NULL,
                                    hConnect = NULL,
                                    hRequest = NULL;

                                // Use WinHttpOpen to obtain a session handle.
                                hSession = WinHttpOpen(L"WinHTTP Globe/1.0",
                                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME,
                                    WINHTTP_NO_PROXY_BYPASS,
                                    WINHTTP_FLAG_ASYNC);

                                // Specify an HTTP server.
                                if (hSession) {
                                    if (WinHttpSetStatusCallback(hSession, (WINHTTP_STATUS_CALLBACK)onImageLoading, WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE | WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE | WINHTTP_CALLBACK_FLAG_DATA_AVAILABLE | WINHTTP_CALLBACK_STATUS_READ_COMPLETE | WINHTTP_CALLBACK_STATUS_REQUEST_ERROR, (DWORD_PTR)NULL) == NULL) {
                                        hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
                                    }
                                }

                                // Create an HTTP request handle.
                                if (hConnect) {
                                    wchar_t wId[25];
                                    mbstowcs(wId, id, 25);
                                    _swprintf(path, pathFormat, wId);
                                    hRequest = WinHttpOpenRequest(hConnect, L"GET", path,
                                        NULL, WINHTTP_NO_REFERER,
                                        WINHTTP_DEFAULT_ACCEPT_TYPES,
                                        WINHTTP_FLAG_SECURE);
                                }
                                else {
                                    if (hSession) {
                                        WinHttpSetStatusCallback(hSession,
                                            NULL,
                                            WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
                                            (DWORD_PTR)NULL);
                                        WinHttpCloseHandle(hSession);
                                    }
                                }

                                // Send a request.
                                if (hRequest) {
                                    asyncId* aId = malloc(sizeof(asyncId));
                                    if (aId != NULL) {
                                        aId->id = malloc(idLength);
                                        if (aId->id != NULL) {
                                            memcpy(aId->id, id, idLength);
                                            aId->idLength = idLength;
                                            aId->hRequest = hRequest;
                                            aId->hConnect = hConnect;
                                            aId->hSession = hSession;
                                            aId->buffer = NULL;
                                            aId->bytesRead = 0;
                                            LOG(("%s\n", id));
                                            if (hashmap_sets_left_before_resize(imgPresent) <= 2) {         // <= 1 should suffice but crash was observed in hashmap_get(imgPresent, ...)
                                                dontWaitForCollector = 0;
                                                int countRastering;
                                                do {
                                                    if (!notquitrequested) {
                                                        free(aId->id);
                                                        free(aId);
                                                        goto LIKE_AIDNULL;
                                                    }
                                                    countRastering = 0;
                                                    for (int j = 0; j < maxThreads; ++j) {
                                                        countRastering += threadsData[j].rastering;
                                                    }
                                                } while (countRastering || !notScheduled || wantsCompletion || !allImagesRequestedPresent);
                                                hashmap_set(imgRequested, (void*)(aId->id), aId->idLength, (uintptr_t)aId);
                                                hashmap_set(imgPresent, (void*)(aId->id), aId->idLength, (uintptr_t)NULL);
                                                dontWaitForCollector = 1;
                                            }
                                            else {
                                                hashmap_set(imgRequested, (void*)(aId->id), aId->idLength, (uintptr_t)aId);
                                                hashmap_set(imgPresent, (void*)(aId->id), aId->idLength, (uintptr_t)NULL);          // preset memory for it be already present during async callbacks
                                            }
                                            allImagesRequestedPresent = 0;
                                            if (!WinHttpSendRequest(hRequest,
                                                WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                                WINHTTP_NO_REQUEST_DATA, 0,
                                                0, (DWORD_PTR)aId)) {
                                                hashmap_set(imgRequested, (void*)(aId->id), aId->idLength, (uintptr_t)0);
                                                queueFillLevel counts;
                                                counts.count = 0;
                                                hashmap_iterate(imgRequested, measurePresence, (void*)&counts);
                                                if (counts.count == 0) {
                                                    allImagesRequestedPresent = 1;
                                                }
                                                free(aId);
                                                goto LIKE_AIDNULL;
                                            }
                                        }
                                        else {
                                            free(aId);
                                            goto LIKE_AIDNULL;
                                        }
                                    }
                                    else {
LIKE_AIDNULL: ;
                                        WinHttpSetStatusCallback(hSession,
                                            NULL,
                                            WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
                                            (DWORD_PTR)NULL);
                                        WinHttpCloseHandle(hRequest);
                                        WinHttpCloseHandle(hConnect);
                                        WinHttpCloseHandle(hSession);
                                        if (!notquitrequested) {
                                            return 0;
                                        }
                                    }
                                }
                                else {
                                    if (hConnect) WinHttpCloseHandle(hConnect);
                                    if (hSession) {
                                        WinHttpSetStatusCallback(hSession,
                                            NULL,
                                            WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
                                            (DWORD_PTR)NULL);
                                        WinHttpCloseHandle(hSession);
                                    }
                                }
                            }
AFTER_IMG_REQUEST: ;
                            dId->idLength = 0;
                        }
                        dId = dId->next;
                    } while (dId != NULL);
                }
            }
            collecting = 0;
            if (doCollecting && (!rastered || !notScheduled)) {
                processPossibleAdditions = 1;
            }
            else {
                break;
            }
        } while (true);
    } while (doCollecting && processPossibleAdditions--);
    checkingImageRequests = 0;
    return 0;
}


_Atomic int queued;

typedef enum MovementDirection {
    ZIN, ZOUT, SIDE, INIT, REFRESH
} movementDirection;

movementDirection dir;

void* buffer;
void* region;
int pitch;

unsigned char* elevationData;
int elevationDataAvailable = 0;

typedef struct Pixel {
    int sourceX, sourceY;
    int targetX, targetY;
} pixel;

void pickPixel(pixel* p, unsigned char* pixels) {
    memcpy((void*)(((unsigned char*)buffer) + (p->targetY * pitch + p->targetX * 3)), (void*)(pixels + (p->sourceY * rasterTileSize + p->sourceX) * 3), 3);
}

const double sx = 0.57735;
const double sy = 0.57735;
const double sz = -0.57735;
const unsigned char w = 255;
const unsigned int r = 0;
const unsigned int g = 1;
const unsigned int b = 2;

const float maxZoomLighting = 2.5F;

/// <summary>
/// lights a pixel from a predefined light ray
/// </summary>
/// <param name="x">x coordinate of the pixel in the window</param>
/// <param name="y">y coordinate of the pixel in the window</param>
/// <param name="rgb">array of 3 unsigned char containing the original color, gets overwritten to the lighted color</param>
void lightPixel(int x, int y, unsigned char* rgb) {
    ptF angles = atFWithoutOffsets(x, y);
    double ct = cos(-angles.t);                             // doubles to alleviate banding, does not avail
    double nx = ct * cos(angles.p + PID);
    double ny = ct * sin(angles.p + PID);
    double nz = sin(-angles.t);
    double sn = 2.0 * (sx * nx + sy * ny + sz * nz);
    double rx = sx - sn * nx;
    double ry = sy - sn * ny;
    double rz = sz - sn * nz;
    double a = -ry / sqrt(rx * rx + ry * ry + rz * rz);
    a = a < 0.0 ? 0.0 : a;
    double f;
    unsigned char m = max(rgb[r], max(rgb[g], rgb[b]));
    if (m == rgb[r])
        f = 0.275;
    else
        if (m == rgb[g])
            f = 0.35;
        else
            f = 0.5;
    double t = f * a * a * (1.0F - zoomF / maxZoomLighting);
    rgb[r] += (unsigned char)((w - rgb[r]) * t);
    rgb[g] += (unsigned char)((w - rgb[g]) * t);
    rgb[b] += (unsigned char)((w - rgb[b]) * t);
}

void pickPixelWithLighting(pixel* p, unsigned char* pixels) {
    unsigned char rgb[3];
    memcpy((void*)rgb, (void*)(pixels + (p->sourceY * rasterTileSize + p->sourceX) * 3), 3);
    lightPixel(p->targetX, p->targetY, rgb);
    memcpy((void*)(((unsigned char*)buffer) + (p->targetY * pitch + p->targetX * 3)), (void*)rgb, 3);
}

typedef struct Link {
    pixel* p;
    struct Link* l;
} link;

void pickPixels(void* key, size_t ksize, uintptr_t value, void* usr) {
    unsigned char* pixels;
    hashmap_get(imgPresent, key, ksize, (uintptr_t*)&pixels);
    if (pixels != NULL) {
        link* l = (link*)value;
        do {
            pickPixel(l->p, pixels);
            free(l->p);
            link* currentLink = l;
            l = l->l;
            free(currentLink);
        } while (l != NULL);
    }
    else {
        link* l = (link*)value;
        do {
            free(l->p);
            link* currentLink = l;
            l = l->l;
            free(currentLink);
        } while (l != NULL);
    }
    free((char*)key);
}

void pickPixelsWithLighting(void* key, size_t ksize, uintptr_t value, void* usr) {
    unsigned char* pixels;
    hashmap_get(imgPresent, key, ksize, (uintptr_t*)&pixels);
    if (pixels != NULL) {
        link* l = (link*)value;
        do {
            pickPixelWithLighting(l->p, pixels);
            free(l->p);
            link* currentLink = l;
            l = l->l;
            free(currentLink);
        } while (l != NULL);
    }
    else {
        link* l = (link*)value;
        do {
            free(l->p);
            link* currentLink = l;
            l = l->l;
            free(currentLink);
        } while (l != NULL);
    }
    free((char*)key);
}

void clearQueue(void* key, size_t ksize, uintptr_t value, void* usr) {
    link* l = (link*)value;
    do {
        free(l->p);
        link* currentLink = l;
        l = l->l;
        free(currentLink);
    } while (l != NULL);
    free((char*)key);
}

// t->t from [0, +/- pi/2] to [0, +/- pi/2] is stretched/mapped to t -> 1/2 * ln(tan(t/2 + pi/4)) from [0, +/- pi/2] to [0, +/- 1.75]
long double stretchWebMercator(long double t) {
    // approximation for ln(tan(t/2 + pi/4)) used:
    long double t4 = 4 * t;
    return .5L * (619.96L * PI * t) / ((PI * PI - 55.3536L + t4 * (t - PI)) * (PI * PI - 55.3536L + t4 * (t + PI)));
}

const long double cutoffLatitude = 1.484422229745332366961L;            // for web mercator projection

unsigned __stdcall raster(void* data) {
    threadData* tData = (threadData*)data;
    int yStart = tData->yStart;
    int yEnd = tData->yEnd;
    hashmap* imgQueue = hashmap_create();
    tData->rastering = 1;
    rastered = 0;
    notScheduled = 1;
    for (int y = yStart; y < yEnd; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            pt angles = at(x, y);
            if (angles.t != 2.0L) {
                long double amount = pow(2, zoom);
                long double xTile = angles.p * amount / PIDouble;
                angles.t = stretchWebMercator(angles.t);
                angles.t = fabsl(angles.t) < cutoffLatitude ? angles.t : copysignl(cutoffLatitude, angles.t);
                long double yTile = (angles.t - -cutoffLatitude - .0001L) * amount / (2.0L * cutoffLatitude);        // - .0001 = prevent exact 1 as result (values in image are [0,1), [ = including, ) = excluding )
                int tileX = (int)xTile;                                                 // (int) floors towards 0
                int tileY = (int)yTile;
                char id[25];                                                            // maximum length of id incl. \0 at maximum zoom of 30
                int length = sprintf_s(id, 25, idFormat, zoom, tileX, tileY);
                if (length == 0) {
                    length = 5;
                    strcpy(id, "0/0/0");
                    xTile = angles.p / PIDouble;
                    if (xTile < 0.0L) {
                        xTile = 0.0L;
                    }
                    else if (xTile >= 1.0L) {
                        xTile = .9999L;
                    }
                    yTile = (angles.t - -cutoffLatitude - .0001L) / (2.0L * cutoffLatitude);
                    if (yTile < 0.0L) {
                        yTile = 0.0L;
                    }
                    else if (yTile >= 1.0L) {
                        yTile = .9999L;
                    }
                    tileX = 0;
                    tileY = 0;
                }
                int steps = 0;
                int iXTile;
                int iYTile;
                uintptr_t result;
                switch (dir) {
                    case REFRESH:
                    case ZIN:
                    case ZOUT:
                    case SIDE: {
                        int z = zoom;
                        long double xTileI = xTile;
                        long double yTileI = yTile;
                        int tileXI = tileX;
                        int tileYI = tileY;
                        char newId[25];
                        char* key = id;
                        int newLength = length;
                        uintptr_t newResult;
                        while (hashmap_get(imgPresent, (void*)key, newLength, &newResult) && newResult != (uintptr_t)NULL) {
                            result = newResult;
                            ++steps;
                            iXTile = (int)((xTileI - tileXI) * rasterTileSize);
                            iYTile = (int)((yTileI - tileYI) * rasterTileSize);
                            break;                                                      // no break intended for ZIN, ZOUT, SIDE = get as detailed zoom as available uninterrupted from here but this costs too much framerate on Ryzen 5800X
                            long double amount = pow(2, ++z);
                            xTileI = angles.p * amount / PIDouble;
                            yTileI = (angles.t - -cutoffLatitude - .0001L) * amount / (2.0L * cutoffLatitude);
                            tileXI = (int)xTileI;
                            tileYI = (int)yTileI;
                            newLength = sprintf_s(newId, 25, idFormat, z, tileXI, tileYI);
                            if (newLength == 0) {
                                break;
                            }
                            key = newId;
                        }
                        break;
                    }
                    case INIT:
                    default:
                        break;
                }
                if (steps > 0)
                {
                    pixel p;
                    p.sourceX = iXTile;
                    p.sourceY = iYTile;
                    p.targetX = x;
                    p.targetY = y;
                    pickPixel(&p, (unsigned char*)result);
                    continue;
                }
                else {
                    int iXTile = (int)((xTile - tileX) * rasterTileSize);
                    int iYTile = (int)((yTile - tileY) * rasterTileSize);

                    int picked = 0;
                    switch (dir) {
                        case ZIN:
                        case SIDE:
                            if (zoom > 0) {
                                long double amount = pow(2, zoom - 1);
                                long double xTile = angles.p * amount / PIDouble;
                                long double yTile = (angles.t - -cutoffLatitude - .0001L) * amount / (2.0L * cutoffLatitude);
                                int tileX = (int)xTile;
                                int tileY = (int)yTile;
                                char id[25];
                                int length = sprintf_s(id, 25, idFormat, zoom - 1, tileX, tileY);
                                if (length == 0) {
                                    length = 5;
                                    strcpy(id, "0/0/0");
                                    xTile = angles.p / PIDouble;
                                    if (xTile < 0.0L) {
                                        xTile = 0.0L;
                                    }
                                    else if (xTile >= 1.0L) {
                                        xTile = .9999L;
                                    }
                                    yTile = (angles.t - -cutoffLatitude - .0001L) / (2.0L * cutoffLatitude);
                                    if (yTile < 0.0L) {
                                        yTile = 0.0L;
                                    }
                                    else if (yTile >= 1.0L) {
                                        yTile = .9999L;
                                    }
                                    tileX = 0;
                                    tileY = 0;
                                }
                                uintptr_t result;
                                if (hashmap_get(imgPresent, (void*)id, length, &result) && result != (uintptr_t)NULL) {
                                    pixel p;
                                    p.sourceX = (int)((xTile - tileX) * rasterTileSize);
                                    p.sourceY = (int)((yTile - tileY) * rasterTileSize);
                                    p.targetX = x;
                                    p.targetY = y;
                                    pickPixel(&p, (unsigned char*)result);
                                    picked = 1;
                                }
                            }
                            break;
                        case ZOUT:
                        case INIT:
                        case REFRESH:
                        default:
                            break;
                    }
                    if (!picked) {
                        memcpy((void*)(((unsigned char*)buffer) + (y * pitch + x * 3)), (void*)zero3, 3);
                    }

                    if (imgQueue != NULL) {
                        pixel* p = malloc(sizeof(pixel));
                        uintptr_t result;
                        if (hashmap_get(imgQueue, (void*)id, length, &result)) {
                            if (p != NULL) {
                                p->sourceX = iXTile;
                                p->sourceY = iYTile;
                                p->targetX = x;
                                p->targetY = y;
                                link* l = malloc(sizeof(link));
                                if (l != NULL) {
                                    l->p = p;
                                    l->l = (link*)result;
                                    hashmap_set(imgQueue, (void*)id, length, (uintptr_t)l);
                                    continue;
                                }
                                else {
                                    free(p);
                                }
                            }
                        }
                        else {
                            if (p != NULL) {
                                p->sourceX = iXTile;
                                p->sourceY = iYTile;
                                p->targetX = x;
                                p->targetY = y;
                                link* l = malloc(sizeof(link));
                                if (l != NULL) {
                                    l->p = p;
                                    l->l = NULL;
                                    char* pId = malloc(length);
                                    if (pId != NULL) {
                                        idData* lastRequest = NULL;
                                        idData* request = &(tData->dId);
                                        while (request->idLength != 0) {
                                            if (request->next != NULL) {
                                                request = request->next;
                                            }
                                            else {
                                                idData* newRequest = malloc(sizeof(idData));
                                                if (newRequest == NULL) {
                                                    free(pId);
                                                    goto LIKE_LNULL;
                                                }
                                                newRequest->next = NULL;
                                                lastRequest = request;
                                                request = newRequest;
                                                break;
                                            }
                                        }
                                        memcpy(request->id, id, length + 1);
                                        request->idLength = length;
                                        if (lastRequest != NULL) {
                                            lastRequest->next = request;
                                        }
                                        memcpy(pId, id, length);
                                        hashmap_set(imgQueue, (void*)pId, length, (uintptr_t)l);
                                        queued = 1;
                                        tData->imageRequestRequested = 1;
                                        continue;
                                    }
                                    else {
LIKE_LNULL:
                                        free(l);
                                        free(p);
                                    }
                                }
                                else {
                                    free(p);
                                }
                            }
                        }
                    }
                }
            }
            else {
                memcpy((void*)(((unsigned char*)buffer) + (y * pitch + x * 3)), (void*)zero3, 3);
            }
        }
    }
    //memcpy(((unsigned char*)region) + yStart * pitch, ((unsigned char*)buffer) + yStart * pitch, pitch * (yEnd - yStart));                            // copy all once in main thread appears to be faster than copy parts parallely from threads
    if (tData->lastQueue != NULL) {
        hashmap_iterate(tData->lastQueue, clearQueue, NULL);
        hashmap_free(tData->lastQueue);
    }
    tData->lastQueue = imgQueue;
    tData->rastering = 0;
    return 0;
}

// t->t from [0, +/- pi/2] to [0, +/- pi/2] is stretched/mapped to t -> 1/2 * ln(tan(t/2 + pi/4)) from [0, +/- pi/2] to [0, +/- 1.75]
double stretchWebMercatorD(double t) {
    // approximation for ln(tan(t/2 + pi/4)) used:
    double t4 = 4 * t;
    return .5 * (619.96 * PID * t) / ((PID * PID - 55.3536 + t4 * (t - PID)) * (PID * PID - 55.3536 + t4 * (t + PID)));
}

const double cutoffLatitudeD = 1.484422229745332366961;            // for web mercator projection

unsigned __stdcall rasterD(void* data) {
    threadData* tData = (threadData*)data;
    int yStart = tData->yStart;
    int yEnd = tData->yEnd;
    hashmap* imgQueue = hashmap_create();
    tData->rastering = 1;
    rastered = 0;
    notScheduled = 1;
    for (int y = yStart; y < yEnd; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            ptD angles = atD(x, y);
            if (angles.t != 2.0) {
                double amount = pow(2, zoom);
                double xTile = angles.p * amount / PIDoubleD;
                angles.t = stretchWebMercatorD(angles.t);
                angles.t = fabs(angles.t) < cutoffLatitudeD ? angles.t : copysign(cutoffLatitudeD, angles.t);
                double yTile = (angles.t - -cutoffLatitudeD - .0001) * amount / (2.0 * cutoffLatitudeD);            // - .0001 = prevent exact 1 as result (values in image are [0,1), [ = including, ) = excluding )
                int tileX = (int)xTile;                                                 // (int) floors towards 0
                int tileY = (int)yTile;
                char id[25];                                                            // maximum length of id incl. \0 at maximum zoom of 30
                int length = sprintf_s(id, 25, idFormat, zoom, tileX, tileY);
                if (length == 0) {
                    length = 5;
                    strcpy(id, "0/0/0");
                    xTile = angles.p / PIDoubleD;
                    if (xTile < 0.0) {
                        xTile = 0.0;
                    }
                    else if (xTile >= 1.0) {
                        xTile = .9999;
                    }
                    yTile = (angles.t - -cutoffLatitudeD - .0001) / (2.0 * cutoffLatitudeD);
                    if (yTile < 0.0) {
                        yTile = 0.0;
                    }
                    else if (yTile >= 1.0) {
                        yTile = .9999;
                    }
                    tileX = 0;
                    tileY = 0;
                }
                int steps = 0;
                int iXTile;
                int iYTile;
                uintptr_t result;
                switch (dir) {
                    case REFRESH:
                    case ZIN:
                    case ZOUT:
                    case SIDE: {
                        int z = zoom;
                        double xTileI = xTile;
                        double yTileI = yTile;
                        int tileXI = tileX;
                        int tileYI = tileY;
                        char newId[25];
                        char* key = id;
                        int newLength = length;
                        uintptr_t newResult;
                        while (hashmap_get(imgPresent, (void*)key, newLength, &newResult) && newResult != (uintptr_t)NULL) {
                            result = newResult;
                            ++steps;
                            iXTile = (int)((xTileI - tileXI) * rasterTileSize);
                            iYTile = (int)((yTileI - tileYI) * rasterTileSize);
                            break;                                                      // no break intended for ZIN, ZOUT, SIDE = get as detailed zoom as available uninterrupted from here but this costs too much framerate on Ryzen 5800X
                            double amount = pow(2, ++z);
                            xTileI = angles.p * amount / PIDoubleD;
                            yTileI = (angles.t - -cutoffLatitudeD - .0001) * amount / (2.0 * cutoffLatitudeD);
                            tileXI = (int)xTileI;
                            tileYI = (int)yTileI;
                            newLength = sprintf_s(newId, 25, idFormat, z, tileXI, tileYI);
                            if (newLength == 0) {
                                break;
                            }
                            key = newId;
                        }
                        break;
                    }
                    case INIT:
                    default:
                        break;
                }
                if (steps > 0)
                {
                    pixel p;
                    p.sourceX = iXTile;
                    p.sourceY = iYTile;
                    p.targetX = x;
                    p.targetY = y;
                    pickPixel(&p, (unsigned char*)result);
                    continue;
                }
                else {
                    int iXTile = (int)((xTile - tileX) * rasterTileSize);
                    int iYTile = (int)((yTile - tileY) * rasterTileSize);

                    int picked = 0;
                    switch (dir) {
                        case ZIN:
                        case SIDE:
                            if (zoom > 0) {
                                double amount = pow(2, zoom - 1);
                                double xTile = angles.p * amount / PIDoubleD;
                                double yTile = (angles.t - -cutoffLatitudeD - .0001) * amount / (2.0 * cutoffLatitudeD);
                                int tileX = (int)xTile;
                                int tileY = (int)yTile;
                                char id[25];
                                int length = sprintf_s(id, 25, idFormat, zoom - 1, tileX, tileY);
                                if (length == 0) {
                                    length = 5;
                                    strcpy(id, "0/0/0");
                                    xTile = angles.p / PIDoubleD;
                                    if (xTile < 0.0) {
                                        xTile = 0.0;
                                    }
                                    else if (xTile >= 1.0) {
                                        xTile = .9999;
                                    }
                                    yTile = (angles.t - -cutoffLatitudeD - .0001) / (2.0 * cutoffLatitudeD);
                                    if (yTile < 0.0) {
                                        yTile = 0.0;
                                    }
                                    else if (yTile >= 1.0) {
                                        yTile = .9999;
                                    }
                                    tileX = 0;
                                    tileY = 0;
                                }
                                uintptr_t result;
                                if (hashmap_get(imgPresent, (void*)id, length, &result) && result != (uintptr_t)NULL) {
                                    pixel p;
                                    p.sourceX = (int)((xTile - tileX) * rasterTileSize);
                                    p.sourceY = (int)((yTile - tileY) * rasterTileSize);
                                    p.targetX = x;
                                    p.targetY = y;
                                    pickPixel(&p, (unsigned char*)result);
                                    picked = 1;
                                }
                            }
                            break;
                        case ZOUT:
                        case INIT:
                        case REFRESH:
                        default:
                            break;
                    }
                    if (!picked) {
                        memcpy((void*)(((unsigned char*)buffer) + (y * pitch + x * 3)), (void*)zero3, 3);
                    }

                    if (imgQueue != NULL) {
                        pixel* p = malloc(sizeof(pixel));
                        uintptr_t result;
                        if (hashmap_get(imgQueue, (void*)id, length, &result)) {
                            if (p != NULL) {
                                p->sourceX = iXTile;
                                p->sourceY = iYTile;
                                p->targetX = x;
                                p->targetY = y;
                                link* l = malloc(sizeof(link));
                                if (l != NULL) {
                                    l->p = p;
                                    l->l = (link*)result;
                                    hashmap_set(imgQueue, (void*)id, length, (uintptr_t)l);
                                    continue;
                                }
                                else {
                                    free(p);
                                }
                            }
                        }
                        else {
                            if (p != NULL) {
                                p->sourceX = iXTile;
                                p->sourceY = iYTile;
                                p->targetX = x;
                                p->targetY = y;
                                link* l = malloc(sizeof(link));
                                if (l != NULL) {
                                    l->p = p;
                                    l->l = NULL;
                                    char* pId = malloc(length);
                                    if (pId != NULL) {
                                        idData* lastRequest = NULL;
                                        idData* request = &(tData->dId);
                                        while (request->idLength != 0) {
                                            if (request->next != NULL) {
                                                request = request->next;
                                            }
                                            else {
                                                idData* newRequest = malloc(sizeof(idData));
                                                if (newRequest == NULL) {
                                                    free(pId);
                                                    goto LIKE_LNULLD;
                                                }
                                                newRequest->next = NULL;
                                                lastRequest = request;
                                                request = newRequest;
                                                break;
                                            }
                                        }
                                        memcpy(request->id, id, length + 1);
                                        request->idLength = length;
                                        if (lastRequest != NULL) {
                                            lastRequest->next = request;
                                        }
                                        memcpy(pId, id, length);
                                        hashmap_set(imgQueue, (void*)pId, length, (uintptr_t)l);
                                        queued = 1;
                                        tData->imageRequestRequested = 1;
                                        continue;
                                    }
                                    else {
LIKE_LNULLD:
                                        free(l);
                                        free(p);
                                    }
                                }
                                else {
                                    free(p);
                                }
                            }
                        }
                    }
                }
            }
            else {
                memcpy((void*)(((unsigned char*)buffer) + (y * pitch + x * 3)), (void*)zero3, 3);
            }
        }
    }
    //memcpy(((unsigned char*)region) + yStart * pitch, ((unsigned char*)buffer) + yStart * pitch, pitch * (yEnd - yStart));                            // copy all once in main thread appears to be faster than copy parts parallely from threads
    if (tData->lastQueue != NULL) {
        hashmap_iterate(tData->lastQueue, clearQueue, NULL);
        hashmap_free(tData->lastQueue);
    }
    tData->lastQueue = imgQueue;
    tData->rastering = 0;
    return 0;
}

// t->t from [0, +/- pi/2] to [0, +/- pi/2] is stretched/mapped to t -> 1/2 * ln(tan(t/2 + pi/4)) from [0, +/- pi/2] to [0, +/- 1.75]
float stretchWebMercatorF(float t) {
    // approximation for ln(tan(t/2 + pi/4)) used:
    float t4 = 4 * t;
    return .5F * (619.96F * PIF * t) / ((PIF * PIF - 55.3536F + t4 * (t - PIF)) * (PIF * PIF - 55.3536F + t4 * (t + PIF)));
}

const float cutoffLatitudeF = 1.484422229745332366961F;            // for web mercator projection

unsigned __stdcall rasterF(void* data) {
    threadData* tData = (threadData*)data;
    int yStart = tData->yStart;
    int yEnd = tData->yEnd;
    hashmap* imgQueue = hashmap_create();
    tData->rastering = 1;
    rastered = 0;
    notScheduled = 1;
    for (int y = yStart; y < yEnd; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            ptF angles = atF(x, y);
            if (angles.t != 2.0F) {
                float amount = pow(2, zoom);
                float xTile = angles.p * amount / PIDoubleF;
                angles.t = stretchWebMercatorF(angles.t);
                angles.t = fabsf(angles.t) < cutoffLatitudeF ? angles.t : copysignf(cutoffLatitudeF, angles.t);
                float yTile = (angles.t - -cutoffLatitudeF - .0001F) * amount / (2.0F * cutoffLatitudeF);            // - .0001 = prevent exact 1 as result (values in image are [0,1), [ = including, ) = excluding )
                int tileX = (int)xTile;                                                 // (int) floors towards 0
                int tileY = (int)yTile;
                char id[25];                                                            // maximum length of id incl. \0 at maximum zoom of 30
                int length = sprintf_s(id, 25, idFormat, zoom, tileX, tileY);
                if (length == 0) {
                    length = 5;
                    strcpy(id, "0/0/0");
                    xTile = angles.p / PIDoubleF;
                    if (xTile < 0.0F) {
                        xTile = 0.0F;
                    }
                    else if (xTile >= 1.0F) {
                        xTile = .9999F;
                    }
                    yTile = (angles.t - -cutoffLatitudeF - .0001F) / (2.0F * cutoffLatitudeF);
                    if (yTile < 0.0F) {
                        yTile = 0.0F;
                    }
                    else if (yTile >= 1.0F) {
                        yTile = .9999F;
                    }
                    tileX = 0;
                    tileY = 0;
                }
                int steps = 0;
                int iXTile;
                int iYTile;
                uintptr_t result;
                switch (dir) {
                    case REFRESH:
                    case ZIN:
                    case ZOUT:
                    case SIDE: {
                        int z = zoom;
                        float xTileI = xTile;
                        float yTileI = yTile;
                        int tileXI = tileX;
                        int tileYI = tileY;
                        char newId[25];
                        char* key = id;
                        int newLength = length;
                        uintptr_t newResult;
                        while (hashmap_get(imgPresent, (void*)key, newLength, &newResult) && newResult != (uintptr_t)NULL) {
                            result = newResult;
                            ++steps;
                            iXTile = (int)((xTileI - tileXI) * rasterTileSize);
                            iYTile = (int)((yTileI - tileYI) * rasterTileSize);
                            break;                                                      // no break intended for ZIN, ZOUT, SIDE = get as detailed zoom as available uninterrupted from here but this costs too much framerate on Ryzen 5800X
                            float amount = pow(2, ++z);
                            xTileI = angles.p * amount / PIDoubleF;
                            yTileI = (angles.t - -cutoffLatitudeF - .0001F) * amount / (2.0F * cutoffLatitudeF);
                            tileXI = (int)xTileI;
                            tileYI = (int)yTileI;
                            newLength = sprintf_s(newId, 25, idFormat, z, tileXI, tileYI);
                            if (newLength == 0) {
                                break;
                            }
                            key = newId;
                        }
                        break;
                    }
                    case INIT:
                    default:
                        break;
                }
                if (steps > 0)
                {
                    pixel p;
                    p.sourceX = iXTile;
                    p.sourceY = iYTile;
                    p.targetX = x;
                    p.targetY = y;
                    pickPixel(&p, (unsigned char*)result);
                    continue;
                }
                else {
                    int iXTile = (int)((xTile - tileX) * rasterTileSize);
                    int iYTile = (int)((yTile - tileY) * rasterTileSize);

                    int picked = 0;
                    switch (dir) {
                        case ZIN:
                        case SIDE:
                            if (zoom > 0) {
                                float amount = pow(2, zoom - 1);
                                float xTile = angles.p * amount / PIDoubleF;
                                float yTile = (angles.t - -cutoffLatitudeF - .0001F) * amount / (2.0F * cutoffLatitudeF);
                                int tileX = (int)xTile;
                                int tileY = (int)yTile;
                                char id[25];
                                int length = sprintf_s(id, 25, idFormat, zoom - 1, tileX, tileY);
                                if (length == 0) {
                                    length = 5;
                                    strcpy(id, "0/0/0");
                                    xTile = angles.p / PIDoubleF;
                                    if (xTile < 0.0F) {
                                        xTile = 0.0F;
                                    }
                                    else if (xTile >= 1.0F) {
                                        xTile = .9999F;
                                    }
                                    yTile = (angles.t - -cutoffLatitudeF - .0001F) / (2.0F * cutoffLatitudeF);
                                    if (yTile < 0.0F) {
                                        yTile = 0.0F;
                                    }
                                    else if (yTile >= 1.0F) {
                                        yTile = .9999F;
                                    }
                                    tileX = 0;
                                    tileY = 0;
                                }
                                uintptr_t result;
                                if (hashmap_get(imgPresent, (void*)id, length, &result) && result != (uintptr_t)NULL) {
                                    pixel p;
                                    p.sourceX = (int)((xTile - tileX) * rasterTileSize);
                                    p.sourceY = (int)((yTile - tileY) * rasterTileSize);
                                    p.targetX = x;
                                    p.targetY = y;
                                    pickPixel(&p, (unsigned char*)result);
                                    picked = 1;
                                }
                            }
                            break;
                        case ZOUT:
                        case INIT:
                        case REFRESH:
                        default:
                            break;
                    }
                    if (!picked) {
                        memcpy((void*)(((unsigned char*)buffer) + (y * pitch + x * 3)), (void*)zero3, 3);
                    }

                    if (imgQueue != NULL) {
                        pixel* p = malloc(sizeof(pixel));
                        uintptr_t result;
                        if (hashmap_get(imgQueue, (void*)id, length, &result)) {
                            if (p != NULL) {
                                p->sourceX = iXTile;
                                p->sourceY = iYTile;
                                p->targetX = x;
                                p->targetY = y;
                                link* l = malloc(sizeof(link));
                                if (l != NULL) {
                                    l->p = p;
                                    l->l = (link*)result;
                                    hashmap_set(imgQueue, (void*)id, length, (uintptr_t)l);
                                    continue;
                                }
                                else {
                                    free(p);
                                }
                            }
                        }
                        else {
                            if (p != NULL) {
                                p->sourceX = iXTile;
                                p->sourceY = iYTile;
                                p->targetX = x;
                                p->targetY = y;
                                link* l = malloc(sizeof(link));
                                if (l != NULL) {
                                    l->p = p;
                                    l->l = NULL;
                                    char* pId = malloc(length);
                                    if (pId != NULL) {
                                        idData* lastRequest = NULL;
                                        idData* request = &(tData->dId);
                                        while (request->idLength != 0) {
                                            if (request->next != NULL) {
                                                request = request->next;
                                            }
                                            else {
                                                idData* newRequest = malloc(sizeof(idData));
                                                if (newRequest == NULL) {
                                                    free(pId);
                                                    goto LIKE_LNULLF;
                                                }
                                                newRequest->next = NULL;
                                                lastRequest = request;
                                                request = newRequest;
                                                break;
                                            }
                                        }
                                        memcpy(request->id, id, length + 1);
                                        request->idLength = length;
                                        if (lastRequest != NULL) {
                                            lastRequest->next = request;
                                        }
                                        memcpy(pId, id, length);
                                        hashmap_set(imgQueue, (void*)pId, length, (uintptr_t)l);
                                        queued = 1;
                                        tData->imageRequestRequested = 1;
                                        continue;
                                    }
                                    else {
LIKE_LNULLF:
                                        free(l);
                                        free(p);
                                    }
                                }
                                else {
                                    free(p);
                                }
                            }
                        }
                    }
                }
            }
            else {
                memcpy((void*)(((unsigned char*)buffer) + (y * pitch + x * 3)), (void*)zero3, 3);
            }
        }
    }
    //memcpy(((unsigned char*)region) + yStart * pitch, ((unsigned char*)buffer) + yStart * pitch, pitch * (yEnd - yStart));                            // copy all once in main thread appears to be faster than copy parts parallely from threads
    if (tData->lastQueue != NULL) {
        hashmap_iterate(tData->lastQueue, clearQueue, NULL);
        hashmap_free(tData->lastQueue);
    }
    tData->lastQueue = imgQueue;
    tData->rastering = 0;
    return 0;
}

unsigned __stdcall rasterFWithLighting(void* data) {
    threadData* tData = (threadData*)data;
    int yStart = tData->yStart;
    int yEnd = tData->yEnd;
    hashmap* imgQueue = hashmap_create();
    tData->rastering = 1;
    rastered = 0;
    notScheduled = 1;
    for (int y = yStart; y < yEnd; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            ptF angles = atF(x, y);
            if (angles.t != 2.0F) {
                float amount = pow(2, zoom);
                float xTile = angles.p * amount / PIDoubleF;
                angles.t = stretchWebMercatorF(angles.t);
                angles.t = fabsf(angles.t) < cutoffLatitudeF ? angles.t : copysignf(cutoffLatitudeF, angles.t);
                float yTile = (angles.t - -cutoffLatitudeF - .0001F) * amount / (2.0F * cutoffLatitudeF);            // - .0001 = prevent exact 1 as result (values in image are [0,1), [ = including, ) = excluding )
                int tileX = (int)xTile;                                                 // (int) floors towards 0
                int tileY = (int)yTile;
                char id[25];                                                            // maximum length of id incl. \0 at maximum zoom of 30
                int length = sprintf_s(id, 25, idFormat, zoom, tileX, tileY);
                if (length == 0) {
                    length = 5;
                    strcpy(id, "0/0/0");
                    xTile = angles.p / PIDoubleF;
                    if (xTile < 0.0F) {
                        xTile = 0.0F;
                    }
                    else if (xTile >= 1.0F) {
                        xTile = .9999F;
                    }
                    yTile = (angles.t - -cutoffLatitudeF - .0001F) / (2.0F * cutoffLatitudeF);
                    if (yTile < 0.0F) {
                        yTile = 0.0F;
                    }
                    else if (yTile >= 1.0F) {
                        yTile = .9999F;
                    }
                    tileX = 0;
                    tileY = 0;
                }
                int steps = 0;
                int iXTile;
                int iYTile;
                uintptr_t result;
                switch (dir) {
                    case REFRESH:
                    case ZIN:
                    case ZOUT:
                    case SIDE: {
                        int z = zoom;
                        float xTileI = xTile;
                        float yTileI = yTile;
                        int tileXI = tileX;
                        int tileYI = tileY;
                        char newId[25];
                        char* key = id;
                        int newLength = length;
                        uintptr_t newResult;
                        while (hashmap_get(imgPresent, (void*)key, newLength, &newResult) && newResult != (uintptr_t)NULL) {
                            result = newResult;
                            ++steps;
                            iXTile = (int)((xTileI - tileXI) * rasterTileSize);
                            iYTile = (int)((yTileI - tileYI) * rasterTileSize);
                            break;                                                      // no break intended for ZIN, ZOUT, SIDE = get as detailed zoom as available uninterrupted from here but this costs too much framerate on Ryzen 5800X
                            float amount = pow(2, ++z);
                            xTileI = angles.p * amount / PIDoubleF;
                            yTileI = (angles.t - -cutoffLatitudeF - .0001F) * amount / (2.0F * cutoffLatitudeF);
                            tileXI = (int)xTileI;
                            tileYI = (int)yTileI;
                            newLength = sprintf_s(newId, 25, idFormat, z, tileXI, tileYI);
                            if (newLength == 0) {
                                break;
                            }
                            key = newId;
                        }
                        break;
                    }
                    case INIT:
                    default:
                        break;
                }
                if (steps > 0)
                {
                    pixel p;
                    p.sourceX = iXTile;
                    p.sourceY = iYTile;
                    p.targetX = x;
                    p.targetY = y;
                    pickPixelWithLighting(&p, (unsigned char*)result);
                    continue;
                }
                else {
                    int iXTile = (int)((xTile - tileX) * rasterTileSize);
                    int iYTile = (int)((yTile - tileY) * rasterTileSize);

                    int picked = 0;
                    switch (dir) {
                        case ZIN:
                        case SIDE:
                            if (zoom > 0) {
                                float amount = pow(2, zoom - 1);
                                float xTile = angles.p * amount / PIDoubleF;
                                float yTile = (angles.t - -cutoffLatitudeF - .0001F) * amount / (2.0F * cutoffLatitudeF);
                                int tileX = (int)xTile;
                                int tileY = (int)yTile;
                                char id[25];
                                int length = sprintf_s(id, 25, idFormat, zoom - 1, tileX, tileY);
                                if (length == 0) {
                                    length = 5;
                                    strcpy(id, "0/0/0");
                                    xTile = angles.p / PIDoubleF;
                                    if (xTile < 0.0F) {
                                        xTile = 0.0F;
                                    }
                                    else if (xTile >= 1.0F) {
                                        xTile = .9999F;
                                    }
                                    yTile = (angles.t - -cutoffLatitudeF - .0001F) / (2.0F * cutoffLatitudeF);
                                    if (yTile < 0.0F) {
                                        yTile = 0.0F;
                                    }
                                    else if (yTile >= 1.0F) {
                                        yTile = .9999F;
                                    }
                                    tileX = 0;
                                    tileY = 0;
                                }
                                uintptr_t result;
                                if (hashmap_get(imgPresent, (void*)id, length, &result) && result != (uintptr_t)NULL) {
                                    pixel p;
                                    p.sourceX = (int)((xTile - tileX) * rasterTileSize);
                                    p.sourceY = (int)((yTile - tileY) * rasterTileSize);
                                    p.targetX = x;
                                    p.targetY = y;
                                    pickPixelWithLighting(&p, (unsigned char*)result);
                                    picked = 1;
                                }
                            }
                            break;
                        case ZOUT:
                        case INIT:
                        case REFRESH:
                        default:
                            break;
                    }
                    if (!picked) {
                        unsigned char rgb[3];
                        memcpy((void*)rgb, (void*)zero3, 3);
                        lightPixel(x, y, rgb);
                        memcpy((void*)(((unsigned char*)buffer) + (y * pitch + x * 3)), (void*)rgb, 3);
                    }

                    if (imgQueue != NULL) {
                        pixel* p = malloc(sizeof(pixel));
                        uintptr_t result;
                        if (hashmap_get(imgQueue, (void*)id, length, &result)) {
                            if (p != NULL) {
                                p->sourceX = iXTile;
                                p->sourceY = iYTile;
                                p->targetX = x;
                                p->targetY = y;
                                link* l = malloc(sizeof(link));
                                if (l != NULL) {
                                    l->p = p;
                                    l->l = (link*)result;
                                    hashmap_set(imgQueue, (void*)id, length, (uintptr_t)l);
                                    continue;
                                }
                                else {
                                    free(p);
                                }
                            }
                        }
                        else {
                            if (p != NULL) {
                                p->sourceX = iXTile;
                                p->sourceY = iYTile;
                                p->targetX = x;
                                p->targetY = y;
                                link* l = malloc(sizeof(link));
                                if (l != NULL) {
                                    l->p = p;
                                    l->l = NULL;
                                    char* pId = malloc(length);
                                    if (pId != NULL) {
                                        idData* lastRequest = NULL;
                                        idData* request = &(tData->dId);
                                        while (request->idLength != 0) {
                                            if (request->next != NULL) {
                                                request = request->next;
                                            }
                                            else {
                                                idData* newRequest = malloc(sizeof(idData));
                                                if (newRequest == NULL) {
                                                    free(pId);
                                                    goto LIKE_LNULLFWL;
                                                }
                                                newRequest->next = NULL;
                                                lastRequest = request;
                                                request = newRequest;
                                                break;
                                            }
                                        }
                                        memcpy(request->id, id, length + 1);
                                        request->idLength = length;
                                        if (lastRequest != NULL) {
                                            lastRequest->next = request;
                                        }
                                        memcpy(pId, id, length);
                                        hashmap_set(imgQueue, (void*)pId, length, (uintptr_t)l);
                                        queued = 1;
                                        tData->imageRequestRequested = 1;
                                        continue;
                                    }
                                    else {
LIKE_LNULLFWL:
                                        free(l);
                                        free(p);
                                    }
                                }
                                else {
                                    free(p);
                                }
                            }
                        }
                    }
                }
            }
            else {
                memcpy((void*)(((unsigned char*)buffer) + (y * pitch + x * 3)), (void*)zero3, 3);
            }
        }
    }
    //memcpy(((unsigned char*)region) + yStart * pitch, ((unsigned char*)buffer) + yStart * pitch, pitch * (yEnd - yStart));                            // copy all once in main thread appears to be faster than copy parts parallely from threads
    if (tData->lastQueue != NULL) {
        hashmap_iterate(tData->lastQueue, clearQueue, NULL);
        hashmap_free(tData->lastQueue);
    }
    tData->lastQueue = imgQueue;
    tData->rastering = 0;
    return 0;
}

unsigned __stdcall rasterCompletion(void* data) {
    threadData* tData = (threadData*)data;
    if (tData->lastQueue != NULL) {
        hashmap_iterate(tData->lastQueue, pickPixels, NULL);
        hashmap_free(tData->lastQueue);
        //int yStart = tData->yStart;
        //int yEnd = tData->yEnd;
        //memcpy(((unsigned char*)region) + yStart * pitch, ((unsigned char*)buffer) + yStart * pitch, pitch * (yEnd - yStart));                        // copy all once in main thread appears to be faster than copy parts parallely from threads
        tData->lastQueue = NULL;
    }
    return 0;
}

const float maxRElevate = 0.2F;
const float elevationExaggeration = 40.0F;

void copyPixel(int sourceX, int sourceY, int destX, int destY) {
    memcpy((void*)(((unsigned char*)buffer) + (destY * pitch + destX * 3)), (void*)(((unsigned char*)buffer) + (sourceY * pitch + sourceX * 3)), 3);
}

// sourceX, sourceY is x3, y3
void paintTriangle(int x1, int y1, int x2, int y2, int sourceX, int sourceY) {
    int yTop = min(min(y1, y2), sourceY);
    int yBottom = max(max(y1, y2), sourceY);
    int yMiddle = yTop == y1 ? (yBottom == y2 ? sourceY : y2) : (yTop == y2 ? (yBottom == y1 ? sourceY : y1) : (yBottom == y2 ? y1 : y2));
    int yTopX = yTop == y1 ? x1 : (yTop == y2 ? x2 : sourceX);
    int yBottomX = yBottom == y1 ? x1 : (yBottom == y2 ? x2 : sourceX);
    int yMiddleX = yMiddle == y1 ? x1 : (yMiddle == y2 ? x2 : sourceX);
    if (yMiddle > yTop) {
        if(yTop >= 0 && yTop < HEIGHT && yTopX >= 0 && yTopX < WIDTH)
            copyPixel(sourceX, sourceY, yTopX, yTop);
        float m1 = ((float)(yMiddleX - yTopX)) / (yMiddle - yTop);
        float m2 = ((float)(yBottomX - yTopX)) / (yBottom - yTop);
        int start;
        int end;
        int* lower;
        int* upper;
        if (yBottomX > yTopX + m1 * (yBottom - yTop)) {
            lower = &start;
            upper = &end;
        }
        else {
            lower = &end;
            upper = &start;
        }
        int maxy = (yMiddle < HEIGHT ? yMiddle + 1 : HEIGHT) - yTop;
        for (int y = yTop >= 0 ? 1 : -yTop; y < maxy; ++y) {
            start = yTopX + m1 * y;
            end = yTopX + m2 * y;
            int maxbound = *upper < WIDTH ? *upper + 1 : WIDTH;
            for (int l = *lower >= 0 ? (*lower < WIDTH ? *lower : (WIDTH - 1)) : 0; l < maxbound; ++l) {
                copyPixel(sourceX, sourceY, l, yTop + y);
            }
        }
        if (yBottom > yMiddle) {
            float m3 = ((float)(yBottomX - yMiddleX)) / (yBottom - yMiddle);
            int maxy = (yBottom < HEIGHT ? yBottom + 1 : HEIGHT) - yMiddle;
            for (int y = yMiddle >= 0 ? 1 : -yMiddle; y < maxy; ++y) {
                start = yMiddleX + m3 * y;
                end = yTopX + m2 * (yMiddle - yTop + y);
                int maxbound = *upper < WIDTH ? *upper + 1 : WIDTH;
                for (int l = *lower >= 0 ? (*lower < WIDTH ? *lower : (WIDTH - 1)) : 0; l < maxbound; ++l) {
                    copyPixel(sourceX, sourceY, l, yMiddle + y);
                }
            }
        }
    }
    else {
        if (yTop >= 0 && yTop < HEIGHT) {
            int lower;
            int upper;
            if (yTopX > yMiddleX) {
                lower = yMiddleX >= 0 ? (yMiddleX < WIDTH ? yMiddleX : WIDTH - 1) : 0;
                upper = yTopX < WIDTH ? yTopX + 1 : WIDTH;
            }
            else {
                lower = yTopX >= 0 ? (yTopX < WIDTH ? yTopX : WIDTH - 1) : 0;
                upper = yMiddleX < WIDTH ? yMiddleX + 1 : WIDTH;
            }
            for (int l = lower; l < upper; ++l) {
                copyPixel(sourceX, sourceY, l, yTop);
            }
        }
        if (yBottom > yMiddle) {
            float m2 = ((float)(yBottomX - yTopX)) / (yBottom - yTop);
            float m3 = ((float)(yBottomX - yMiddleX)) / (yBottom - yMiddle);
            int start;
            int end;
            int* lower;
            int* upper;
            if (yTopX > yMiddleX) {
                lower = &start;
                upper = &end;
            }
            else {
                lower = &end;
                upper = &start;
            }
            int maxy = (yBottom < HEIGHT ? yBottom + 1 : HEIGHT) - yMiddle;
            for (int y = yMiddle >= 0 ? 1 : -yMiddle; y < maxy; ++y) {
                start = yMiddleX + m3 * y;
                end = yTopX + m2 * y;
                int maxbound = *upper < WIDTH ? *upper + 1 : WIDTH;
                for (int l = *lower >= 0 ? (*lower < WIDTH ? *lower : WIDTH - 1) : 0; l < maxbound; ++l) {
                    copyPixel(sourceX, sourceY, l, yMiddle + y);
                }
            }
        }
        else {
            if (yTop >= 0 && yTop < HEIGHT) {
                int lower = 0;
                int upper = 0;
                if (yTopX > yMiddleX) {
                    if (yBottomX > yTopX) {
                        lower = (yTopX + 1) >= 0 ? (yTopX + 1) < WIDTH ? (yTopX + 1) : (WIDTH - 1) : 0;
                        upper = yBottomX < WIDTH ? yBottomX + 1 : WIDTH;
                    }
                    else if (yBottomX < yMiddleX) {
                        lower = yBottomX >= 0 ? yBottomX < WIDTH ? yBottomX : (WIDTH - 1) : 0;
                        upper = yMiddleX < WIDTH ? yMiddleX : WIDTH;
                    }
                }
                else {
                    if (yBottomX > yMiddleX) {
                        lower = (yMiddleX + 1) >= 0 ? (yMiddleX + 1) < WIDTH ? (yMiddleX + 1) : (WIDTH - 1) : 0;
                        upper = yBottomX < WIDTH ? yBottomX + 1 : WIDTH;
                    }
                    else if (yBottomX < yTopX) {
                        lower = yBottomX >= 0 ? yBottomX < WIDTH ? yBottomX : (WIDTH - 1) : 0;
                        upper = yTopX < WIDTH ? yTopX : WIDTH;
                    }
                }
                for (int l = lower; l < upper; ++l) {
                    copyPixel(sourceX, sourceY, l, yTop);
                }
            }
        }
    }
}

void putElevation(int xC, int yC/*, int r*/) {
    int x = centerX + xC;
    int y = centerY + yC;
    ptF gAngles = atF(x, y);
    if (gAngles.t != 2.0F) {
        //float a = 1.0F / (1.0F - maxRElevate);                        // outcommented smooth transition to non-elevation
        //float f = a * r / roundf(rScaleF) - maxRElevate * a;
        int16_t h = *((int16_t*)(elevationData + (((int)((gAngles.t - -PIHalfF - .0001F) / PIF * 1080)) * 2160 + (int)(gAngles.p / PIDoubleF * 2160)) * 2));
        float RNew = rScaleF * (1.0F + /*(pow(2, maxZoomLighting - 1 - zoom) - 1) * f **/ elevationExaggeration * h / 6378000.0F);
        ptF sAngles = atFWithoutOffsets(x, y);                          // when gAngles on globe, so are sAngles expected to be on globe
        float sint = sinf(sAngles.t);
        int nX = centerX + roundf(RNew * sqrtf(1.0f - sint * sint) * cosf(sAngles.p + PIF));
        int nY = centerY + roundf(RNew * sint);
        int s = xC * yC > 0 ? 1 : -1;                                   // omitting xC, yC == 0
        ptF sAngles2 = atFWithoutOffsets(x - .4F, y + s * .4F);         // .5 which is the theroretical limit creates pixel smearing for small triangles due to points being rounded to neighbouring pixel, so does .425 slightly
        if (sAngles2.t != 2.0F) {
            float sint = sinf(sAngles2.t);
            int nX2 = centerX + roundf(RNew * sqrtf(1.0f - sint * sint) * cosf(sAngles2.p + PIF));
            int nY2 = centerY + roundf(RNew * sint);
            paintTriangle(nX, nY, nX2, nY2, x, y);
        }
        ptF sAngles3 = atFWithoutOffsets(x + .4F, y - s * .4F);
        if (sAngles3.t != 2.0F) {
            float sint = sinf(sAngles3.t);
            int nX3 = centerX + roundf(RNew * sqrtf(1.0f - sint * sint) * cosf(sAngles3.p + PIF));
            int nY3 = centerY + roundf(RNew * sint);
            paintTriangle(nX, nY, nX3, nY3, x, y);
        }
    }
}

void elevate() {
    int R = roundf(rScaleF);
    int maxR = roundf(maxRElevate * R);
    for (int r = R; r > maxR; --r) {
        if(centerX - r >= 0 && HEIGHT > 0)
            putElevation(-r, 0/*, r*/);
        int lastY = 0;
        int maxX = 1 < (WIDTH - centerX) ? 1 : (WIDTH - centerX);
        for (int x = (centerX - r + 1) >= 0 ? (-r + 1) : (-centerX + 1); x < maxX; ++x) {
            int y = roundf(sqrtf(r * r - x * x));
            if (centerY - y >= 0)
                putElevation(x, -y/*, r*/);
            if (centerY + y < HEIGHT)
                putElevation(x, y/*, r*/);
            int a = (y - lastY) / 2;                                    // integers! is equal to ceil((y - lastY - 1) / 2)
            if (centerY + y - 1 < HEIGHT && centerY - (y - 1) >= 0) {
                for (int c = 1; c <= a; ++c) {
                    putElevation(x - 1, lastY + c/*, r*/);
                    putElevation(x - 1, -(lastY + c)/*, r*/);
                    putElevation(x, y - c/*, r*/);
                    putElevation(x, -(y - c)/*, r*/);
                }
            }
            else {
                for (int c = 1; c <= a; ++c) {
                    if (centerY + lastY + c < HEIGHT)
                        putElevation(x - 1, lastY + c/*, r*/);
                    if (centerY - (lastY + c) >= 0)
                        putElevation(x - 1, -(lastY + c)/*, r*/);
                    if (centerY + y - c < HEIGHT)
                        putElevation(x, y - c/*, r*/);
                    if (centerY - (y - c) >= 0)
                        putElevation(x, -(y - c)/*, r*/);
                }
            }
            lastY = y;
        }
        maxX = r < (WIDTH - centerX) ? r + 1 : (WIDTH - centerX);
        for (int x = 1; x < maxX; ++x) {
            int y = roundf(sqrtf(r * r - x * x));
            if (centerY - y >= 0)
                putElevation(x, -y/*, r*/);
            if (centerY + y < HEIGHT)
                putElevation(x, y/*, r*/);
            int a = (lastY - y) / 2;                                    // integers! is equal to ceil((y - lastY - 1) / 2)
            if (centerY + lastY - 1 < HEIGHT && centerY - (lastY - 1) >= 0) {
                for (int c = 1; c <= a; ++c) {
                    putElevation(x - 1, lastY - c/*, r*/);
                    putElevation(x - 1, -(lastY - c)/*, r*/);
                    putElevation(x, y + c/*, r*/);
                    putElevation(x, -(y + c)/*, r*/);
                }
            }
            else {
                for (int c = 1; c <= a; ++c) {
                    if (centerY + lastY - c < HEIGHT)
                        putElevation(x - 1, lastY - c/*, r*/);
                    if (centerY - (lastY - c) >= 0)
                        putElevation(x - 1, -(lastY - c)/*, r*/);
                    if (centerY + y + c < HEIGHT)
                        putElevation(x, y + c/*, r*/);
                    if (centerY - (y + c) >= 0)
                        putElevation(x, -(y + c)/*, r*/);
                }
            }
            lastY = y;
        }
    }
}

unsigned __stdcall rasterCompletionWithLighting(void* data) {
    threadData* tData = (threadData*)data;
    if (tData->lastQueue != NULL) {
        hashmap_iterate(tData->lastQueue, pickPixelsWithLighting, NULL);
        hashmap_free(tData->lastQueue);
        //int yStart = tData->yStart;
        //int yEnd = tData->yEnd;
        //memcpy(((unsigned char*)region) + yStart * pitch, ((unsigned char*)buffer) + yStart * pitch, pitch * (yEnd - yStart));                        // copy all once in main thread appears to be faster than copy parts parallely from threads
        tData->lastQueue = NULL;
    }
    return 0;
}


void freeAsyncIdMemory(void* key, size_t ksize, uintptr_t value, void* usr) {
    if (value != (uintptr_t)0) {
        if(((asyncId*)value)->buffer != NULL)
            free(((asyncId*)value)->buffer);
        free((asyncId*)value);
    }
}

void freeImgPresentMemory(void* key, size_t ksize, uintptr_t value, void* usr)
{
    if (value != (uintptr_t)NULL) {
        free((unsigned char*)value);
    }
    free((char*)key);
}

typedef struct IndexBounds {
    unsigned int start;
    unsigned int end;
} indexBounds;

indexBounds getFirstLineUTF8(unsigned char* text, size_t textLength) {
    indexBounds indices;

    indices.start = 0;
    if (textLength > 3 && text[0] == (unsigned char)0xEF && text[1] == (unsigned char)0xBB && text[2] == (unsigned char)0xBF) {
        indices.start = 3;
    }
    indices.end = indices.start;
    int continuation = 0;
    while (indices.end < textLength) {
        if (continuation) {
            if (text[indices.end] > (unsigned char)127 && text[indices.end] < (unsigned char)192) {
                ++indices.end;                                                                  // not checking for ("printable") control characters having codepoint 128 to 159 (across 2 bytes!)
                ++continuation;                                                                 // don't assume nothing "larger" than 4-byte - codes exists (ever)
                continue;
            }
            if (text[indices.end] < (unsigned char)128) {
                if (continuation > 1) {
                    continuation = 0;
                }
                else {
                    break;                                                                      // error
                }
            }
            else {
                break;                                                                          // unknown state
            }
        }
        if (text[indices.end] > (unsigned char)31 && text[indices.end] < (unsigned char)127) {  // non - control characters in Unicode UTF-8
            ++indices.end;
            continue;
        }
        if (text[indices.end] > (unsigned char)127) {
            ++indices.end;
            ++continuation;
            continue;
        }
        else {
            break;                                                                              // break on control character having codepoint 127, not expected as text character, and control characters having codepoint < 32
        }
    }
    indices.end -= indices.end < textLength || continuation < 2 ? continuation : 0;             // (potentially) roll back to last known valid character

    return indices;
}

int main(int argc, char* argv[])
{
#ifdef DEBUG
    AttachConsole(ATTACH_PARENT_PROCESS);
    freopen("CONOUT$", "w", stdout);
    LOG(("\n"));
#endif

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    Uint32 windowID;
    SDL_Surface* icon = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
        window = SDL_CreateWindow("Globe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
        if (window != NULL) {
            windowID = SDL_GetWindowID(window);
            if (windowID != 0) {
                renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                if (renderer != NULL) {
                    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
                    if (texture != NULL) {
                        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
                        if (SDL_LockTexture(texture, NULL, &region, &pitch) == 0) {
                            buffer = malloc(pitch * HEIGHT);
                            if (buffer != NULL) {
                                icon = SDL_LoadBMP("assets/appicon.bmp");
                                if (icon != NULL) {
                                    SDL_SetColorKey(icon, SDL_TRUE, SDL_MapRGB(icon->format, 0, 0, 0));
                                    SDL_SetWindowIcon(window, icon);
                                }
                                goto SDL_STARTED;
                            }
                            SDL_UnlockTexture(texture);
                        }
                        SDL_DestroyTexture(texture);
                    }
                    SDL_DestroyRenderer(renderer);
                }
            }
            SDL_DestroyWindow(window);
        }
    }
    const char* error = SDL_GetError();
    if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "the following error occurred:", error, NULL) != 0) {
        LOG((error));
    }
    SDL_Quit();
    return 0;


SDL_STARTED:
    const char* errors[] = {
        "error reading url file, using default url",                            // 1
        "failed allocating memory for url, using default url",                  // 2
        "%-character found in url",                                             // 4
        "key placeholder present in url but error opening key file",            // 8
        "error reading key file",                                               // 16
        "failed allocating memory for key",                                     // 32
        "failed allocating required memory",                                    // 64
        "missing map part id placeholder in url",                               // 128
        "%-character found in key",                                             // 256
        "missing path in url",                                                  // 512
        "missing key in key file"                                               // 1024
    };
    const int NUM_ERRORS = 11;
    unsigned int errorsMask = 0;
    int freeUrl = 0;
    wchar_t defaultURL[] = L"api.mapbox.com/v4/mapbox.satellite/^^.jpg90?access_token=<<";
    wchar_t* url;
    FILE* urlFile = fopen("mapservice-url.txt", "r");
    if (urlFile != NULL) {
        unsigned char urlBuffer[4100];
        size_t sizeRead = fread(urlBuffer, sizeof(unsigned char), 4099, urlFile);
        if (sizeRead != 0 && (sizeRead == 4099 || feof(urlFile))) {
            fclose(urlFile);
            indexBounds indices = getFirstLineUTF8(urlBuffer, sizeRead);
            if (indices.end > indices.start) {
                url = malloc((indices.end - indices.start + 1) * sizeof(wchar_t));
                if (url != NULL) {
                    freeUrl = 1;
                    mbstowcs(url, urlBuffer + indices.start, indices.end - indices.start);
                    url[indices.end - indices.start] = L'\0';
                    goto URL_END;
                }
                else {
                    errorsMask |= 2;
                }
            }
        }
        else {
            if (ferror(urlFile))
                errorsMask |= 1;
            fclose(urlFile);
        }
    }
    url = defaultURL;

URL_END: ;
    wchar_t* urlFormat;
    if (wcschr(url, L'%') == NULL) {
        wchar_t* indexKey = wcsstr(url, L"<<");
        if (indexKey != NULL) {
            indexKey[0] = L'%';
            indexKey[1] = L's';
            FILE* keyFile = fopen("mapservice-key.txt", "r");
            if (keyFile != NULL) {
                unsigned char keyBuffer[4100];
                size_t sizeRead = fread(keyBuffer, sizeof(unsigned char), 4099, keyFile);
                if (sizeRead != 0 && (sizeRead == 4099 || feof(keyFile))) {
                    fclose(keyFile);
                    indexBounds indices = getFirstLineUTF8(keyBuffer, sizeRead);
                    if (indices.end > indices.start) {
                        wchar_t* sKey = malloc((indices.end - indices.start + 1) * sizeof(wchar_t));
                        if (sKey != NULL) {
                            mbstowcs(sKey, keyBuffer + indices.start, indices.end - indices.start);
                            sKey[indices.end - indices.start] = L'\0';
                            if (wcschr(sKey, L'%') == NULL) {
                                urlFormat = malloc((wcslen(url) - 2 + indices.end - indices.start + 1) * sizeof(wchar_t));
                                if (urlFormat != NULL) {
                                    _swprintf(urlFormat, url, sKey);
                                    free(sKey);
                                    goto KEY_END;
                                }
                                else {
                                    errorsMask |= 64;
                                }
                            } else {
                                errorsMask |= 256;
                            }
                            free(sKey);
                        }
                        else {
                            errorsMask |= 32;
                        }
                    }
                    else {
                        errorsMask |= 1024;
                    }
                }
                else {
                    if(ferror(keyFile))
                        errorsMask |= 16;
                    else
                        errorsMask |= 1024;
                    fclose(keyFile);
                }
            } else {
                errorsMask |= 8;
            }
        }
        else {
            size_t urlMemoryLength = (wcslen(url) + 1) * sizeof(wchar_t);
            urlFormat = malloc(urlMemoryLength);
            if (urlFormat != NULL) {
                memcpy(urlFormat, url, urlMemoryLength);
                goto KEY_END;
            }
            else {
                errorsMask |= 64;
            }
        }
        goto EXIT_APP;

KEY_END: ;
        wchar_t* pathBegin = wcschr(urlFormat, L'/');
        if (pathBegin != NULL) {
            size_t hostLength = pathBegin - urlFormat;
            size_t hostSize = (hostLength + 1) * sizeof(wchar_t);
            host = malloc(hostSize);
            if (host != NULL) {
                memcpy(host, urlFormat, hostSize);
                host[pathBegin - urlFormat] = L'\0';
                if (wcslen(urlFormat) > hostLength + 1) {
                    pathFormat = pathBegin + 1;
                    wchar_t* indexId = wcsstr(pathFormat, L"^^");
                    if (indexId != NULL) {
                        indexId[0] = L'%';
                        indexId[1] = L's';
                        path = malloc((wcslen(pathFormat) - 2 + 24 + 1) * sizeof(wchar_t));          // 24 is max length id
                        if (path != NULL) {
                            goto FORMAT_END;
                        }
                        else {
                            errorsMask |= 64;
                        }
                    }
                    else {
                        errorsMask |= 128;
                    }
                }
                else {
                    errorsMask |= 512;
                }
                free(host);
            }
            else {
                errorsMask |= 64;
            }
        }
        else {
            errorsMask |= 512;
        }
        free(urlFormat);
    }
    else {
        errorsMask |= 4;
    }

EXIT_APP: ;
    int count = 0;
    do {
        if ((errorsMask >> count) & 1) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "the following error occurred:", errors[count], window);
        }
    } while (++count < NUM_ERRORS);
    if (freeUrl)
        free(url);
    free(buffer);
    SDL_UnlockTexture(texture);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;


FORMAT_END: ;
    int count2 = 0;
    do {
        if ((errorsMask >> count2) & 1) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "the following error occurred:", errors[count2], window);
        }
    } while (++count2 < NUM_ERRORS);

    size_t maxNumber = wcslen(url);
    cachePath = malloc(5 + 1 + maxNumber + 1 + 1);                  // "cache/URL/"
    if (cachePath == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "the following error occurred:", errors[6], window);
        if (freeUrl)
            free(url);
        free(buffer);
        SDL_UnlockTexture(texture);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }
    cachePathCollector = malloc(5 + 1 + maxNumber + 1 + 24 + 1);    // "cache/URL/ID"
    if (cachePathCollector == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "the following error occurred:", errors[6], window);
        free(cachePath);
        if (freeUrl)
            free(url);
        free(buffer);
        SDL_UnlockTexture(texture);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }
    sprintf(cachePath, "cache/");
    _mkdir("cache");
    int startNumber = 6;
    int number = 0;
    wchar_t lastCharacter = L' ';
    while (number < maxNumber) {
        wchar_t character = url[number];
        wchar_t output;
        if (character < 48 || character > 57 && character < 65 || character > 90 && character < 97 || character > 122 || lastCharacter == L'%') {           // % = account for possible key placeholder having been set
            output = L'-';
        }
        else {
            output = character;
        }
        lastCharacter = character;
        cachePath[startNumber + number] = (char)output;
        ++number;
    }
    cachePathLength = startNumber + number + 1;
    cachePath[startNumber + number] = '/';
    cachePath[cachePathLength] = '\0';
    _mkdir(cachePath);
    memcpy(cachePathCollector, cachePath, cachePathLength);
    idStartInCachePath = cachePathCollector + cachePathLength;
    if (freeUrl)
        free(url);


    FILE* compressedElevationDataFile = fopen("data/elevation.lzo", "rb");
    if (compressedElevationDataFile != NULL) {
        unsigned char* compressedElevationData = malloc(1543766);
        if (compressedElevationData != NULL) {
            if (fread(compressedElevationData, 1, 1543766, compressedElevationDataFile) == 1543766) {
                fgetc(compressedElevationDataFile);
                if (feof(compressedElevationDataFile)) {
                    fclose(compressedElevationDataFile);
                    if (lzo_init() == LZO_E_OK) {
                        elevationData = malloc(4665600);
                        if (elevationData != NULL) {
                            lzo_uint decompressedSize = 4665600;
                            if (lzo1x_decompress_safe(compressedElevationData, 1543766, elevationData, &decompressedSize, NULL) == LZO_E_OK && decompressedSize == 4665600) {
                                free(compressedElevationData);
                                elevationDataAvailable = 1;
                                goto ELEVATION_DONE;
                            }
                            free(elevationData);
                        }
                    }
                }
                else {
                    fclose(compressedElevationDataFile);
                }
            }
            else {
                fclose(compressedElevationDataFile);
            }
            free(compressedElevationData);
        }
        else {
            fclose(compressedElevationDataFile);
        }
    }
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "elevation data not usable", "presenting without elevation", window);


ELEVATION_DONE:
    int cores = SDL_GetCPUCount() - 2;
    if (cores < 1)
    {
        cores = 1;
        maxThreads = 1;
    }
    else if (cores > HEIGHT) {
        maxThreads = HEIGHT;
    }
    else {
        maxThreads = cores;
    }

    threadsData = malloc(cores * sizeof(threadData));
    if (threadsData != NULL) {
        imgRequested = hashmap_create();
        if (imgRequested != NULL) {
            imgPresent = hashmap_create();
            if (imgPresent != NULL) {
                goto MEMORY_DONE;
            }
            free(imgRequested);
        }
        free(threadsData);
    }
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "the following error occurred:", "failed allocating required memory", window);
    if (elevationDataAvailable)
        free(elevationData);
    free(cachePathCollector);
    free(cachePath);
    free(path);
    free(host);
    free(urlFormat);
    free(buffer);
    SDL_UnlockTexture(texture);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;


MEMORY_DONE:
    allImagesRequestedPresent = 1;
    rastered = 0;
    notScheduled = 0;
    wantsCompletion = 0;
    dontWaitForCollector = 1;

    notquitrequested = 1;

    centerX = WIDTH / 2;
    centerY = HEIGHT / 2;
    if (WIDTH < HEIGHT) {
        rScale = WIDTH / 4;
    }
    else {
        rScale = HEIGHT / 4;
    }
    rScaleSqr = rScale * rScale;
    rScaleSqrF = rScaleSqr;
    rScaleF = rScale;

    determineZoom();

    dir = INIT;

    for (int i = 0; i < cores; ++i) {
        threadsData[i].rastering = 0;
        threadsData[i].dId.idLength = 0;
        threadsData[i].dId.next = NULL;
        threadsData[i].imageRequestRequested = 0;
        threadsData[i].lastQueue = NULL;
        threadsData[i].hComplete = 0;
    }

    for (int i = 0; i < maxThreads; ++i) {
        threadsData[i].yStart = (i * HEIGHT) / maxThreads;
        threadsData[i].yEnd = ((i + 1) * HEIGHT) / maxThreads;
        threadsData[i].hThread = (HANDLE)_beginthreadex(NULL, 0, zoomF < maxZoomLighting ? rasterFWithLighting : zoom < 16 ? rasterF : zoom < 24 ? rasterD : raster, (void*)&(threadsData[i]), 0, NULL);
        if (threadsData[i].hThread == 0) {
            notquitrequested = 0;
        }
    }

    doCollecting = 1;
    checkingImageRequests = 1;
    HANDLE hCollector = (HANDLE)_beginthreadex(NULL, 0, collector, NULL, 0, NULL);
    if (hCollector == 0) {
        notquitrequested = 0;
    }


    Uint64 starttime;
    long double startphi, starttilt;
    long double phiLeftWaiting = phiLeft;
    long double axisTiltWaiting = axisTilt;
    long double rScaleWaiting = rScale;

    pt clicked;
    int mousedown = 0;
    int mouseX = centerX, mouseY = centerY;

    int act = 0;

    int newWidth, newHeight;
    int windowSizeChanged = 0;

    int dequeueing = 0;

    int textureLock = 1;
    int nonRequestedExit = 1;

    SDL_Event event;
    while (notquitrequested) {
        while (SDL_PollEvent(&event)) {                 // poll until all events are handled!
            switch (event.type) {
                case SDL_QUIT:
                    notquitrequested = 0;
                    nonRequestedExit = 0;
                    goto AFTER_LOOP;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT:
                        case SDLK_a:
                            if (event.key.repeat == 0) {
                                starttime = event.key.timestamp;
                                startphi = phiLeft;
                                phiLeftWaiting = fmodl((startphi - stepSize) + PIDouble, PIDouble);
                            } else {
                                phiLeftWaiting = fmodl((startphi - 24 * stepSize * (event.key.timestamp - starttime) / 10000) + PIDouble, PIDouble);
                            }
                            dir = SIDE;
                            break;
                        case SDLK_RIGHT:
                        case SDLK_d:
                            if (event.key.repeat == 0) {
                                starttime = event.key.timestamp;
                                startphi = phiLeft;
                                phiLeftWaiting = fmodl(startphi + stepSize, PIDouble);
                            } else {
                                phiLeftWaiting = fmodl(startphi + 24 * stepSize * (event.key.timestamp - starttime) / 10000, PIDouble);
                            }
                            dir = SIDE;
                            break;
                        case SDLK_UP:
                            if (event.key.repeat == 0) {
                                starttime = event.key.timestamp;
                                starttilt = axisTilt;
                                axisTiltWaiting = axisTilt + stepSize;
                                if (axisTiltWaiting > PIHalf) {
                                    axisTiltWaiting = PIHalf;
                                }
                            } else {
                                axisTiltWaiting = starttilt + 24 * stepSize * (event.key.timestamp - starttime) / 10000;
                                if (axisTiltWaiting > PIHalf) {
                                    axisTiltWaiting = PIHalf;
                                }
                            }
                            dir = SIDE;
                            break;
                        case SDLK_DOWN:
                            if (event.key.repeat == 0) {
                                starttime = event.key.timestamp;
                                starttilt = axisTilt;
                                axisTiltWaiting = axisTilt - stepSize;
                                if (axisTiltWaiting < -PIHalf) {
                                    axisTiltWaiting = -PIHalf;
                                }
                            } else {
                                axisTiltWaiting = starttilt - 24 * stepSize * (event.key.timestamp - starttime) / 10000;
                                if (axisTiltWaiting < -PIHalf) {
                                    axisTiltWaiting = -PIHalf;
                                }
                            }
                            dir = SIDE;
                            break;
                        case SDLK_w:
                            if (event.key.repeat == 0) {
                                starttime = event.key.timestamp;
                                rScaleWaiting = rScale * 1.02L;
                            } else {
                                rScaleWaiting = rScale * (1 + .05L * (event.key.timestamp - starttime) / 1000);
                            }
                            dir = ZIN;
                            break;
                        case SDLK_s:
                            if (event.key.repeat == 0) {
                                starttime = event.key.timestamp;
                                rScaleWaiting = rScale / 1.02L;
                            } else {
                                rScaleWaiting = rScale * (1 - .05L * (event.key.timestamp - starttime) / 1000);
                            }
                            if (rScaleWaiting < 64L) {
                                rScaleWaiting = 64L;
                            }
                            dir = ZOUT;
                            break;
                        default:
                            dir = REFRESH;
                            break;
                    }
                    act = 1;
                    break;
                case SDL_KEYUP:
                    starttime = event.key.timestamp;
                    startphi = phiLeft;
                    starttilt = axisTilt;
                    break;
                case SDL_MOUSEBUTTONDOWN: {
                    clicked = at(event.button.x, event.button.y);
                    if (clicked.t != 2.0L) {
                        pt center = at(centerX, centerY);
                        phiLeftWaiting = phiLeft + (clicked.p - center.p);
                        axisTiltWaiting = axisTilt - (clicked.t - center.t);
                        mousedown = 1;
                    }
                    break;
                }
                case SDL_MOUSEMOTION: {
                    if (mousedown && !act) {
                        pt offsets = getOffsetsFrom(event.motion.x, event.motion.y, clicked);
                        if (offsets.t != 2.0L) {
                            phiLeftWaiting = offsets.p;
                            axisTiltWaiting = offsets.t;
                            if (axisTiltWaiting < -PIHalf) {
                                axisTiltWaiting = -PIHalf;
                            } else if (axisTiltWaiting > PIHalf) {
                                axisTiltWaiting = PIHalf;
                            }
                            dir = REFRESH;
                            act = 1;
                        }
                    }
                    mouseX = event.motion.x;
                    mouseY = event.motion.y;
                    break;
                }
                case SDL_MOUSEBUTTONUP: {
                    mousedown = 0;
                    dir = REFRESH;
                    act = 1;
                    break;
                }
                case SDL_MOUSEWHEEL: {
                    pt target = at(mouseX, mouseY);
                    if (target.t != 2.0L) {
                        if ((event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? 1 : -1) * event.wheel.y > 0) {
                            rScaleWaiting = rScale * 1.2L;
                            dir = ZIN;
                        }
                        else {
                            rScaleWaiting = rScale / 1.2L;
                            if (rScaleWaiting < 64L) {
                                rScaleWaiting = 64L;
                            }
                            dir = ZOUT;
                        }
                        target = getOffsetsFromWithRadius(mouseX, mouseY, target, rScaleWaiting);
                        if (target.t != 2.0L) {
                            phiLeftWaiting = target.p;
                            axisTiltWaiting = target.t;
                            if (axisTiltWaiting < -PIHalf) {
                                axisTiltWaiting = -PIHalf;
                            }
                            else if (axisTiltWaiting > PIHalf) {
                                axisTiltWaiting = PIHalf;
                            }
                            act = 1;
                        }
                        else {
                            rScaleWaiting = rScale;
                        }
                    }
                    break;
                }
                case SDL_WINDOWEVENT:
                    if (event.window.windowID == windowID) {
                        switch (event.window.event) {
                            case SDL_WINDOWEVENT_SIZE_CHANGED:
                                newWidth = event.window.data1;
                                newHeight = event.window.data2;
                                windowSizeChanged = 1;
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        if (act && rastered && !dequeueing && notScheduled) {
            notScheduled = 0;
            if (dontWaitForCollector) {
                act = 0;
                phiLeft = phiLeftWaiting;
                axisTilt = axisTiltWaiting;
                axisTiltF = axisTilt;
                phiLeftF = phiLeft;
                axisTiltD = axisTilt;
                phiLeftD = phiLeft;
                if (rScale != rScaleWaiting) {
                    rScale = rScaleWaiting;
                    rScaleSqr = rScale * rScale;
                    rScaleSqrF = rScaleSqr;
                    rScaleF = rScale;
                    rScaleSqrD = rScaleSqr;
                    rScaleD = rScale;
                    determineZoom();
                }
                queued = 0;
                for (int i = 0; i < maxThreads; ++i) {
                    WaitForSingleObject(threadsData[i].hThread, INFINITE);
                    CloseHandle(threadsData[i].hThread);
                    threadsData[i].hThread = (HANDLE)_beginthreadex(NULL, 0, zoomF < maxZoomLighting ? rasterFWithLighting : zoom < 16 ? rasterF : zoom < 24 ? rasterD : raster, (void*)&(threadsData[i]), 0, NULL);
                    if (threadsData[i].hThread == 0) {
                        notquitrequested = 0;
                        goto AFTER_LOOP;
                    }
                }
                if (!collecting) {
                    doCollecting = 0;
                    WaitForSingleObject(hCollector, INFINITE);
                    CloseHandle(hCollector);
                    doCollecting = 1;
                    checkingImageRequests = 1;
                    hCollector = (HANDLE)_beginthreadex(NULL, 0, collector, NULL, 0, NULL);
                    if (hCollector == 0) {
                        notquitrequested = 0;
                        goto AFTER_LOOP;
                    }
                }
            }
            else {
                notScheduled = 1;
            }
        }
        if (!rastered) {
            int countRastering = 0;
            for (int i = 0; i < maxThreads; ++i) {
                countRastering += threadsData[i].rastering;
            }
            if (countRastering == 0) {
                if (zoomF < maxZoomLighting && elevationDataAvailable) {
                    elevate();
                }
                memcpy(region, buffer, pitch * HEIGHT);                             // copy all once in main thread appears to be faster than copy parts parallely from threads
                SDL_UnlockTexture(texture);
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);
                if (SDL_LockTexture(texture, NULL, &region, &pitch) != 0) {
                    textureLock = 0;
                    notquitrequested = 0;
                    goto AFTER_LOOP;
                }
                rastered = 1;
            }
        }
        else if (queued) {
            if (!checkingImageRequests && allImagesRequestedPresent) {
                wantsCompletion = 1;
                if (dontWaitForCollector) {
                    queued = 0;
                    dequeueing = 1;
                    for (int i = 0; i < maxThreads; ++i) {
                        if (threadsData[i].hComplete != 0) {
                            WaitForSingleObject(threadsData[i].hComplete, INFINITE);
                            CloseHandle(threadsData[i].hComplete);
                        }
                        threadsData[i].hComplete = (HANDLE)_beginthreadex(NULL, 0, zoomF < maxZoomLighting ? rasterCompletionWithLighting : rasterCompletion, (void*)&(threadsData[i]), 0, NULL);
                    }
                }
                else {
                    wantsCompletion = 0;
                }
            }
        }
        else if (dequeueing) {
            int countNotEmpties = 0;
            for (int i = 0; i < maxThreads; ++i) {
                if (threadsData[i].hComplete != 0) {
                    countNotEmpties += (threadsData[i].lastQueue == NULL ? 0 : 1);
                }
            }
            if (countNotEmpties == 0) {
                if (zoomF < maxZoomLighting && elevationDataAvailable) {
                    elevate();
                }
                memcpy(region, buffer, pitch * HEIGHT);                             // copy all once in main thread appears to be faster than copy parts parallely from threads
                SDL_UnlockTexture(texture);
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);
                if (SDL_LockTexture(texture, NULL, &region, &pitch) != 0) {
                    textureLock = 0;
                    notquitrequested = 0;
                    goto AFTER_LOOP;
                }
                wantsCompletion = 0;
                dequeueing = 0;
            }
        }

        if (windowSizeChanged) {
            if (rastered && !dequeueing && notScheduled) {
                notScheduled = 0;
                if (dontWaitForCollector) {
                    windowSizeChanged = 0;
                    WIDTH = newWidth;
                    HEIGHT = newHeight;
                    free(buffer);
                    SDL_UnlockTexture(texture);
                    SDL_DestroyTexture(texture);
                    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
                    if (texture == NULL) {
                        notquitrequested = 0;
                        goto AFTER_LOOP;
                    }
                    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
                    if (SDL_LockTexture(texture, NULL, &region, &pitch) != 0) {
                        textureLock = 0;
                        notquitrequested = 0;
                        goto AFTER_LOOP;
                    }
                    buffer = malloc(pitch * HEIGHT);
                    if (buffer == NULL) {
                        notquitrequested = 0;
                        goto AFTER_LOOP;
                    }
                    centerX = WIDTH / 2;
                    centerY = HEIGHT / 2;
                    dir = REFRESH;
                    phiLeft = phiLeftWaiting;
                    axisTilt = axisTiltWaiting;
                    rScale = rScaleWaiting;
                    rScaleSqr = rScale * rScale;
                    rScaleSqrF = rScaleSqr;
                    rScaleF = rScale;
                    rScaleSqrD = rScaleSqr;
                    rScaleD = rScale;
                    axisTiltF = axisTilt;
                    phiLeftF = phiLeft;
                    axisTiltD = axisTilt;
                    phiLeftD = phiLeft;
                    determineZoom();
                    queued = 0;
                    for (int i = 0; i < maxThreads; ++i) {
                        WaitForSingleObject(threadsData[i].hThread, INFINITE);
                        CloseHandle(threadsData[i].hThread);
                    }
                    if (cores > HEIGHT) {
                        maxThreads = HEIGHT;
                    }
                    else {
                        maxThreads = cores;
                    }
                    for (int i = 0; i < maxThreads; ++i) {
                        threadsData[i].yStart = (i * HEIGHT) / maxThreads;
                        threadsData[i].yEnd = ((i + 1) * HEIGHT) / maxThreads;
                        threadsData[i].hThread = (HANDLE)_beginthreadex(NULL, 0, zoomF < maxZoomLighting ? rasterFWithLighting : zoom < 16 ? rasterF : zoom < 24 ? rasterD : raster, (void*)&(threadsData[i]), 0, NULL);
                        if (threadsData[i].hThread == 0) {
                            notquitrequested = 0;
                            maxThreads = i;
                            goto AFTER_LOOP;
                        }
                    }
                    if (!collecting) {
                        doCollecting = 0;
                        WaitForSingleObject(hCollector, INFINITE);
                        CloseHandle(hCollector);
                        doCollecting = 1;
                        checkingImageRequests = 1;
                        hCollector = (HANDLE)_beginthreadex(NULL, 0, collector, NULL, 0, NULL);
                        if (hCollector == 0) {
                            notquitrequested = 0;
                            goto AFTER_LOOP;
                        }
                    }
                }
                else {
                    notScheduled = 1;
                }
            }
        }
    }


AFTER_LOOP:
    if(nonRequestedExit)
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "an error occurred:", "likely one of\n\n\t'failed allocating required memory',\n\t'failed starting thread'\n\n.", window);

    if (elevationDataAvailable)
        free(elevationData);

    if (hCollector != 0) {
        doCollecting = 0;
        WaitForSingleObject(hCollector, INFINITE);
        CloseHandle(hCollector);
    }

    for (int i = 0; i < maxThreads; ++i) {
        if (threadsData[i].hThread != 0) {
            WaitForSingleObject(threadsData[i].hThread, INFINITE);
            CloseHandle(threadsData[i].hThread);
        }
    }
    for (int i = 0; i < cores; ++i) {
        if (threadsData[i].lastQueue != NULL) {
            hashmap_iterate(threadsData[i].lastQueue, clearQueue, NULL);
            hashmap_free(threadsData[i].lastQueue);
        }
        if (threadsData[i].hComplete != 0) {
            WaitForSingleObject(threadsData[i].hComplete, INFINITE);
            CloseHandle(threadsData[i].hComplete);
        }
        idData* id = threadsData[i].dId.next;
        while (id != NULL) {
            idData* currentId = id;
            id = id->next;
            free(currentId);
        }
    }

    free(threadsData);

    hashmap_iterate(imgRequested, freeAsyncIdMemory, NULL);
    hashmap_iterate(imgPresent, freeImgPresentMemory, NULL);

    hashmap_free(imgPresent);
    hashmap_free(imgRequested);

    free(cachePathCollector);
    free(cachePath);
    free(path);
    free(host);
    free(urlFormat);
    if(buffer != NULL)
        free(buffer);
    SDL_FreeSurface(icon);
    if (texture != NULL) {
        if (textureLock)
            SDL_UnlockTexture(texture);
        SDL_DestroyTexture(texture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    LOG(("app end\n"));

    return 0;
}