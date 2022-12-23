#include <EEPROM.h>
#include <LiquidCrystal.h>
#include "LedControl.h"

// Joystick pins
const int xPin = A0;
const int yPin = A1;
const int pinSW = 0;

// Joystick variables
byte swReading; // variable that holds the joystick button state
unsigned PressTimer = 0;
int PressStart = 0;
bool joyMoved = false;
const int minThreshold = 50;
const int maxThreshold = 950;
const int resetMinThreshold = 450;
const int resetMaxThreshold = 580;

// LED matrix pins
const int dinPin = 12;
const int clockPin = 10;
const int loadPin = 11;

// LED matrix variables
byte matrixBrightness = 5;
const byte matrixSize = 8;
bool matrixChanged = true;
unsigned long trapBlinkTimer = 0;
float trapBlink = 1;
int trapBlinkStage = 1;
bool trapActive = 0;
int simpleStaticTrapTime = 2000;
int complexStaticTrapTime = 3000;
int positionsToDelete[64]; // the led positions of the traps

// LED matrix object
LedControl lc = LedControl(dinPin, loadPin, clockPin, 1);

// LCD screen pins
const byte RS = 9;
const byte enable = 8;
const byte d4 = 7;
const byte d5 = A2;
const byte d6 = A3;
const byte d7 = 4;
const byte backLightPin = 6;
const byte contrastPin = 5;

// LCD screen variables
int brightness = 0;
int contrast = 0;
unsigned long startTimer = 0;
char time = '3';

// LCD screen object
LiquidCrystal lcd(RS, enable, d4, d5, d6, d7);

// Buzzer pin
int buzzerPin = 3;

// Buzzer volume
byte buzzerVolume = 1;

// Volume control pin
int VolumePin = A4;

// Player-Dot variables
byte xPos = 0;
byte yPos = 0;
byte newFoodPosX = 0;
byte newFoodPosY = 0;
byte xLastPos = 0;
byte yLastPos = 0;
bool eaten = 0;
byte moveInterval = 450;
unsigned long lastMoved = 0;
unsigned long blink = 0;
bool blinked = 0;
int score = 0;
int addScore = 10;
long gameTime = 20000;
int addTime = 3000;

// Menu variables
byte xMenu = 0;
byte panel = 0;
byte aboutPanel = 1;
byte helpPanel = 1;
bool clear = 0; // check if the LCD screen has been cleared
byte deleteChoice = 1;
byte difficultyChoice = 1;
unsigned long scrollTimer = 0;

// Highscore variables
unsigned long nameTimer = 0;
int nameLetters[3] = {65, 65, 65}; // the vector the hold the letters for the name as integers
byte leterPosition = 0; // the position of the selected letter
char highscoreName[3];

// State control variables
byte menuState = 0;
byte settingsState = 0;
byte gameState = 0;
byte gameOver = 0;

// saves matrix led states
byte matrix[matrixSize][matrixSize] = {
    {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};

// saves positions that kills the player
byte matrixTrap[matrixSize][matrixSize] = {
    {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};

// labels for the menu
const int menuSize = 5;
const char* menuChoices[] = {
    "START GAME  ", "HIGHSCORES  ",     "DIFFICULTY",
    "SETTINGS",     "RESET HIGH SCORE", "INSTRUCTIONS"};

// labels for the settings sub-menu
const int settingsSize = 3;
const char* settingsChoices[] = {"LCD-CONTRAST",
                                 "LCD-BRIGHTNESS",
                                 "MX-BRIGHTNESS", "VOLUME"};
                                
// saves trap designs
const uint64_t staticTraps[] = {
    0xffffc3c3c3c3ffff, 0x00003c3c3c3c0000,
    0x0f0f0f0f0f0f0f0f, 0xf0f0f0f0f0f0f0f0,
    0x00000000ffffffff, 0xffffffff00000000,
    0x2424ff2424ff2424, 0x0066660000666600,
    0x0066666666666600, 0x007e7e00007e7e00,
    0xc3e766000066e7c3, 0x00fcf8c0031f3f00,
    0x0e0e466666627070, 0xc763210080c6e3f1,
    0xe3c684000163c78f, 0x6666420000246666,
    0xdbdb00dbdb00dbdb, 0xb4b794fe23fe949f,
    0xff115545ff9fc3ff, 0xff81bda5b5b585fd,
    0x99667ea5a57e6699, 0xdbdb10fbdf08dbdb,
    0xdbdb10fbdf08dbdb, 0xffbd81e7ff993ce7};
const int staticTrapNumber = sizeof(staticTraps) / 8;
int staticTrapImage = 1;

// saves images to be displayed on the matrix while scrolling through the menu
const uint64_t menuImages[] = {
    0x3c5a66ffdb99ff7e, 0x3c18183c3c7e7eff,
    0xffc3bdffff5abdff, 0xff81ffffff18ffff,
    0xbdc3ffffdbbd7eff, 0x1800181878627e7e,
    0xffe7ffe7e7e7e7ff, 0x1818181818187e3e,
    0x003c429999423c00, 0x50a4a6a7a7a6a450};
const int menuImagesLen = sizeof(menuImages) / 8;

// a structure that holds the score and a name for highscore display purposes
struct highscore {
  char name[3];
  int score;
};

highscore highscores[6];

// function that writes a string to the EEPROM
void writeStringToEEPROM(int addrOffset, String strToWrite) {
  byte len = strToWrite.length();
  EEPROM.update(addrOffset, len);
  for (int i = 0; i < len; i++) {
    EEPROM.update(addrOffset + 1 + i, strToWrite[i]);
  }
}

// function that reads a string from the EEPROM
String readStringFromEEPROM(int addrOffset) {
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++) {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\ 0';

  return String(data);
}

void setup() {
  lcd.begin(16, 2);

  Serial.begin(9600);

  // activate pins
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(backLightPin, OUTPUT);
  pinMode(contrastPin, OUTPUT);

  // get the values stored in EEPROM
  EEPROM.get(0, brightness);
  EEPROM.get(2, contrast);
  EEPROM.get(4, matrixBrightness);
  EEPROM.get(8, VolumePin);
  EEPROM.get(12, buzzerVolume);

  int j = 0;
  for (int i = 20; i <= 45; i = i + 5) {
    EEPROM.get(i, highscores[j].name);
    EEPROM.get(i + 3, highscores[j].score);
    j++;
  }

  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0, false);  // turn off power saving, enables display
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);  // clear screen
  matrix[xPos][yPos] = 1;

  // sets the default contrast and brightness from the EEPROM
  analogWrite(backLightPin, brightness);
  analogWrite(contrastPin, contrast);

  // welcome screen on the LCD
  lcd.setCursor(0, 0);
  lcd.write("   WELCOME TO   ");
  lcd.setCursor(0, 1);
  lcd.write("  TRAP-DIVERS   ");
  delay(1500);
  lcd.clear();
}

