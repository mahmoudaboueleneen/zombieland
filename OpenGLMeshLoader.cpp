#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>

// Utility Classes
class Vector {
	public:
		GLdouble x, y, z;
		Vector() {}
		Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}

		//================================================================================================//
		// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
		// Here we are overloading the += operator to add a given value to all vector coordinates.        //
		//================================================================================================//
		void operator +=(float value) {
			x += value;
			y += value;
			z += value;
		}
};

// Global Variables
int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex;

char title[] = "Zombieland";
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 1000;

Vector Eye(20, 5, 20);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);

int cameraZoom = 0;

Model_3DS model_car;
Model_3DS model_jeep;
Model_3DS model_spaceCraft;
Model_3DS model_tree;
Model_3DS model_player;
Model_3DS model_moster;
Model_3DS model_statue;
Model_3DS model_medkit;
Model_3DS model_medicine2;
Model_3DS model_medicine3;
Model_3DS model_rocks;
Model_3DS model_house;
Model_3DS model_bunker;
Model_3DS model_zombie;
Model_3DS model_zombie2;
Model_3DS model_fence;
Model_3DS model_streetlamp;

GLTexture tex_ground;

// Configs
void InitLightSource() {
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}
void InitMaterial() {
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}
void myInit(void) {
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

// Drawing
void drawGround() {
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-40, 0, -40);
	glTexCoord2f(5, 0);
	glVertex3f(40, 0, -40);
	glTexCoord2f(5, 5);
	glVertex3f(40, 0, 40);
	glTexCoord2f(0, 5);
	glVertex3f(-40, 0, 40);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}
void drawSurroundingStatues() {
	int x = 40;
	for (int i = 0;i > -10;i--) {
		glPushMatrix();
		glTranslatef(-40, 0.0, x+ (10*i) );
		glRotatef(0, 0, 1, 0);
		glScaled(0.1, 0.1, 0.1);
		model_statue.Draw();
		glPopMatrix();
	}

	for (int i = 0;i > -10;i--) {
		glPushMatrix();
		glTranslatef(x + (10 * i), 0.0,-50);
		glRotatef(-90, 0, 1, 0);
		glScaled(0.1, 0.1, 0.1);
		model_statue.Draw();
		glPopMatrix();
	}
	
}

// Display
void myDisplay(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	// Draw Ground
	drawGround();

	// Draw Tree
	glPushMatrix();
	glTranslatef(10, 0, 0);
	glScalef(0.7, 0.7, 0.7);
	model_tree.Draw();
	glPopMatrix();

	// Draw Car
	glPushMatrix();
	glRotatef(90.f, 1, 0, 0);
	glScaled(0.7, 0.7, 1);
	model_car.Draw();
	glPopMatrix();

	// Draw Player
	glPushMatrix();
	glTranslatef(0, 0, 10);
	glRotatef(-90, 0, 1, 0);
	glScaled(0.03, 0.03, 0.03);
	model_player.Draw();
	glPopMatrix();

	// Draw Monster
	glPushMatrix();
	glTranslatef(0, 0.0, 20);
	glRotatef(130, 0, 1,0);
	glScaled(0.02, 0.02, 0.02);
	model_moster.Draw();
	glPopMatrix();

	// Draw Surrounding Statues (Walls)
	drawSurroundingStatues();
	
	// Draw Medkit
    glPushMatrix();
	glTranslatef(20, 0.0, 20);
	glRotatef(130, 0, 1, 0);
	glScaled(0.05, 0.05, 0.05);
	model_medkit.Draw();
	glPopMatrix();

	// Draw Medicine2
	glPushMatrix();
	glTranslatef(18, 0.0, 20);
	glRotatef(130, 0, 1, 0);
	glScaled(0.05, 0.05, 0.05);
	model_medicine2.Draw();
	glPopMatrix();

	// Draw Jeep
	glPushMatrix();
	glTranslatef(18, 3, 20);
	glRotatef(130, 0, 1, 0);
	glScaled(0.1, 0.1, 0.1);
	model_jeep.Draw();
	glPopMatrix();

	// Draw SpaceCraft
	glPushMatrix();
	glTranslatef(20, 0, -10);
	glRotatef(130, 0, 1, 0);
	glScaled(0.1, 0.1, 0.1);
	model_spaceCraft.Draw();
	glPopMatrix();

	// Draw Rocks
	glPushMatrix();
	glTranslatef(20, 0, 32);
	glRotatef(130, 0, 1, 0);
	glScaled(0.05, 0.05, 0.05);
	model_rocks.Draw();
	glPopMatrix();

	// Draw House
	glPushMatrix();
	glTranslatef(-30, 0, -20);
	glRotatef(90.f, 1, 0, 0);
	glScaled(3, 3, 3);
	model_house.Draw();
	glPopMatrix();

	// Draw Bunker
	glPushMatrix();
	glTranslatef(-20, 0, 32);
	glRotatef(90, 0, 1, 0);
	glScaled(0.01, 0.01, 0.01);
	model_bunker.Draw();
	glPopMatrix();

	// Draw Zombie
	glPushMatrix();
	glTranslatef(20, 3.3, 10);
	glRotatef(90.f, 1, 0, 0);
	glRotatef(270.f, 0, 0, 1);
	glScaled(3, 3, 3);
	model_zombie.Draw();
	glPopMatrix();

	// Draw Zombie 2
	glPushMatrix();
	glTranslatef(27, 0, 10);
	glScaled(0.07, 0.07, 0.07);
	model_zombie2.Draw();
	glPopMatrix();

	// Draw Fence
	glPushMatrix();
	glTranslatef(20, 0, 0);
	glRotatef(0.0f, 1, 0, 0);
	glScaled(0.01, 0.01, 0.01);
	model_fence.Draw();
	glPopMatrix();

	// Draw Streetlamp
	glPushMatrix();
	glTranslatef(25, 0, 0);
	glRotatef(0.0f, 1, 0, 0);
	glScaled(1, 1, 1);
	model_streetlamp.Draw();
	glPopMatrix();
	
	// Draw Skybox
	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);
	glPopMatrix();

	glutSwapBuffers();
}

