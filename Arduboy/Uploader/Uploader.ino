#include <Arduboy2.h>
#include "sprites.h"
Sprites sprites;
Arduboy2 arduboy;

String buffer = "";
long nextEvent = 0, nextPing = 0, lastReceivedPing = 0;
char selectedItem = 0;
const byte maximumItems = 5;
String menuItems[] = { "Browse collection", "Clock", "Dock settings", "Update dock app", /*"Send game", "Send ANOTHER game",*/ "Charge mode" };

const byte MENU = 0, WAITING = 1, SHUTDOWN = 99, FAIL = 98, TRANSFER = 10, REPO = 11, CLOCK = 12;
byte currentMode = CLOCK;
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
  arduboy.setFrameRate(60);
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

      case CLOCK:
        doClock();
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

const byte paddlew = 20, paddleh = 3, paddley = HEIGHT - paddleh;
double ballx = 4, bally = 4;
double ballspeedx = -0.4, ballspeedy = -0.4;
const byte ballsize = 3;
void doClock()
{
  // Clock does not need to be synced
  lastReceivedPing = millis();

  // Draw paddle
  byte x = (WIDTH - paddlew) / 2;
  arduboy.fillRect(x, paddley, paddlew, paddleh);

  // Draw time
  arduboy.setTextSize(3);
  arduboy.setCursor((WIDTH - (5 * 18)) / 2, 0);
  arduboy.print("12:00");

  // Draw ball
  arduboy.fillCircle(ballx, bally, ballsize, ballsize);
  ballx += ballspeedx;
  bally += ballspeedy;

  if (ballx < 0 || ballx > WIDTH)
    ballspeedx *= -1;
  else if (ballx > 0)
    ballspeedx *= arduboy.getPixel(ballx + ballsize / 2, bally) == WHITE ? -1 : 1;
  else
    ballspeedx *= arduboy.getPixel(ballx + ballsize / 2, bally + ballsize) == WHITE ? -1 : 1;

  if (bally < 0 || bally > HEIGHT)
    ballspeedy *= -1;
  else if (bally > 0)
    ballspeedy *= arduboy.getPixel(ballx, bally + ballsize / 2) == WHITE ? -1 : 1;
  else
    ballspeedy *= arduboy.getPixel(ballx + ballsize / 2, bally + ballsize / 2) == WHITE ? -1 : 1;

  // Collisions with text

}