void loop() {

  // set the see for random with millis()
  randomSeed(millis());

  // game states are represented through this if and its sub-branches
  if (gameState == 0) {
    if (menuState == 0) {
      resetScreen();

      displayMenuImage(menuImages[0]);

      navigateMenu();

      // display main menu
      lcd.setCursor(1, 0);
      lcd.write(menuChoices[panel]);
      lcd.setCursor(1, 1);
      lcd.write(menuChoices[panel + 1]);

      lcd.setCursor(0, xMenu % 2);
      lcd.write(">");

    } else if (menuState == 1) {
      const int buzzTone = 500;
      const int buzzToneStart = 600;
      const int buzzTime = 200;
      const int buzzTimeStart = 300;
      resetScreen();

      if (startTimer == 0) {
        startTimer = millis();
      }

      // game start timer
      lcd.setCursor(0, 0);
      lcd.write("GAME STARTS IN");
      lcd.setCursor(15, 0);
      if (millis() - startTimer >= 0 && time == '3') {
        tone(buzzerPin, buzzTone, buzzTime);
        noTone(VolumePin);
        lcd.write(time);
        time = '2';
      }
      if (millis() - startTimer >= 1000 && time == '2') {
        lcd.write(time);
        tone(buzzerPin, buzzTone, buzzTime);
        noTone(VolumePin);
        time = '1';
      }
      if (millis() - startTimer >= 2000 && time == '1') {
        lcd.write(time);
        tone(buzzerPin, buzzTone, buzzTime);
        noTone(VolumePin);
        time = '0';
      }
      if (millis() - startTimer >= 3000 && time == '0') {
        tone(buzzerPin, buzzToneStart, buzzTimeStart);
        noTone(VolumePin);
        gameState = 1;
        startTimer = 0;
        time = '3';
        xPos = 0;
        yPos = 0;
        lcd.clear();
        lc.clearDisplay(0);
      }

    } else if (menuState == 2) {
      resetScreen();

      displayMenuImage(menuImages[1]);

      navigateMenu();

      // displays the highscores (2 highscores on the screen)
      lcd.setCursor(1, 0);
      lcd.write(highscores[panel].name);
      char char_arr[100];
      unsigned long num = highscores[panel].score;
      sprintf(char_arr, "%d", num);
      lcd.setCursor(4, 0);
      lcd.write(": ");
      lcd.setCursor(6, 0);
      lcd.write(char_arr);

      lcd.setCursor(1, 1);
      lcd.write(highscores[panel + 1].name);
      char_arr[100];
      num = highscores[panel + 1].score;
      sprintf(char_arr, "%d", num);
      lcd.setCursor(4, 1);
      lcd.write(": ");
      lcd.setCursor(6, 1);
      lcd.write(char_arr);

      lcd.setCursor(0, xMenu % 2);
      lcd.write(">");

    } else if (menuState == 3) {
      resetScreen();

      // displays difficulties
      lcd.setCursor(0, 0);
      lcd.write("PICK DIFFICULTY");

      if (difficultyChoice == 1) {
        displayMenuImage(menuImages[2]);
        lcd.setCursor(3, 1);
        lcd.write(" > EASY < ");
      } else if (difficultyChoice == 2) {
        displayMenuImage(menuImages[3]);
        lcd.setCursor(3, 1);
        lcd.write("> NORMAL <");
      } else if (difficultyChoice == 3) {
        displayMenuImage(menuImages[4]);
        lcd.setCursor(3, 1);
        lcd.write(" > HARD < ");
      }

      pickDifficulty();
    } else if (menuState == 4) {
      if (settingsState == 0) {
        resetScreen();
        
        // displays settings sub-menu
        displayMenuImage(menuImages[7]);

        navigateMenu();

        lcd.setCursor(1, 0);
        lcd.write(settingsChoices[panel]);
        lcd.setCursor(1, 1);
        lcd.write(settingsChoices[panel + 1]);

        lcd.setCursor(0, xMenu % 2);
        lcd.write(">");

      } else if (settingsState == 1) {
        resetScreen();

        displayMenuImage(menuImages[8]);

        // displays the LCD contrast power
        swReading = digitalRead(pinSW);
        lcd.setCursor(1, 0);
        lcd.write(" LED CONTRAST ");
        int contrastBars = map(contrast, 0, 150, 16, -1);
        for (int i = 0; i < contrastBars; i++) {
          lcd.setCursor(i, 1);
          lcd.write("O");
        }
        setBrightnessContrast();
        analogWrite(contrastPin, contrast);

      } else if (settingsState == 2) {
        resetScreen();

        displayMenuImage(menuImages[8]);

        // displays the LCD brightness power
        swReading = digitalRead(pinSW);
        lcd.setCursor(0, 0);
        lcd.write(" LED BRIGHTNESS ");
        int brightnessBars = map(brightness, 0, 255, 16, -1);
        for (int i = 0; i < brightnessBars; i++) {
          lcd.setCursor(i, 1);
          lcd.write("O");
        }
        setBrightnessContrast();
        analogWrite(backLightPin, brightness);

      } else if (settingsState == 3) {
        resetScreen();

        // displays the MATRIX birghtness power
        swReading = digitalRead(pinSW);
        lcd.setCursor(0, 0);
        lcd.write(" MX BRIGHTNESS ");
        lightUpMatrix();
        int brightnessBars = map(matrixBrightness, 0, 15, 0, 17);
        for (int i = 0; i < brightnessBars; i++) {
          lcd.setCursor(i, 1);
          lcd.write("O");
        }
        setBrightnessContrast();
        lc.setIntensity(0, matrixBrightness);

      } else if (settingsState == 4) {
        resetScreen();

        displayMenuImage(menuImages[9]);

        // displays volume options (Mute or Unmute)
        lcd.setCursor(0, 0);
        if (buzzerVolume == 1) {
          lcd.write(" MUTE VOLUME? ");
        } else if (buzzerVolume == 0) {
          lcd.write(" UNMUTE VOLUME");
        }

        lcd.setCursor(3, 1);
        lcd.write("Yes");
        lcd.setCursor(10, 1);
        lcd.write("No");

        muteOrUnmuteVolume();
      }
    } else if (menuState == 5) {
      resetScreen();

      displayMenuImage(menuImages[1]);

      // displays the options for deleting highscores
      lcd.setCursor(0, 0);
      lcd.write(" ARE YOU SURE? ");
      lcd.setCursor(3, 1);
      lcd.write("Yes");
      lcd.setCursor(10, 1);
      lcd.write("No");

      selectChoice();
    } else if (menuState == 6) {
      resetScreen();

      // displays the instructions section
      displayMenuImage(menuImages[6]);

      displayHelp();
    }
  } else if (gameState == 1) {
    if (startTimer == 0) {
      startTimer = millis();
    }

    displayStaticTrap(staticTraps[staticTrapImage]);

    if (millis() - startTimer <= gameTime) {
      displayScoreTime();
    }

    // updateByteMatrix();
    if (millis() - lastMoved > moveInterval) {
      // game logic
      updatePositions();
      lastMoved = millis();
    }
    if (matrixChanged == true) {
      // matrix display logic
      updateMatrix();
      matrixChanged = false;
    }

    generateFood();

    // decreases time remaining
    if (millis() - startTimer >= 1000) {
      gameTime = gameTime - 1000;
      startTimer = millis();
    }

    // checks if game time has expired
    if (gameTime == 0) {
      tone(buzzerPin, 400, 1000);
      tone(buzzerPin, 300, 1000);
      tone(buzzerPin, 200, 1000);
      noTone(VolumePin);
      lcd.clear();
      lc.clearDisplay(0);
      gameState = 2;
      gameOver = 1;
      gameTime = 20000;
      startTimer = 0;
    }
  } else if (gameState == 2) {
    if (gameOver == 1) {
      // displays the game over screen
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.write("   GAME OVER!   ");
      delay(1500);
      lcd.clear();
      lc.clearDisplay(0);
      if (score > highscores[5].score) {
        lcd.setCursor(0, 0);
        lcd.write(" NEW HIGHSCORE! ");
        delay(1000);
        lcd.clear();
      }
      gameOver = 0;
      startTimer = 0;
    }

    // requests the player's name if he has achived a new highscore in order to save it
    if (score > highscores[5].score) {
      if (enterName() == 1) {
        gameState = 0;
        menuState = 0;

        // saves the name and score
        for (int i = 0; i <= 2; i++) {
          highscores[5].name[i] = highscoreName[i];
        }
        highscores[5].score = score;

        // sorts the new highscores
        sortHighscores(6);

        // resets temporary score and name
        score = 0;
        for (int i = 0; i <= 2; i++) {
          highscoreName[i] = ' ';
        }

        // saves the highscores in EEPROM
        int j = 0;
        for (int i = 20; i <= 45; i = i + 5) {
          EEPROM.put(i, highscores[j].name);
          EEPROM.put(i + 3, highscores[j].score);
          j++;
        }
      }
    } else {
      gameState = 0;
      menuState = 0;
    }
  }
}

