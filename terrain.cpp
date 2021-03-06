// Colby Jeffries & Tommy Bacher
// terrain.cpp

// This is the main driver to generate a fractal terrain.

// Controls:
//  Right click: Open menu.

// Modules
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
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

// Static variables
static float framesPerSecond = 0.0f;
static int lastTime	= 0;
static int frameCount = 0;
static char strFrameRate[20] = {0};

// Global variables.
int LOD = 5;
float rough = 0.5;
long long STEPS = (long long) pow(2, LOD);
long long NUM_TRIANGLES = STEPS * STEPS * 2;
float rot = 0;
int water = 0;
float waterLevel = 0.2;
float exaggeration = 0.6;

// Global data structures
int keyboard[256] = {0};
vector<Triangle> triangles;
vector<vector<Triple> > map(STEPS+1, vector<Triple>(STEPS+1));

// Window Height and Width Respectively
static GLsizei wh = 1000, ww = 1000;

// Globals materials & light info.
GLfloat no_mat[] = {0.0, 0.0, 0.0, 1.0};
GLfloat no_shine[] = {0.0};
GLfloat light_pos[] = {1.0, 3.0, 1.0, 1.0};

// Function prototypes
void display(void);
void keyboardCheck(void);
void init(void);
void resetMats(void);
void CalculateFrameRate(void);
void generateTerrain(void);
void idle(void);

// Generates the terrain values.
void generateTerrain() {
  vector<vector<RGB> > colors(STEPS+1, vector<RGB>(STEPS+1));
  // Generates terrain using square diamond.
  FractalTerrain terrain(LOD, rough);

  // Populate map and colors from terrain.
  for (int i = 0; i <= STEPS; ++ i) {
    for (int j = 0; j <= STEPS; ++ j) {
      double x = 1.0 * i / STEPS, z = 1.0 * j / STEPS;
      double altitude = terrain.getAltitude (x, z);
      map[i][j] = Triple(x, altitude * exaggeration, z);
      colors[i][j] = terrain.getColor (x, z);
    }
  }

  // Populate triangle list.
  for (int i = 0; i < STEPS; ++ i) {
    for (int j = 0; j < STEPS; ++ j) {
      triangles.push_back(Triangle (i, j, i + 1, j, i, j + 1));
      triangles.push_back(Triangle (i + 1, j, i + 1, j + 1, i, j + 1));
    }
  }

  // Light info.
  double ambient = .3;
  double diffuse = 6.0;

  vector<vector<Triple> > normals(STEPS+1, vector<Triple>(STEPS+1));
  vector<vector <double> > shade(STEPS + 1, vector<double>(STEPS + 1));

  // Sun position.
  Triple sun = Triple (1.0, 3.0, 1.0);

  // Calculate shadows.
  for (int i = 0; i <= STEPS; ++ i) {
    for (int j = 0; j <= STEPS; ++ j) {
      shade[i][j] = 1.0;
      Triple vertex = map[i][j];
      Triple ray = sun.subtract(vertex);
      double distance = STEPS * sqrt (ray.getX() * ray.getY() + ray.getZ() * ray.getZ());
      for (double place = 1.0; place < distance; place += 1.0) {
        Triple sample = vertex.add (ray.scale (place / distance));
        double sx = sample.getX(), sy = sample.getY(), sz = sample.getZ();
        if ((sx < 0.0) || (sx > 1.0) || (sz < 0.0) || (sz > 1.0))
          break;

        double ground = exaggeration * terrain.getAltitude (sx, sz);
        if (ground >= sy) {
          shade[i][j] = 0.0;
          break;
        }
      }
    }
  }

  // Poplulate Normal vector.
  for (long i = 0; i < STEPS; i++)
    for (int j = 0; j < STEPS; j++)
      normals[i][j] = Triple(0.0, 0.0, 0.0);

  // Calculate normals.
  for (int i = 0; i < NUM_TRIANGLES; ++ i) {
    Triple v0 = map[triangles[i].i[0]][triangles[i].j[0]],
      v1 = map[triangles[i].i[1]][triangles[i].j[1]],
      v2 = map[triangles[i].i[2]][triangles[i].j[2]];
    Triple normal = v0.subtract(v1).cross(v2.subtract(v1)).normalize();
    triangles[i].n = normal;
    for (int j = 0; j < 3; ++ j) {
      normals[triangles[i].i[j]][triangles[i].j[j]] =
        normals[triangles[i].i[j]][triangles[i].j[j]].add(normal);
    }
  }

  // Calculate colors.
  for (long i = 0; i < NUM_TRIANGLES; ++i) {
    RGB avg = RGB(0.0, 0.0, 0.0);
    for (int j = 0; j < 3; ++j) {
      int k = triangles[i].i[j], l = triangles[i].j[j];
      Triple vertex = map[k][l];
      RGB color = colors[k][l];
      Triple normal = normals[k][l].normalize();
      Triple light = vertex.subtract(sun);
      double distance2 = light.length2();
      double dot = light.normalize().dot(normal);
      double shadow = shade[k][l];
      double lighting = ambient + diffuse * ((dot < 0.0) ? - dot : 0.0) / distance2 * shadow;
      color = color.scale(lighting);
      triangles[i].color[j] = color;
      avg = avg.add(color);
    }
    triangles[i].finalColor = avg.scale (1.0 / 3.0);
  }
}

