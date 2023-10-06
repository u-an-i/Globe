#define WIN32_LEAN_AND_MEAN             // discard rarely used components from Windows header

#include <SDL.h>
#include <SDL_main.h>                   // only include this one in the source file with main()!
#include <stdio.h>
#include <mathimf.h>
#include <map.h>
#include <stdlib.h>
#include <windows.h>
#include <winhttp.h>
#include <turbojpeg.h>

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
            long double ySpace = r2Sqrt * sinl(asinl(yC / r2Sqrt) - axisTilt);
            long double t = asinl(ySpace / rScale);
            long double p;
            long double r2ScAT = r2Sqrt * cosl(axisTilt);
            long double s = ((axisTilt > 0.0L && -yC > r2ScAT) || (axisTilt < 0.0L && yC > r2ScAT)) ? -1.0L : 1.0L;
            long double r3Sqr = rScaleSqr - ySpace * ySpace;
            if (r3Sqr > 0.0L)
            {
                p = phiLeft + s * acosl(-xC / sqrtl(r3Sqr));
            }
            else
            {
                p = phiLeft + s * PIHalf;
            }
            p = fmodl(p + PIDouble, PIDouble);
            pt value;
            value.p = p;
            value.t = t;
            return value;
        }
        else
        {
            long double p = phiLeft;
            if (xC > 0.0L)
            {
                p += PI;
            }
            p = fmodl(p + PIDouble, PIDouble);
            pt value;
            value.p = p;
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
            long double axisTilt = asinl(yC / r2Sqrt) - asin(rScale * sin(sc.t) / r2Sqrt);
            long double phiLeft;
            long double r2ScAT = r2Sqrt * cosl(axisTilt);
            long double s = ((axisTilt > 0.0L && -yC > r2ScAT) || (axisTilt < 0.0L && yC > r2ScAT)) ? -1.0L : 1.0L;
            long double ySpace = r2Sqrt * sinl(asinl(yC / r2Sqrt) - axisTilt);
            long double r3Sqr = rScaleSqr - ySpace * ySpace;
            if (r3Sqr > 0.0L)
            {
                phiLeft = sc.p - s * acosl(-xC / sqrtl(r3Sqr));
            }
            else
            {
                phiLeft = sc.p - s * PIHalf;
            }
            phiLeft = fmodl(phiLeft + PIDouble, PIDouble);
            pt value;
            value.p = phiLeft;
            value.t = axisTilt;
            return value;
        } 
        else {
            long double phiLeft = sc.p;
            if (xC > 0.0L)
            {
                phiLeft -= PI;
            }
            phiLeft = fmodl(phiLeft + PIDouble, PIDouble);
            pt value;
            value.p = phiLeft;
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
            long double axisTilt = asinl(yC / r2Sqrt) - asin(rScale * sin(sc.t) / r2Sqrt);
            long double phiLeft;
            long double r2ScAT = r2Sqrt * cosl(axisTilt);
            long double s = ((axisTilt > 0.0L && -yC > r2ScAT) || (axisTilt < 0.0L && yC > r2ScAT)) ? -1.0L : 1.0L;
            long double ySpace = r2Sqrt * sinl(asinl(yC / r2Sqrt) - axisTilt);
            long double r3Sqr = rScaleSqr - ySpace * ySpace;
            if (r3Sqr > 0.0L)
            {
                phiLeft = sc.p - s * acosl(-xC / sqrtl(r3Sqr));
            }
            else
            {
                phiLeft = sc.p - s * PIHalf;
            }
            phiLeft = fmodl(phiLeft + PIDouble, PIDouble);
            pt value;
            value.p = phiLeft;
            value.t = axisTilt;
            return value;
        }
        else {
            long double phiLeft = sc.p;
            if (xC > 0.0L)
            {
                phiLeft -= PI;
            }
            phiLeft = fmodl(phiLeft + PIDouble, PIDouble);
            pt value;
            value.p = phiLeft;
            value.t = 0.0L;
            return value;
        }
    }
    pt value;
    value.p = 0.0L;
    value.t = 2.0L;
    return value;
}


int zoom;
long double stepSize;

void determineZoom() {
    pt middleLeft = at(0, HEIGHT / 2);
    int newZoom;
    long double newStepSize;
    if (middleLeft.t == 2.0L) {
        newZoom = ceill(log2l(2 * rScale * 2 / rasterTileSize));
        newStepSize = PIDouble / 48;
    } else {
        long double deltaP = at(WIDTH - 1, HEIGHT / 2).p - middleLeft.p;
        if (deltaP <= 0.0L) {
            deltaP += PIDouble;
        }
        long double pixelPerPhi = WIDTH / deltaP;
        newZoom = ceill(log2l(pixelPerPhi * PIDouble / rasterTileSize));
        newStepSize = deltaP / 16;
    }
    if (newZoom >= 0 && newZoom <= 30) {
        zoom = newZoom;
        stepSize = newStepSize;
    }
}