// function that displays on the matrix the image for its respective menu
void displayMenuImage(uint64_t image) {
  for (int i = 0; i < 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j < 8; j++) {
      lc.setLed(0, i, j, bitRead(row, j));
    }
  }
}

// function that handles the traps
void displayStaticTrap(uint64_t image) {
  int position; // variable that saves matrix x and y coordonates in the form of an integer
  int numberOfElementsToDelete; // number of matrix leds that have to be deleted 
  int i, j;
  const int buzzToneTrap1 = 20;
  const int buzzToneTrap2 = 10;
  const int buzzToneTrapActive = 60;
  const int buzzTime = 50;

  if (trapBlinkTimer == 0) {
    trapBlinkTimer = millis();
  }

  // goes through the trap states from blinking to active to inactive
  if (millis() - trapBlinkTimer >= 0 && trapBlinkStage == 1) {
    numberOfElementsToDelete = 0;
    for (int i = 0; i < 8; i++) {
      byte row = (image >> i * 8) & 0xFF;
      for (int j = 0; j < 8; j++) {
        if (bitRead(row, j) == 1) {
          matrix[i][j] = bitRead(row, j);
          position = i * 10 + j;
          positionsToDelete[numberOfElementsToDelete] = position;
          numberOfElementsToDelete++;
        }
      }
    }
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        matrixTrap[i][j] = !matrix[i][j];
      }
    }
    matrixTrap[xPos][yPos] = 1;
    matrixTrap[newFoodPosY][newFoodPosY] = 1;
    trapBlinkStage++;
    matrixChanged = true;
  }
  if (millis() - trapBlinkTimer >= trapBlink * 500 && trapBlinkStage == 2) {
    for (int k = 0; k < numberOfElementsToDelete; k++) {
      i = positionsToDelete[k] / 10;
      j = positionsToDelete[k] % 10;
      matrix[i][j] = 0;
    }
    trapBlinkStage++;
    matrixChanged = true;
  }
  if (millis() - trapBlinkTimer >= trapBlink * 1000 && trapBlinkStage == 3) {
    numberOfElementsToDelete = 0;
    for (int i = 0; i < 8; i++) {
      byte row = (image >> i * 8) & 0xFF;
      for (int j = 0; j < 8; j++) {
        if (bitRead(row, j) == 1) {
          matrix[i][j] = bitRead(row, j);
          position = i * 10 + j;
          positionsToDelete[numberOfElementsToDelete] = position;
          numberOfElementsToDelete++;
        }
      }
    }
    trapBlinkStage++;
    matrixChanged = true;
  }
  if (millis() - trapBlinkTimer >= trapBlink * 1500 && trapBlinkStage == 4) {
    for (int k = 0; k < numberOfElementsToDelete; k++) {
      i = positionsToDelete[k] / 10;
      j = positionsToDelete[k] % 10;
      matrix[i][j] = 0;
    }
    trapBlinkStage++;
    matrixChanged = true;
  }
  if (millis() - trapBlinkTimer >= trapBlink * 2000 && trapBlinkStage == 5) {
    numberOfElementsToDelete = 0;
    for (int i = 0; i < 8; i++) {
      byte row = (image >> i * 8) & 0xFF;
      for (int j = 0; j < 8; j++) {
        if (bitRead(row, j) == 1) {
          matrix[i][j] = bitRead(row, j);
          position = i * 10 + j;
          positionsToDelete[numberOfElementsToDelete] = position;
          numberOfElementsToDelete++;
        }
      }
    }
    trapBlinkStage++;
    matrixChanged = true;
  }
  if (millis() - trapBlinkTimer >= trapBlink * 2500 && trapBlinkStage == 6) {
    for (int k = 0; k < numberOfElementsToDelete; k++) {
      i = positionsToDelete[k] / 10;
      j = positionsToDelete[k] % 10;
      matrix[i][j] = 0;
    }
    trapBlinkStage++;
    matrixChanged = true;
  }
  if (millis() - trapBlinkTimer >= trapBlink * 3000 && trapBlinkStage == 7) {
    numberOfElementsToDelete = 0;
    for (int i = 0; i < 8; i++) {
      byte row = (image >> i * 8) & 0xFF;
      for (int j = 0; j < 8; j++) {
        if (bitRead(row, j) == 1) {
          matrix[i][j] = bitRead(row, j);
          position = i * 10 + j;
          positionsToDelete[numberOfElementsToDelete] = position;
          numberOfElementsToDelete++;

          // check if the player is on a trap when it activates
          if (position == xPos * 10 + yPos) {
            gameState = 2;
            gameOver = 1;
            matrix[xLastPos][yLastPos] = 0;
            tone(buzzerPin, 400, 1000);
            tone(buzzerPin, 300, 1000);
            tone(buzzerPin, 200, 1000);
            noTone(VolumePin);
            noTone(VolumePin);
            for (int i = 0; i < 8; i++) {
              for (int j = 0; j < 8; j++) {
                matrix[i][j] = 0;
              }
            }
            for (i = 0; i < 64; i++) {
              positionsToDelete[i] = 0;
            }
          }
        }
      }
    }
    trapActive = 1;
    trapBlinkStage++;
    matrixChanged = true;
  }

  // a trap lasts longet based on its type
  if (millis() - trapBlinkTimer >= trapBlink * (3000 + simpleStaticTrapTime) && staticTrapImage < 6) {
    for (int k = 0; k < numberOfElementsToDelete; k++) {
      i = positionsToDelete[k] / 10;
      j = positionsToDelete[k] % 10;
      matrix[i][j] = 0;
    }
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        matrixTrap[i][j] = 1;
      }
    }
    matrixChanged = true;
  } else if (millis() - trapBlinkTimer >= trapBlink * (3000 + complexStaticTrapTime) && staticTrapImage >= 6) {
    for (int k = 0; k < numberOfElementsToDelete; k++) {
      i = positionsToDelete[k] / 10;
      j = positionsToDelete[k] % 10;
      matrix[i][j] = 0;
    }
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        matrixTrap[i][j] = 1;
      }
    }
    matrixChanged = true;
  }

  // a small break before the next trap activates
  if (millis() - trapBlinkTimer >= trapBlink * (3500 + simpleStaticTrapTime) && staticTrapImage < 6) {
    trapBlinkTimer = 0;
    trapBlinkStage = 1;
    trapActive = 0;
    for (i = 0; i < 64; i++) {
      positionsToDelete[i] = 0;
    }
    if (difficultyChoice != 3) {
      staticTrapImage = random() % (staticTrapNumber - 8);
    } else {
      staticTrapImage = random() % staticTrapNumber;
    }
  } else if (millis() - trapBlinkTimer >= trapBlink * (3500 + complexStaticTrapTime) && staticTrapImage >= 6) {
    trapBlinkTimer = 0;
    trapBlinkStage = 1;
    trapActive = 0;
    for (i = 0; i < 64; i++) {
      positionsToDelete[i] = 0;
    }
    if (difficultyChoice != 3) {
      staticTrapImage = random() % (staticTrapNumber - 8);
    } else {
      staticTrapImage = random() % staticTrapNumber;
    }
  }
}

