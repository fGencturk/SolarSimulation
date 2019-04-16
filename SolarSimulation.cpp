/*********
   CTIS164 - Template Source Program
----------
STUDENT :
SECTION :
HOMEWORK:
----------
PROBLEMS:
----------
ADDITIONAL FEATURES:
*********/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define WINDOW_WIDTH  1400
#define WINDOW_HEIGHT 900

#define TIMER_PERIOD  25 // Period for the timer.
#define TIMER_ON       1	 // 0:disable timer, 1:enable timer

#define D2R 0.0174532

GLubyte  miniMap[200][200][3];

typedef struct {
	float x, y, z;
} vec_t;

typedef struct {
	vec_t coordinate;
	float radius;
} sun_t;

typedef struct {
	vec_t coordinate;
	float radius,
		distanceFromOrigin,
		angleAroundOrigin;
} planet_t;


typedef struct {
	vec_t coordinate;
	float distanceFromOrigin,
		angleAroundOrigin;
	bool isRotating;
} camera_t;
sun_t sun;
planet_t earth,
	moon;
GLint gridDisplayList;
camera_t camera;
/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height
GLfloat pos[] = { 0, 50.0, 0, 1.0 };

void initializeGlobals()
{
	sun.coordinate = { 0,50,0 };
	sun.radius = 20;
	earth.distanceFromOrigin = 120;
	earth.radius = 10;
	earth.coordinate.y = 50;
	moon.coordinate.y = 50;
	moon.distanceFromOrigin = 30;
	moon.radius = 4;
	camera.isRotating = true;
	camera.coordinate.y = 200;
	camera.distanceFromOrigin = 300;
	camera.angleAroundOrigin = -45;
	camera.coordinate.x = cos(D2R * camera.angleAroundOrigin) * camera.distanceFromOrigin;
	camera.coordinate.z = -sin(D2R * camera.angleAroundOrigin) * camera.distanceFromOrigin;
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, w / h, 1.0, 1500.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
	float angle;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void print(int x, int y, const char *string, void *font)
{
	int len, i;

	glRasterPos2f(x, y);
	len = (int)strlen(string);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, string[i]);
	}
}

// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void *font, const char *string, ...)
{
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, str[i]);
	}
}

// vprint2(-50, 0, 0.35, "00:%02d", timeCounter);
void vprint2(int x, int y, float size, const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size, size, 1);

	int len, i;
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
	}
	glPopMatrix();
}

void static draw2DGridXZ(float centerx, float centery, float centerz, float gridSize, int count)
{
	float totalLength = gridSize * count;
	float startX = centerx - totalLength / 2;
	float startZ = centerz - totalLength / 2;

	glBegin(GL_LINES);
	for (int i = 0; i <= count; i++)
	{
		glVertex3f(startX, centery, startZ + i * gridSize);
		glVertex3f(startX + totalLength, centery, startZ + i * gridSize);

		glVertex3f(startX + i * gridSize, centery, startZ);
		glVertex3f(startX + i * gridSize, centery, startZ + totalLength);
	}
	glEnd();
}

void displayGrid()
{
	glDisable(GL_LIGHTING);
	glColor3f(0, 0, 1);
	draw2DGridXZ(0, 0, 0, 30, 30);
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(0, 450, -450);
	glRotatef(90, 1, 0, 0);
	draw2DGridXZ(0, 0, 0, 30, 30);
	glPopMatrix();
	glPushMatrix();
	glColor3f(0, 1, 0);
	glTranslatef(-450, 450, 0);
	glRotatef(90, 0, 0, 1);
	draw2DGridXZ(0, 0, 0, 30, 30);
	glPopMatrix();
	glEnable(GL_LIGHTING);

}

void displayWorld()
{
	//SUN
	glDisable(GL_LIGHTING);
	glColor3f(0.7, 0.3, 0.1);
	glPushMatrix();
	glTranslatef(sun.coordinate.x, sun.coordinate.y, sun.coordinate.z);
	glutSolidSphere(sun.radius, 20, 20);
	glPopMatrix();
	glEnable(GL_LIGHTING);
	//EARTH
	glColor3f(0.1, 0.4, 0.8);
	glPushMatrix();
	glTranslatef(earth.coordinate.x, earth.coordinate.y, earth.coordinate.z);
	GLfloat mat[] = { 0, 0.5, 0.5, 1.0 };
	GLfloat dif[] = { 0.8, 0.2, 0.3, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	glMaterialf(GL_FRONT, GL_SHININESS, 25.0);
	glutSolidSphere(earth.radius, 20, 20);
	glPopMatrix();
	//MOON
	glColor3f(0.3, 0.7, 0.5);
	glPushMatrix();
	glTranslatef(moon.coordinate.x, moon.coordinate.y, moon.coordinate.z);
	GLfloat mat2[] = { 0.5, 0.1, 0.1, 1.0 };
	GLfloat dif2[] = { 1.0,1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat2);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif2);
	glMaterialf(GL_FRONT, GL_SHININESS, 10.0);
	glutSolidSphere(moon.radius, 10, 10);
	glPopMatrix();

}

