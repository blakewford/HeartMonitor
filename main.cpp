#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <fstream>

#include <thread>
#include <chrono>
using namespace std::chrono;

#include "MEDBENCH/MEDBENCH.ino"

bool gKeepGoing = true;
const int32_t SCALE = 8;
float SCREEN_DATA[WIDTH*HEIGHT];

struct pgm
{
    const char* magic = "P2";
    const int32_t max = 255;
    int32_t width, height;
    float* image;

    pgm()
    {
        width = 0;
        height = 0;
        image = nullptr;
    }
};

pgm gScreen;
uint64_t gFrame = 0;
system_clock::time_point gSyncPoint;
system_clock::time_point gAudioSyncPoint;
milliseconds gFrameRate = milliseconds(1000);

bool inRange(int32_t x, int32_t y)
{
    return (x >= 0 && x < WIDTH) && (y >= 0 && y < HEIGHT);
}

float getPixel(const pgm& image, int32_t x, int32_t y)
{
    int32_t index = (y*image.width)+x;
    return image.image[index];
}

void setPixel(const pgm& image, int32_t x, int32_t y, float value)
{
    if(!inRange(x, y)) return;

    int32_t index = (y*image.width)+x;
    image.image[index] = value;
}

void writeImage(const pgm& image, const char* file)
{
    std::ofstream stream;
    stream.open(file);
    stream << image.magic << "\n" << image.width << " " << image.height << "\n" << image.max << "\n";

    int32_t j = 0;
    int32_t i = (image.height*image.width);
    while(i--)
    {
         stream << image.image[j] << "\n";
         j++;
    }

    stream.close();
}

void convertImage(const unsigned char* bitmap, const uint8_t width, const uint8_t height, pgm& converted)
{
    converted.width  = width;
    converted.height = height;
    converted.image = new float[converted.width*converted.height];

    uint16_t i = 2; //offset past dimensions

    uint16_t j = 0;
    uint16_t k = 0;
    uint16_t pixel = 0;
    while(j < converted.height)
    {
        while(k < converted.width)
        {
            converted.image[pixel++] = (bitmap[i+k] & 1<<(j%8)) ? 255.0f: 0.0f;
            k++;
        }
        k = 0;
        j++;
        if(j%8 == 0)
        {
            i+=converted.width;
        }
    }
}

void convertImageAndMask(const unsigned char* bitmap, const uint8_t width, const uint8_t height, pgm& converted, pgm& mask)
{
    converted.width  = width;
    converted.height = height;
    converted.image = new float[converted.width*converted.height];

    mask.width  = width;
    mask.height = height;
    mask.image = new float[mask.width*mask.height];

    uint16_t i = 2; //offset past dimensions

    uint16_t j = 0;
    uint16_t k = 0;
    uint16_t pixel = 0;
    while(j < converted.height)
    {
        while(k < (converted.width*2))
        {
            converted.image[pixel] = (bitmap[i+k] & 1<<(j%8)) ? 255.0f: 0.0f;
            k++;
            mask.image[pixel++] = (bitmap[i+k] & 1<<(j%8)) ? 255.0f: 0.0f;
            k++;
        }
        k = 0;
        j++;
        if(j%8 == 0)
        {
            i+=(converted.width*2);
        }
    }
}

bool clipImage(pgm& modified, int16_t x, int16_t y)
{
    if((x >= WIDTH) || (y >= HEIGHT))
    {
        return false;
    }

    bool canClip = false;

    int16_t visibleX = 0;
    int16_t visibleY = 0;
    int16_t originalWidth = modified.width;
    int16_t originalHeight = modified.height;
    if(x < 0)
    {
        visibleX = x+modified.width;
        if(visibleX < 0) return false;

        modified.width = visibleX;
        canClip = true;
    }

    if(y < 0)
    {
        visibleY = y+modified.height;
        if(visibleY < 0)
        {
            return false;
        }

        modified.height = visibleY;
        canClip = true;
    }

    if(canClip)
    {
        int16_t start = originalWidth-visibleX;
        int16_t diffX = originalHeight-visibleY;
    }

    return canClip;
}

