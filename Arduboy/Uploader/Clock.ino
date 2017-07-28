const byte paddlew = 20, paddleh = 3, paddley = HEIGHT - paddleh;
double ballx = 0, bally = 15, paddlex = (WIDTH - paddlew) / 2;
double ballspeedx = -1.35, ballspeedy = -1.1;
const byte ballsize = 3;
void doClock()
{
  // Clock does not need to be synced that often
  lastReceivedPing = millis();

  // Draw paddle
  arduboy.fillRect(paddlex, paddley, paddlew, paddleh);

  paddlex -= (paddlex + (paddlew / 2) - ballx) / ((1 + HEIGHT - bally) / 2);

  // Draw time
  arduboy.setTextSize(3);
  arduboy.setCursor((WIDTH - ((hour() < 10 ? 4 : 5) * 18)) / 2, 10);
  arduboy.print(hour());
  arduboy.print(":");
  arduboy.print(minute() < 10 ? "0" : "");
  arduboy.print(minute());

  if (ballx < 0 || ballx > WIDTH || (ballx > 2 && ballx < WIDTH - 2 && ( arduboy.getPixel(ballx, bally + 1) || arduboy.getPixel(ballx + 3, bally + 1))))
    ballspeedx *= -1;

  if (bally < 0 || bally > HEIGHT || (bally > 2 && bally < HEIGHT - paddleh - 1 && ( arduboy.getPixel(ballx + 2, bally) || arduboy.getPixel(ballx + 1, bally + 3))))
    ballspeedy *= -1;

  // Draw ball
  arduboy.fillRect(ballx, bally, ballsize, ballsize);
  ballx += ballspeedx;
  bally += ballspeedy;
}
