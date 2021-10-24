#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "cube.h"
#include "QuadMesh.h"
#include <iostream>

const int vWidth = 800;    // Viewport width in pixels
const int vHeight = 600;    // Viewport height in pixels

// Note how everything depends on robot body dimensions so that can scale entire robot proportionately
// just by changing robot body scale
float robotBodyWidth = 4.0;
float robotBodyLength = 10.0;
float robotBodyDepth = 6.0;
float headWidth = 0.4*robotBodyWidth;
float headLength = headWidth;
float headDepth = headWidth;
float upperArmLength = robotBodyLength;
float upperArmWidth = 0.125*robotBodyWidth;
float gunLength = upperArmLength / 4.0;
float gunWidth = upperArmWidth;
float gunDepth = upperArmWidth;
float stanchionLength = robotBodyLength;
float stanchionRadius = 0.1*robotBodyDepth;
float baseWidth = 2 * robotBodyWidth;
float baseLength = 0.25*stanchionLength;

// Control Robot body rotation on base
float robotAngle = 0.0;

// Control arm rotation
float shoulderAngle = -40.0;
float gunAngle = 0.0;
float movement = 0.0;
float wheelRotate = 0.0;

// Spin Cube Mesh
float cubeAngle = 0.0;

// Lighting/shading and material properties for robot - upcoming lecture - just copy for now
// Robot RGBA material properties (NOTE: we will learn about this later in the semester)
GLfloat robotBody_mat_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.45f,0.55f,0.45f,1.0f };
GLfloat robotBody_mat_diffuse[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat robotBody_mat_shininess[] = { 32.0F };


GLfloat robotArm_mat_ambient[] = { 0.0215f, 0.1745f, 0.0215f, 0.55f };
GLfloat robotArm_mat_diffuse[] = { 0.5f,0.0f,0.0f,1.0f };
GLfloat robotArm_mat_specular[] = { 0.7f, 0.6f, 0.6f, 1.0f };
GLfloat robotArm_mat_shininess[] = { 32.0F };

GLfloat gun_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gun_mat_diffuse[] = { 0.01f,0.0f,0.01f,0.01f };
GLfloat gun_mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat gun_mat_shininess[] = { 100.0F };

GLfloat robotLowerBody_mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat robotLowerBody_mat_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat robotLowerBody_mat_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
GLfloat robotLowerBody_mat_shininess[] = { 76.8F };


// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };


// Mouse button
int currentButton;

// A template cube mesh
CubeMesh *cubeMesh = createCubeMesh();

// A flat open mesh
QuadMesh *groundMesh = NULL;

// Structure defining a bounding box, currently unused
typedef struct BoundingBox {
	VECTOR3D min;
	VECTOR3D max;
} BBox;

// Default Mesh Size
int meshSize = 16;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void animationHandler(int param);
void drawRobot();
void drawBody();
void drawHead();
void drawHeadDetails();
void drawWheel();
void drawLeftArm();
void drawRightArm();

int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Bot 3: Press F1 for Help Key");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	// glutMouseFunc(mouse);
	// glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). 
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);   // This second light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.0, 0.5, 0.8, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// Other initializatuion
	// Set up ground quad mesh
	VECTOR3D origin = VECTOR3D(-16.0f, 5.0f, 16.0f);
	VECTOR3D dir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
	VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
	groundMesh = new QuadMesh(meshSize, 32.0);
	groundMesh->InitMesh(meshSize, origin, 32.0, 32.0, dir1v, dir2v);

	VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
	VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
	VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float shininess = 0.2;
	groundMesh->SetMaterial(ambient, diffuse, specular, shininess);

}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Create Viewing Matrix V
	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(3.0, 7.0, 30.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Draw Robot

	// Apply modelling transformations M to move robot
	// Current transformation matrix is set to IV, where I is identity matrix
	// CTM = IV
	drawRobot();

	// Example of drawing a mesh (closed cube mesh)
	glPushMatrix();
		// spin cube
	glTranslatef(8.0, 0, 3.0);
	glRotatef(cubeAngle, 0.0, 1.0, 0.0);
	glTranslatef(-8.0, 0, -3.0);
	// position and draw cube
	glTranslatef(8.0, 0, 10.0);
	glScalef(2, 1, 3);
	drawCubeMesh(cubeMesh);
	glPopMatrix();

	// Draw ground
	glPushMatrix();
	glTranslatef(0.0, -20.0, 0.0);
	groundMesh->DrawMesh(meshSize);
	glPopMatrix();

	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawRobot()
{
	glPushMatrix();
	// spin robot on base. 
	glRotatef(robotAngle, 0.0, 1.0, 0.0);
	glTranslatef(0, 0, movement);

	drawBody();
		drawHead();
			drawHeadDetails();
		drawLeftArm();
		drawRightArm();
		drawWheel();

	glPopMatrix();

	glPopMatrix();
}

void drawBody()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	/*
	To make "capsule" body: I used a cylinder connected with a sphere
	The sphere will be scaled and translated to cover half of it inside 
	the cylinder, so it looks kinda like a capsule
	*/

	// The cylinder part
	glPushMatrix();
	glTranslatef(0, 7, 0);
	glScalef(robotBodyWidth, robotBodyLength, robotBodyWidth);
	glRotatef(90, 1, 0, 0);
	gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 60, 60);
	glPopMatrix();

	// The sphere part
	glPushMatrix();
	glTranslatef(0, -2, 0);
	glScalef(1, 2, 1);
	gluSphere(gluNewQuadric(), 4, 30,10);
	glPopMatrix();

	// Little dot at the center of body
	glPushMatrix();
	glTranslatef(0, 2, 3.8);
	glScalef(.2, .2, .2);
	gluSphere(gluNewQuadric(), 2.5, 30, 10);
	glPopMatrix();
}