const float PIF = 3.141592653589793238462643383279F;
const float PIHalfF = 1.570796326794896619231321691639F;
const float PIDoubleF = 6.28318530717958647692528676655F;

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
            float ySpace = r2Sqrt * sinf(asinf(yC / r2Sqrt) - axisTiltF);
            float t = asinf(ySpace / rScaleF);
            float p;
            float r2ScAT = r2Sqrt * cosf(axisTiltF);
            float s = ((axisTiltF > 0.0F && -yC > r2ScAT) || (axisTiltF < 0.0F && yC > r2ScAT)) ? -1.0F : 1.0F;
            float r3Sqr = rScaleSqrF - ySpace * ySpace;
            if (r3Sqr > 0.0F)
            {
                p = phiLeftF + s * acosf(-xC / sqrtf(r3Sqr));
            }
            else
            {
                p = phiLeftF + s * PIHalfF;
            }
            p = fmodf(p + PIDoubleF, PIDoubleF);
            ptF value;
            value.p = p;
            value.t = t;
            return value;
        }
        else
        {
            float p = phiLeftF;
            if (xC > 0.0F)
            {
                p += PIF;
            }
            p = fmodf(p + PIDoubleF, PIDoubleF);
            ptF value;
            value.p = p;
            value.t = 0.0F;
            return value;
        }
    }
    ptF value;
    value.p = 0.0F;
    value.t = 2.0F;
    return value;
}


