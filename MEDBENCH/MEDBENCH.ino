#include "bitmaps.h"
#include "Arduboy2.h"
#include "ArduboyTones.h"

Sprites sprites;
Arduboy2Base arduboy;
ArduboyTones sound(arduboy.audio.enabled);

void setup ()
{
    arduboy.begin();
    arduboy.setFrameRate(60);
    arduboy.initRandomSeed();
}

uint8_t gCounter = 0;
uint8_t gVariable = 0;
uint16_t gAccumulator = 0;
int32_t gFrameCounter = 0;
void loop()
{
    const uint8_t bpmX = 94;
    const uint8_t bpmY = 32;

    if (!(arduboy.nextFrame())) return;

    gFrameCounter++;
    arduboy.pollButtons();
    arduboy.clear();

    if(gFrameCounter < 180)
    {
        sprites.drawSelfMasked(0, 0, Logo, 0);      
    }
    else if(gFrameCounter < 360) //TODO button input
    {
        sprites.drawSelfMasked(0, 0, Heart, 0);      
    }    
    else if(gFrameCounter%60 && gFrameCounter < 960)
    {
        if(gFrameCounter%60 < 20)
        {
            sprites.drawSelfMasked(0, 0, Heart, 0);
            gVariable = random(0, 9);
            gAccumulator += gVariable;
            gCounter++;
        }
    }
    else if(gFrameCounter > 960) //TODO play sound
    {
        gVariable = gAccumulator / gCounter;
        sprites.drawSelfMasked(0, 0, Heart, 0);  
    }      
    else
    {
        sprites.drawSelfMasked(0, 0, Heart, 0);   
    }

    if(gFrameCounter > 360)
    {
        sprites.drawSelfMasked(bpmX, bpmY, Numbers, 8);
        sprites.drawSelfMasked(bpmX + 5, bpmY, Numbers, gVariable);
        sprites.drawSelfMasked(bpmX-20, bpmY, BPM, 0); 
    }

    arduboy.display();
}
