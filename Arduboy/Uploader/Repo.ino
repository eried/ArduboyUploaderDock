const int totalGameNamesToBuffer = 20, screenSizeGames = 7;
String games[totalGameNamesToBuffer];

void clearGameNames()
{
  for (int i = 0; i < totalGameNamesToBuffer; i++)
  {
    games[i].reserve(20);
    games[i] = "...";
  }
}

void doRepo()
{
  arduboy.pollButtons();

  if (arduboy.justPressed(B_BUTTON))
    currentMode = MENU;

  if (arduboy.justReleased(DOWN_BUTTON))
    repoSelectedGame++;

  if (arduboy.justReleased(UP_BUTTON))
    repoSelectedGame--;

  repoSelectedGame = repoSelectedGame < 0 ? repoTotalGames - 1 : (repoSelectedGame >= repoTotalGames ? 0 : repoSelectedGame);

  if (arduboy.justPressed(A_BUTTON))
  {
    Serial.print("<REPOSEND:");
    Serial.print(repoSelectedGame);
    Serial.print(">");
    SwitchToTransfer();
  }

  arduboy.print("Repo games:");
  arduboy.println(repoTotalGames);

  for (int i = 0; i < screenSizeGames; i++)
  {
    arduboy.print(repoSelectedGame == i ? F("->") : F("  "));
    arduboy.println(games[i]);
  }

  if (repoLoaded < min(screenSizeGames,repoTotalGames) )
  {
    String tmp;
    tmp.reserve(15);
    tmp = "<REPONAME:";
    tmp += repoLoaded;
    tmp += ">";
    if (getDockString(tmp, &games[repoLoaded]))
      repoLoaded++;
  }
}
