#include <Arduboy2.h>
#include <TimeLib.h>
#include "sprites.h"
#include "string.h"
Sprites sprites;
Arduboy2 arduboy;

unsigned long nextEvent = 0, nextPing = 0, lastReceivedPing = 0;
char selectedItem = 0;
const byte maximumItems = 5;

const byte MENU = 0, WAITING = 1, SHUTDOWN = 99, FAIL = 98, TRANSFER = 10,
           REPO = 30, REPOINIT = 31, REPOINITBUFFER = 32,
           CLOCK = 20, CLOCKINIT = 21;
byte currentMode = MENU;
int currentErrorAnimationFrame = 0, currentTransferAnimationFrame = 0, repoTotalGames = -1;
bool freshBoot = true;

void setup()
{
  arduboy.boot();
  arduboy.setFrameRate(60);
  //arduboy.setTextWrap(true);

  Serial.begin(115200);

  // Disable TX & RX leds
  DDRD &= ~(1 << DDD5);
  DDRB &= ~(1 << DDB0);
}