void writeToScreen(const pgm& image, int16_t x, int16_t y)
{
    int16_t i = 0;
    int16_t j = 0;
    int32_t pixel = 0;

    int16_t offsetX = 0;
    int16_t offsetY = 0;
    pgm modified = image;
    if(!inRange(x, y))
    {
        if(!clipImage(modified, x, y)) return;
        offsetX = image.width - modified.width;
        offsetY = image.height - modified.height;
    }

    while(j < modified.height)
    {
        while(i < modified.width)
        {
            pixel = getPixel(image, i+offsetX, j+offsetY);
            if(pixel != 0.0f)
            {
                setPixel(gScreen, offsetX+x+i, offsetY+y+j, pixel);
            }
            i++;
        }
        i=0;
        j++;
    }
}

void eraseFromScreen(const pgm& image, int16_t x, int16_t y)
{
    if(!inRange(x, y)) return;

    int16_t i = 0;
    int16_t j = 0;
    int32_t pixel = 0;
    while(j < image.height)
    {
        while(i < image.width)
        {
            pixel = getPixel(image, i, j);
            if(pixel != 0.0f)
            {
                setPixel(gScreen, x+i, y+j, 0.0f);
            }
            i++;
        }
        i=0;
        j++;
    }
}

int32_t calculateOffset(const unsigned char* bitmap, const uint8_t frame)
{
    return ((bitmap[0]*bitmap[1])*frame)/8;
}

void writeToScreen(const unsigned char* bitmap, int16_t x, int16_t y, uint8_t frame)
{
    pgm item;
    int32_t offset = calculateOffset(bitmap, frame);
    convertImage(bitmap+offset, bitmap[0], bitmap[1], item);
    item.height = bitmap[1];
    writeToScreen(item, x, y);

    delete[] item.image;
    item.image = nullptr;
}

void maskToScreen(const unsigned char* bitmap, int16_t x, int16_t y, uint8_t frame)
{
    pgm item;
    pgm mask;
    int32_t offset = (((bitmap[0]*bitmap[1])*frame)*2)/8;
    convertImageAndMask(bitmap+offset, bitmap[0], bitmap[1], item, mask);

    int16_t offsetX = 0;
    int16_t offsetY = 0;
    pgm modified = item;
    if(!inRange(x, y))
    {
        if(!clipImage(modified, x, y)) return;
        offsetX = item.width - modified.width;
        offsetY = item.height - modified.height;
    }

    int16_t i = 0;
    int16_t j = 0;
    int32_t pixel = 0;
    while(j < modified.height)
    {
        while(i < modified.width)
        {
            pixel = getPixel(mask, i+offsetX, j+offsetY);
            if(pixel != 0.0f)
            {
                pixel = getPixel(item, i+offsetX, j+offsetY);
                setPixel(gScreen, offsetX+x+i, offsetY+y+j, pixel);
            }
            i++;
        }
        i=0;
        j++;
    }

    delete[] item.image;
    item.image = nullptr;

    delete[] mask.image;
    mask.image = nullptr;
}

void delay(uint32_t ms)
{
    std::this_thread::sleep_for(milliseconds(ms));
}

long random(long howsmall, long howbig)
{
    long diff = howbig - howsmall;
    return howsmall + (rand()%diff);
}

char* ltoa(long l, char * buffer, int radix)
{
    if(radix != 10) assert(0);

    sprintf(buffer, "%ld", l);
    return buffer;
}

void Arduboy2Base::begin()
{
}

void Arduboy2Base::setFrameRate(uint8_t rate)
{
    gFrameRate = milliseconds(1000/rate);
}

void Arduboy2Base::initRandomSeed()
{
}

bool Arduboy2Base::everyXFrames(uint8_t frames)
{
    return gFrame%frames == 0;
}

struct buttonState
{
    bool upButton = false;
    bool leftButton = false;
    bool downButton = false;
    bool rightButton = false;
    bool buttonA = false;
    bool buttonB = false;

    void clear()
    {
        upButton = false;
        leftButton = false;
        downButton = false;
        rightButton = false;
        buttonA = false;
        buttonB = false;
    }
};

