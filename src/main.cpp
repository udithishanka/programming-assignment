#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

// Pin definitions for ILI9341 display
#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Joystick pin definitions
#define JOY_VERT  A0 // Vertical movement (Y-axis)
#define JOY_HORIZ A1 // Horizontal movement (X-axis)
#define JOY_SEL   7  // Joystick button

// Snake parameters
#define SNAKE_SIZE 10
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 190

// Initial snake parameters
int snakeX[100], snakeY[100];
int snakeLength = 3;
int direction = 1;  // 0 - UP, 1 - RIGHT, 2 - DOWN, 3 - LEFT
int foodX, foodY;
int score = 0;
int level = 1;
int maxFood = 2;
int snakeSpeed = 200;

// Joystick deadzone
int deadzone = 200;

// Function to display a bootup introduction screen
void showIntroScreen() {
    tft.fillScreen(ILI9341_BLACK);  // Clear the screen
    
    // Draw a border around the screen
    uint16_t borderColor = ILI9341_BLUE;
    tft.drawRect(5, 5, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 20, borderColor);  // Outer border
    tft.drawRect(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30, ILI9341_WHITE);  // Inner border

    // Display "Snake Game" with a fade-in effect and scaling animation
    for (int size = 4; size <= 4; size++) {
        for (int i = 0; i < 25; i += 5) {
            tft.fillScreen(ILI9341_BLACK);  // Clear screen for fade effect
            tft.drawRect(5, 5, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 20, borderColor);  // Re-draw border
            tft.drawRect(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30, ILI9341_WHITE);  // Re-draw inner border
            
            // Display the text in the center with color and size animations
            tft.setCursor(40, 100);
            tft.setTextColor(tft.color565(i, 255 - i, i));  // Color fade
            tft.setTextSize(size);  // Text size scaling
            tft.print("Snake Game");
            delay(50);  // Control speed of fade-in
        }
    }

    // Add a glowing snake icon effect (using simple blocks)
    for (int j = 0; j < 5; j++) {
        tft.fillRect(120 + j * 20, 50, SNAKE_SIZE, SNAKE_SIZE, ILI9341_GREEN);  // Draw snake blocks
        delay(100);  // Simulate snake animation
    }

    delay(1000);  // Hold the screen for a moment

    // Clear the screen and draw a solid black rectangle for the main text
    tft.fillRect(0, 120, SCREEN_WIDTH, 40, ILI9341_BLACK);  // Create a black box for text
    tft.setCursor(50, 130);
    tft.setTextColor(ILI9341_YELLOW);
    tft.setTextSize(2);
    tft.print("Press Button to Start");

    // Add some decorative horizontal lines
    for (int linePos = 40; linePos < 300; linePos += 30) {
        tft.drawFastHLine(linePos, 180, 20, ILI9341_RED);  // Horizontal lines for design
        delay(50);  // Animation effect for drawing lines
    }

    // Wait for button press to start the game
    while (digitalRead(JOY_SEL) == LOW);  // Wait until the joystick button is pressed
    delay(200);  // Debounce delay
}


void drawBlock(int x, int y, uint16_t color) {
    tft.fillRect(x, y, SNAKE_SIZE, SNAKE_SIZE, color);
}

void generateFood() {
    drawBlock(foodX, foodY, ILI9341_BLACK);
    bool validPosition = false;

    while (!validPosition) {
        foodX = (random(SCREEN_WIDTH / SNAKE_SIZE)) * SNAKE_SIZE;
        foodY = (random(SCREEN_HEIGHT / SNAKE_SIZE)) * SNAKE_SIZE;

        validPosition = true;
        for (int i = 0; i < snakeLength; i++) {
            if (snakeX[i] == foodX && snakeY[i] == foodY) {
                validPosition = false;
                break;
            }
        }
    }

    drawBlock(foodX, foodY, ILI9341_RED);
}

void initSnake() {
    snakeX[0] = 120;
    snakeY[0] = 160;
    snakeX[1] = 110;
    snakeY[1] = 160;
    snakeX[2] = 100;
    snakeY[2] = 160;
}

