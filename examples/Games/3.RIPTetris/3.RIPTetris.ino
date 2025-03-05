#include <math.h> // Math library for functions like pow()

#include <Dis7Seg.h>

#define buzzerPin A5

// pins in order (A, B, C, D, E, F, G, DP)
int segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 9};

// digit pins from left to right
int digitPins[] = {10, 11, 12, 13};

Dis7Seg dis('-', segmentPins, 4, digitPins);

// Initialize array with -1, and the ones place with 0
int scoreNum[4] = {-1, -1, -1, -1};
int score = 0;

// Pin Definitions for Controls
#define ROTATE_PIN 32
#define LEFT_OR_RIGHT_PIN 23
#define UP_OR_DOWN_PIN 22

// Grid Constants
#define GRID_WIDTH 8                      // Number of columns in the grid
#define GRID_HEIGHT (GRID_WIDTH * 2)      // Total grid height (16 rows for an 8x8 matrix)
#define CHECKING_HEIGHT (GRID_HEIGHT + 2) // Includes extra rows for overflow checking

#include <LEDMatrixChip.h>

// Define SPI communication pins
#define DATA_PIN 2
#define CHIP_SELECT_PIN 3
#define CLOCK_PIN 4

LEDMatrixChip LM(CHIP_SELECT_PIN, CLOCK_PIN, DATA_PIN, 2, 0);

// Game Data Structures

// Represents a Tetrimino (Tetris shape)
typedef struct
{
    int shape[4][2];       // Relative positions of blocks in the shape
    int coordinates[4][2]; // Absolute positions of blocks on the grid
    bool active;           // Whether the shape is currently active
} Tetrimino;

// Global Variables

Tetrimino currentShape = {0}; // Current active Tetrimino

// Memory structure for the game grid
struct GridMemory
{
    int stable[CHECKING_HEIGHT][GRID_WIDTH];  // Stable grid (occupied blocks)
    int display[CHECKING_HEIGHT][GRID_WIDTH]; // Display grid (active shapes + stable grid)
    uint8_t top[GRID_WIDTH];                  // Top LED matrix data
    uint8_t bottom[GRID_WIDTH];               // Bottom LED matrix data

    // Clears the stable grid
    void clearStable()
    {
        memset(stable, 0, sizeof(stable));
    }

    // Clears the display grid
    void clearDisplay()
    {
        memset(display, 0, sizeof(display));
    }
};

// Create an instance of GridMemory
GridMemory grid;

// Timing for game
unsigned long prev = 0;       // Previous time for game interval
unsigned long interval = 500; // Time between automatic block drops (ms)

// timing for inputs
unsigned long inputprev = 0;       // Previous time for input
unsigned long inputinterval = 100; // Time between accpeted inputs (ms)

// Track the previous state of the rotate button
bool previousRotateState = LOW;

// Game State Variables

bool end = false;   // Whether the game is over
bool ended = false; // Whether the end animation has been shown

/// --- Function Definitions ---

/**
 * Displays a predefined symbol on the LED matrix.
 * @param LM - Reference to the LEDMatrix object to display on.
 * @param input - Character representing the symbol ('1', '2', '3', or 'E').
 * @param duration - Optional duration (in milliseconds) for displaying the symbol. Default is 0 (persistent display).
 */
