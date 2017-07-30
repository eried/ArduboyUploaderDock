const int screenSizeGames = 13;
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
  checkBackButton();

  if (arduboy.justReleased(DOWN_BUTTON))
  {
    //tunes.tone(1100, 10);
    repoSelectedGame++;
  }

  if (arduboy.justReleased(UP_BUTTON))
  {
    //tunes.tone(1100, 10);
    repoSelectedGame--;
  }

  if (arduboy.justReleased(LEFT_BUTTON))
  {
    //tunes.tone(700, 30);
    repoSelectedGame -= screenSizeGames - 1;
  }

  if (arduboy.justReleased(RIGHT_BUTTON))
  {
    //tunes.tone(700, 30);
    repoSelectedGame += screenSizeGames - 1;
  }

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
    tinyfont.setCursor(0, 5 * i);
    tinyfont.print(repoSelectedGame == gameNumber ? F(">") : F(" "));
    tinyfont.print(games[i]);
    tinyfont.setCursor(WIDTH - 4, 5 * i);
    tinyfont.print(repoSelectedGame == gameNumber ? F("<") : F(" "));

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
