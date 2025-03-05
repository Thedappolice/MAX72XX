// this program uses Arduino Nano

#include <LEDMatrixChip.h>

// Define SPI communication pins
#define DATA_PIN 2
#define CHIP_SELECT_PIN 3
#define CLOCK_PIN 4

LEDMatrixChip LM(CHIP_SELECT_PIN, CLOCK_PIN, DATA_PIN, 1, 0);

// Wall pattern array
uint8_t WALL = 0b00111000;

// Pins for the buttons
#define P1_LEFT A4
#define P1_RIGHT A5
#define P2_LEFT A7
#define P2_RIGHT A6

struct Player
{
  int shift;
  bool changed;
  int leftPin;
  int rightPin;
};

Player player1 = {0, false, P1_LEFT, P1_RIGHT};
Player player2 = {0, false, P2_LEFT, P2_RIGHT};

struct Ball
{
  int x;
  int y;
  bool xDirPos;
  bool yDirPos;
  unsigned long delayTime;
  unsigned long lastUpdateTime;
  bool delayed;
};

Ball ball = {(int)random(3, 5), (int)random(3, 5), random(0, 2) == 0, random(0, 2) == 0, 1000, 0, false};

int memory[8][8] = {{0}};

void ShowSymbol(char input, unsigned long delayTime = 0)
{
  // display arrays
  uint8_t display[8] = {0};
  uint8_t W[8] =
      {0b10000001,
       0b10000001,
       0b10011001,
       0b11011011,
       0b11011011,
       0b11100111,
       0b01100110,
       0b01000010};

  uint8_t I[8] =
      {0b01111110,
       0b01111110,
       0b00011000,
       0b00011000,
       0b00011000,
       0b00011000,
       0b01111110,
       0b01111110};

  uint8_t N[8] =
      {0b11000011,
       0b11100011,
       0b11110011,
       0b11111011,
       0b11011111,
       0b11001111,
       0b11000111,
       0b11000011};

  uint8_t P[8] =
      {0b01111100,
       0b01100110,
       0b01100110,
       0b01111100,
       0b01100000,
       0b01100000,
       0b01100000,
       0b01100000};

  uint8_t N1[8] =
      {0b00011000,
       0b00111000,
       0b01111000,
       0b00011000,
       0b00011000,
       0b00011000,
       0b01111110,
       0b01111110};

  uint8_t N2[8] =
      {0b00111100,
       0b01111110,
       0b01100110,
       0b00001110,
       0b00011110,
       0b00111000,
       0b01111110,
       0b01111110};

  uint8_t N3[8] =
      {0b00111100,
       0b01111110,
       0b00000110,
       0b00111110,
       0b00111110,
       0b00000110,
       0b01111110,
       0b00111100};

  // Copy input symbol to display array
  switch (input)
  {
  case 'W':
    memcpy(display, W, 8 * sizeof(uint8_t));
    break;
  case 'I':
    memcpy(display, I, 8 * sizeof(uint8_t));
    break;
  case 'N':
    memcpy(display, N, 8 * sizeof(uint8_t));
    break;
  case 'P':
    memcpy(display, P, 8 * sizeof(uint8_t));
    break;
  case '1':
    memcpy(display, N1, 8 * sizeof(uint8_t));
    break;
  case '2':
    memcpy(display, N2, 8 * sizeof(uint8_t));
    break;
  case '3':
    memcpy(display, N3, 8 * sizeof(uint8_t));
    break;
  }

  // Call the LM.Symbol function to display the symbol on the LED matrix with time if given
  if (delayTime != 0)
  {
    LM.Symbol(display);
    delay(delayTime);
  }
  else
  {
    LM.Symbol(display);
    delay(1000);
  }
};

int limitShift(int value, bool positiveDir)
{
  if (positiveDir && value < 3)
    return value + 1;
  if (!positiveDir && value > -2)
    return value - 1;
  return value;
}

void fasterBall(Ball &ball)
{
  ball.delayTime = max(500 - millis() / 150, 50);
}

void checkButton(Player &player)
{
  bool leftState = digitalRead(player.leftPin) == HIGH;
  bool rightState = digitalRead(player.rightPin) == HIGH;

  if (!player.changed)
  {
    if (leftState)
    {
      player.shift = limitShift(player.shift, false);
    }

    if (rightState)
    {
      player.shift = limitShift(player.shift, true);
    }

    player.changed = true;
  }
  else if (!leftState && !rightState)
  {
    player.changed = false;
  }
}

void updateMemory(Player &player1, Player &player2, Ball &ball)
{
  memset(memory, 0, sizeof(memory));

  for (int j = 2 + player1.shift; j <= 4 + player1.shift; j++)
  {
    if (j >= 0 && j < 8)
      memory[0][j] = 1;
  }
  for (int j = 2 + player2.shift; j <= 4 + player2.shift; j++)
  {
    if (j >= 0 && j < 8)
      memory[7][j] = 1;
  }

  memory[ball.y][ball.x] = 1;
}

void display(Player &player1, Player &player2, Ball &ball)
{
  LM.customRow(WALL, 0, player1.shift);
  LM.customRow(WALL, 7, player2.shift);
  LM.turnOn(ball.y, ball.x);
}

void updateBall(Ball &ball)
{
  if (ball.delayed)
  {
    if (millis() - ball.lastUpdateTime > ball.delayTime)
    {
      if (ball.y == 0 || ball.y == 7)
      {
        ball.yDirPos = !ball.yDirPos;
      }

      ball.y += ball.yDirPos ? 1 : -1;

      if (ball.x == 0 || ball.x == 7)
      {
        if (memory[ball.y][ball.x] != 1)
        {
          ball.x == 0 ? endGame(1) : endGame(2);
        }
        ball.xDirPos = !ball.xDirPos;
      }
      ball.x += ball.xDirPos ? 1 : -1;

      ball.delayed = false;
    }
  }
  else
  {
    ball.lastUpdateTime = millis();
    ball.delayed = true;
  }
}

void endGame(bool isP1)
{
  for (int i = 0; i < 20; i++)
  {
    LM.turnOn(ball.y, ball.x);
    delay(100);
  }
  for (int i = 0; i < 3; i++)
  {
    ShowSymbol('P');
    ShowSymbol(isP1 ? '1' : '2');
    ShowSymbol('W');
    ShowSymbol('I');
    ShowSymbol('N');
  }
  exit(1);
}

void setup()
{
  ShowSymbol('3', 1000);
  ShowSymbol('2', 1000);
  ShowSymbol('1', 1000);
  for (int i = 0; i < 20; i++)
  {
    LM.turnOn(ball.y, ball.x);
    delay(100);
  }
}

void loop()
{
  updateMemory(player1, player2, ball);
  display(player1, player2, ball);
  checkButton(player1);
  checkButton(player2);
  updateBall(ball);
  fasterBall(ball);
}
