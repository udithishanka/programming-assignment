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
int foodsEaten = 0;
bool obstacleCreated = false;

// Joystick deadzone
int deadzone = 200;


int obstacleX[] = {130,140, 150, 160, 170, 180, 130, 130, 130, 140, 150, 160, 170, 180, 180, 180, 180, 170, 160, 150, 140, 130 };
int obstacleY[] = {50, 50, 50, 50, 50, 50, 60, 70, 80, 80, 80, 80, 80, 80, 90, 100, 110, 110, 110, 110, 110, 110, 110};
                // 13  14  15  16  17  18  13  13  13  14  15  16  17  18  18  18   18   17   16   15   14
int obstacleLength = sizeof(obstacleX) / sizeof(obstacleX[0]);





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
            if (snakeX[i] == foodX && snakeY[i] == foodY && foodX == obstacleX[i] + 10 && foodY == obstacleY[i] + 10 && foodX == obstacleX[i] - 10 && foodY == obstacleY[i] - 10) {
                validPosition = false;
                break;
            }
        }

        // Ensure food does not spawn inside the obstacle
        if (obstacleCreated) {
    validPosition = true; // Initialize validPosition to true

    for (int i = 0; i < obstacleLength; i++) {
        // Check if food spawns exactly at any obstacle position
        if (foodX == obstacleX[i] && foodY == obstacleY[i]) {
            validPosition = false; // Food is on top of an obstacle
            break;
        }
        
        // Check if food spawns too close to the obstacle (assuming a margin)
        if (foodX >= obstacleX[i] - 10 && foodX <= obstacleX[i] + 10 &&
            foodY >= obstacleY[i] - 10 && foodY <= obstacleY[i] + 10) {
            validPosition = false; // Food is too close to an obstacle
            break;
        }
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

bool checkObstacleCollision() {
    if (!obstacleCreated) return false;

    for (int i = 0; i < obstacleLength; i++) {
        if (snakeX[0] == obstacleX[i] && snakeY[0] == obstacleY[i]) {
            return true;
        }
    }
    return false;
}

// Interrupt service routine for joystick button
void buttonPress() {
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

void drawObstacle() {
    for (int i = 0; i < obstacleLength; i++) {
        drawBlock(obstacleX[i], obstacleY[i], ILI9341_BLUE);
    }
}

void setup() {
    Serial.begin(9600);
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(ILI9341_BLACK);
    pinMode(JOY_SEL, INPUT_PULLUP);

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
        foodsEaten++;
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

        // Create the obstacle after two pieces of food are eaten
        if (foodsEaten >= 2 && !obstacleCreated) {
            drawObstacle();
            obstacleCreated = true;
        }
    }

    // Check for self-collision or obstacle collision
    if (checkSelfCollision() || checkObstacleCollision()) {
        tft.fillScreen(ILI9341_RED);
        tft.setCursor(60, 160);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(3);
        tft.print("GAME OVER!");
        while (true);  // Stop the game
    }

    // Draw the snake head
    drawBlock(snakeX[0], snakeY[0], ILI9341_GREEN);

    // Slow down the game based on the snake speed
    delay(snakeSpeed);
}
