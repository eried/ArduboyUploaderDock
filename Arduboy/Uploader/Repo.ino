int repoSelectedGame = -1;
const int totalGameNamesToBuffer = 20;
String games[totalGameNamesToBuffer];

void clearGameNames()
{
  for (int i = 0; i < totalGameNamesToBuffer; i++)
    games[i] = "...";
}

void doRepo()
{
  arduboy.print("Repo games:");
  arduboy.println(repoTotalGames);

  for (int i = 0; i < 7; i++)
    arduboy.println(games[i]);
}
