#pragma once

#include "Sprites.h"
#include "Arduboy2Audio.h"

struct Rect
{
  int16_t x;
  int16_t y;
  uint8_t width;
  uint8_t height;
};

class Arduboy2Base
{
public:
    void begin();
    void setFrameRate(uint8_t rate);
    void initRandomSeed();
    bool everyXFrames(uint8_t frames);
    bool justPressed(uint8_t button);
    bool collide(Rect rect1, Rect rect2);
    bool nextFrame();
    void pollButtons();
    void clear();
    void display();

    Arduboy2Audio audio;
};
