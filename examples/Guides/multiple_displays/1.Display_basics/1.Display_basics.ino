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

// Custom row/column patterns (each bit represents an LED state: 1 = ON, 0 = OFF)
uint8_t Example_Col = 0b00011100; // Custom pattern for a column
uint8_t Example_Row = 0b11100011; // Custom pattern for a row

void setup()
{
    LM.Test(); // Runs a test sequence to verify wiring
}

void loop()
{
    LM.turnOn(2, 4, 0);
    /*
      Turns on a single LED at a specific position.
      Parameters:
      - Column index (0-7),
      - Row index (0-7),
      - nth matrix (default 0)
    */

    LM.OnCol(3, 1);
    /*
      Turns on all LEDs in a specific column.
      Parameters:
      - Column index (0-7),
      - nth matrix (default 0)
    */

    LM.OnRow(6, 0);
    /*
      Turns on all LEDs in a specific row.
      Parameters:
      - Row index (0-7),
      - nth matrix (default 0)
    */

    LM.customCol(Example_Col, 1, 1);
    /*
      Displays a custom column pattern.
      Parameters:
      - Byte pattern (each bit controls an LED in the column),
      - Column index (0-7),
      - nth matrix (default 0),
      - shift (default 0)
    */

    LM.customRow(Example_Row, 2, 0);
    /*
      Displays a custom row pattern.
      Parameters:
      - Byte pattern (each bit controls an LED in the row),
      - Row index (0-7),
      - nth matrix (default 0),
      - shift (default 0)
    */

    // LM.Symbol(2D array); // Displays a full 8x8 custom symbol (not used in this example)
}
