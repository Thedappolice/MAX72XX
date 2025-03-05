#include <LEDMatrixChip.h>

// Define SPI communication pins
#define DATA_PIN 2
#define CHIP_SELECT_PIN 3
#define CLOCK_PIN 4

// Initialization: LEDMatrixChip(CHIP_SELECT, CLOCK, DATA, num_of_chips, orientation)
LEDMatrixChip LM(CHIP_SELECT_PIN, CLOCK_PIN, DATA_PIN, 2, 0);

/*
  Class name Object_name(
  Chip Select pin,
  Clock pin,
  Data pin,
  Number of matrix chips (default 1),
  Orientation (default 0)
  )
*/

// "H"
uint8_t H[8] = {
    0b01100110,
    0b01100110,
    0b01100110,
    0b01111110,
    0b01111110,
    0b01100110,
    0b01100110,
    0b01100110};

// "E"
uint8_t E[8] = {
    0b01111110,
    0b01111110,
    0b01100000,
    0b01111110,
    0b01111110,
    0b01100000,
    0b01111110,
    0b01111110};

void setup()
{
}

void loop()
{
    LM.Symbol(H, 1); // Object_name.Symbol(name_of_character);
    LM.Symbol(E, 0);
    delay(500);
    LM.clear();
    delay(500);
}