// Display callback.
void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glEnable(GL_LIGHT0);

  // Place the light.
  glPushMatrix();
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
  glPopMatrix();

  // Look at the cetner from an angle.
  gluLookAt(0.9, 1.2, 0.9, 0.0, 0.2, 0.0, 0, 1, 0);

  // Rotate around the center of the terrain.
  glRotatef(rot, 0.0, 1.0, 0.0);
  glPushMatrix();
  glTranslatef(-0.5, 0.0, -0.5);

  // Draw triangles of the terrain.
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

  // Draw sides of "land slice".
  for(long k=0; k<STEPS; k++) {
    glColor3f(0.35, 0.3, 0.27);
    glBegin(GL_QUADS);
      glVertex3f(map[k][0].getX(), map[k][0].getY(), map[k][0].getZ());
      glVertex3f(map[k][0].getX(), -2.0, map[k][0].getZ());
      glVertex3f(map[k+1][0].getX(), -2.0, map[k+1][0].getZ());
      glVertex3f(map[k+1][0].getX(), map[k+1][0].getY(), map[k+1][0].getZ());
    glEnd();

    glBegin(GL_QUADS);
      glVertex3f(map[k][STEPS].getX(), map[k][STEPS].getY(), map[k][STEPS].getZ());
      glVertex3f(map[k][STEPS].getX(), -2.0, map[k][STEPS].getZ());
      glVertex3f(map[k+1][STEPS].getX(), -2.0, map[k+1][STEPS].getZ());
      glVertex3f(map[k+1][STEPS].getX(), map[k+1][STEPS].getY(), map[k+1][STEPS].getZ());

    glEnd();

    glBegin(GL_QUADS);
      glVertex3f(map[0][k].getX(), map[0][k].getY(), map[0][k].getZ());
      glVertex3f(map[0][k].getX(), -2.0, map[0][k].getZ());
      glVertex3f(map[0][k+1].getX(), -2.0, map[0][k+1].getZ());
      glVertex3f(map[0][k+1].getX(), map[0][k+1].getY(), map[0][k+1].getZ());
    glEnd();

    glBegin(GL_QUADS);
      glVertex3f(map[STEPS][k].getX(), map[STEPS][k].getY(), map[STEPS][k].getZ());
      glVertex3f(map[STEPS][k].getX(), -2.0, map[STEPS][k].getZ());
      glVertex3f(map[STEPS][k+1].getX(), -2.0, map[STEPS][k+1].getZ());
      glVertex3f(map[STEPS][k+1].getX(), map[STEPS][k+1].getY(), map[STEPS][k+1].getZ());
    glEnd();
  }

  // If water, draw water.
  if (water == 1) {
    glColor3f(0.0, waterLevel, 1.0);
    glBegin(GL_QUADS);
      glVertex3f(0.0, waterLevel, 0.0);
      glVertex3f(1.0, waterLevel, 0.0);
      glVertex3f(1.0, waterLevel, 1.0);
      glVertex3f(0.0, waterLevel, 1.0);
    glEnd();

    for(long k=0; k<STEPS; k++) {
      if (map[k][0].getY() <= waterLevel) {
        glBegin(GL_QUADS);
          glVertex3f(map[k][0].getX(), waterLevel, map[k][0].getZ());
          glVertex3f(map[k][0].getX(), map[k][0].getY(), map[k][0].getZ());
          glVertex3f(map[k+1][0].getX(), map[k+1][0].getY(), map[k+1][0].getZ());
          glVertex3f(map[k+1][0].getX(), waterLevel, map[k+1][0].getZ());
        glEnd();
      }

      if (map[k][STEPS].getY() <= waterLevel) {
        glBegin(GL_QUADS);
          glVertex3f(map[k][STEPS].getX(), waterLevel, map[k][STEPS].getZ());
          glVertex3f(map[k][STEPS].getX(), map[k][STEPS].getY(), map[k][STEPS].getZ());
          glVertex3f(map[k+1][STEPS].getX(), map[k+1][STEPS].getY(), map[k+1][STEPS].getZ());
          glVertex3f(map[k+1][STEPS].getX(), waterLevel, map[k+1][STEPS].getZ());
        glEnd();
      }

      if (map[0][k].getY() <= waterLevel) {
        glBegin(GL_QUADS);
          glVertex3f(map[0][k].getX(), waterLevel, map[0][k].getZ());
          glVertex3f(map[0][k].getX(), map[0][k].getY(), map[0][k].getZ());
          glVertex3f(map[0][k+1].getX(), map[0][k+1].getY(), map[0][k+1].getZ());
          glVertex3f(map[0][k+1].getX(), waterLevel, map[0][k+1].getZ());
        glEnd();
      }

      if (map[STEPS][k].getY() <= waterLevel) {
        glBegin(GL_QUADS);
          glVertex3f(map[STEPS][k].getX(), waterLevel, map[STEPS][k].getZ());
          glVertex3f(map[STEPS][k].getX(), map[STEPS][k].getY(), map[STEPS][k].getZ());
          glVertex3f(map[STEPS][k+1].getX(), map[STEPS][k+1].getY(), map[STEPS][k+1].getZ());
          glVertex3f(map[STEPS][k+1].getX(), waterLevel, map[STEPS][k+1].getZ());
        glEnd();
      }
    }
  }

  glPopMatrix();

  glutSwapBuffers();
	CalculateFrameRate();
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
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  generateTerrain();
}

