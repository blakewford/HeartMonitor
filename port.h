#include <stdint.h>
#include <string.h>

typedef uint8_t byte;
typedef bool boolean;

const int16_t HEIGHT = 64;
const int16_t WIDTH  = 128;
const int32_t F_CPU = 16000000;

#define B00000000  0x0
#define B00000001  0x1
#define B00000010  0x2
#define B00000100  0x4
#define B00001000  0x8
#define B00001010  0xA
#define B00100000 0x20

void delay(uint32_t ms);
long random(long howsmall, long howbig);

enum
{
    LEFT_BUTTON,
    RIGHT_BUTTON,
    UP_BUTTON,
    DOWN_BUTTON,
    A_BUTTON,
    B_BUTTON,
};

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

char* ltoa(long l, char * buffer, int radix);

#define pgm_read_word
