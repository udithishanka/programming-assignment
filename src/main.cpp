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
#define BUZZER_PIN 6
#define SOUND_DURATION 200

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
bool foodOnScreen = false; 
int foodType = 0;

unsigned long foodAppearTime = 0;
bool timerActive = false;
bool isLevelThree = false; 

int obstacleX[] = {130,140, 150, 160, 170, 180, 130, 130, 130, 140, 150, 160, 170, 180, 180, 180, 180, 170, 160, 150, 140, 130 };
int obstacleY[] = {50, 50, 50, 50, 50, 50, 60, 70, 80, 80, 80, 80, 80, 80, 90, 100, 110, 110, 110, 110, 110, 110, 110};
                // 13  14  15  16  17  18  13  13  13  14  15  16  17  18  18  18   18   17   16   15   14
int obstacleLength = sizeof(obstacleX) / sizeof(obstacleX[0]);

void playSound(int frequency) {
    tone(BUZZER_PIN, frequency); // Play the sound
    delay(SOUND_DURATION); // Wait for the duration
    noTone(BUZZER_PIN); // Stop the sound
}

void drawBlock(int x, int y, uint16_t color) {
    tft.fillRect(x, y, SNAKE_SIZE, SNAKE_SIZE, color);
}

void generateFood() {
    if (!foodOnScreen) {  // Only generate food if there is none currently on the screen
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

            // Ensure food does not spawn too close to obstacles or barriers (if applicable)
            if (obstacleCreated) {
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

        // Determine the type of food based on the level
        if (level >= 4) {
            foodType = random(2); // Randomly choose between normal (0) and red food (1)
        } else {
            foodType = 0; // Normal food for Levels 1 to 3
        }

        // Draw the appropriate food color
        if (foodType == 0) {
            drawBlock(foodX, foodY, ILI9341_ORANGE); // Normal food color
        } else {
            drawBlock(foodX, foodY, ILI9341_RED); // Red food color
        }

        foodOnScreen = true;  // Mark that food is on the screen
        foodAppearTime = millis(); // Set time when food is generated

        // Start timer only if at Level 3
        if (isLevelThree) {
            timerActive = true; // Activate the timer for Level 3
        }
    }
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

void readJoystick() {
    int xVal = analogRead(JOY_HORIZ) - 512;
    int yVal = analogRead(JOY_VERT) - 512;

    if (abs(xVal) > abs(yVal)) {
        if (xVal > 0 && direction != 3) direction = 3; // Move right
        if (xVal < 0 && direction != 1) direction = 1; // Move left
    } else {
        if (yVal > 0 && direction != 0) direction = 0; // Move down
        if (yVal < 0 && direction != 2) direction = 2; // Move up
    }
    // delay(50);  // Short delay to prevent excessive polling
}

void drawObstacle() {
    for (int i = 0; i < obstacleLength; i++) {
        drawBlock(obstacleX[i], obstacleY[i], ILI9341_BLUE);
    }
}

void playGoodFoodSound(void) {
  tone(BUZZER_PIN, 2058, 100);  // Play a 1000 Hz tone for 100ms
}

void playBadFoodSound(void) {
  tone(BUZZER_PIN, 456, 100);   // Play a 500 Hz tone for 100ms
}

void playGameOverSound(void) {
  tone(BUZZER_PIN, 258, 1000);  // Play a 250 Hz tone for 1 second
}

void displayGameover(){
    playGameOverSound();  // Play a sound for game over
    tft.fillScreen(ILI9341_RED);
        tft.setCursor(60, 160);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(3);
        tft.print("GAME OVER!");
        while (true);  // Stop the game
}

void setup() {
    Serial.begin(9600);
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(ILI9341_BLACK);
    pinMode(JOY_SEL, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);

    // attachInterrupt(digitalPinToInterrupt(JOY_SEL), buttonPress, FALLING);

    // Initialize snake
    initSnake();
    generateFood();

    // Directly update the score and level in the setup
    // Update Score
    tft.fillRect(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, 20, ILI9341_BLACK); // Clear specific area
    tft.setCursor(10, SCREEN_HEIGHT - 20 + 35);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.print("Score: ");
    tft.print(score);

    // Update Level
    
    tft.setCursor(SCREEN_WIDTH / 3 + 10, SCREEN_HEIGHT - 20 + 35);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("Level: ");
    tft.print(level);
}

void countDown(){
    if (timerActive && isLevelThree) {
        int remainingTime = 5 - (millis() - foodAppearTime) / 1000;  // Calculate remaining time in seconds
        if (remainingTime < 0) remainingTime = 0;  // Ensure the timer doesn't show negative values
        
        // Clear previous timer and update it
        tft.fillRect(SCREEN_WIDTH / 3 * 2 + 10, SCREEN_HEIGHT - 20 + 35, 80, 20, ILI9341_BLACK);  // Clear previous timer area
        tft.setCursor(SCREEN_WIDTH / 3 * 2 + 10, SCREEN_HEIGHT - 20 + 35);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(2);
        tft.print("Time: ");
        tft.print(remainingTime);

        // If the timer hits 0, remove the food and reset.
        if (remainingTime <= 0) {
            drawBlock(foodX, foodY, ILI9341_BLACK);  // Remove the food if the timer hits 0.
            timerActive = false;  // Deactivate the timer to avoid repeated removal.
            foodOnScreen = false;  // Mark that no food is on the screen
            generateFood();  // Generate new food after the previous food disappears
        }
    }
}

void loop() {
    readJoystick(); // Read joystick every loop iteration

    // Clear the snake tail
    drawBlock(snakeX[snakeLength - 1], snakeY[snakeLength - 1], ILI9341_BLACK);
    
    updateSnake();

    // Update the countdown timer in every loop iteration
    countDown();  // Ensure the countdown updates continuously

    // Check for food collision.
    if (checkFoodCollision()) {
        if (foodType == 0) { // Normal food
            score++;
            snakeLength++;
            foodsEaten++;
            playGoodFoodSound();  // Play a sound for eating good food
        } else if (foodType == 1) { // Red food
            score--; // Reduce score
            playBadFoodSound(); // Play sound for eating bad food
        }

        foodOnScreen = false;  // Mark that food is no longer on the screen
        generateFood();  // Generate new food

        // Clear and update score display
        tft.fillRect(85 , SCREEN_HEIGHT - 20 +35 ,50 ,20 , ILI9341_BLACK); // Clear specific area
        tft.setCursor(10 , SCREEN_HEIGHT -20 +35);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(2);
        tft.print("Score: ");
        tft.print(score);

        // Update level
        level = score / maxFood + 1;
        if (level >= 3) {
            isLevelThree = true;
        }

        // Introduce Level 4
        if (level >= 4) {
            // Additional logic for level 4 can be implemented here, if needed
        }

        // Clear and update level display
        tft.fillRect(SCREEN_WIDTH / 3 + 80, SCREEN_HEIGHT - 20 + 35 ,80 ,20 , ILI9341_BLACK); // Clear specific area
        tft.setCursor(SCREEN_WIDTH / 3 + 10, SCREEN_HEIGHT - 20 +35);
        tft.print("Level: ");
        tft.print(level);

        if (snakeSpeed > 50) 
            snakeSpeed -= 10;

        // Create the obstacle after two pieces of food are eaten.
        if (foodsEaten >= 2 && !obstacleCreated) {
            drawObstacle();
            obstacleCreated = true;
        }
    }

    // Check for self-collision or obstacle collision.
    if (checkSelfCollision() || checkObstacleCollision()) {
        displayGameover(); 
    }

    // Draw the snake head.
    drawBlock(snakeX[0], snakeY[0], ILI9341_GREEN);

    // Slow down the game based on the speed of the Snake.
    delay(snakeSpeed);
}