void createMiniMap()
{
	//HUD CREATION
	//Upside View
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-winWidth / 2, winWidth / 2, -winHeight / 2, winHeight / 2, -1, 1500);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadIdentity();
	gluLookAt(0, 400, 0, 0, 50, 0, 0, 0, -1);
	glScalef(0.5, 1, 0.5);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glCallList(gridDisplayList);
	displayWorld();
	//2D Photo
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-winWidth / 2, winWidth / 2, -winHeight / 2, winHeight / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRasterPos2d(winWidth / 2 - 225, winHeight / 2 - 225);
	//glCopyPixels(600, 350, 200, 200, GL_COLOR);
	glReadPixels(winWidth / 2 - 100, winHeight / 2 - 100, 200, 200, GL_RGB, GL_UNSIGNED_BYTE, miniMap);

}

//
// To display onto window using OpenGL commands
//
void display() {
	//
	// clear window to black
	//
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	createMiniMap();
	
	//3D WORLD
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	onResize(winWidth, winHeight);
	glLoadIdentity();
	gluLookAt(camera.coordinate.x, camera.coordinate.y, camera.coordinate.z, 0, 50, 0, 0, 1, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glCallList(gridDisplayList);
	displayWorld();

	//HUD
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-winWidth / 2, winWidth / 2, -winHeight / 2, winHeight / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(0.2, 0.2, 0.2);
	glRectf(winWidth / 2 - 250, winHeight / 2 - 275, winWidth / 2, winHeight / 2);
	glColor3f(1, 1, 1);
	vprint2(winWidth / 2 - 170, winHeight / 2 - 265, 0.2, "MINIMAP");
	glDrawPixels(200, 200, GL_RGB, GL_UNSIGNED_BYTE, miniMap);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	
	glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);
	else if (key == 'r' || key == 'R')
		camera.isRotating = !camera.isRotating;

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = true; break;
	case GLUT_KEY_DOWN: down = true; break;
	case GLUT_KEY_LEFT: left = true; break;
	case GLUT_KEY_RIGHT: right = true; break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = false; break;
	case GLUT_KEY_DOWN: down = false; break;
	case GLUT_KEY_LEFT: left = false; break;
	case GLUT_KEY_RIGHT: right = false; break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{
	// Write your codes here.


	// to refresh the window it calls display() function
	glutPostRedisplay();
}

void onMoveDown(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function   
	glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onMove(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function
	glutPostRedisplay();
}

#if TIMER_ON == 1
void onTimer(int v) {

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	// Write your codes here.
	earth.angleAroundOrigin += 2;
	moon.angleAroundOrigin += 3;
	earth.coordinate.x = cos(D2R * earth.angleAroundOrigin) * earth.distanceFromOrigin;
	earth.coordinate.z = -sin(D2R * earth.angleAroundOrigin) * earth.distanceFromOrigin;
	moon.coordinate.x = earth.coordinate.x + cos(D2R * moon.angleAroundOrigin) * moon.distanceFromOrigin;
	moon.coordinate.z = earth.coordinate.z - sin(D2R * moon.angleAroundOrigin) * moon.distanceFromOrigin;
	if (camera.isRotating)
		camera.angleAroundOrigin += 0.6;
	if (up)
	{
		if (camera.distanceFromOrigin != 5)
			camera.distanceFromOrigin -= 5;

	}
	if (down)
		camera.distanceFromOrigin += 5;
	camera.coordinate.x = cos(D2R * camera.angleAroundOrigin) * camera.distanceFromOrigin;
	camera.coordinate.z = -sin(D2R * camera.angleAroundOrigin) * camera.distanceFromOrigin;
	// to refresh the window it calls display() function
	glutPostRedisplay(); // display()

}
#endif

void Init() {
	gridDisplayList = glGenLists(1);
	glNewList(gridDisplayList, GL_COMPILE);
	displayGrid();
	glEndList();


	GLfloat light_pos[] = { 0,0, 50.0f, 1, 1.0 };
	GLfloat col1[] = { 1,0.4,0 };
	GLfloat col2[] = { 0.3,0.3,0.3 };
	GLfloat col3[] = { 0,0,1 };
	glLoadIdentity();
	gluLookAt(200, 200, 200, 0, 50, 0, 0, 1, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, col1);
	glLightfv(GL_LIGHT0, GL_AMBIENT, col2);
	glLightfv(GL_LIGHT0, GL_SPECULAR, col3);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	// Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

}

void main(int argc, char *argv[]) {
	initializeGlobals();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	//glutInitWindowPosition(100, 100);
	glutCreateWindow("Template File");
	
	glutDisplayFunc(display);
	glutReshapeFunc(onResize);

	//
	// keyboard registration
	//
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecialKeyDown);

	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecialKeyUp);

	//
	// mouse registration
	//
	glutMouseFunc(onClick);
	glutMotionFunc(onMoveDown);
	glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
	// timer event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

	Init();

	glutMainLoop();
}

