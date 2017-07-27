#include <Arduboy2.h>
#include "sprites.h"
Sprites sprites;
Arduboy2 arduboy;

String buffer = "";
unsigned long nextEvent = 0, nextPing = 0, lastReceivedPing = 0;
char selectedItem = 0;
const byte maximumItems = 5;
String menuItems[] = { "Browse collection", "Clock", "Dock settings", "Update dock app", /*"Send game", "Send ANOTHER game",*/ "Charge mode" };

const byte MENU = 0, WAITING = 1, SHUTDOWN = 99, FAIL = 98, TRANSFER = 10, REPO = 11;
byte currentMode = MENU;
int error_frame = 0, transfer_frame = 0;
bool booting = true;

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
  if (repoTotalGames == -1)
  {
    // Do initialization of the list
  }
}

bool pendingAnswerFromDock = false;
String received = "";
bool getDockInt(String command, int *output)
{
  if (!pendingAnswerFromDock)

  {
    Serial.print(command);
    pendingAnswerFromDock = true;
  }
  else
  {
    if (received)
    {
      pendingAnswerFromDock = false;
      *output = received.toInt();
      return true;
    }
  }
  return false;
}

void doMenu()
{
  arduboy.pollButtons();
  arduboy.println("DOCK MENU V06");

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
        int r;
        if (getDockInt("<REPOSIZE>", &r))
        {
          arduboy.println(r);
          arduboy.display();
          delay(600);
        }
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
    nextPing = millis() + 250;
  }
}

void loop()
{
  if (Serial.available())
    readSerial();

  if (!(arduboy.nextFrame()))
    return;

  arduboy.clear();

  if (millis() - lastReceivedPing > (booting ? 2000 : 1000))
  {
    arduboy.println(millis() - lastReceivedPing);
    arduboy.display();
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

void readSerial()
{
  int rec = min(64, Serial.available());
  if (rec)
  {
    for (; rec > 0; rec--)
    {
      char c = Serial.read();

      switch (c)
      {
        case '<':
          buffer = "";
          break;

        case '>':
          if (buffer == "PING")
          {
            booting = false;
            lastReceivedPing = millis();
          }
          else
            received = buffer;
          return;

        default:
          buffer += c;
          // TODO: Check for overflow!
          break;
      }
    }
  }
}

