#include <Arduboy2.h>
#include "sprites.h"
Sprites sprites;
Arduboy2 arduboy;

String buffer = "";
long nextEvent = 0, nextPing = 0, lastReceivedPing = 0;
char selectedItem = 0;
const byte maximumItems = 5;
String menuItems[] = { "Browse collection", "Clock", "Dock settings", "Update dock app", /*"Send game", "Send ANOTHER game",*/ "Charge mode" };

const byte MENU = 0, WAITING = 1, SHUTDOWN = 99, FAIL = 98, TRANSFER = 10, REPO = 11;
byte currentMode = MENU;
int error_frame = 0, transfer_frame = 0;

void SwitchToTransfer()
{
  transfer_frame = 0;
  currentMode = TRANSFER;
  nextEvent = millis() + 10000;
}

void setup()
{
  arduboy.boot();
  arduboy.setFrameRate(30);
  arduboy.setTextWrap(true);

  Serial.begin(115200);

  // Disable TX & RX leds
  DDRD &= ~(1 << DDD5);
  DDRB &= ~(1 << DDB0);
}

int repoTotalGames = -1, repoSelectedGame = -1;


void doRepo()
{
  if(repoTotalGames == -1)
  {
    // Do initialization of the list
  }
}

void doMenu()
{
  arduboy.pollButtons();
  arduboy.println("DOCK MENU V03");

  if (arduboy.justReleased(DOWN_BUTTON))
    selectedItem++;

  if (arduboy.justReleased(UP_BUTTON))
    selectedItem--;

  selectedItem = selectedItem < 0 ? maximumItems : (selectedItem >= maximumItems ? 0 : selectedItem);

  for (byte i = 0; i < maximumItems; i++)
  {
    arduboy.print(selectedItem == i ? "->" : "  ");
    arduboy.println(menuItems[i]);
  }
  if (arduboy.justReleased(A_BUTTON))
  {
    switch (selectedItem)
    {
      case 0:
        //Serial.println("REPO:0"); // Get page 0
        /*arduboy.clear();
        arduboy.println("->1010");
        arduboy.println("  2048");
        arduboy.println("  ABAsm DP1");
        arduboy.println("  Abshell");
        arduboy.println("  abSynth FM");
        arduboy.println("  APara");
        arduboy.println("  Arcodia");
        arduboy.display();

        while (arduboy.notPressed(DOWN_BUTTON))ping();

        arduboy.clear();
        arduboy.println("  1010");
        arduboy.println("->2048");
        arduboy.println("  ABAsm DP1");
        arduboy.println("  Abshell");
        arduboy.println("  abSynth FM");
        arduboy.println("  APara");
        arduboy.println("  Arcodia");
        arduboy.display();

        while (arduboy.notPressed(A_BUTTON))ping();

        Serial.println("SEND:Puzzle.hex");
        SwitchToTransfer();*/
        currentMode = REPO;

        break;

      case 1:
        Serial.print("<TIME>");
        //startclock();
        break;

      case 2:
        Serial.print("<ABOUT>"); // TEMPORAL
        break;

      case 3:
        Serial.print("<UPDATE>");
        SwitchToTransfer();
        break;

      case 4:
        /*      Serial.println("SEND:game.hex");
              SwitchToTransfer();
              break;

            case 5:
              Serial.println("SEND:game2.hex");
              SwitchToTransfer();
              break;

            case 6:*/
        Serial.print("<SHUTDOWN>");
        nextEvent = millis() + 3000;
        currentMode = SHUTDOWN;
        break;
    }
  }
}

void ping()
{
  if (millis() > nextPing)
  {
    Serial.print("<PING>");
    nextPing = millis() + 1000;
  }
}

