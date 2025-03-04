#include "LEDMatrixChip.h"

// Register constants for the LED matrix driver
#define DECODE_MODE_REG 0x09
#define INTENSITY_REG 0x0A
#define SCAN_LIMIT_REG 0x0B
#define SHUTDOWN_REG 0x0C
#define DISPLAY_TEST_REG 0x0F

// Constructor
LEDMatrixChip::LEDMatrixChip(int CS, int CLK, int MOSI, int amount, int orientation)
{
    this->CS = CS;
    this->CLK = CLK;
    this->MOSI = MOSI;
    this->amount = amount;
    this->orientation = orientation;

    pinMode(this->CS, OUTPUT);
    pinMode(this->CLK, OUTPUT);
    pinMode(this->MOSI, OUTPUT);

    digitalWrite(this->CS, HIGH);
    digitalWrite(this->CLK, LOW);
    digitalWrite(this->MOSI, LOW);

    uint8_t initRegs[5] = {DECODE_MODE_REG, INTENSITY_REG, SCAN_LIMIT_REG, SHUTDOWN_REG, DISPLAY_TEST_REG};

    for (int j = 0; j < 5; j++)
    {
        for (int i = 0; i < amount; i++)
        {
            write_reg(initRegs[j], 0);
        }
        deselectChip();
    }

    rows = new uint8_t *[8];
    for (int i = 0; i < 8; i++)
    {
        rows[i] = new uint8_t[amount];
    }

    clear();
}

// Clears the display or a specific matrix
void LEDMatrixChip::clear(int nth)
{
    (nth == 0) ? clearAll() : clearNth(nth - 1);
    deselectChip();
}

// Turns on an individual LED
void LEDMatrixChip::turnOn(int Col, int Row, int nth)
{
    if (nth > 0)
        nth -= 1;
    rows[limitingGrid(Row)][nth] = 0x01 << limitingGrid(Col);
    write_display();
}

// Turns on an entire column
void LEDMatrixChip::OnCol(int Col, int nth)
{
    if (nth > 0)
        nth -= 1;
    for (int i = 0; i < 8; i++)
    {
        rows[i][nth] = 0x01 << limitingGrid(Col);
    }
    write_display();
}

// Turns on an entire row
void LEDMatrixChip::OnRow(int Row, int nth)
{
    if (nth > 0)
        nth -= 1;
    rows[limitingGrid(Row)][nth] = 0xFF;
    write_display();
}

// Custom column display with shift support
void LEDMatrixChip::customCol(uint8_t userByte, int Col, int nth, int shift)
{
    if (nth > 0)
        nth -= 1;
    adjustShift(shift, userByte);
    for (int i = 0; i < 8; i++)
    {
        rows[i][nth] |= (0x01 << limitingGrid(Col));
    }
    write_display();
}

// Custom row display with shift support
void LEDMatrixChip::customRow(uint8_t userByte, int Row, int nth, int shift)
{
    if (nth > 0)
        nth -= 1;
    adjustShift(shift, userByte);
    rows[limitingGrid(Row)][nth] = displayByte;
    write_display();
}

// Test mode: enable and disable display test
void LEDMatrixChip::Test()
{
    for (int j = 0; j < amount; j++)
    {
        write_reg(DISPLAY_TEST_REG, 1);
    }
    deselectChip();
    delay(250);

    for (int j = 0; j < amount; j++)
    {
        write_reg(DISPLAY_TEST_REG, 0);
    }
    deselectChip();
    delay(250);
}

// Loads a custom symbol onto a matrix
void LEDMatrixChip::Symbol(uint8_t UserMatrix[8], int nth)
{
    if (nth > 0)
        nth -= 1;
    for (int i = 0; i < 8; i++)
    {
        rows[i][nth] = UserMatrix[i];
    }
    deselectChip();
}

// SPI Data Transfer
void LEDMatrixChip::transfer(uint8_t *p_data, uint8_t len)
{
    digitalWrite(CS, LOW);
    for (int i = 0; i < len; i++)
    {
        for (uint8_t mask = 0x80; mask; mask >>= 1)
        {
            digitalWrite(MOSI, (p_data[i] & mask) ? HIGH : LOW);
            digitalWrite(CLK, HIGH);
            digitalWrite(CLK, LOW);
        }
    }
}

// Writes a value to a register
void LEDMatrixChip::write_reg(uint8_t reg, uint8_t value)
{
    uint8_t tx_data[2] = {reg, value};
    transfer(tx_data, 2);
}

// Updates the display with buffer contents
void LEDMatrixChip::write_display()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < amount; j++)
        {
            write_reg(i + 1, rows[i][j]);
        }
    }
    deselectChip();
}

// Constrains a value between 0 and 7
uint8_t LEDMatrixChip::limitingGrid(int value)
{
    return constrain(value, 0, 7);
}

// Deselects the chip
void LEDMatrixChip::deselectChip()
{
    digitalWrite(CS, HIGH);
}

// Adjusts a byte value by shifting
void LEDMatrixChip::adjustShift(int shift, uint8_t userArray)
{
    shift = constrain(shift, -7, 7);
    displayByte = userArray;

    if (shift > 0)
    {
        displayByte <<= shift;
    }
    else if (shift < 0)
    {
        displayByte >>= -shift;
    }
}

// Clears all matrices
void LEDMatrixChip::clearAll()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < amount; j++)
        {
            rows[i][j] = 0x00;
        }
    }
    write_display();
}

// Clears a specific matrix
void LEDMatrixChip::clearNth(int nth)
{
    for (int i = 0; i < 8; i++)
    {
        rows[i][nth] = 0x00;
    }
    write_display();
}