buttonState gButtonState;
buttonState gCachedButtonState;

bool Arduboy2Base::justPressed(uint8_t button)
{
    bool pressed = false;
    switch(button)
    {
        case LEFT_BUTTON:
            pressed = gCachedButtonState.leftButton;
            break;
        case RIGHT_BUTTON:
            pressed = gCachedButtonState.rightButton;
            break;
        case UP_BUTTON:
            pressed = gCachedButtonState.upButton;
            break;
        case DOWN_BUTTON:
            pressed = gCachedButtonState.downButton;
            break;
        case A_BUTTON:
            pressed = gCachedButtonState.buttonA;
            break;
        case B_BUTTON:
            pressed = gCachedButtonState.buttonB;
            break;
        default:
            break;
    }

    return pressed;
}

bool Arduboy2Base::collide(Rect rect1, Rect rect2)
{
  return !(rect2.x >= rect1.x + rect1.width || rect2.x + rect2.width <= rect1.x || rect2.y >= rect1.y + rect1.height || rect2.y + rect2.height <= rect1.y);
}

bool Arduboy2Base::nextFrame()
{
    while(system_clock::now() < gSyncPoint)
    {
//        std::this_thread::yield();
        std::this_thread::sleep_for(nanoseconds(1));
    }

    gSyncPoint = system_clock::now() + gFrameRate;
    gFrame++;

    return true;
}

void Arduboy2Base::pollButtons()
{
    gCachedButtonState = gButtonState;
    gButtonState.clear();
}

void Arduboy2Base::clear()
{
    gScreen.width = WIDTH;
    gScreen.height = HEIGHT;
    gScreen.image = SCREEN_DATA;
    memset(gScreen.image, 0, WIDTH*HEIGHT*sizeof(float));
}

void Arduboy2Base::display()
{
//    writeImage(gScreen, "test.pgm");
}

ArduboyTones::ArduboyTones(bool (*outEn)())
{
}

#include <SDL.h>

SDL_AudioDeviceID gAudioDevice = ~0;
SDL_AudioSpec gAudioSpec = {.freq=44100, .format=32784, .channels=2, .silence=0, .samples=4096, .padding=0, .size=0, .callback=nullptr, .userdata=nullptr};

void ArduboyTones::tone(uint16_t freq, uint16_t dur)
{
    if(system_clock::now() < gAudioSyncPoint) return; //busy

    assert(freq >= 16 && freq <= 32767);
    assert(dur < (uint16_t)~0);

    if(gAudioDevice < 0) return;

    const int32_t scale = (gAudioSpec.freq/freq);

    int32_t count = scale*2;
    uint8_t* wav = new uint8_t[count];
    while(count-- > scale)
        wav[count] = 255;
    while(count--)
        wav[count] = 50;

    count = ((gAudioSpec.freq/1000)*dur)/(scale*2);
    while(count--)
    {
        if(SDL_QueueAudio(gAudioDevice, wav, scale*2) != 0)
        {
            break;
        }
    }

    SDL_PauseAudioDevice(gAudioDevice, 0);
    gAudioSyncPoint = system_clock::now() + milliseconds(200);
    delete[] wav;
}

bool gAudioEnabled = true;

bool Arduboy2Audio::enabled()
{
    return gAudioEnabled;
}

void Arduboy2Audio::off()
{
    gAudioEnabled = false;
}

void Arduboy2Audio::on()
{
    gAudioEnabled = true;
}

void Arduboy2Audio::saveOnOff()
{
}

unsigned long int getImageSize(const uint8_t *bitmap)
{
    unsigned long int size = 0;

    if(bitmap == Logo)
    {
        size = sizeof(Logo);
    }
    else if(bitmap == Heart)
    {
        size = sizeof(Heart);
    }
    else if(bitmap == Numbers)
    {
        size = sizeof(Numbers);
    }
    else if(bitmap == BPM)
    {
        size = sizeof(BPM);
    }
    else if(bitmap == Start)
    {
        size = sizeof(Start);
    }
    else
    {
        assert(0);
    }

    return size;
}