void drawHead()
{
	// Set robot material properties per body part. Can have separate material properties for each part
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	glPushMatrix();
	// Position head with respect to parent (body)
	glTranslatef(0, .95 * robotBodyLength, .2 * headLength);

	// The head consists of 2 parts, the head and the attached cannon/arm on top

	glPushMatrix();

	// The main sphere
	glPushMatrix();
	gluSphere(gluNewQuadric(), 3.5, 30, 10);
	glPopMatrix();

	/*The arm/cannon on top of the head consists of 3 parts connected: The vertical cylinder,
	the small sphere, and the horizontal cylinder rotated 90 deg
	*/
	GLfloat MaterialColor[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat ambientColor[] = { 1.0, 1.0, 1.0, 1.0 };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialColor);
	glPushMatrix();
	glTranslatef(0, 4.8, 0);

	// Vertical cylinder under the sphere
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glScalef(1,1,4);
	gluCylinder(gluNewQuadric(), 1, 1, 2, 40, 10);
	glPopMatrix();

	// Small sphere
	glScalef(1.5,1.5,1.5);
	gluSphere(gluNewQuadric(), 1, 30, 10);

	// Horizontal cylinder
	glScalef(.7,.7,3);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 40, 10);
	glPopMatrix();

	glPopMatrix();
}

void drawHeadDetails()
{
	// The disk around the sphere
	glPushMatrix();
	gluDisk(gluNewQuadric(), 0, 4, 35, 35);
	glPopMatrix();

	// The disk in front of the sphere
	glPushMatrix();
	glTranslatef(0, 0, 3);
	gluDisk(gluNewQuadric(), 0, 2, 55, 55);
	glPopMatrix();

	glPopMatrix();
	// Little dot at the center of body
	glPushMatrix();
	glTranslatef(0, 9.5, 3);
	glScalef(.2, .2, .2);
	gluSphere(gluNewQuadric(), 5, 30, 10);
	glPopMatrix();
}


void drawWheel()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLowerBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLowerBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLowerBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLowerBody_mat_shininess);
	glPushMatrix();

	glTranslatef(0, -1.0*robotBodyLength, 0.0);
	glRotatef(wheelRotate, 1.0, 0.0, 0.0);
	
	glPushMatrix();
	glRotatef(90, 0.0, 1.0, 0.0);
	// The wheel
	glutSolidTorus(1, 4.5, 40, 100);
	// Wheel fill to show wheel movement
	gluPartialDisk(gluNewQuadric(), .6, 4, 30, 50, 30, 180);
	glPopMatrix();

	glPopMatrix();
}

void drawLeftArm()
{
	GLfloat MaterialColor[] = { 1.0, 1.0, 0, 1.0 };
	GLfloat ambientColor[] = { 1.0, 1.0, 0, 1.0 };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialColor);

	// Same with the cannon on top of the head but different translate & rotate
	glPushMatrix();

	glTranslatef(1.2 * robotBodyWidth + 1.5 * upperArmWidth, 1.5 * robotBodyWidth, 0.0);
	glRotatef(gunAngle, 1.0, 0.0, 0.0);

	glPushMatrix();
	glRotatef(-90, 0, 1, 0);
	glScalef(1, 1, 5);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 40, 30);
	glPopMatrix();

	glScalef(1.4,1.4,1.4);
	gluSphere(gluNewQuadric(), 1, 30, 10);

	glRotatef(30, 1, 0, 0);
	glScalef(.7,.7,6);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 40, 30);
	glPopMatrix();
}

void drawRightArm()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	// Same with left arm but mirrored on x axis
	glPushMatrix();

	glTranslatef(-(1.2 * robotBodyWidth + 1.5 * upperArmWidth), 1.5 * robotBodyWidth, 0.0);
	glRotatef(gunAngle, 1.0, 0.0, 0.0);

	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	glScalef(1, 1, 5);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 40, 30);
	glPopMatrix();

	glScalef(1.4, 1.4, 1.4);
	gluSphere(gluNewQuadric(), 1, 30, 10);

	glRotatef(-30, 1, 0, 0);
	glScalef(.7, .7, 6);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 40, 30);
	glPopMatrix();
}

// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool stop = false;

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		robotAngle += 2.0;
		break;
	case 'd':
		robotAngle -= 2.0;
		break;
	case 'f':
		movement += 0.3;
		wheelRotate += 3;
		break;
	case 'b':
		movement -= 0.3;
		wheelRotate -= 3;
		break;
	case 'k':
		stop = true;
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


void animationHandler(int param)
{
	if (!stop)
	{
		cubeAngle += 2.0;
		glutPostRedisplay();
		glutTimerFunc(10, animationHandler, 0);
	}
}



// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_F1)
	{
		std::cout << "Rotate bot: Press A to turn left, D to turn right \n";
		std::cout << "Move boot: Press F to move forward, B to backward \n";
		std::cout << "Rotate cannon: Arrow key up to go upward, Arrow key down to go downward";

	}
	// Do transformations with arrow keys
	else if (key == GLUT_KEY_UP)
	{
		gunAngle -= 3.0;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		gunAngle += 3.0;
	}
	glutPostRedisplay();   // Trigger a window redisplay
}

