#include <LEDMatrixChip.h>

// Define SPI communication pins
#define DATA_PIN 2
#define CHIP_SELECT_PIN 3
#define CLOCK_PIN 4

// Initialization: LEDMatrixChip(CHIP_SELECT, CLOCK, DATA, num_of_chips, orientation)
LEDMatrixChip LM(CHIP_SELECT_PIN, CLOCK_PIN, DATA_PIN, 1, 0);

/*
  Class name Object_name(
  Chip Select pin,
  Clock pin,
  Data pin,
  Number of matrix chips (default 1),
  Orientation (default 0)
  )
*/

// "Z"
uint8_t Z[8] = {
    0b01111110,
    0b01111110,
    0b00001110,
    0b00011100,
    0b00111000,
    0b01110000,
    0b01111110,
    0b01111110};

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

// "N"
uint8_t N[8] = {
    0b01100110,
    0b01100110,
    0b01110110,
    0b01111110,
    0b01111110,
    0b01101110,
    0b01100110,
    0b01100110};

// "I"
uint8_t I[8] = {
    0b01111110,
    0b01111110,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b01111110,
    0b01111110};

// "U"
uint8_t U[8] = {
    0b01100110,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01111110,
    0b00111100};

// "S"
uint8_t S[8] = {
    0b00111100,
    0b01111110,
    0b01100000,
    0b01111100,
    0b00111110,
    0b00000110,
    0b01111110,
    0b00111100};

// "End"
uint8_t End[8] = {
    0b00000000,
    0b00000000,
    0b11100001,
    0b10000001,
    0b11001001,
    0b10010110,
    0b11110101,
    0b00000000};

void setup()
{
    LM.Symbol(Z); // Object_name.Symbol(name_of_character);
    delay(500);
    LM.Symbol(E);
    delay(500);
    LM.Symbol(N);
    delay(500);
    LM.Symbol(I);
    delay(500);
    LM.Symbol(U);
    delay(500);
    LM.Symbol(S);
    delay(500);
    LM.Symbol(End);
    delay(500);
    LM.clear();
}

void loop()
{
}