// function that displays the score and game time left on the LCD
void displayScoreTime() {
  lcd.setCursor(0, 0);
  lcd.write("SCORE:");
  lcd.setCursor(7, 0);
  char char_arr[100];
  unsigned long num = score;
  sprintf(char_arr, "%d", num);
  lcd.write(char_arr);
  lcd.setCursor(0, 1);
  lcd.write("TIME LEFT:");
  num = (gameTime - (millis() - startTimer)) / 1000;
  sprintf(char_arr, "%d", num);

  if (num < 10) {
    lcd.setCursor(12, 1);
    lcd.write(" ");
  }
  lcd.setCursor(11, 1);
  lcd.write(char_arr);
  if (millis() % 800 == 0) {
    lcd.clear();
  }
}

// function that randomly generates food on the matrix
void generateFood() {
  if (eaten == 0) {
    newFoodPosX = random() % 8;
    newFoodPosY = random() % 8;

    // selects a new spawn position until it's not above a trap
    while (matrixTrap[newFoodPosX][newFoodPosY] == 0) {
      newFoodPosX = random() % 8;
      newFoodPosY = random() % 8;
    }
    eaten = 1;
  }

  if (millis() - blink > 150 && blinked == 0) {
    matrix[newFoodPosX][newFoodPosY] = 1;
    updateMatrix();
    blink = millis();
    blinked = 1;
  }

  if (millis() - blink > 150 && blinked == 1) {
    matrix[newFoodPosX][newFoodPosY] = 0;
    updateMatrix();
    blink = millis();
    blinked = 0;
  }
}