void loop()
{
  if (Serial.available())
    readSerial();

  if (!(arduboy.nextFrame()))
    return;

  arduboy.clear();

  if (millis() - lastReceivedPing > 2000)
  {
    /*arduboy.println("Please, put me in my");
      arduboy.println("dock...");*/

    sprites.drawSelfMasked(0, 0, error, error_frame);
    if (arduboy.everyXFrames(30)) error_frame++;
    if (error_frame > 2) error_frame = 0;

  }
  else
  {
    ping();

    switch (currentMode)
    {
      case SHUTDOWN:
        arduboy.println("Dock is turning off");

        if (nextEvent < millis())
          currentMode = WAITING;
        break;

      case WAITING:
        arduboy.println("Waiting for dock");
        break;

      case FAIL:
        arduboy.println("Error.");

        if (nextEvent < millis())
          currentMode = MENU;
        break;


      case TRANSFER:
        sprites.drawSelfMasked(0, 0, transfer, transfer_frame);
        if (arduboy.everyXFrames(3)) transfer_frame++;
        if (transfer_frame > 4) transfer_frame = 0;

        if (nextEvent < millis())
        {
          currentMode = FAIL;
          nextEvent = millis() + 2000;
        }
        break;

      case MENU:
        doMenu();
        break;
    }
  }
  arduboy.display();
}

void readSerial() {

  String r;

  while (Serial.available())
  {
    ping();
    r += (char)Serial.read();
  }

  if (r.startsWith("<PING>"))
  {
    lastReceivedPing = millis();
    return;
  }

  arduboy.clear();
  arduboy.println(r);
  arduboy.display();

  for (int i = 0; i < 10; i++)
  {
    ping();
    delay(200);
  }
}



/*#include <Arduboy2.h>
  Arduboy2 arduboy;

  String buffer = "";
  long nextEvent = 0, nextPing = 0;
  char selectedItem = 0;
  const byte maximumItems = 7;
  String menuItems[] = { "Browse collection", "Clock", "Dock settings", "Update dock app", "Send game", "Send ANOTHER game", "Charge mode" };

  const byte MENU = 0, WAITING = 1, SHUTDOWN = 99;
  byte currentMode = MENU;

  void setup()
  {
  arduboy.boot();
  arduboy.setFrameRate(30);
  arduboy.setTextWrap(true);

  Serial.begin(115200);

  // Disable TX & RX leds
  DDRD &= ~(1 << DDD5);
  DDRB &= ~(1 << DDB0);
  }
  void doMenu()
  {
  arduboy.pollButtons();
  arduboy.println("DOCK MENU 3");

  if (arduboy.justReleased(DOWN_BUTTON))
    selectedItem++;

  if (arduboy.justReleased(UP_BUTTON))
    selectedItem--;

  selectedItem = selectedItem < 0 ? maximumItems : (selectedItem >= maximumItems ? 0 : selectedItem);

  for (byte i = 0; i < maximumItems; i++)
  {
    arduboy.print(selectedItem == i ? "->" : "  ");
    arduboy.println(menuItems[i]);
  }
  if (arduboy.justReleased(A_BUTTON))
  {
    switch (selectedItem)
    {
      case 0:
        Serial.println("REPO:1:6");
        break;

      case 1:
        Serial.println("TIME");
        //startclock();
        break;

      case 2:
        Serial.println("ABOUT"); // TEMPORAL
        break;

      case 3:
        Serial.println("UPDATE");
        delay(1000);
        break;

      case 4:
        Serial.println("SEND:game.hex");
        delay(1000);
        break;

      case 5:
        Serial.println("SEND:game2.hex");
        delay(1000);
        break;

      case 6:
        Serial.println("SHUTDOWN");
        nextEvent = millis() + 3000;
        currentMode = SHUTDOWN;
        break;
    }
  }
  }

  void ping()
  {
  if (millis() > nextPing)
  {
    Serial.println("PING");
    nextPing = millis() + 1000;
  }
  }

  void loop()
  {
  ping();

  if (Serial.available())
    readSerial();

  if (!(arduboy.nextFrame()))
    return;

  arduboy.clear();

  switch (currentMode)
  {
    case SHUTDOWN:
      arduboy.println("Dock is turning off");

      if (nextEvent < millis())
        currentMode = WAITING;
      break;

    case WAITING:
      arduboy.println("Waiting for dock");
      break;

    case MENU:
      doMenu();
      break;
  }

  arduboy.display();
  }

  void readSerial() {

  String r;

  while (Serial.available())
  {
    ping();
    r += (char)Serial.read();
  }

  if (r.startsWith("PING"))
    return;

  arduboy.clear();
  arduboy.println(r);
  arduboy.display();

  for (int i = 0; i < 10; i++)
  {
    ping();
    delay(200);
  }
  }
*/