// Reshape callback that preserves aspect ratio. Also implements the zoom.
void myReshape(int w, int h) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w, h);
  gluPerspective(55.0f, (GLfloat)w/(GLfloat)h, 0.1, 4000.0);
  glMatrixMode(GL_MODELVIEW);
  ww = w;
 	wh = h;
}

// Idle callback. Drives the game for the most part.
void idle() {
  rot += 0.3;
  if (rot > 360.0) rot = rot - 360.0;
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

// Large menu callback.
void menuFunc(int id) {
  if (id == 1) {
    if (water == 0) {
      water = 1;
    }
    else {
      water = 0;
    }
  }
  if (id == 2) {
    generateTerrain();
  }
}

// LOD Menu callback.
void lodMenu(int id) {
  LOD = id;

  STEPS = (long long) pow(2, LOD);
  NUM_TRIANGLES = STEPS * STEPS * 2;
  triangles.clear();
  map.resize(STEPS+1);
  for (int i=0; i<map.size(); i++) {
    map[i].resize(STEPS+1);
  }

  generateTerrain();
}

// Roughness menu callback.
void roughMenu(int id) {
  if (id == 1) {
    if (rough < 0.95) {
      rough = rough + 0.05;
    }
  }
  else if (id == 2) {
    if (rough > 0.05) {
      rough = rough - 0.05;
    }
  }

  generateTerrain();
}

// Exaggeration menu callback.
void exaggerationMenu(int id) {
  if (id == 1) {
    if (exaggeration < 0.95) {
      exaggeration = exaggeration + 0.05;
    }
  }
  else if (id == 2) {
    if (exaggeration > 0.05) {
      exaggeration = exaggeration - 0.05;
    }
  }

  generateTerrain();
}

// Roughness menu callback.
void waterMenu(int id) {
  if (id == 1) {
    if (waterLevel < 2.0) {
      waterLevel = waterLevel + 0.05;
    }
  }
  else if (id == 2) {
    if (waterLevel > 0.05) {
      waterLevel = waterLevel - 0.05;
    }
  }
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
  glutReshapeFunc(myReshape);
  glutIdleFunc(idle);

  // Initialization function.
  init();

  // Menus
  int menu, l_menu, r_menu, w_menu, e_menu;
  e_menu = glutCreateMenu(exaggerationMenu);
  glutAddMenuEntry("  +  ", 1);
  glutAddMenuEntry("  -  ", 2);
  w_menu = glutCreateMenu(waterMenu);
  glutAddMenuEntry("  +  ", 1);
  glutAddMenuEntry("  -  ", 2);
  r_menu = glutCreateMenu(roughMenu);
  glutAddMenuEntry("  +  ", 1);
  glutAddMenuEntry("  -  ", 2);
  l_menu = glutCreateMenu(lodMenu);
  glutAddMenuEntry("  2  ", 2);
  glutAddMenuEntry("  3  ", 3);
  glutAddMenuEntry("  4  ", 4);
  glutAddMenuEntry("  5  ", 5);
  glutAddMenuEntry("  6  ", 6);
  glutAddMenuEntry("  7  ", 7);
  menu = glutCreateMenu(menuFunc);
  glutAddMenuEntry("Water", 1);
  glutAddMenuEntry("Reset", 2);
  glutAddSubMenu("LOD", l_menu);
  glutAddSubMenu("Roughness", r_menu);
  glutAddSubMenu("Water Level", w_menu);
  glutAddSubMenu("Exaggeration", e_menu);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // Mainloop
  glutMainLoop();

  return 0;
}