// function that updates the matrix leds (player, food, traps)
void updateMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, matrix[row][col]);
    }
  }
}

// function that lights all the matrix leds at once in order to see the changes when changing the matrix brightness
void lightUpMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, 1);
    }
  }
}

// function that handles the movements of the player and its interactions with food and traps
void updatePositions() {
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);
  const int buzzToneMove = 50;
  const int buzzToneEat = 400;
  const int buzzTime = 100;
  xLastPos = xPos;
  yLastPos = yPos;
  if (xValue < minThreshold) {
    tone(buzzerPin, buzzToneMove, buzzTime / 2);
    noTone(VolumePin);
    if (xPos < matrixSize - 1) {
      xPos++;
    } else {
      xPos = 0;
    }
  }
  if (xValue > maxThreshold) {
    tone(buzzerPin, buzzToneMove, buzzTime / 2);
    noTone(VolumePin);
    if (xPos > 0) {
      xPos--;
    } else {
      xPos = matrixSize - 1;
    }
  }
  if (yValue > maxThreshold) {
    tone(buzzerPin, buzzToneMove, buzzTime / 2);
    noTone(VolumePin);
    if (yPos < matrixSize - 1) {
      yPos++;
    } else {
      yPos = 0;
    }
  }
  if (yValue < minThreshold) {
    tone(buzzerPin, buzzToneMove, buzzTime / 2);
    noTone(VolumePin);
    if (yPos > 0) {
      yPos--;
    } else {
      yPos = matrixSize - 1;
    }
  }

  // if the new spot is a trap tile the player dies/ if the new pos is a food tile the score increases  
  if (xPos != xLastPos || yPos != yLastPos) {
    matrixChanged = true;
    if (matrixTrap[xPos][yPos] == 0 && trapActive == 1 && (xPos != newFoodPosX && yPos != newFoodPosY)) {
      gameState = 2;
      gameOver = 1;
      matrix[xLastPos][yLastPos] = 0;
      tone(buzzerPin, 400, 1000);
      tone(buzzerPin, 300, 1000);
      tone(buzzerPin, 200, 1000);
      noTone(VolumePin);
    } else {
      matrix[xLastPos][yLastPos] = 0;
      matrix[xPos][yPos] = 1;
      if (xPos == newFoodPosX && yPos == newFoodPosY) {
        tone(buzzerPin, buzzToneEat, buzzTime);
        noTone(VolumePin);
        eaten = 0;
        score = score + addScore;
        gameTime = gameTime + addTime;
      }
    }
  }
}

