#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <time.h>
#include <unistd.h> // for sleep() function

bool startTimer = false; // Flag to indicate if the timer should start
bool startTimerUpsideDown = false; // Flag to indicate if the timer should start
double startTime = 0.0;
double duration = 5.0; // Time duration in seconds
bool firstClick = false;
bool rightClick = false;
bool menuPause = false;
bool bonus3 = false;
bool bonusEarned = false;
bool selectMenu = true;
bool startGame = false;
bool victory = false;
bool defeat = false;
bool menuSelected = false;
int leftClick = 0;
double cursorX = 0.0;
double cursorY = 0.0;
int PV = 5;
int toWin = 0;
int typeBonus = 1;
int cptBonus = 1;

// Window dimensions
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
static const char WINDOW_TITLE[] = "The Light Corridor";
static float aspectRatio = 1.0;

/* Virtual windows space */
static const float GL_VIEW_SIZE = 20.;

// Corridor dimensions
const float CORRIDOR_WIDTH = 400.0f;
const float CORRIDOR_HEIGHT = 300.0f;
const float CORRIDOR_DEPTH = 800.0f;

// Color structure
typedef struct Color {
    float r;
    float g;
    float b;
    float a;
} Color;

// Racket structure
typedef struct Racket {
    float width;
    float height;
    Color color;
    float positionX;
    float positionY;
    float positionZ;
} Racket;

// Ball properties
float BALL_SPEED = 1.0f;

// Obstacle properties
float OBSTACLE_SPEED = 1.0f;
const int MAX_OBSTACLES = 10;

// Set the number of obstacles you want to draw
const int NUM_OBSTACLES = 3;

// Set the range of obstacle positions
const float MIN_OBSTACLE_Z = -800.0;
const float MAX_OBSTACLE_Z = -10.0;

typedef struct {
    float positionX;
    float positionY;
    float positionZ;
    float directionX;
    float directionY;
    float directionZ;
    float radius;
} Ball;

typedef struct {
    float positionX;
    float positionY;
    float positionZ;
    float width;
    float height;
} Obstacle;

// Set the initial ball position and direction
Ball *ball;

// Set the initial bonus position and direction
Ball *bonus;

// Set the initial obstacle position
Obstacle *obstacle;

// Initialize the racket
Racket *racket;

/* Error handling function */
void errorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW Error: %s\n", description);
}

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(path, &width, &height, &channels, 0);
    if (image) {
        GLenum format = 0; // Initialize format variable
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
        stbi_image_free(image);
	    glBindTexture(GL_TEXTURE_2D, 0);
    }
    else {
        fprintf(stderr, "Failed to load texture: %s\n", path);
        stbi_image_free(image);
    }

    return textureID;
}

// Set up lighting
void setupLighting()
{
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Set light properties
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // Set the light position
    GLfloat lightPosition[] = { 0.0f, 0.0f, -CORRIDOR_DEPTH / 2 + 100.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // Set material properties
    GLfloat materialAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat materialDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat materialShininess = 32.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);
}

