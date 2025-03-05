#pragma once

#include <Arduino.h>

class LEDMatrixChip
{
public:
    LEDMatrixChip(int CS, int CLK, int MOSI, int amount = 1, int orientation = 0);

    void clear(int nth = -1);                    // Clears the display (all or specific matrix)
    
    void turnOn(int Col, int Row, int nth = 0); // Turns on a specific LED

    void OnCol(int Col, int nth = 0);           // Turns on an entire column
    void OnRow(int Row, int nth = 0);           // Turns on an entire row

    void customCol(uint8_t userByte, int Col, int nth = 0, int shift = 0); // Custom column display
    void customRow(uint8_t userByte, int Row, int nth = 0, int shift = 0); // Custom row display

    void Test();                                     // Runs a test pattern
    void Symbol(uint8_t UserMatrix[8], int nth = 0); // Displays a custom symbol

private:
    int CS, CLK, MOSI, amount, orientation;
    uint8_t displayByte = 0x00;
    uint8_t **rows;

    void transfer(uint8_t *p_data, uint8_t len);    // SPI-like data transfer
    void write_reg(uint8_t reg, uint8_t value);     // Writes a value to a register
    void write_display();                           // Updates the display with the buffer
    void deselectChip();                            // Deselects the chip
    uint8_t limitingGrid(int value);                // Constrains value between 0-7
    void adjustShift(int shift, uint8_t userArray); // Shifts a byte
    void clearAll();                                // Clears all matrices
    void clearNth(int nth);                         // Clears a specific matrix
};
