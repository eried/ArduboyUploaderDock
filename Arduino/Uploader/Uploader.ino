#include <Arduboy2.h>
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

