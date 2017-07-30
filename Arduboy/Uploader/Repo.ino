const int screenSizeGames = 8;
int currentGameToBuffer = -1, screenOffset = 0;
String games[screenSizeGames];
int loaded[screenSizeGames];
String command;

void clearGameNames()
{
  command.reserve(14);

  for (int i = 0; i < screenSizeGames; i++)
  {
    loaded[i] = -1;
    games[i].reserve(20);
    games[i] = "...";
  }
}

void doRepo()
{
  if (arduboy.justPressed(B_BUTTON))
    currentMode = MENU;

  if (arduboy.justReleased(DOWN_BUTTON))
    repoSelectedGame++;

  if (arduboy.justReleased(UP_BUTTON))
    repoSelectedGame--;

  int oldSelectedGame = repoSelectedGame;
  repoSelectedGame = repoSelectedGame < 0 ? repoTotalGames - 1 : (repoSelectedGame >= repoTotalGames ? 0 : repoSelectedGame);
  //screenOffset = max(0, repoSelectedGame + 1 - screenSizeGames);

  if (repoSelectedGame >= screenOffset + screenSizeGames)
    screenOffset = repoSelectedGame + 1 - screenSizeGames;

  if (repoSelectedGame < screenOffset)
    screenOffset = repoSelectedGame;

  //screenOffset = max(0,screenOffset);

  if (oldSelectedGame != repoSelectedGame)
    if (abs(oldSelectedGame - repoSelectedGame) > 1)
    {
      for (int i = 0; i < screenSizeGames; i++)
        loaded[i] = -1;
    }
    else
    {
      if (oldSelectedGame > repoSelectedGame)
      {
        // Optimize, offset the list of games
      }
    }

  int minimum = min(screenSizeGames, repoTotalGames);

  if (arduboy.justPressed(A_BUTTON))
  {
    Serial.print("<REPOSEND:");
    Serial.print(repoSelectedGame);
    Serial.print(">");
    SwitchToTransfer();
  }

  // Draw the game names
  int c = 0;
  for (int i = minimum - 1; i >= 0; i--)
  {
    int gameNumber = (i + screenOffset);
    arduboy.setCursor(0, 8 * i);
    arduboy.print(repoSelectedGame == gameNumber ? F("->") : F("  "));
    arduboy.println(games[i]);

    if (loaded[i] != gameNumber)
      c = i;
  }

  //currentGameToBuffer = -1;

  do
  {
    int gameNumber = (c + screenOffset);
    if (currentGameToBuffer == -1)
    {
      if (loaded[c] != gameNumber)
      {
        /*Serial.print("<REQUESTING");
          Serial.print(c);
          Serial.print("WITH");
          Serial.print(gameNumber);
          Serial.print(">");*/
        if (currentGameToBuffer != gameNumber)
        {
          command = "<REPONAME:";
          command += gameNumber;
          command += ">";
          currentGameToBuffer = gameNumber;
        }
      }
      else
        c++;
    }
    else
    {
      if (getDockString(command, &games[c]))
      {
        loaded[c] = gameNumber;
        currentGameToBuffer = -1;
      }
    }

    if (c >= minimum)
      break;
  }
  while (!arduboy.nextFrame());
}