void Sprites::drawSelfMasked(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame)
{
    unsigned long int size = getImageSize(bitmap);
    if(size != 0)
    {
        writeToScreen(bitmap, x, y, frame);
    }
}

void Sprites::drawErase(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame)
{
    unsigned long int size = getImageSize(bitmap);
    if(size != 0)
    {
        pgm mask;
        int32_t offset = calculateOffset(bitmap, frame);
        convertImage(bitmap+offset, bitmap[0], bitmap[1], mask);
        mask.height = bitmap[1];
        eraseFromScreen(mask, x, y);
        delete[] mask.image;
        mask.image = nullptr;
    }
}

void Sprites::drawPlusMask(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame)
{
    unsigned long int size = getImageSize(bitmap)/2;
    if(size != 0)
    {
        maskToScreen(bitmap, x, y, frame);
    }
}

struct SDL_Components
{
    SDL_Window* w;
    SDL_Texture* t;
    SDL_Renderer* r;
};

SDL_Components gComponents;

int32_t SDL_Init()
{
    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) return -1;

    gAudioDevice = SDL_OpenAudioDevice(nullptr, 0, &gAudioSpec, nullptr, 0);

    gComponents.w = SDL_CreateWindow("Heart Monitor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH*SCALE, HEIGHT*SCALE, SDL_WINDOW_SHOWN);
    if(gComponents.w == nullptr) return -1;

    gComponents.r = SDL_CreateRenderer(gComponents.w, -1, 0);
    if(gComponents.r == nullptr) return -1;

    SDL_RenderSetScale(gComponents.r, SCALE, SCALE);

    SDL_Surface* s = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, sizeof(uint32_t), 0, 0, 0, 0);
    if(s == nullptr) return -1;

    gComponents.t = SDL_CreateTextureFromSurface(gComponents.r, s);
    if(gComponents.t == nullptr) return -1;

    SDL_FreeSurface(s);

    return 0;
}

void SDL_Destroy()
{
    SDL_CloseAudioDevice(gAudioDevice);

    SDL_DestroyTexture(gComponents.t);
    SDL_DestroyRenderer(gComponents.r);
    SDL_DestroyWindow(gComponents.w);
    SDL_Quit();
}

const uint32_t SDL_BLACK = 0x00000000;
const uint32_t SDL_WHITE = 0x00FFFFFF;

void* RenderThread(void* buffer)
{
    SDL_Event e;
    uint32_t* p = (uint32_t*)buffer;

    int32_t j = 0;
    int32_t i = (gScreen.height*gScreen.width);
    while(i--)
    {
        p[j] = gScreen.image[j] == 0.0f ? SDL_BLACK: SDL_WHITE;
        j++;
    }

    SDL_UpdateTexture(gComponents.t, nullptr, p, WIDTH*sizeof(uint32_t));
    SDL_RenderClear(gComponents.r);
    SDL_RenderCopy(gComponents.r, gComponents.t, nullptr, nullptr);
    SDL_RenderPresent(gComponents.r);
    while(SDL_PollEvent(&e) != 0)
    {
        if(e.type == SDL_QUIT)
        {
            SDL_Destroy();
            gKeepGoing = false;
            return nullptr;
        }
        else if(e.type == SDL_KEYDOWN)
        {
            switch(e.key.keysym.sym)
            {
                case SDLK_UP:
                    gButtonState.upButton = true;
                    break;
                case SDLK_LEFT:
                    gButtonState.leftButton = true;
                    break;
                case SDLK_DOWN:
                    gButtonState.downButton = true;
                    break;
                case SDLK_RIGHT:
                    gButtonState.rightButton = true;
                    break;
                case SDLK_a:
                    gButtonState.buttonA = true;
                    break;
                case SDLK_b:
                    gButtonState.buttonB = true;
                    break;
            }
        }
    }
}

int main()
{
    arduboy.clear();
    if(SDL_Init() < 0) return -1;
    uint32_t texture[WIDTH*HEIGHT];

    setup();
    while(gKeepGoing)
    {
        loop();
        RenderThread(texture);
    }
}
