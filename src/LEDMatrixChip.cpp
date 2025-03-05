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

    for (int i = 0; i < amount; i++)
        write_reg(DISPLAY_TEST_REG, 0);

    for(int i = 0; i < amount; i++)
        write_reg(SCAN_LIMIT_REG, 7);

    for (int i = 0; i < amount; i++)
        write_reg(DECODE_MODE_REG, 0);

    for (int i = 0; i < amount; i++)
        write_reg(SHUTDOWN_REG, 1);

    for (int i = 0; i < amount; i++)
        write_reg(DISPLAY_TEST_REG, 0);

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
    (nth == -1) ? clearAll() : clearNth(nth);
}

// Turns on an individual LED
void LEDMatrixChip::turnOn(int Col, int Row, int nth)
{
    rows[limitingGrid(Row)][nth] = 0x01 << limitingGrid(Col);
    write_display();
}

// Turns on an entire column
void LEDMatrixChip::OnCol(int Col, int nth)
{
    for (int i = 0; i < 8; i++)
    {
        rows[i][nth] = 0x01 << limitingGrid(Col);
    }
    write_display();
}

// Turns on an entire row
void LEDMatrixChip::OnRow(int Row, int nth)
{
    rows[limitingGrid(Row)][nth] = 0xFF;
    write_display();
}

// Custom column display with shift support
void LEDMatrixChip::customCol(uint8_t userByte, int Col, int nth, int shift)
{
    adjustShift(shift, userByte);

    for (int i = 0; i < 8; i++)
    {
        if (userByte & (0x01 << i))
            rows[i][nth] |= (0x01 << limitingGrid(Col));
    }

    write_display();
}

// Custom row display with shift support
void LEDMatrixChip::customRow(uint8_t userByte, int Row, int nth, int shift)
{

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
    delay(500);

    for (int j = 0; j < amount; j++)
    {
        write_reg(DISPLAY_TEST_REG, 0);
    }
    deselectChip();
    delay(500);
}

// Loads a custom symbol onto a matrix
void LEDMatrixChip::Symbol(uint8_t UserMatrix[8], int nth)
{
    for (int i = 0; i < 8; i++)
    {
        rows[i][nth] = UserMatrix[i];
    }
    write_display();
}

// SPI Data Transfer
void LEDMatrixChip::transfer(uint8_t *p_data, uint8_t len)
{
    uint8_t mask;

    digitalWrite(CS, LOW); // Select the chip
    delayMicroseconds(1);  // Small delay

    for (int i = 0; i < len; i++) // Iterate through the bytes
    {
        mask = 0x80; // Start with the most significant bit
        do
        {
            // Set MOSI high or low based on the current bit
            digitalWrite(MOSI, (p_data[i] & mask) ? HIGH : LOW);

            delayMicroseconds(1);
            digitalWrite(CLK, HIGH); // Clock in the data
            delayMicroseconds(1);
            digitalWrite(CLK, LOW); // Prepare for next bit

            mask >>= 1; // Shift the mask to the next bit
        } while (mask != 0); // Continue until all bits are sent
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
        deselectChip();
    }
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
