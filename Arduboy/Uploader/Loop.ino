void loop()
{
  if (Serial.available())
    readSerial();

  if (!(arduboy.nextFrame()))
    return;

  arduboy.clear();

  if (millis() - lastReceivedPing > (freshBoot ? 2000 : 1000))
  {
    sprites.drawSelfMasked(0, 0, error, currentErrorAnimationFrame);
    if (arduboy.everyXFrames(30)) currentErrorAnimationFrame++;
    if (currentErrorAnimationFrame > 2) currentErrorAnimationFrame = 0;

  }
  else
  {
    ping();
    long tmp;

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

      case REPOINIT:
        if (getDockInt("<REPOSIZE>", &tmp))
        {
          arduboy.println(tmp);
          arduboy.display();
          currentMode = REPO;
        }
        break;

      case CLOCKINIT:
        if (getDockInt("<TIME>", &tmp))
        {
          setTime(tmp);
          currentMode = CLOCK;
        }
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
        sprites.drawSelfMasked(0, 0, transfer, currentTransferAnimationFrame);
        if (arduboy.everyXFrames(3)) currentTransferAnimationFrame++;
        if (currentTransferAnimationFrame > 4) currentTransferAnimationFrame = 0;

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