// the function that handles menu navigation
void navigateMenu() {
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);
  const int buzzToneScroll = 300;
  const int buzzToneSelect = 400;
  const int buzzTime = 100;
  if (xValue > maxThreshold && joyMoved == false && xMenu >= 1) {
    xMenu--;
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    if (xMenu % 2 == 1) {
      panel = panel - 2;
    }
    joyMoved = true;
    lcd.clear();
  }
  if (xValue < minThreshold && joyMoved == false && xMenu <= settingsSize - 1 && menuState == 4) {
    xMenu++;
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    if (xMenu % 2 == 0) {
      panel = panel + 2;
    }
    joyMoved = true;
    lcd.clear();
  }
  if (xValue < minThreshold && joyMoved == false && xMenu <= menuSize - 1 && menuState == 0) {
    xMenu++;
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    if (xMenu % 2 == 0) {
      panel = panel + 2;
    }
    joyMoved = true;
    lcd.clear();
  }
  if (xValue < minThreshold && joyMoved == false && xMenu <= 4 && menuState == 2) {
    xMenu++;
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    if (xMenu % 2 == 0) {
      panel = panel + 2;
    }
    joyMoved = true;
    lcd.clear();
  }
  if (yValue > maxThreshold && joyMoved == false) {
    tone(buzzerPin, buzzToneSelect, buzzTime);
    noTone(VolumePin);
    if (menuState == 4) {
      settingsState = xMenu + 1;
    } else if (menuState == 0) {
      menuState = xMenu + 1;
    }
    joyMoved = true;
    resetMenu();
  }
  if (yValue < minThreshold && joyMoved == false) {
    tone(buzzerPin, buzzToneSelect, buzzTime);
    noTone(VolumePin);
    if (menuState == 4 && settingsState == 0) {
      menuState = 0;
    } else if (settingsState != 0) {
      settingsState = 0;
    } else if (menuState != 0) {
      menuState = 0;
    }
    joyMoved = true;
    resetMenu();
  }

  if (xValue > resetMinThreshold && xValue < resetMaxThreshold && yValue > resetMinThreshold && yValue < resetMaxThreshold) {
    joyMoved = false;
  }
}

// function that resets values for certain menu variables
void resetMenu() {
  xMenu = 0;
  panel = 0;
  clear = 0;
}

// function that resets once the LCD and matrix on changing the menu
void resetScreen() {
  if (clear == 0) {
    lcd.clear();
    lc.clearDisplay(0);
    clear = 1;
    scrollTimer = 0;
  }
}

// function that handles the changes of brightness LCD/matrix and contrast LCD and saves the changes to EEPROM
void setBrightnessContrast() {
  int yValue = analogRead(yPin);
  const int buzzToneScroll = 300;
  const int buzzToneSelect = 400;
  const int buzzTime = 100;
  if (yValue > maxThreshold && joyMoved == false) {
    if (settingsState == 2 && brightness >= 30) {
      tone(buzzerPin, buzzToneScroll, buzzTime);
      noTone(VolumePin);
      brightness = brightness - 30;
      EEPROM.update(0, brightness);
    } else if (settingsState == 1 && contrast >= 15) {
      tone(buzzerPin, buzzToneScroll, buzzTime);
      noTone(VolumePin);
      contrast = contrast - 15;
      EEPROM.update(2, contrast);
    } else if (settingsState == 3 && matrixBrightness <= 14) {
      tone(buzzerPin, buzzToneScroll, buzzTime);
      noTone(VolumePin);
      matrixBrightness = matrixBrightness + 1;
      EEPROM.update(4, matrixBrightness);
    }
    joyMoved = true;
    lcd.clear();
  }
  if (yValue < minThreshold && joyMoved == false) {
    if (settingsState == 2 && brightness <= 233) {
      tone(buzzerPin, buzzToneScroll, buzzTime);
      noTone(VolumePin);
      brightness = brightness + 30;
      EEPROM.update(0, brightness);
    } else if (settingsState == 1 && contrast <= 135) {
      tone(buzzerPin, buzzToneScroll, buzzTime);
      noTone(VolumePin);
      contrast = contrast + 15;
      EEPROM.update(2, contrast);
    } else if (settingsState == 3 && matrixBrightness >= 1) {
      tone(buzzerPin, buzzToneScroll, buzzTime);
      noTone(VolumePin);
      matrixBrightness = matrixBrightness - 1;
      EEPROM.update(4, matrixBrightness);
    }
    joyMoved = true;
    lcd.clear();
  }
  if (yValue > resetMinThreshold && yValue < resetMaxThreshold) {
    joyMoved = false;
  }

  // When satisfied with the choice press the joystick to leave the menu
  if (swReading == LOW && PressStart == 0) {
    PressTimer = millis();
    PressStart = 1;
  }
  if (millis() - PressTimer >= 100 && PressStart == 1) {
    tone(buzzerPin, buzzToneSelect, buzzTime);
    noTone(VolumePin);
    settingsState = 0;
    PressStart = 0;
    PressTimer = millis();
    resetMenu();
    lc.clearDisplay(0);
  }
  if (swReading != LOW) {
    PressTimer = millis();
    PressStart = 0;
  }
}

