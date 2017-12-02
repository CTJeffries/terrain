
// Colby Jeffries & Tommy Bacher
// terrain.cpp

// PROGRAM DESCRIPTION HERE

// Controls:
//  CONTROLS HERE

// Modules
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <iostream>
#include "tgaClass.h"
#include <string.h>
#include "glm.h"
#include <vector>
#include <time.h>

using namespace std;

// OpenGL
#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glut.h>
#endif

// Pi and related Numbers
#define PI 3.141592653
#define RADIANS_TO_DEGREES (180 / PI)

// Convenient way to store vertices.
struct vertex {
  GLfloat x, y, z;
} typedef vertex;

// Convenient way to store normals. (Not different that vertices, but provides
// clear code.)
struct normal {
  GLfloat x, y, z;
} typedef normal;

// Convenient way to store color info.
struct color {
  GLfloat r, g, b;
} typedef color

// Static variables
static float framesPerSecond = 0.0f;
static int lastTime	= 0;
static int frameCount = 0;
static char strFrameRate[20] = {0};

// Global data structures
int keyboard[256] = {0};

// Window Height and Width Respectively
static GLsizei wh = 1000, ww = 1000;

// Globals materials
GLfloat no_mat[] = {0.0, 0.0, 0.0, 1.0};
GLfloat no_shine[] = {0.0};

GLfloat light_pos[] = {0.0, 1.0, 0.0, 1.0};

// Function prototypes
// Its probably about time for a header file.
void display(void);
void makeVertex(vertex, normal);
static void mouse(int, int, int, int);
static void motion(int, int);
static void keyboardUp(unsigned char, int, int);
static void keyboardDown(unsigned char, int, int);
void keyboardCheck(void);
void init(void);
void resetMats(void);
void CalculateFrameRate(void);
color calculateColor(float);

// Calculates color based on altitudes. Pretty simple.
color calculateColor(float h) {
  color temp;
  if (h < 0.5) {
    temp.r = 0.0;
    temp.g = h * 2;
    temp.b = 1.0 - (h * 2);
  }
  else {
    temp.r = h;
    temp.g = 1.0 - ((h - 0.5) * 2)
    temp.b = 0.0;
  }

  return color;
}

// Allows quick and easy vertex creation.
// Takes a vertex v and a normal n and draws a vertex at that location.
void makeVertex(vertex v, normal n) {
  glNormal3f(n.x, n.y, n.z);
  glVertex3f(v.x, v.y, v.z);
}

// Display callback.
void display(void) {
  resetMats();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glEnable(GL_LIGHT0);

  // Place the light.
  glPushMatrix();
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
  glPopMatrix();

  resetMats();
  glutSwapBuffers();
	CalculateFrameRate();
}

// Unused.
static void mouse(int button, int state, int x, int y) {
  NULL;
}

// Unused.
static void motion(int x, int y) {
  NULL;
}

// Resets all materials to ensure no unintentional effects.
void resetMats(void) {
  glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, no_mat);
  glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
  glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
  glMaterialfv(GL_FRONT, GL_SHININESS, no_shine);
}

// Initialization function.
void init(void) {
  glClearColor(0.7, 0.7, 0.7, 1.0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_AUTO_NORMAL);
  glEnable(GL_NORMALIZE);
  glShadeModel(GL_SMOOTH);
  glMatrixMode(GL_PROJECTION);
}

// Updates keyboard state when a key is released.
void keyboardUp(unsigned char key, int x, int y) {
  keyboard[key] = 0;
}

// Updates keyboard state when a key is pressed. Also handles the toggles.
void keyboardDown(unsigned char key, int x, int y) {
  keyboard[key] = 1;
}

// Does the actions of the keys.
void keyboardCheck(void) {
  NULL;
}

// Reshape callback that preserves aspect ratio. Also implements the zoom.
void myReshape(int w, int h) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w, h);
  gluPerspective(75.0f, (GLfloat)w/(GLfloat)h, 0.1, 4000.0);
  glMatrixMode(GL_MODELVIEW);
  ww = w;
 	wh = h;
}

// Idle callback. Drives the game for the most part.
void idle() {
  keyboardCheck();
  glutPostRedisplay();
}

// Gives us the framerate in the title.
void CalculateFrameRate(void) {
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	++frameCount;
	int interval = currentTime - lastTime;
	if((interval) > 1000) {
	  framesPerSecond = frameCount /((interval)/1000.0f);
	  lastTime = currentTime;
		sprintf(strFrameRate, "OpenGL Fractal Terrain: %d", int(framesPerSecond));
		glutSetWindowTitle(strFrameRate);
	  frameCount = 0;
	}
	glutPostRedisplay();
}

int main(int argc, char **argv) {
  srand(time(NULL));
  // Initialize OpenGL.
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);

  // Initial window size and position.
  glutInitWindowSize(1500,750);
  glutInitWindowPosition(300,100);

  // Name the window.
  glutCreateWindow("OpenGL Fractal Terrain: 0");

  // Set callbacks.
  glutIgnoreKeyRepeat(1);
  glutDisplayFunc(display);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboardDown);
  glutKeyboardUpFunc(keyboardUp);
  glutReshapeFunc(myReshape);
  glutIdleFunc(idle);

  // Initialization function.
  init();

  // Mainloop
  glutMainLoop();

  return 0;
}
