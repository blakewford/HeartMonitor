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
void drawImage(const unsigned char* image, uint16_t* count, uint8_t x, uint8_t y, bool autoClear, uint8_t frame)
{
    if (!(arduboy.nextFrame())) return;
    if((*count)--)
    {   
        sprites.drawSelfMasked(x, y, image, frame);
        arduboy.display();
    }
    else
    {
        if(autoClear) arduboy.clear();
        gCurrentFrame++;
    }
}

void nop(uint16_t* count)
{
    if((*count)--)
    {
    }
    else
    {
        gCurrentFrame++;
    }  
}

void waitFrame(uint16_t* count)
{
    if (!(arduboy.nextFrame())) return;  
    nop(count);
}

void randomizeVariable(uint8_t* variable, long small, long big)
{
    *variable = random(small, big);
}

void addToVariable(uint8_t* variable, uint8_t value)
{
    *variable += value; 
}

void addToVariable(uint16_t* variable, uint8_t value)
{
    *variable += value;     
}

#define TOTAL_FRAMES 13

void loop()
{
    if(gCurrentFrame < TOTAL_FRAMES)
    {
        void (*frame)(uint16_t*) = (void (*)(uint16_t*))gFrames[gCurrentFrame].method;
        (*frame)(gFrames[gCurrentFrame].count);
    }
}

//---------------------- Program Start ------------------------------//

uint8_t gCounter = 0;
uint8_t gVariable = 0;
uint16_t gAccumulator = 0;

void frame0(uint16_t* count)
{
    drawImage(Logo, count, 0, 0, true, 0);   
}

void frame1(uint16_t* count)
{
    drawImage(Heart, count, 0, 0, true, 0); 
}

void frame2(uint16_t* count)
{
    randomizeVariable(&gVariable, 0, 9);
    nop(count);
}

void frame3(uint16_t* count)
{
    addToVariable(&gAccumulator, gVariable);
    nop(count);
}

void frame4(uint16_t* count)
{
    addToVariable(&gCounter, 1);
    nop(count);    
}

void frame5(uint16_t* count)
{ 
    const uint8_t bpmX = 94;
    const uint8_t bpmY = 32;

    drawImage(BPM, count, bpmX - 20, bpmY, false, 0);
}

void frame6(uint16_t* count)
{ 
    const uint8_t bpmX = 94;
    const uint8_t bpmY = 32;

    drawImage(Numbers, count, bpmX, bpmY, false, 8);
}

void frame7(uint16_t* count)
{ 
    const uint8_t bpmX = 94;
    const uint8_t bpmY = 32;

    drawImage(Numbers, count, bpmX + 5, bpmY, false, gVariable);    
}

void frame8(uint16_t* count)
{
    drawImage(Heart, count, 0, 0, true, 0);
}

void frame9(uint16_t* count)
{
    randomizeVariable(&gVariable, 0, 9);
    nop(count);
}

void frame10(uint16_t* count)
{
    addToVariable(&gAccumulator, gVariable);
    nop(count);
}

void frame11(uint16_t* count)
{
    addToVariable(&gCounter, 1);   
    nop(count);    
}

void frame12(uint16_t* count)
{
    nop(count);
    if(*count != 0)
    {
        *gFrames[2].count = 0;  
        *gFrames[3].count = 0; 
        *gFrames[4].count = 0;               
        *gFrames[5].count = 1;
        *gFrames[6].count = 1;        
        *gFrames[7].count = 20;        
        *gFrames[8].count = 20;
        *gFrames[9].count = 0;  
        *gFrames[10].count = 0; 
        *gFrames[11].count = 0;         
        gCurrentFrame = 2;
    }
    else
    {
        gCurrentFrame = 13;
    }
}

void setup ()
{
    arduboy.begin();
    arduboy.setFrameRate(60);
    arduboy.initRandomSeed();

    frameInfo info0(180, &frame0);
    frameInfo info1(180, &frame1);
    frameInfo info2(0, &frame2);
    frameInfo info3(0, &frame3); 
    frameInfo info4(0, &frame4);        
    frameInfo info5(1, &frame5);
    frameInfo info6(1, &frame6);    
    frameInfo info7(20, &frame7);    
    frameInfo info8(20, &frame8);
    frameInfo info9(0, &frame9);
    frameInfo info10(0, &frame10); 
    frameInfo info11(0, &frame11); 
    frameInfo info12(20, &frame12);
    gFrames[0] = info0;
    gFrames[1] = info1;
    gFrames[2] = info2;    
    gFrames[3] = info3;
    gFrames[4] = info4;
    gFrames[5] = info5;
    gFrames[6] = info6;
    gFrames[7] = info7;    
    gFrames[8] = info8;
    gFrames[9] = info9;     
    gFrames[10] = info10;  
    gFrames[11] = info11; 
    gFrames[12] = info12;                
}