// function that handless the process of entering a name for the new highscore
int enterName() {
  const int buzzToneScroll = 300;
  const int buzzTonescrollLetter = 350;
  const int buzzToneSelect = 400;
  const int buzzTime = 100;

  if (nameTimer == 0) {
    nameTimer = millis();
  }

  lcd.setCursor(0, 0);
  lcd.write("   ENTER NAME   ");
  lcd.setCursor(5, 1);
  lcd.write("<");
  lcd.setCursor(9, 1);
  lcd.write(">");

  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);
  swReading = digitalRead(pinSW);

  if (xValue > maxThreshold && joyMoved == false && nameLetters[leterPosition] <= 89) {
    tone(buzzerPin, buzzTonescrollLetter, buzzTime);
    noTone(VolumePin);
    nameLetters[leterPosition]++;
    joyMoved = true;
  }
  if (xValue < minThreshold && joyMoved == false && nameLetters[leterPosition] >= 66) {
    tone(buzzerPin, buzzTonescrollLetter, buzzTime);
    noTone(VolumePin);
    nameLetters[leterPosition]--;
    joyMoved = true;
  }
  if (yValue > maxThreshold && joyMoved == false && leterPosition <= 1) {
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    leterPosition++;
    joyMoved = true;
  }
  if (yValue < minThreshold && joyMoved == false && leterPosition >= 1) {
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    leterPosition--;
    joyMoved = true;
  }

  if (xValue > resetMinThreshold && xValue < resetMaxThreshold && yValue > resetMinThreshold && yValue < resetMaxThreshold) {
    joyMoved = false;
  }

  // displays the name letters on the screen and slowly blinks the currently selected letter
  lcd.setCursor(6 + leterPosition, 1);
  if (millis() - nameTimer >= 300) {
    lcd.write(" ");
  }

  if (millis() - nameTimer >= 600) {
    lcd.setCursor(6, 1);
    lcd.write((char)(nameLetters[0]));
    lcd.setCursor(7, 1);
    lcd.write((char)(nameLetters[1]));
    lcd.setCursor(8, 1);
    lcd.write((char)(nameLetters[2]));
    nameTimer = millis();
  }

  if (swReading == LOW && PressStart == 0) {
    PressTimer = millis();
    PressStart = 1;
  }
  if (millis() - PressTimer >= 100 && PressStart == 1) {
    tone(buzzerPin, buzzToneSelect, buzzTime);
    noTone(VolumePin);
    lcd.clear();
    PressTimer = millis();
    PressStart = 0;
    highscoreName[0] = (char)(nameLetters[0]);
    highscoreName[1] = (char)(nameLetters[1]);
    highscoreName[2] = (char)(nameLetters[2]);
    return 1;
  }
  if (swReading != LOW) {
    PressTimer = millis();
    PressStart = 0;
  }

  return 0;
}

// function used to sort the hoghscores
void sortHighscores(int len) {
  highscore temp;

  for (int i = 0; i < len - 1; i++) {
    for (int j = i + 1; j < len; j++) {
      if (highscores[i].score < highscores[j].score) {
        temp = highscores[i];
        highscores[i] = highscores[j];
        highscores[j] = temp;
      }
    }
  }
}

// function used to reset all the highscores
void deleteHighscores() {
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 3; j++) {
      highscores[i].name[j] = 'A';
    }
    highscores[i].score = 0;
  }
  int j = 0;
  char cuv[3] = "AAA";
  for (int i = 20; i <= 45; i = i + 5) {
    EEPROM.put(i, cuv);
    EEPROM.put(i + 3, 0);
    j++;
  }
}

// function used to select YES or NO in the RESET HIGHSCORES section
void selectChoice() {
  int yValue = analogRead(yPin);
  swReading = digitalRead(pinSW);
  const int buzzToneScroll = 300;
  const int buzzToneSelect = 400;
  const int buzzTime = 100;

  if (yValue > maxThreshold && joyMoved == false) {
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    deleteChoice = 0;
    lcd.clear();
    lcd.setCursor(9, 1);
    lcd.write(">");
    lcd.setCursor(12, 1);
    lcd.write("<");
    joyMoved = true;
  }
  if (yValue < minThreshold && joyMoved == false) {
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    deleteChoice = 1;
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.write(">");
    lcd.setCursor(6, 1);
    lcd.write("<");
    joyMoved = true;
  }

  if (yValue > resetMinThreshold && yValue < resetMaxThreshold) {
    joyMoved = false;
  }

  if (swReading == LOW && PressStart == 0) {
    PressTimer = millis();
    PressStart = 1;
  }
  if (millis() - PressTimer >= 100 && PressStart == 1) {
    tone(buzzerPin, buzzToneSelect, buzzTime);
    noTone(VolumePin);
    menuState = 0;
    PressTimer = millis();
    PressStart = 0;
    resetMenu();
    if (deleteChoice == 1) {
      deleteHighscores();
    }
  }
  if (swReading != LOW) {
    PressTimer = millis();
    PressStart = 0;
  }
}