void determineZoomF() {
    ptF middleLeft = atF(0, HEIGHT / 2);
    int newZoom;
    long double newStepSize;
    if (middleLeft.t == 2.0F) {
        newZoom = ceilf(log2f(2 * rScaleF * 2 / rasterTileSize));
        newStepSize = PIDouble / 48;
    }
    else {
        float deltaP = atF(WIDTH - 1, HEIGHT / 2).p - middleLeft.p;
        if (deltaP <= 0.0F) {
            deltaP += PIDoubleF;
        }
        float pixelPerPhi = WIDTH / deltaP;
        newZoom = ceilf(log2f(pixelPerPhi * PIDoubleF / rasterTileSize));
        newStepSize = deltaP / 16;
    }
    if (newZoom >= 0 && newZoom <= 30) {
        zoom = newZoom;
        stepSize = newStepSize;
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
            aId->buffer = malloc(rasterTileSize * rasterTileSize * 3);                      // assume size of uncompressed image is sufficient (checked above)
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
    char id[25];
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
                                            if (hashmap_sets_left_before_resize(imgPresent) <= 1) {
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
                                    LIKE_AIDNULL:
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

typedef struct Pixel {
    int sourceX, sourceY;
    int targetX, targetY;
} pixel;

void pickPixel(pixel* p, unsigned char* pixels) {
    memcpy((void*)(((unsigned char*)buffer) + (p->targetY * pitch + p->targetX * 3)), (void*)(pixels + (p->sourceY * rasterTileSize + p->sourceX) * 3), 3);
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
                long double yTile = (angles.t - -PIHalf - .0001L) * amount / PI;        // - .0001 = prevent exact 1 as result (values in image are [0,1), [ = including, ) = excluding )
                int tileX = (int)xTile;                                                 // (int) floors towards 0
                int tileY = (int)yTile;
                char id[25];                                                            // maximum length of id incl. \0 at maximum zoom of 30
                int length = sprintf_s(id, 25, idFormat, zoom, tileX, tileY);
                if (length == 0) {
                    length = 5;
                    strcpy(id, "0/0/0");
                    zoom = 0;
                    xTile = angles.p / PIDouble;
                    if (xTile < 0.0L) {
                        xTile = 0.0L;
                    }
                    else if (xTile >= 1.0L) {
                        xTile = .9999L;
                    }
                    yTile = (angles.t - -PIHalf - .0001L) / PI;
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
                            yTileI = (angles.t - -PIHalf - .0001L) * amount / PI;
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
                                long double yTile = (angles.t - -PIHalf - .0001L) * amount / PI;
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
                                    yTile = (angles.t - -PIHalf - .0001L) / PI;
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
                float yTile = (angles.t - -PIHalfF - .0001F) * amount / PIF;            // - .0001 = prevent exact 1 as result (values in image are [0,1), [ = including, ) = excluding )
                int tileX = (int)xTile;                                                 // (int) floors towards 0
                int tileY = (int)yTile;
                char id[25];                                                            // maximum length of id incl. \0 at maximum zoom of 30
                int length = sprintf_s(id, 25, idFormat, zoom, tileX, tileY);
                if (length == 0) {
                    length = 5;
                    strcpy(id, "0/0/0");
                    zoom = 0;
                    xTile = angles.p / PIDoubleF;
                    if (xTile < 0.0F) {
                        xTile = 0.0F;
                    }
                    else if (xTile >= 1.0F) {
                        xTile = .9999F;
                    }
                    yTile = (angles.t - -PIHalfF - .0001F) / PIF;
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
                        yTileI = (angles.t - -PIHalfF - .0001F) * amount / PIF;
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
                            float yTile = (angles.t - -PIHalfF - .0001F) * amount / PIF;
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
                                yTile = (angles.t - -PIHalfF - .0001F) / PIF;
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
                                icon = SDL_LoadBMP("globe.bmp");
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

    determineZoomF();


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
                            if (freeUrl)
                                free(url);
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
        threadsData[i].hThread = (HANDLE)_beginthreadex(NULL, 0, zoom < 16 ? rasterF : raster, (void*)&(threadsData[i]), 0, NULL);
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

    int nonRequestedExit = 1;

    int x, y;
    SDL_GetWindowPosition(window, &x, &y);
    SDL_WarpMouseGlobal(x + centerX, y + centerY);
    INPUT fakeFirstClick[2];
    fakeFirstClick[0].type = INPUT_MOUSE;
    fakeFirstClick[0].mi.dx = 0;
    fakeFirstClick[0].mi.dy = 0;
    fakeFirstClick[0].mi.mouseData = 0;
    fakeFirstClick[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    fakeFirstClick[0].mi.time = 0;
    fakeFirstClick[0].mi.dwExtraInfo = (ULONG_PTR)NULL;
    fakeFirstClick[1].type = INPUT_MOUSE;
    fakeFirstClick[1].mi.dx = 0;
    fakeFirstClick[1].mi.dy = 0;
    fakeFirstClick[1].mi.mouseData = 0;
    fakeFirstClick[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    fakeFirstClick[1].mi.time = 0;
    fakeFirstClick[1].mi.dwExtraInfo = (ULONG_PTR)NULL;
    SendInput(2, fakeFirstClick, sizeof(INPUT));

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
            if (act && rastered && !dequeueing && notScheduled) {
                notScheduled = 0;
                if (dontWaitForCollector) {
                    act = 0;
                    phiLeft = phiLeftWaiting;
                    axisTilt = axisTiltWaiting;
                    axisTiltF = axisTilt;
                    phiLeftF = phiLeft;
                    if (rScale != rScaleWaiting) {
                        rScale = rScaleWaiting;
                        rScaleSqr = rScale * rScale;
                        rScaleSqrF = rScaleSqr;
                        rScaleF = rScale;
                        determineZoomF();
                    }
                    queued = 0;
                    for (int i = 0; i < maxThreads; ++i) {
                        WaitForSingleObject(threadsData[i].hThread, INFINITE);
                        CloseHandle(threadsData[i].hThread);
                        threadsData[i].hThread = (HANDLE)_beginthreadex(NULL, 0, zoom < 16 ? rasterF : raster, (void*)&(threadsData[i]), 0, NULL);
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
        }

        if (!rastered) {
            int countRastering = 0;
            for (int i = 0; i < maxThreads; ++i) {
                countRastering += threadsData[i].rastering;
            }
            if (countRastering == 0) {
                memcpy(region, buffer, pitch * HEIGHT);                             // copy all once in main thread appears to be faster than copy parts parallely from threads
                SDL_UnlockTexture(texture);
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);
                if (SDL_LockTexture(texture, NULL, &region, &pitch) != 0) {
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
                        threadsData[i].hComplete = (HANDLE)_beginthreadex(NULL, 0, rasterCompletion, (void*)&(threadsData[i]), 0, NULL);
                    }
                }
                else {
                    wantsCompletion = 0;
                }
            }
        }
        else if (dequeueing) {
            int countEmpties = 0;
            for (int i = 0; i < maxThreads; ++i) {
                if (threadsData[i].hComplete != 0) {
                    countEmpties += (threadsData[i].lastQueue == NULL ? 0 : 1);
                }
            }
            if (countEmpties == 0) {
                memcpy(region, buffer, pitch * HEIGHT);                             // copy all once in main thread appears to be faster than copy parts parallely from threads
                SDL_UnlockTexture(texture);
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);
                if (SDL_LockTexture(texture, NULL, &region, &pitch) != 0) {
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
                    axisTiltF = axisTilt;
                    phiLeftF = phiLeft;
                    determineZoomF();
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
                        threadsData[i].hThread = (HANDLE)_beginthreadex(NULL, 0, zoom < 16 ? rasterF : raster, (void*)&(threadsData[i]), 0, NULL);
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

    free(path);
    free(host);
    free(urlFormat);
    free(buffer);
    SDL_FreeSurface(icon);
    if (texture != NULL) {
        if (!rastered || dequeueing)
            SDL_UnlockTexture(texture);
        SDL_DestroyTexture(texture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    LOG(("app end\n"));

    return 0;
}