void SwitchToTransfer()
{
  currentTransferAnimationFrame = 0;
  currentMode = TRANSFER;
  nextEvent = millis() + 10000;
}

void doMenu()
{
  arduboy.pollButtons();
  arduboy.println("DOCK MENU V07");

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
        currentMode = REPOINIT;
        break;

      case 1:
        currentMode = CLOCKINIT;
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