void updateSnake() {
    for (int i = snakeLength - 1; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }

    if (direction == 0) snakeY[0] -= SNAKE_SIZE;  // UP
    else if (direction == 1) snakeX[0] += SNAKE_SIZE;  // RIGHT
    else if (direction == 2) snakeY[0] += SNAKE_SIZE;  // DOWN
    else if (direction == 3) snakeX[0] -= SNAKE_SIZE;  // LEFT

    // Edge wrapping logic
    if (snakeX[0] >= SCREEN_WIDTH)  snakeX[0] = 0;
    if (snakeX[0] < 0)              snakeX[0] = SCREEN_WIDTH - SNAKE_SIZE;
    if (snakeY[0] >= SCREEN_HEIGHT) snakeY[0] = 0;
    if (snakeY[0] < 0)              snakeY[0] = SCREEN_HEIGHT - SNAKE_SIZE;
}

bool checkFoodCollision() {
    return (snakeX[0] == foodX && snakeY[0] == foodY);
}

bool checkSelfCollision() {
    for (int i = 1; i < snakeLength; i++) {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
            return true;
        }
    }
    return false;
}

// Interrupt service routine for joystick button
void buttonPress() {
    // This is where you could add logic to change direction
    // For now, just print a message for testing
    Serial.println("Button pressed");
}

// Read joystick input for movement
void readJoystick() {
    int xVal = analogRead(JOY_HORIZ) - 512;
    int yVal = analogRead(JOY_VERT) - 512;

    if (abs(xVal) > deadzone || abs(yVal) > deadzone) {
        if (abs(xVal) > abs(yVal)) {
            if (xVal > deadzone && direction != 3) direction = 3; // Move right
            if (xVal < -deadzone && direction != 1) direction = 1; // Move left
        } else {
            if (yVal > deadzone && direction != 0) direction = 0; // Move down
            if (yVal < -deadzone && direction != 2) direction = 2; // Move up
        }
    }
}

void setup() {
    Serial.begin(9600);
    tft.begin();
    tft.setRotation(3);

    // Call the intro screen function
    showIntroScreen();

    tft.fillScreen(ILI9341_BLACK);  // Clear screen for the game
    pinMode(JOY_SEL, INPUT_PULLUP);

    // Attach interrupt for joystick button press
    attachInterrupt(digitalPinToInterrupt(JOY_SEL), buttonPress, FALLING);

    // Initialize snake
    initSnake();
    generateFood();

    tft.fillRect(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, 20, ILI9341_BLACK);
    tft.setCursor(10, SCREEN_HEIGHT - 20 + 35);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.print("Score: ");
    tft.print(score);
    tft.setCursor(SCREEN_WIDTH / 2 + 10, SCREEN_HEIGHT - 20 + 35);
    tft.print("Level: ");
    tft.print(level);
}

void loop() {
    readJoystick(); // Read joystick every loop iteration

    // Clear the snake tail
    drawBlock(snakeX[snakeLength - 1], snakeY[snakeLength - 1], ILI9341_BLACK);
    updateSnake();

    // Check for food collision
    if (checkFoodCollision()) {
        score++;
        snakeLength++;
        generateFood();

        tft.fillRect(85, SCREEN_HEIGHT - 20 + 35, 50, 20, ILI9341_BLACK);
        tft.fillRect(SCREEN_WIDTH / 2 + 80, SCREEN_HEIGHT - 20 + 35, 30, 20, ILI9341_BLACK);

        tft.setCursor(10, SCREEN_HEIGHT - 20 + 35);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(2);
        tft.print("Score: ");
        tft.setCursor(85, SCREEN_HEIGHT - 20 + 35);
        tft.print(score);

        tft.setCursor(SCREEN_WIDTH / 2 + 10, SCREEN_HEIGHT - 20 + 35);
        tft.print("Level: ");
        level = score / maxFood + 1;
        tft.setCursor(SCREEN_WIDTH / 2 + 80, SCREEN_HEIGHT - 20 + 35);
        tft.print(level);

        if (snakeSpeed > 50) snakeSpeed -= 10;
    }

    if (checkSelfCollision()) {
        tft.fillScreen(ILI9341_RED);
        tft.setCursor(60, 160);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(3);
        tft.print("Game Over");
        while (true); // Halt the game
    }

    // Draw the snake head
    drawBlock(snakeX[0], snakeY[0], ILI9341_GREEN);
    
    delay(snakeSpeed); // Control game speed
}