void ShowSymbol(LEDMatrix<8, 8> &LM, char input, unsigned long duration = 0)
{
    // Predefined symbols
    int symbols[4][8][8] = {
        {// Symbol '1'
         {0, 0, 0, 1, 1, 0, 0, 0},
         {0, 0, 1, 1, 1, 0, 0, 0},
         {0, 1, 1, 1, 1, 0, 0, 0},
         {0, 0, 0, 1, 1, 0, 0, 0},
         {0, 0, 0, 1, 1, 0, 0, 0},
         {0, 0, 0, 1, 1, 0, 0, 0},
         {0, 1, 1, 1, 1, 1, 1, 0},
         {0, 1, 1, 1, 1, 1, 1, 0}},
        {// Symbol '2'
         {0, 0, 1, 1, 1, 1, 0, 0},
         {0, 1, 1, 1, 1, 1, 1, 0},
         {0, 1, 1, 0, 0, 1, 1, 0},
         {0, 0, 0, 0, 1, 1, 1, 0},
         {0, 0, 0, 1, 1, 1, 0, 0},
         {0, 0, 1, 1, 1, 0, 0, 0},
         {0, 1, 1, 1, 1, 1, 1, 0},
         {0, 1, 1, 1, 1, 1, 1, 0}},
        {// Symbol '3'
         {0, 0, 1, 1, 1, 1, 0, 0},
         {0, 1, 1, 1, 1, 1, 1, 0},
         {0, 0, 0, 0, 0, 1, 1, 0},
         {0, 0, 1, 1, 1, 1, 0, 0},
         {0, 0, 1, 1, 1, 1, 0, 0},
         {0, 0, 0, 0, 0, 1, 1, 0},
         {0, 1, 1, 1, 1, 1, 1, 0},
         {0, 0, 1, 1, 1, 1, 0, 0}},
        {// Symbol 'E' (End)
         {0, 0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0, 0},
         {1, 1, 1, 0, 0, 0, 0, 1},
         {1, 0, 0, 0, 0, 0, 0, 1},
         {1, 1, 0, 0, 1, 0, 0, 1},
         {1, 0, 0, 1, 0, 1, 1, 0},
         {1, 1, 1, 1, 0, 1, 0, 1},
         {0, 0, 0, 0, 0, 0, 0, 0}}};

    // Map the input character to the symbol index
    int symbolIndex = -1;
    switch (input)
    {
    case '1':
        symbolIndex = 0;
        break;
    case '2':
        symbolIndex = 1;
        break;
    case '3':
        symbolIndex = 2;
        break;
    case 'E':
        symbolIndex = 3;
        break;
    default:
        break;
    }

    // Display the symbol on the LED matrix
    LM.Symbol(symbols[symbolIndex]);
    delay(duration);
}

/**
 * Updates the position or rotation of the current shape.
 * @param mode - Movement or rotation mode:
 *               -1: Left, 1: Right, 0: Down, 3: Rotate
 */
void alterShape(int mode)
{
    int newCoordinates[4][2]; // Temporary coordinates for validation

    // Calculate the new positions
    for (int i = 0; i < 4; i++)
    {
        newCoordinates[i][0] = currentShape.coordinates[i][0];
        newCoordinates[i][1] = currentShape.coordinates[i][1];

        if (mode == -1) // Move left
            newCoordinates[i][1] -= 1;
        else if (mode == 1) // Move right
            newCoordinates[i][1] += 1;
        else if (mode == 0) // Move down
            newCoordinates[i][0] += 1;
        else if (mode == 3) // Rotate
        {
            int y = currentShape.coordinates[0][0];
            int x = currentShape.coordinates[0][1];
            newCoordinates[i][0] = y - (currentShape.coordinates[i][1] - x);
            newCoordinates[i][1] = x + (currentShape.coordinates[i][0] - y);
        }
    }

    // Validate the new positions
    bool validMove = true;
    for (int i = 0; i < 4; i++)
    {
        int y = newCoordinates[i][0];
        int x = newCoordinates[i][1];

        if (y >= CHECKING_HEIGHT || x < 0 || x >= GRID_WIDTH || grid.stable[y][x])
        {
            validMove = false;
            break;
        }
    }

    // Apply the new positions if valid
    if (validMove)
    {
        memcpy(currentShape.coordinates, newCoordinates, sizeof(newCoordinates));

        if (mode == 0) // Handle locking when moving down
        {
            for (int i = 0; i < 4; i++)
            {
                if (currentShape.coordinates[i][0] == CHECKING_HEIGHT - 1 ||
                    grid.stable[currentShape.coordinates[i][0] + 1][currentShape.coordinates[i][1]])
                {
                    // Lock the shape into the stable grid
                    for (int j = 0; j < 4; j++)
                        grid.stable[currentShape.coordinates[j][0]][currentShape.coordinates[j][1]] = 1;

                    currentShape.active = false; // Deactivate the current shape
                    scanAndClearGrid();          // Check for and clear full rows
                    break;
                }
            }
        }
    }
}

