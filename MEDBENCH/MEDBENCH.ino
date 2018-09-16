#include "bitmaps.h"
#include "Arduboy2.h"
#include "ArduboyTones.h"

Sprites sprites;
Arduboy2Base arduboy;
ArduboyTones sound(arduboy.audio.enabled);

struct frameInfo
{
    uint16_t* count;
    void (*method)(uint16_t*); 

    frameInfo(uint16_t value, void (*function)(uint16_t*))
    {
        count = new uint16_t(value);
        method = function;
    }
};

uint8_t gCurrentFrame = 0;
frameInfo* gFrames = (frameInfo*)malloc(sizeof(frameInfo)*256);
void drawImage(const unsigned char* image, uint16_t* count)
{
    if (!(arduboy.nextFrame())) return;
    if((*count)--)
    {   
        sprites.drawSelfMasked(0, 0, image, 0);
        arduboy.display();
    }
    else
    {
        arduboy.clear();
        gCurrentFrame++;
    }
}

void nop(uint16_t* count)
{
    if (!(arduboy.nextFrame())) return;
    if((*count)--)
    {
    }
    else
    {
        gCurrentFrame++;
    }
}

void frame0(uint16_t* count)
{
    drawImage(Logo, count);   
}

void frame1(uint16_t* count)
{
    drawImage(Heart, count); 
}

uint8_t gCounter = 0;
uint8_t gVariable = 0;
uint16_t gAccumulator = 0;

void frame2(uint16_t* count)
{
    const uint8_t bpmX = 94;
    const uint8_t bpmY = 32;

    nop(count);
    
    gVariable = random(0, 9);
    gAccumulator += gVariable;
    gCounter++;

    arduboy.clear();

    sprites.drawSelfMasked(bpmX, bpmY, Numbers, 8);
    sprites.drawSelfMasked(bpmX + 5, bpmY, Numbers, gVariable);
    sprites.drawSelfMasked(bpmX-20, bpmY, BPM, 0);     
}

void frame3(uint16_t* count)
{
    const uint8_t bpmX = 94;
    const uint8_t bpmY = 32;

    drawImage(Heart, count);
    gVariable = random(0, 9);
    gAccumulator += gVariable;
    gCounter++;
}

void frame4(uint16_t* count)
{
    nop(count);
    if(*count != 0)
    {
        *gFrames[2].count = 20;
        *gFrames[3].count = 20;
        gCurrentFrame = 2;
    }
    else
    {
        gCurrentFrame = 5;
    }
}

void setup ()
{
    arduboy.begin();
    arduboy.setFrameRate(60);
    arduboy.initRandomSeed();

    frameInfo info0(180, &frame0);
    frameInfo info1(180, &frame1);
    frameInfo info2(20, &frame2);
    frameInfo info3(20, &frame3);
    frameInfo info4(20, &frame4);
    gFrames[0] = info0;
    gFrames[1] = info1;
    gFrames[2] = info2;
    gFrames[3] = info3;
    gFrames[4] = info4;
}

#define TOTAL_FRAMES 5

void loop()
{
    if(gCurrentFrame < TOTAL_FRAMES)
    {
        void (*frame)(uint16_t*) = (void (*)(uint16_t*))gFrames[gCurrentFrame].method;
        (*frame)(gFrames[gCurrentFrame].count);
    }
}