// Draw the corridor
void drawCorridor()
{
    // Draw the corridor walls
    // Left wall
    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-CORRIDOR_WIDTH / 2, -CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glVertex3f(-CORRIDOR_WIDTH / 2, CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glVertex3f(-CORRIDOR_WIDTH / 2, CORRIDOR_HEIGHT / 2, CORRIDOR_DEPTH);
    glVertex3f(-CORRIDOR_WIDTH / 2, -CORRIDOR_HEIGHT / 2, CORRIDOR_DEPTH);
    glEnd();

    // Right wall
    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(CORRIDOR_WIDTH / 2, -CORRIDOR_HEIGHT / 2, CORRIDOR_DEPTH);
    glVertex3f(CORRIDOR_WIDTH / 2, CORRIDOR_HEIGHT / 2, CORRIDOR_DEPTH);
    glVertex3f(CORRIDOR_WIDTH / 2, CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glVertex3f(CORRIDOR_WIDTH / 2, -CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glEnd();

    // Draw the corridor floor
    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-CORRIDOR_WIDTH / 2, -CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glVertex3f(CORRIDOR_WIDTH / 2, -CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glVertex3f(CORRIDOR_WIDTH / 2, -CORRIDOR_HEIGHT / 2, CORRIDOR_DEPTH);
    glVertex3f(-CORRIDOR_WIDTH / 2, -CORRIDOR_HEIGHT / 2, CORRIDOR_DEPTH);
    glEnd();

    // Draw the corridor ceiling
    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-CORRIDOR_WIDTH / 2, CORRIDOR_HEIGHT / 2, CORRIDOR_DEPTH);
    glVertex3f(CORRIDOR_WIDTH / 2, CORRIDOR_HEIGHT / 2, CORRIDOR_DEPTH);
    glVertex3f(CORRIDOR_WIDTH / 2, CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glVertex3f(-CORRIDOR_WIDTH / 2, CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glEnd();

    // Draw the corridor back
    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-CORRIDOR_WIDTH / 2, CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glVertex3f(CORRIDOR_WIDTH / 2, CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glVertex3f(CORRIDOR_WIDTH / 2, -CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glVertex3f(-CORRIDOR_WIDTH / 2, -CORRIDOR_HEIGHT / 2, -CORRIDOR_DEPTH);
    glEnd();
}

void drawPVBarForm(){
    // Disable lighting temporarily
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Render PV bar form */
	glColor4f(1.0, 1.0, 1.0, 0.5); // Set color to white
    glBegin(GL_QUADS);
    glVertex2f(-160, 118); // Top-left vertex
    glVertex2f(-110, 118); // Top-right vertex
    glVertex2f(-110, 112); // Bottom-right vertex
    glVertex2f(-160, 112); // Bottom-left vertex
    glEnd();

	glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void drawPV(){
    // Disable lighting temporarily
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    /* Calculate the width of the PV bar based on the PV level */
    int barWidth = PV * -10; // Each PV level is 10 units wide

    /* Render PV bar */
    glColor4f(0.04,0.2,0.48,1);
    glBegin(GL_QUADS);
    glVertex2f(-160, 118); // Top-left vertex
    glVertex2f(-160-barWidth, 118); // Top-right vertex
    glVertex2f(-160-barWidth, 112); // Bottom-right vertex
    glVertex2f(-160, 112); // Bottom-left vertex
    glEnd();

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void updateBallPosition()
{
    if (!firstClick){
        // Get the normalized device coordinates of the cursor position
        double ndcX = (cursorX / WINDOW_WIDTH) * 2.0 - 1.0;
        double ndcY = ((WINDOW_HEIGHT - cursorY) / WINDOW_HEIGHT) * 2.0 - 1.0;

        // Map the normalized device coordinates to the virtual window space
        float ballX = ndcX * (GL_VIEW_SIZE / 2.0) * aspectRatio * 10;
        float ballY = ndcY * (GL_VIEW_SIZE / 2.0) * 10;

        ball->positionX = ballX;
        ball->positionY = ballY;
        ball->positionZ = -25.0 - ball->radius;
    }
    else if (bonus3){
        if (!rightClick){
            
        // Get the normalized device coordinates of the cursor position
        double ndcX = (cursorX / WINDOW_WIDTH) * 2.0 - 1.0;
        double ndcY = ((WINDOW_HEIGHT - cursorY) / WINDOW_HEIGHT) * 2.0 - 1.0;

        // Map the normalized device coordinates to the virtual window space
        float ballX = ndcX * (GL_VIEW_SIZE / 2.0) * aspectRatio * 10;
        float ballY = ndcY * (GL_VIEW_SIZE / 2.0) * 10;

        ball->positionX = ballX;
        ball->positionY = ballY;
        ball->positionZ = -25.0 - ball->radius;
        }
        else {
            bonus3 = false;
        }
    }
    else{
        ball->positionZ += ball->directionZ;
        ball->positionX += ball->directionX;
        ball->positionY += ball->directionY;
        rightClick = false;
    }
}

// Draw the ball
void drawBall(const Ball* ball)
{
    glPushMatrix();
    glTranslatef(ball->positionX, ball->positionY, ball->positionZ);
    glColor3f(1.0f, 1.0f, 1.0f);
    gluSphere(gluNewQuadric(), ball->radius, 20, 20);
    glPopMatrix();
}

// Draw the bonus
void drawBonus(const Ball* ball, int typeBonus)
{
    // Disable lighting temporarily
    glDisable(GL_LIGHTING);

    glPushMatrix();
    glTranslatef(ball->positionX, ball->positionY, ball->positionZ);
    if (typeBonus == 1){
        glColor3f(0.0f, 1.0f, 0.0f);
    }
    if (typeBonus == 2){
        glColor3f(0.0f, 0.0f, 1.0f);
    }
    if (typeBonus == 3){
        glColor3f(1.0f, 0.0f, 1.0f);
    }
    if (typeBonus == 4){
        glColor3f(1.0f, 0.0f, 0.0f);
    }
    if (typeBonus == 5){
        glColor3f(1.0f, 0.5f, 0.0f);
    }
    if (typeBonus == 6){
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    if (typeBonus == 7){
        glColor3f(1.0f, 1.0f, 0.0f);
    }
    gluSphere(gluNewQuadric(), ball->radius, 20, 20);
    glPopMatrix();
    
    // Re-enable lighting
    glEnable(GL_LIGHTING);
}

// Draw the obstacle
void drawObstacle(const Obstacle* obstacle, GLuint obstacleTextureID)
{
    // Disable lighting temporarily
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, obstacleTextureID);
    glTranslatef(obstacle->positionX, obstacle->positionY, obstacle->positionZ);
    //glColor3f(1.0f, 1.0f, 1.0f);
    glScalef(obstacle->width, obstacle->height, 1.0f);
    glBegin(GL_QUADS);
    glColor4f(1,1,1,1);
	glTexCoord3f(0.0f,0.0f,0.0f);
    glVertex3f(-0.5f, -0.5f, 0.0f);
	glTexCoord3f(1.0f,0.0f,0.0f);
    glVertex3f(0.5f, -0.5f, 0.0f);
	glTexCoord3f(1.0f,1.0f,0.0f);
    glVertex3f(0.5f, 0.5f, 0.0f);
	glTexCoord3f(0.0f,1.0f,0.0f);
    glVertex3f(-0.5f, 0.5f, 0.0f);
    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    // Re-enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void updateRacketPosition()
{
    // Get the normalized device coordinates of the cursor position
    double ndcX = (cursorX / WINDOW_WIDTH) * 2.0 - 1.0;
    double ndcY = ((WINDOW_HEIGHT - cursorY) / WINDOW_HEIGHT) * 2.0 - 1.0;

    // Map the normalized device coordinates to the virtual window space
    float racketX = ndcX * (GL_VIEW_SIZE / 2.0) * aspectRatio * 10;
    float racketY = ndcY * (GL_VIEW_SIZE / 2.0) * 10;

    if (startTimerUpsideDown){
        racket->positionX = -racketX;
        racket->positionY = -racketY;
    }else{
        racket->positionX = racketX;
        racket->positionY = racketY;
    }
}

// Draw the racket
void drawRacket(Racket* racket)
{
    glPushMatrix();
    glTranslatef(racket->positionX, racket->positionY, racket->positionZ);

    // Enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set racket color and transparency
    glColor4f(racket->color.r, racket->color.g, racket->color.b, racket->color.a);

    // Draw the racket rectangle
    glBegin(GL_QUADS);
    glVertex2f(-racket->width / 2, -racket->height / 2);
    glVertex2f(racket->width / 2, -racket->height / 2);
    glVertex2f(racket->width / 2, racket->height / 2);
    glVertex2f(-racket->width / 2, racket->height / 2);
    glEnd();

    // Disable transparency and enable writing to the depth buffer
    glDisable(GL_BLEND);

    glPopMatrix();
}

void onWindowResized(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / (float) height;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if( aspectRatio > 1)
	{
		gluOrtho2D(
		-GL_VIEW_SIZE / 2. * aspectRatio, GL_VIEW_SIZE / 2. * aspectRatio,
		-GL_VIEW_SIZE / 2., GL_VIEW_SIZE / 2.);
	}
	else
	{
		gluOrtho2D(
		-GL_VIEW_SIZE / 2., GL_VIEW_SIZE / 2.,
		-GL_VIEW_SIZE / 2. / aspectRatio, GL_VIEW_SIZE / 2. / aspectRatio);
	}
	glMatrixMode(GL_MODELVIEW);

}

// Keyboard input callback function
// Update the obstacle
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        if (menuPause){
            menuPause = false;
            // Move the ball
            BALL_SPEED = 1.0f;
            ball->directionX = BALL_SPEED;
            ball->directionY = BALL_SPEED;
            ball->directionZ = BALL_SPEED;
            // Move the obstacle
            OBSTACLE_SPEED = 1.0f;
            obstacle->positionZ += OBSTACLE_SPEED;
            bonus->positionZ += OBSTACLE_SPEED;
            // Delay for 1 second before checking again
            sleep(1);
        }
        else if (!menuPause){            
            menuPause = true;
            // Freeze the ball
            BALL_SPEED = 0.0f;
            ball->directionX = BALL_SPEED;
            ball->directionY = BALL_SPEED;
            ball->directionZ = BALL_SPEED;
            // Freeze the obstacle
            OBSTACLE_SPEED = 0.0f;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    cursorX = xpos;
    cursorY = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    if (selectMenu && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        if (cursorX >= 232 && cursorX <= 562 && cursorY >= 317 && cursorY <= 403){
			selectMenu = false;
            startGame = true;
            // Enable lighting
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
		}
		else if (cursorX >= 232 && cursorX <= 562 && cursorY >= 446 && cursorY <= 539){
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        if (menuPause){
            leftClick = false;
        }
        if (!menuPause){
            leftClick = true;
        }
        if (victory){
            if (cursorX >= 232 && cursorX <= 562 && cursorY >= 317 && cursorY <= 403){
                victory = false;
                selectMenu = true;
                menuSelected = true;
            }
            else if (cursorX >= 232 && cursorX <= 562 && cursorY >= 446 && cursorY <= 539){
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
        else if (defeat){
            if (cursorX >= 232 && cursorX <= 562 && cursorY >= 317 && cursorY <= 403){
                defeat = false;
                startGame = true;

                // Enable lighting
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT0);
            }
            else if (cursorX >= 232 && cursorX <= 562 && cursorY >= 446 && cursorY <= 539){
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
        if (!firstClick){
            leftClick = 0;
        }
        else{
            leftClick = 1;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        leftClick = 0;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        if (menuPause){
            rightClick = false;
        }
        else{
            rightClick = true;
        }
        firstClick = true;
        if (bonus3){
		    rightClick = true;
        }
        else{
            rightClick = false;
        }
	}
}

void checkCollision(){

    /* Ball */

    if (ball->positionZ == -25.0 &&
    ball->positionX - ball->radius <= racket->positionX + racket->width / 2 && racket->positionX - racket->width / 2 <= ball->positionX + ball->radius &&
    ball->positionY + ball->radius <= racket->positionY + racket->height / 2 && racket->positionY - racket->height / 2 <= ball->positionY + ball->radius){
        // Reverse the direction of the ball in the Z-axis
        ball->directionZ = -ball->directionZ;
    }
    if (ball->positionZ + ball->radius == -CORRIDOR_DEPTH) {
        // Reverse the direction of the ball in the Z-axis
        ball->directionZ = -ball->directionZ;
    }
    if (ball->positionZ == obstacle->positionZ &&
    ball->positionX - ball->radius <= obstacle->positionX + obstacle->width / 2 && obstacle->positionX - obstacle->width / 2 <= ball->positionX + ball->radius &&
    ball->positionY + ball->radius <= obstacle->positionY + obstacle->height / 2 && obstacle->positionY - obstacle->height / 2 <= ball->positionY + ball->radius){
        // Reverse the direction of the ball in the Z-axis
        ball->directionZ = -ball->directionZ;
    }
    if (ball->positionZ > 25.0){
        firstClick = false;
        PV -= 1;
        // Delay for 1 second before checking again
        sleep(1);
    }

    if (ball->positionX + ball->radius >= CORRIDOR_WIDTH / 2 || ball->positionX - ball->radius <= -CORRIDOR_WIDTH / 2) {
        ball->directionX = -1*ball->directionX;
    }
    
    if (ball->positionY + ball->radius >= CORRIDOR_HEIGHT / 2 || ball->positionY - ball->radius <= -CORRIDOR_HEIGHT / 2) {
        ball->directionY = -1*ball->directionY;
    }

    /* Racket on obstacle */
    if (obstacle->positionZ == -25.0  &&
    racket->positionX - racket->width / 2 <= obstacle->positionX + obstacle->width / 2 && obstacle->positionX - obstacle->width / 2 <= racket->positionX + racket->width / 2 &&
    racket->positionY + racket->height / 2 <= obstacle->positionY + obstacle->height / 2 && obstacle->positionY - obstacle->height / 2 <= racket->positionY + racket->height / 2){
        // Block the racket
        leftClick = 0;
    }

    /* Bonus */
    if (bonus->positionZ == -25.0 &&
    bonus->positionX - bonus->radius <= racket->positionX + racket->width / 2 && racket->positionX - racket->width / 2<= bonus->positionX + bonus->radius &&
    bonus->positionY + bonus->radius <= racket->positionY + racket->height / 2 && racket->positionY - racket->height / 2 <= bonus->positionY + bonus->radius){
        if (typeBonus == 1){
            if (PV < 5){
                PV ++;
            }
        }
        else if (typeBonus == 2){
            racket->height = 150.0f;
            racket->width = 150.0f;
            startTimer = true;
            startTime = glfwGetTime();
        }
        else if (typeBonus == 3){
            bonus3 = true;
        }
        else if (typeBonus == 4){
            racket->height = 50.0f;
            racket->width = 50.0f;
            startTimer = true;
            startTime = glfwGetTime();
        }
        else if (typeBonus == 5){
            PV --;
        }
        else if (typeBonus == 6){
            startTimerUpsideDown = true;
            startTime = glfwGetTime();
        }
        else if (typeBonus == 7){
            toWin ++;
        }
        bonusEarned = true;
    }
}

void timer (){
    double currentTime = glfwGetTime();
    double elapsedTime = currentTime - startTime;
    if (elapsedTime >= duration)
    {
        racket->height = 100.0f;
        racket->width = 100.0f;
        startTimer = false;
        startTimerUpsideDown = false;
    }
}

// Draw the menu screen
void drawMenuScreen(GLuint menuTextureID){
    // Disable lighting
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    // Enable depth testing
    glDisable(GL_DEPTH_TEST);

    /* Enable 2D texturing */
    glEnable(GL_TEXTURE_2D);

    /* Enable blending */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Set projection matrix */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);

     /* Clear the screen */
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /* Render background image */
    glColor4d(1.0, 1.0, 1.0, selectMenu);
    glBindTexture(GL_TEXTURE_2D, menuTextureID);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex2f(0, 0);
    glTexCoord2f(1, 1);
    glVertex2f(WINDOW_WIDTH, 0);
    glTexCoord2f(1, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glTexCoord2f(0, 0);
    glVertex2f(0, WINDOW_HEIGHT);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
}

// Draw the victory screen
void drawVictoryScreen(GLuint victoryTextureID)
{
    // Disable lighting
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    /* Enable 2D texturing */
    glEnable(GL_TEXTURE_2D);

    /* Enable blending */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2, WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 2, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the textured rectangle
    glColor4d(1.0, 1.0, 1.0, victory);
    glBindTexture(GL_TEXTURE_2D, victoryTextureID);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glEnd();

    PV = 5;
    toWin = 0;
}

// Draw the defeat screen
void drawDefeatScreen(GLuint defeatTextureID)
{
    // Disable lighting
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    /* Enable 2D texturing */
    glEnable(GL_TEXTURE_2D);

    /* Enable blending */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2, WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 2, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the textured rectangle
    glColor4d(1.0, 1.0, 1.0, defeat);
    glBindTexture(GL_TEXTURE_2D, defeatTextureID);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glEnd();

    PV = 5;
    toWin = 0;
}

// Draw the pause screen
void drawPauseScreen(GLuint pauseTextureID)
{
    // Disable lighting
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    /* Enable 2D texturing */
    glEnable(GL_TEXTURE_2D);

    /* Enable blending */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2, WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 2, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the textured rectangle
    glColor4d(1.0, 1.0, 1.0, menuPause);
    glBindTexture(GL_TEXTURE_2D, pauseTextureID);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glEnd();
}

void render(GLFWwindow* window, GLuint obstacleTexture){
    // Check if the timer should start
    if (startTimer || startTimerUpsideDown)
    {
        timer();
    }

    if (PV == 0){
        defeat = true;
    }
    else if (toWin == 5){
        startGame = false;
        victory = true;
    }

    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the viewport to match the window dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Set the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);

    // Set the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set the camera position and orientation
    gluLookAt(0.0f, 0.0f, 300.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // Draw the corridor
    drawCorridor();
    
    /* Render PV */
    drawPV();
    drawPVBarForm();

    // Draw the racket
    racket->positionZ = CORRIDOR_DEPTH / 100;
    updateRacketPosition();
    drawRacket(racket);

    // Update and draw the ball

    checkCollision();
    updateBallPosition();
    drawBall(ball);


    // Update and draw the obstacle
    // Draw the obstacle
    if (leftClick) {
        // Do something when the up key is pressed
        obstacle->positionZ += OBSTACLE_SPEED;
        bonus->positionZ += OBSTACLE_SPEED;
    }

    drawObstacle(obstacle, obstacleTexture);

    if (obstacle->positionZ == MAX_OBSTACLE_Z) {
        obstacle->positionZ = MIN_OBSTACLE_Z;

        int typeObstacle = rand()%3+1;
        if (typeObstacle == 1){
            obstacle->positionX = obstacle->width/2;
            obstacle->positionY = 0.0f;
            obstacle->width = CORRIDOR_WIDTH / 2;
            obstacle->height = CORRIDOR_HEIGHT;
        }
        else if (typeObstacle == 2){
            obstacle->positionX = -obstacle->width/2;
            obstacle->positionY = 0.0f;
            obstacle->width = CORRIDOR_WIDTH / 2;
            obstacle->height = CORRIDOR_HEIGHT;
        }
        else if (typeObstacle == 3){
            obstacle->positionX = 0.0f;
            obstacle->positionY = obstacle->height/2;
            obstacle->width = CORRIDOR_WIDTH;
            obstacle->height = CORRIDOR_HEIGHT / 2;
        }
        else if (typeObstacle == 4){
            obstacle->positionX = 0.0f;
            obstacle->positionY = -obstacle->height/2;
            obstacle->width = CORRIDOR_WIDTH;
            obstacle->height = CORRIDOR_HEIGHT / 2;
        }
    }        
    
    // Draw the bonus
    drawBonus(bonus,typeBonus);

    if (bonus->positionZ == MAX_OBSTACLE_Z || (bonusEarned)) {
        int bonusRadius = (int)bonus->radius;
        bonus->positionX = (float)(rand()%((WINDOW_WIDTH / 2) - bonusRadius - (- (WINDOW_WIDTH / 2) + bonusRadius)) - (WINDOW_WIDTH / 2) + bonusRadius*2);
        bonus->positionY = (float)(rand()%((WINDOW_HEIGHT / 2) - bonusRadius - (- (WINDOW_HEIGHT / 2) + bonusRadius)) - (WINDOW_HEIGHT / 2) + bonusRadius*2);
        bonus->positionZ = MIN_OBSTACLE_Z;
        bonusEarned = false;

        typeBonus = rand()%6+1;

        if (cptBonus%5 == 0){
            typeBonus = 7;
        }

        if (typeBonus == 1 || typeBonus == 6 || typeBonus == 7){
            bonus->radius = 20.0f;
        }
        else if (typeBonus == 2 || typeBonus == 4){
            bonus->radius = 30.0f;
        }
        else if (typeBonus == 3 || typeBonus == 5){
            bonus->radius = 40.0f;
        }
        cptBonus ++;
    }

}

int main()
{    
    typeBonus = rand()%6+1;

    // Set the initial ball position and direction
    ball = (Ball*)malloc(sizeof(Ball));
    ball->directionX = BALL_SPEED;
    ball->directionY = BALL_SPEED;
    ball->directionZ = BALL_SPEED;
    ball->radius = 20.0f;
    
    // Set the initial bonus position and direction
    bonus = (Ball*)malloc(sizeof(Ball));
    bonus->positionX = (float)(rand()%((WINDOW_WIDTH / 2) - 20 - (- (WINDOW_WIDTH / 2) + 20)) - (WINDOW_WIDTH / 2) + 20);
    bonus->positionY = (float)(rand()%((WINDOW_HEIGHT / 2) - 20 - (- (WINDOW_HEIGHT / 2) + 20)) - (WINDOW_HEIGHT / 2) + 20);
    bonus->positionZ = MIN_OBSTACLE_Z;
    bonus->radius = 20.0f;

    // Set the initial obstacle position
    obstacle = (Obstacle*)malloc(sizeof(Obstacle));
    obstacle->positionX = CORRIDOR_WIDTH / 4;
    obstacle->positionY = 0.0f;
    obstacle->positionZ = MIN_OBSTACLE_Z;
    obstacle->width = CORRIDOR_WIDTH / 2;
    obstacle->height = CORRIDOR_HEIGHT;

    // Initialize the racket
    racket = (Racket*)malloc(sizeof(Racket));
    racket->width = 100.0f;
    racket->height = 100.0f;
    racket->color.r = 0.0f;
    racket->color.g = 1.0f;
    racket->color.b = 0.0f;
    racket->color.a = 0.5f;

    // Initialize GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

    /* Set callback functions */
    glfwSetErrorCallback(errorCallback);
    // Set the keyboard input callback function
	glfwSetWindowSizeCallback(window,onWindowResized);
    glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
    // Get the cursor position
    glfwSetCursorPosCallback(window, cursor_position_callback);

	onWindowResized(window,WINDOW_WIDTH,WINDOW_HEIGHT);

	glPointSize(4.0);
    
    /* Load background image texture */
    GLuint backgroundTexture = loadTexture("doc/menu_projet_synthese_image.jpg");
    GLuint backgroundTextureDefeat = loadTexture("doc/menu_projet_synthese_image_defeat.jpg");
    GLuint backgroundTextureVictory = loadTexture("doc/menu_projet_synthese_image_victory.jpg");
    GLuint backgroundTexturePause = loadTexture("doc/menu_projet_synthese_image_pause.jpg");
    GLuint obstacleTexture = loadTexture("doc/obstacle_projet_synthese_image.jpg");

    drawMenuScreen(backgroundTexture);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set up lighting
    setupLighting();

    // Run the main loop
    while (!glfwWindowShouldClose(window))
    {
        if(menuSelected){
            drawMenuScreen(backgroundTexture);
            menuSelected = false;
        }
        if (startGame){
            render(window,obstacleTexture);
            
            // Check game state
            if (victory) {
                drawVictoryScreen(backgroundTextureVictory);
            } else if (defeat) {
                drawDefeatScreen(backgroundTextureDefeat);
            } else if (menuPause) {
                drawPauseScreen(backgroundTexturePause);
            }
        }        

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    /* Disable texturing when done */
    glDisable(GL_TEXTURE_2D);

    // Terminate GLFW
    glfwTerminate();
    
	free(ball);
    free(obstacle);
    free(racket);
    free(bonus);

    return 0;
}