/**
 * Generates a new random Tetrimino at the top of the grid.
 */
void genShape()
{
    if (!currentShape.active)
    {
        const int shapes[7][4][2] = {
            {{0, 0}, {0, 1}, {1, 0}, {1, 1}},  // O shape
            {{0, 0}, {0, 1}, {0, 2}, {0, 3}},  // I shape
            {{0, 0}, {1, 0}, {1, 1}, {1, 2}},  // J shape
            {{0, 0}, {1, 0}, {0, 1}, {0, 2}},  // L shape
            {{0, 0}, {0, 1}, {1, 1}, {1, 2}},  // S shape
            {{0, 0}, {0, 1}, {1, 0}, {1, -1}}, // Z shape
            {{0, 0}, {0, 1}, {1, 0}, {0, -1}}  // T shape
        };

        // Randomly select a shape and initialize it
        int randomIndex = random(0, 7);
        memcpy(currentShape.shape, shapes[randomIndex], sizeof(currentShape.shape));

        for (int i = 0; i < 4; i++)
        {
            currentShape.coordinates[i][0] = currentShape.shape[i][0] + 1;
            currentShape.coordinates[i][1] = currentShape.shape[i][1] + GRID_WIDTH / 2;
        }

        currentShape.active = true;
    }
}

/**
 * Clears a specific row in the stable memory.
 * @param row - The row to be cleared.
 */
void clearRow(int row)
{
    memset(grid.stable[row], 0, sizeof(grid.stable[row]));
}

/**
 * Shifts all rows down starting from a specific row.
 * @param startRow - The row to start shifting down from.
 */
void shiftRowsDown(int startRow)
{
    for (int i = startRow; i > 0; i--)
    {
        memcpy(grid.stable[i], grid.stable[i - 1], sizeof(grid.stable[i]));
    }
}

/**
 * Scans the grid for full rows, clears them, and updates the score.
 */
void scanAndClearGrid()
{
    // Check for overflow in the illegal zone (top 3 rows)
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            if (grid.stable[i][j] == 1)
            {
                end = true; // Game over
                return;
            }
        }
    }

    int clearedRows = 0;

    // Scan from bottom to top to find and clear full rows
    for (int i = CHECKING_HEIGHT - 1; i >= 3; i--)
    {
        bool isFull = true;
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            if (grid.stable[i][j] == 0)
            {
                isFull = false;
                break;
            }
        }

        if (isFull)
        {
            clearedRows++;
            clearRow(i); // Clear this row

            // Shift all rows above down by one

            for (int k = i; k > 0; k--)
            {
                memcpy(grid.stable[k], grid.stable[k - 1], sizeof(grid.stable[k]));
            }

            // Reset the top row to empty
            memset(grid.stable[0], 0, sizeof(grid.stable[0]));

            // Since we shifted everything down, we must re-check this row
            i++;
        }
    }

    // Update the score based on the number of cleared rows
    if (clearedRows > 0)
    {
        // Tetris standard scoring system
        int points[] = {0, 40, 100, 300, 1200};
        score += points[clearedRows];
        // Cap the score at 9999
        if (score > 9999)
        {
            score = 9999; // Cap the score at 9999
        }
    }
}

/**
 * Checks for user inputs and triggers the appropriate actions.
 */
void checkInput()
{
    // Read the current state of the rotation button

    bool currentRotateState = digitalRead(ROTATE_PIN);

    // Detect rising edge: from LOW to HIGH

    if (currentRotateState == HIGH && previousRotateState == LOW)

    {

        alterShape(3); // Rotate once
    }

    // Update the previous state

    previousRotateState = currentRotateState;

    // // Other button inputs (no changes needed)
    int Xvalue = analogRead(LEFT_OR_RIGHT_PIN);
    int Yvalue = analogRead(UP_OR_DOWN_PIN);

    if (Xvalue > 900)
    {
        alterShape(-1); // Move Left
    }

    if (Xvalue < 100)
    {
        alterShape(1); // Move Right
    }

    if (Yvalue > 900)
    {
        alterShape(0); // Move Down
    }
}

/**
 * Updates the display memory and sends it to the LED matrices.
 */
