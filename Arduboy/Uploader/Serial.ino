bool pendingAnswerFromDock = false;
String received = "";
bool getDockInt(String command, long *output)
{
  if (!pendingAnswerFromDock)

  {
    Serial.print(command);
    pendingAnswerFromDock = true;
  }
  else
  {
    if (received.length())
    {
      pendingAnswerFromDock = false;
      *output = received.toInt();
      received = "";
      return true;
    }
  }
  return false;
}

void ping()
{
  if (millis() > nextPing)
  {
    Serial.print("<PING>");
    nextPing = millis() + 250;
  }
}

String serialBuffer = "";
void readSerial()
{
  int receivedBytes = min(64, Serial.available());
  if (receivedBytes)
  {
    for (; receivedBytes > 0; receivedBytes--)
    {
      char c = Serial.read();

      switch (c)
      {
        case '<':
          serialBuffer = "";
          break;

        case '>':
          if (serialBuffer == "PING")
          {
            freshBoot = false;
            lastReceivedPing = millis();
          }
          else
          {
            arduboy.print(received);
            arduboy.display();
            delay(600);
            received = serialBuffer;
          }
          return;

        default:
          serialBuffer += c;
          // TODO: Check for overflow!
          break;
      }
    }
  }
}