// Movement
void myKeyboard(unsigned char button, int x, int y) {
	switch (button)
	{
	case 'w':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'r':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}
void myMotion(int x, int y) {
	y = HEIGHT - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();
}
void myMouse(int button, int state, int x, int y) {
	y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
}

// Reshape
void myReshape(int w, int h) {
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

// Load Assets
void LoadAssets() {
	// Loading Model files
	model_car.Load("Models/car/sportsCar.3ds");
	model_car.Materials[1].tex.BuildColorTexture(0, 0, 0);
	model_car.Materials[2].tex.BuildColorTexture(0, 0, 0);
	model_car.Materials[4].tex.BuildColorTexture(0, 0, 0);
	model_car.Materials[5].tex.BuildColorTexture(0, 0, 0);
	model_car.Materials[6].tex.BuildColorTexture(0, 0, 0);
	model_car.Materials[3].tex.BuildColorTexture(129, 12, 12);
	model_car.rot.x = -90.0f;
	/*m.rot.y = 30.0f;
	 m.rot.z = 0.0f;*/
	model_car.pos.z = -2.3f;
	// m.pos.y = 0.0f;
	// m.pos.z = 0.0f;

	model_jeep.Load("Models/jeep/jeep.3ds");

	model_tree.Load("Models/tree/Tree1.3ds");
	model_player.Load("Models/player/Soldier US N260412.3ds");
	//model_player.Materials[5].tex.BuildColorTexture(0, 0, 0);

	model_moster.Load("Models/monster/death.3ds");
	//model_moster.Materials[0].tex.BuildColorTexture(0, 0, 0);

	model_statue.Load("Models/obstacles/Acient_Statue_02.3ds");
	model_medkit.Load("Models/medicine/medicines/Box.3ds");
	model_medicine2.Load("Models/medicine/medicines/Bottle.3ds");
	//model_spaceCraft.Load("Models/rocket/Soyuz.3ds");
	model_rocks.Load("Models/obstacles/Campfire.3ds");
	model_house.Load("Models/house/house.3ds");
	model_bunker.Load("Models/bunker/Ruin.3ds");

	model_zombie.Load("Models/zombie/ZOMBIE.3ds");
	model_zombie2.Load("Models/zombie2/monster.3ds");

	model_fence.Load("Models/fence/fence_01_3ds.3ds");
	model_streetlamp.Load("Models/streetlamp/Lamp.3ds");

	// Loading Texture files
	tex_ground.Load("Textures/ground.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

// Main
void main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKeyboard);
	glutMotionFunc(myMotion);
	glutMouseFunc(myMouse);
	glutReshapeFunc(myReshape);

	myInit();

	LoadAssets();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}