void gatherAndDisplay()
{

    if (!end)
        // Copy the stable grid into the display grid
        memcpy(grid.display, grid.stable, sizeof(grid.stable));
    memset(grid.top, 0, sizeof(grid.top));
    memset(grid.bottom, 0, sizeof(grid.bottom));

    // Add the active shape to the display grid
    for (int i = 0; i < 4; i++)
    {
        grid.display[currentShape.coordinates[i][0]][currentShape.coordinates[i][1]] = 1;
    }

    // Update the top and bottom matrix displays
    for (int i = 0; i < GRID_WIDTH; i++)
    {
        memcpy(grid.top[i], &grid.display[i + 2][0], sizeof(grid.top[i]));                    // Top matrix
        memcpy(grid.bottom[i], &grid.display[i + GRID_WIDTH + 2][0], sizeof(grid.bottom[i])); // Bottom matrix
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            if (grid.display[i + 2][j] == 1)
                grid.top[i] |= 1 << j;

            if (grid.display[i + GRID_WIDTH + 2][j] == 1)
                grid.bottom[i] |= 1 << j;
        }
    }

    // Send updated data to the LED matrices
    LMtop.Symbol(grid.top, 0);
    LMbot.Symbol(grid.bottom, 1);
}

/**
 * Displays the "end game" animation.
 */
void showEndAnimation()
{
    for (int i = CHECKING_HEIGHT; i > 0; i--)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            grid.display[i][j] = 0; // Clear rows one by one
        }
        gatherAndDisplay();
        delay(50); // Small delay for animation effect
    }
}

/**
 * Resets the game state.
 */
void resetGame()
{
    grid.clearStable();
    grid.clearDisplay();
    memset(grid.top, 0, sizeof(grid.top));
    memset(grid.bottom, 0, sizeof(grid.bottom));
    score = 0;
    end = false;
    ended = false;
    currentShape.active = false;

    Serial.println("Game Reset!");
}

/**
 * Updates the 7-segment display to reflect the current score.
 * Handles leading zeros by blanking unused digits.
 */
void updateScoreDisplay()
{
    // Temporary variable to hold the score for manipulation
    int tempScore = score;

    // Step 1: Extract digits from right to left (units, tens, hundreds, thousands)
    for (int i = 3; i >= 0; i--)
    {
        // Get the last digit of the score
        scoreNum[i] = tempScore % 10;

        // Remove the last digit
        tempScore /= 10;
    }

    // Step 2: Replace leading zeros with -1 to blank the corresponding digits
    bool leadingZero = true; // Flag to detect leading zeros
    for (int i = 0; i < 4; i++)
    {
        if (scoreNum[i] == 0 && leadingZero)
        {
            // Replace leading zero with -1 (blank the digit)
            scoreNum[i] = -1;
        }
        else
        {
            // Leading zeros are no longer applicable after encountering the first non-zero digit
            leadingZero = false;
        }
    }

    // Step 3: Update the 7-segment display with the formatted digits
    dis.scan(scoreNum); // Refresh the display with the updated score
}

/// --- Arduino Setup and Main Loop ---

/**
 * Arduino setup function, initializes pins and the game.
 */
void setup()
{
    // Initialize control pins as input
    pinMode(ROTATE_PIN, INPUT);

    randomSeed(analogRead(21)); // Seed the random number generator
}

/**
 * Arduino main loop function, runs the game logic continuously.
 */
void loop()
{
    if (ended)
    {
        ShowSymbol(LMbot, 'E', 2000); // Show the 'E' symbol for 2000 ms
    }
    else if (!end)
    {
        genShape(); // Generate a new shape if needed

        // Handle automatic downward movement

        if (millis() - prev >= interval)
        {
            prev = millis();
            alterShape(0);
        }

        // Handle input timing here instead of inside checkInput()
        if (millis() - inputprev >= inputinterval)
        {
            inputprev = millis();
            checkInput(); // Now checkInput() just reads and acts on inputs
        }
        gatherAndDisplay();
    }
    else
    {
        showEndAnimation(); // Display the end animation
        ended = true;       // Mark the end animation as shown
    }
}