// function used to select YES or NO when muting/unmuting and saves the choice to EEPROM
void muteOrUnmuteVolume() {
  int yValue = analogRead(yPin);
  swReading = digitalRead(pinSW);
  const int buzzToneScroll = 300;
  const int buzzToneSelect = 400;
  const int buzzTime = 100;

  if (yValue > maxThreshold && joyMoved == false) {
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    deleteChoice = 0;
    lcd.clear();
    lcd.setCursor(9, 1);
    lcd.write(">");
    lcd.setCursor(12, 1);
    lcd.write("<");
    joyMoved = true;
  }
  if (yValue < minThreshold && joyMoved == false) {
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    deleteChoice = 1;
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.write(">");
    lcd.setCursor(6, 1);
    lcd.write("<");
    joyMoved = true;
  }

  if (yValue > resetMinThreshold && yValue < resetMaxThreshold) {
    joyMoved = false;
  }

  if (swReading == LOW && PressStart == 0) {
    PressTimer = millis();
    PressStart = 1;
  }
  if (millis() - PressTimer >= 100 && PressStart == 1) {
    tone(buzzerPin, buzzToneSelect, buzzTime);
    noTone(VolumePin);
    settingsState = 0;
    menuState = 4;
    PressTimer = millis();
    PressStart = 0;
    resetMenu();

    if (deleteChoice == 1 && buzzerVolume == 1) {
      VolumePin = 3;
      buzzerVolume = 0;
      EEPROM.update(8, VolumePin);
      EEPROM.update(12, buzzerVolume);
    } else if (deleteChoice == 1 && buzzerVolume == 0) {
      VolumePin = A4;
      buzzerVolume = 1;
      EEPROM.update(8, VolumePin);
      EEPROM.update(12, buzzerVolume);
    }
  }
  if (swReading != LOW) {
    PressTimer = millis();
    PressStart = 0;
  }
}

// function that lets you select the game difficulty by adjusting some game variables and introducing new traps on the hardest difficulty
void pickDifficulty() {
  int yValue = analogRead(yPin);
  swReading = digitalRead(pinSW);
  const int buzzToneScroll = 300;
  const int buzzToneSelect = 400;
  const int buzzTime = 100;

  if (yValue > maxThreshold && joyMoved == false && difficultyChoice <= 2) {
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    difficultyChoice++;
    joyMoved = true;
  }
  if (yValue < minThreshold && joyMoved == false && difficultyChoice >= 2) {
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    difficultyChoice--;
    joyMoved = true;
  }

  if (yValue > resetMinThreshold && yValue < resetMaxThreshold) {
    joyMoved = false;
  }

  if (swReading == LOW && PressStart == 0) {
    PressTimer = millis();
    PressStart = 1;
  }
  if (millis() - PressTimer >= 200 && PressStart == 1) {
    tone(buzzerPin, buzzToneSelect, buzzTime);
    noTone(VolumePin);
    menuState = 0;
    PressTimer = millis();
    PressStart = 0;

    if (difficultyChoice == 1) {
      addScore = 10;
      addTime = 3000;
      moveInterval = 400;
      trapBlink = 1;
    } else if (difficultyChoice == 2) {
      addScore = 15;
      addTime = 2500;
      moveInterval = 450;
      trapBlink = 0.80;
    } else if (difficultyChoice == 3) {
      addScore = 20;
      addTime = 2200;
      moveInterval = 500;
      trapBlink = 0.65;
    }
    lc.clearDisplay(0);
    resetMenu();
  }
  if (swReading != LOW) {
    PressTimer = millis();
    PressStart = 0;
  }
}

// function that displays the game instructions from the EEPROM
void displayHelp() {
  int yValue = analogRead(yPin);
  swReading = digitalRead(pinSW);
  const int buzzToneScroll = 300;
  const int buzzToneSelect = 400;
  const int buzzTime = 100;
  ;

  if (yValue > maxThreshold && joyMoved == false && helpPanel <= 3) {
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    helpPanel++;
    joyMoved = true;
    lcd.clear();
  }
  if (yValue < minThreshold && joyMoved == false && helpPanel >= 2) {
    tone(buzzerPin, buzzToneScroll, buzzTime);
    noTone(VolumePin);
    helpPanel--;
    joyMoved = true;
    lcd.clear();
  }

  if (yValue > resetMinThreshold && yValue < resetMaxThreshold) {
    joyMoved = false;
  }

  if (scrollTimer == 0) {
    scrollTimer = millis();
  }

  if (helpPanel == 1) {
    lcd.setCursor(2, 0);
    lcd.write(readStringFromEEPROM(450).c_str());
  } else if (helpPanel == 2) {
    lcd.setCursor(2, 0);
    lcd.write(readStringFromEEPROM(491).c_str());
  } else if (helpPanel == 3) {
    lcd.setCursor(2, 0);
    lcd.write(readStringFromEEPROM(532).c_str());
    lcd.setCursor(2, 1);
    lcd.write(readStringFromEEPROM(573).c_str());
  } else if (helpPanel == 4) {
    lcd.setCursor(2, 0);
    lcd.write(readStringFromEEPROM(614).c_str());
    lcd.setCursor(2, 1);
    lcd.write(readStringFromEEPROM(655).c_str());
  }

  if (millis() - scrollTimer >= 600) {
    scrollTimer = millis();
    lcd.scrollDisplayLeft();
  }

  if (swReading == LOW && PressStart == 0) {
    PressTimer = millis();
    PressStart = 1;
  }
  if (millis() - PressTimer >= 200 && PressStart == 1) {
    tone(buzzerPin, buzzToneSelect, buzzTime);
    noTone(VolumePin);
    menuState = 0;
    PressTimer = millis();
    PressStart = 0;
    scrollTimer = 0;
    helpPanel = 1;

    resetMenu();
  }
  if (swReading != LOW) {
    PressTimer = millis();
    PressStart = 0;
  }
}