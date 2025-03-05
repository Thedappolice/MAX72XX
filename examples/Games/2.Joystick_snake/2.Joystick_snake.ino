// this program uses Arduino Nano

#include <LEDMatrixChip.h>

// Define SPI communication pins
#define DATA_PIN 2
#define CHIP_SELECT_PIN 3
#define CLOCK_PIN 4

LEDMatrixChip LM(CHIP_SELECT_PIN, CLOCK_PIN, DATA_PIN, 1, 0);

unsigned long interval = 500; // Refresh interval in milliseconds

// Snake initial positions and state
int length = 3;
int body[64][2] = {
    {4, 4},
    {5, 4},
    {6, 4},
};
int head[2] = {4, 4};

bool foodExists = false;

// Memory to track snake and food positions
int memory[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 2, 0, 0, 0},
    {0, 0, 0, 0, 3, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}};

uint8_t displayBytes[8] = {0};

char direction = 'u'; // Initial direction ('u' for up)
bool end = false;     // Game end condition
bool win = false;     // Game win flag

void ShowSymbol(char input, unsigned long duration = 0)
{
    uint8_t display[8] = {0};

    uint8_t End[8] = {
        0b00000000,
        0b00000000,
        0b11100001,
        0b10000001,
        0b11001001,
        0b10010110,
        0b11110101,
        0b00000000};

    uint8_t Star[8] = {
        0b00010000,
        0b00010000,
        0b00111000,
        0b11111110,
        0b01111100,
        0b00111000,
        0b01100110,
        0b11000011};

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
    switch (input)
    {
    case '1':
        memcpy(display, N1, 8 * sizeof(uint8_t));
        break;
    case '2':
        memcpy(display, N2, 8 * sizeof(uint8_t));
        break;
    case '3':
        memcpy(display, N3, 8 * sizeof(uint8_t));
        break;
    case 'S':
        memcpy(display, Star, 8 * sizeof(uint8_t));
        break;
    case 'E':
        memcpy(display, End, 8 * sizeof(uint8_t));
        break;
    }

    LM.Symbol(display);
    delay(duration > 0 ? duration : 1000);
};

// Function to check user input direction
void checkdInput()
{
    char Command = direction; // Default to current direction

    char oppDir = 'u'; // Initialize opposite direction with default value

    if (analogRead(A7) > 768)
    {
        Command = 'l'; // Left
        oppDir = 'r';
    }
    if (analogRead(A7) < 256)
    {
        Command = 'r'; // Right
        oppDir = 'l';
    }
    if (analogRead(A6) > 768)
    {
        Command = 'd'; // Down
        oppDir = 'u';
    }
    if (analogRead(A6) < 256)
    {
        Command = 'u'; // Up
        oppDir = 'd';
    }

    // Update direction if it's not opposite to the current direction
    if (Command != oppDir)
    {
        direction = Command;
    }
};

// Function to generate food at random empty locations on the board
void generateFood()
{
    if (!foodExists)
    {
        int count = 0;
        int Mayfood[64][2] = {{0}};
        // Iterate through the board to find empty spots for food placement
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (memory[i][j] == 0)
                {
                    // Found an empty spot
                    Mayfood[count][0] = i;
                    Mayfood[count][1] = j;
                    count++;
                }
            }
        }

        // If there are empty spots, randomly place food
        if (count > 0)
        {
            int randIndex = random(0, count);
            memory[Mayfood[randIndex][0]][Mayfood[randIndex][1]] = -1; // Set food at this location
            foodExists = true;
        }
        else
        {
            // No empty spots left, game ends
            end = true;
            win = true; // This could be reconsidered depending on game logic
            return;
        }
    }
};

// Function to update snake's position and check for collisions
void refreshSnake()
{
    // Update head position based on current direction
    switch (direction)
    {
    case 'u':
        if (head[0] - 1 < 0)
        {
            end = true; // Hit top wall, game over
            return;
        }
        else
        {
            head[0] -= 1;
        }
        break;
    case 'd':
        if (head[0] + 1 > 7)
        {
            end = true; // Hit bottom wall, game over
            return;
        }
        else
        {
            head[0] += 1;
        }
        break;
    case 'l':
        if (head[1] - 1 < 0)
        {
            end = true; // Hit left wall, game over
            return;
        }
        else
        {
            head[1] -= 1;
        }
        break;
    case 'r':
        if (head[1] + 1 > 7)
        {
            end = true; // Hit right wall, game over
            return;
        }
        else
        {
            head[1] += 1;
        }
        break;
    }

    // Check if snake hits itself
    for (int i = 0; i < length; i++)
    {
        if (body[i][0] == head[0] && body[i][1] == head[1])
        {
            end = true; // Snake collided with itself, game over
            return;
        }
    }

    // Check if snake eats the food
    if (memory[head[0]][head[1]] == -1)
    {
        length++;           // Increase snake length immediately
        foodExists = false; // Food eaten, generate new food
    }

    // Shift body positions back one space
    for (int i = length - 1; i > 0; i--)
    {
        body[i][0] = body[i - 1][0];
        body[i][1] = body[i - 1][1];
    }

    // Update body with new head position
    body[0][0] = head[0];
    body[0][1] = head[1];

    // Clear memory
    memset(memory, 0, 8 * 8 * sizeof(int)); // Clear the entire memory array

    // Set new head position in memory
    memory[head[0]][head[1]] = 1;

    // Update memory with body positions
    for (int i = 0; i < length; i++)
    {
        memory[body[i][0]][body[i][1]] = 1;
    }
};

// Function to update display based on memory
void display(unsigned long duration = 0)
{
    // Update display based on memory
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (memory[i][j] == 1 || memory[i][j] == -1)
            {
                displayBytes[i] |= (1 << j);
            }
        }
    }

    LM.Symbol(displayBytes);
    delay(duration > 0 ? duration : 1000);
};

// Function to handle end of game animation
void ending()
{
    // Blink display to indicate end of game
    for (int i = 0; i < 10; i++)
    {
        LM.Symbol(displayBytes);
        delay(100);
    }

    for (int c = length; c > 0; c--)
    {
        memory[body[c][0]][body[c][1]] = 0;
        LM.Symbol(displayBytes);
        delay(250);
    }

    // Display end game message
    if (!win)
    {
        ShowSymbol('E', 10000); // Display loss animation
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            ShowSymbol('S', 500); // Display win animation (blink)
        }
    }

    exit(0); // Exit the program after animation
};

void setup()
{
    // Initialize LEDMatrix with countdown
    ShowSymbol('3'); // Countdown 3
    ShowSymbol('2'); // Countdown 2
    ShowSymbol('1'); // Countdown 1

    randomSeed(analogRead(A4)); // Initialize random seed

    // Blink starting animation
    for (int i = 0; i < 10; i++)
    {
        display();
        delay(200);
    }
}

void loop()
{
    // Game loop
    if (!end)
    {
        generateFood();    // Generate new food if needed
        checkdInput();     // Check user input direction
        refreshSnake();    // Update snake position and check collisions
        display(interval); // Update display based on memory
    }
    else
    {
        ending(); // Handle end of game
    }
}
