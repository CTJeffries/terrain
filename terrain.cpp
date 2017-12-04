
// Colby Jeffries & Tommy Bacher
// terrain.cpp

// PROGRAM DESCRIPTION HERE

// Controls:
//  CONTROLS HERE

// Modules
#include <stdio.h>
#include <stdlib.h>
//#include <math.h>
#include <unistd.h>
//#include <iostream>
//#include "tgaClass.h"
#include <string.h>
//#include "glm.h"
#include <vector>
#include <time.h>
#include "fractalTerrain.hpp"

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
#define LOD 7
#define STEPS ((int)pow(2, LOD))


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

long NUM_TRIANGLES = STEPS * STEPS * 2;
vector<Triangle> triangles;
Triple map[STEPS + 1][STEPS + 1];

// Function prototypes
// Its probably about time for a header file.
void display(void);
static void mouse(int, int, int, int);
static void motion(int, int);
static void keyboardUp(unsigned char, int, int);
static void keyboardDown(unsigned char, int, int);
void keyboardCheck(void);
void init(void);
void resetMats(void);
void CalculateFrameRate(void);
void generateTerrain(void);

void generateTerrain() {
  double exaggeration = .7;

  RGB colors[STEPS + 1][STEPS + 1];
  FractalTerrain terrain(LOD, .5);
  for (int i = 0; i <= STEPS; ++ i) {
    for (int j = 0; j <= STEPS; ++ j) {
      double x = 1.0 * i / STEPS, z = 1.0 * j / STEPS;
      double altitude = terrain.getAltitude (x, z);
      map[i][j] = Triple(x, altitude * exaggeration, z);
      colors[i][j] = terrain.getColor (x, z);
    }
  }

  for (int i = 0; i < STEPS; ++ i) {
    for (int j = 0; j < STEPS; ++ j) {
      triangles.push_back(Triangle (i, j, i + 1, j, i, j + 1));
      triangles.push_back(Triangle (i + 1, j, i + 1, j + 1, i, j + 1));
    }
  }

  double ambient = .3;
  double diffuse = 4.0;
  Triple normals[NUM_TRIANGLES][3];
  Triple sun = Triple (1.0, 3.0, 1.0);

  double shade[STEPS + 1][STEPS + 1];
  for (int i = 0; i <= STEPS; ++ i) {
    for (int j = 0; j <= STEPS; ++ j) {
      shade[i][j] = 1.0;
      Triple vertex = map[i][j];
      Triple ray = sun.subtract(vertex);
      double distance = STEPS * sqrt (ray.getX() * ray.getY() + ray.getZ() * ray.getZ());
      /* step along ray in horizontal units of grid width */
      for (double place = 1.0; place < distance; place += 1.0) {
        Triple sample = vertex.add (ray.scale (place / distance));
        double sx = sample.getX(), sy = sample.getY(), sz = sample.getZ();
        if ((sx < 0.0) || (sx > 1.0) || (sz < 0.0) || (sz > 1.0))
          break; /* steppd off terrain */
        double ground = exaggeration * terrain.getAltitude (sx, sz);
        if (ground >= sy) {
          shade[i][j] = 0.0;
          break;
        }
      }
    }
  }

  for (long i = 0; i < NUM_TRIANGLES; i++)
    for (int j = 0; j < 3; j++)
      normals[i][j] = Triple (0.0, 0.0, 0.0);
  /* compute triangle normals and vertex averaged normals */
  for (int i = 0; i < NUM_TRIANGLES; ++ i) {
    Triple v0 = map[triangles[i].i[0]][triangles[i].j[0]],
      v1 = map[triangles[i].i[1]][triangles[i].j[1]],
      v2 = map[triangles[i].i[2]][triangles[i].j[2]];
    Triple normal = v0.subtract (v1).cross (v2.subtract (v1)).normalize ();
    triangles[i].n = normal;
    for (int j = 0; j < 3; ++ j) {
      normals[triangles[i].i[j]][triangles[i].j[j]] =
        normals[triangles[i].i[j]][triangles[i].j[j]].add (normal);
    }
  }

  /* compute vertex colors and triangle average colors */
  for (long i = 0; i < NUM_TRIANGLES; ++ i) {
    RGB avg = RGB (0.0, 0.0, 0.0);
    for (int j = 0; j < 3; ++ j) {
      int k = triangles[i].i[j], l = triangles[i].j[j];
      Triple vertex = map[k][l];
      RGB color = colors[k][l];
      Triple normal = normals[k][l].normalize ();
      Triple light = vertex.subtract (sun);
      double distance2 = light.length2 ();
      double dot = light.normalize ().dot (normal);
      double shadow = shade[k][l];
      double lighting = ambient + diffuse * ((dot < 0.0) ? - dot : 0.0) / distance2 * shadow;
      color = color.scale (lighting);
      triangles[i].color[j] = color;
      avg = avg.add (color);
    }
    triangles[i].finalColor = avg.scale (1.0 / 3.0);
  }
}

// Display callback.
void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  // glEnable(GL_LIGHT0);
  //
  // // Place the light.
  // glPushMatrix();
  //   glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
  // glPopMatrix();

  gluLookAt(1.2, 1.2, 1.2, 0.01, 0.01, 0.01, 0, 1, 0);

  for (long k=0; k<NUM_TRIANGLES; k++) {
    Triangle t = triangles[k];
    glBegin(GL_TRIANGLES);
      glColor3f(t.finalColor.r, t.finalColor.g, t.finalColor.b);
      glNormal3f(t.n.getX(), t.n.getY(), t.n.getZ());
      glVertex3f(map[t.i[0]][t.j[0]].getX(), map[t.i[0]][t.j[0]].getY(), map[t.i[0]][t.j[0]].getZ());
      glNormal3f(t.n.getX(), t.n.getY(), t.n.getZ());
      glVertex3f(map[t.i[1]][t.j[1]].getX(), map[t.i[1]][t.j[1]].getY(), map[t.i[1]][t.j[1]].getZ());
      glNormal3f(t.n.getX(), t.n.getY(), t.n.getZ());
      glVertex3f(map[t.i[2]][t.j[2]].getX(), map[t.i[2]][t.j[2]].getY(), map[t.i[2]][t.j[2]].getZ());
    glEnd();
  }

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
  //glEnable(GL_LIGHTING);
  glEnable(GL_AUTO_NORMAL);
  glEnable(GL_NORMALIZE);
  glShadeModel(GL_SMOOTH);
  glMatrixMode(GL_PROJECTION);
  generateTerrain();
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
