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

int32_t gFrameCounter = 0;
void loop()
{
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
        if(gFrameCounter%60 < 20) sprites.drawSelfMasked(0, 0, Heart, 0);
    }
    else if(gFrameCounter > 960) //TODO play sound
    {
        sprites.drawSelfMasked(0, 0, Heart, 0);   
    }      
    else
    {
        sprites.drawSelfMasked(0, 0, Heart, 0);   
    }

    arduboy.display();
}
