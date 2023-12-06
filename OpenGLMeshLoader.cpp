/*
* Please note that this project was heavily rushed due to tight deadlines.
* 
* TODO:
* Move variables that belong to a specific scene to their respective scene class.
* Fix the hardcoded nature of the bounding box angles, as they don't rotate if we rotate their model. (Do like Fence class)
* Fix performance issues related to too many objects in memory, and player speed being decreased by it.
* Fix HUD colors being affected by lighting.
* Making health circle thicker caused bounding boxes to also be thicker.
* Remove code duplication.
* Reorder code blocks logically and add function headers to the top of the file.
* Fix the warnings for floating point type conversion possible loss of precision.
*/
#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <chrono>
#include <vector>
#include <irrKlang.h>
using namespace irrklang;

#define DEG2RAD(a) (a * 0.0174532925)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Function declarations
void KeyboardDown(unsigned char button, int x, int y);
void KeyboardUp(unsigned char button, int x, int y);
void MouseMoved(int x, int y);

// Enums
enum CameraMode { FIRST_PERSON, THIRD_PERSON };

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

		bool operator !=(const Vector& other) const {
			return x != other.x || y != other.y || z != other.z;
		}

		Vector& operator +=(const Vector& other) {
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Vector operator +(const Vector& v) const {
			return Vector(x + v.x, y + v.y, z + v.z);
		}

		Vector operator -(const Vector& v) const {
			return Vector(x - v.x, y - v.y, z - v.z);
		}

		Vector operator *(float value) const {
			return Vector(x * value, y * value, z * value);
		}

		Vector operator /(float value) const {
			return Vector(x / value, y / value, z / value);
		}

		Vector cross(const Vector& v) const {
			return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
		}

		void normalize() {
			float magnitude = sqrt(x * x + y * y + z * z);
			x /= magnitude;
			y /= magnitude;
			z /= magnitude;
		}

		float length() const {
			return sqrt(x * x + y * y + z * z);
		}
};
class BoundingBox {
public:
	Vector minPoint;  // minimum x, y, z coordinates
	Vector maxPoint;  // maximum x, y, z coordinates

	BoundingBox() {}
	BoundingBox(Vector _minPoint, Vector _maxPoint) : minPoint(_minPoint), maxPoint(_maxPoint) {}

	// Check if this bounding box intersects with another one
	// Modified to only check for intersection in the x and z axes
	// ignoring the verticality of the bounding boxes.
	bool intersects(const BoundingBox& other) const {
		return (minPoint.x <= other.maxPoint.x && maxPoint.x >= other.minPoint.x) &&
			(minPoint.z <= other.maxPoint.z && maxPoint.z >= other.minPoint.z);
	}

};

// Utility Functions
Vector rotateY(Vector point, Vector origin, float angle) {
	float rad = angle * (M_PI / 180.0f); // Convert to radians
	float sinAngle = sin(rad);
	float cosAngle = cos(rad);
	Vector translatedPoint = point - origin;
	return Vector(
		translatedPoint.x * cosAngle - translatedPoint.z * sinAngle,
		translatedPoint.y,
		translatedPoint.x * sinAngle + translatedPoint.z * cosAngle
	) + origin;
}

// Model Classes
class Ghost {
public:
	Model_3DS model_ghost;
	Vector ghostPosition;
	BoundingBox ghostBoundingBox;
	GLdouble ghostAngle;
	int damage;

	Ghost()
	{
	}

	Ghost(Model_3DS model, Vector position)
		: model_ghost(model),
		ghostPosition(position),
		ghostBoundingBox(position - Vector(1, 1, 1), position + Vector(1, 7, 1)),
		ghostAngle(0),
		damage(25)
	{
	}
};
class Medkit {
public:
	Model_3DS model_medkit;
	Vector medkitPosition;
	BoundingBox medkitBoundingBox;
	int worth;

	Medkit()
	{
	}

	Medkit(Model_3DS model, Vector position)
		: model_medkit(model),
		medkitPosition(position),
		medkitBoundingBox(position - Vector(1, 1, 1), position + Vector(1, 1, 1)),
		worth(100)
	{
	}
};
class Fence {
public:
	Model_3DS model_fence;
	Vector fencePosition;
	BoundingBox fenceBoundingBox;
	float rotationAngle;

	Fence()
	{
	}

	Fence(Model_3DS model, Vector position, float angle)
		: model_fence(model),
		fencePosition(position),
		rotationAngle(angle)
	{
		Vector minCorner = position - Vector(0.1, 1, 0.1);
		Vector maxCorner = position + Vector(8, 8, 0.5);
		minCorner = rotateY(minCorner, position, -angle); // Reverse the rotation
		maxCorner = rotateY(maxCorner, position, -angle); // Reverse the rotation
		fenceBoundingBox = BoundingBox(minCorner, maxCorner);
	}
};
class Jeep {
public:
	Model_3DS model_jeep;
	Vector jeepPosition;
	BoundingBox jeepBoundingBox;

	Jeep()
	{
	}

	Jeep(Model_3DS model, Vector position)
		: model_jeep(model),
		jeepPosition(position),
		jeepBoundingBox(position - Vector(0.5, 10, 2) + Vector(0, 0, 8), position + Vector(10, 10, 17) + Vector(0, 0, 8))
	{
	}
};
class Zombie {
public:
	Model_3DS model_zombie;
	Vector zombiePosition;
	BoundingBox zombieBoundingBox;
	GLdouble zombieAngle;
	int damage;

	Zombie()
	{
	}

	Zombie(Model_3DS model, Vector position)
		: model_zombie(model),
		zombiePosition(position),
		zombieBoundingBox(position - Vector(1, 3.5, 1), position + Vector(1, 3, 1)),
		zombieAngle(0),
		damage(25)
	{
	}
};
class Medicine {
public:
	Model_3DS model_medicine;
	Vector medicinePosition;
	BoundingBox medicineBoundingBox;
	int worth;

	Medicine()
	{
	}

	Medicine(Model_3DS model, Vector position)
		: model_medicine(model),
		medicinePosition(position),
		medicineBoundingBox(position - Vector(1, 1, 1), position + Vector(1, 1, 1)),
		worth(100)
	{
	}
};
class Tree {
public:
	Model_3DS model_tree;
	Vector treePosition;
	BoundingBox treeBoundingBox;

	Tree()
	{
	}

	Tree(Model_3DS model, Vector position)
		: model_tree(model),
		treePosition(position),
		treeBoundingBox(position - Vector(1, 1, 1), position + Vector(1, 1, 1))
	{
	}
};
class Rock {
public:
	Model_3DS model_rocks;
	Vector rocksPosition;
	BoundingBox rocksBoundingBox;

	Rock()
	{
	}

	Rock(Model_3DS model, Vector position)
		: model_rocks(model),
		rocksPosition(position),
		rocksBoundingBox(position - Vector(1, 1, 1) + Vector(-3, 0, -7), position + Vector(10, 10, 10) + Vector(-3, 0, -7))
	{
	}
};
class House {
public:
	Model_3DS model_house;
	Vector housePosition;
	BoundingBox houseBoundingBox;

	House()
	{
	}

	House(Model_3DS model, Vector position)
		: model_house(model),
		housePosition(position),
		houseBoundingBox(position - Vector(1, 1, 1) + Vector(-6, 0, -7), position + Vector(16, 16, 16) + Vector(-6, 0, -7))
	{
	}
};
class Bunker {
public:
	Model_3DS model_bunker;
	Vector bunkerPosition;
	BoundingBox bunkerBoundingBox;

	Bunker()
	{
	}

	Bunker(Model_3DS model, Vector position)
		: model_bunker(model),
		bunkerPosition(position),
		bunkerBoundingBox(position - Vector(1, 1, 1) + Vector(0, 0, -8), position + Vector(15, 15, 15) + Vector(0, 0, -8))
	{
	}
};
class Streetlamp {
public:
	Model_3DS model_streetlamp;
	Vector streetlampPosition;
	BoundingBox streetlampBoundingBox;

	Streetlamp()
	{
	}

	Streetlamp(Model_3DS model, Vector position)
		: model_streetlamp(model),
		streetlampPosition(position),
		streetlampBoundingBox(position - Vector(1, 1, 1), position + Vector(1, 8, 1))
	{
	}
};
class Player {
public:
	Model_3DS model_player;
	Vector playerPosition;
	BoundingBox playerBoundingBox;
	GLdouble playerAngle;
	int health;
	int score;

	Player()
	{
	}

	Player(Model_3DS model, Vector position)
		: model_player(model),
		playerPosition(position),
		playerBoundingBox(position - Vector(1, 1, 1), position + Vector(1, 1, 1)),
		playerAngle(0),
		health(100),
		score(0)
	{
	}
};

// Scene Classes
class Scene1 {
public:
	Player player;
	Bunker bunker;
	std::vector<Rock> rocks;
	std::vector<Tree> trees;
	std::vector<Medicine> medicines;
	std::vector<Zombie>	zombies;
};
class Scene2 {
public:
	Player player;
	House house;
	Streetlamp streetlamp;
	std::vector<Jeep> jeeps;
	std::vector<Fence> fences;
	std::vector<Medkit> medkits;
	std::vector<Ghost> ghosts;
};

// Global Variables
Scene1 scene1;
Scene2 scene2;
int currentScene = 1;

int scoreToBeDisplayed;

int WIDTH = 1280;
int HEIGHT = 720;
char title[] = "Zombieland";

GLuint tex; // for sky
GLTexture tex_ground;
GLuint sunTex;

GLuint tex2; // for dark sky
GLTexture tex_ground2;
GLuint sun2Tex;

GLdouble fovy = 90.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 1000;
Vector Eye(20, 5, 20);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);
CameraMode cameraMode = THIRD_PERSON;
int cameraZoom = 0;
int lastMouseX = -1, lastMouseY = -1;
float cameraYaw = 0, cameraPitch = 0;

// Unused for now
Model_3DS model_car;
Model_3DS model_spaceCraft;
Model_3DS model_statue;
Model_3DS model_medicine3;
Model_3DS model_zombie2;

// Models in use
Model_3DS model_player;
Model_3DS model_rocks;
Model_3DS model_tree;
Model_3DS model_medicine;
Model_3DS model_bunker;
Model_3DS model_zombie;
Model_3DS model_jeep;
Model_3DS model_fence;
Model_3DS model_medkit;
Model_3DS model_house;
Model_3DS model_ghost;
Model_3DS model_streetlamp;

bool keys[256];
int totalGameTime = 30;
int startTime;

bool isGameOver;

// For handling jumping
auto lastFrameTime = std::chrono::high_resolution_clock::now();
bool isJumping = false;
float verticalVelocity = 0;
float jumpTime = 0;
float gravity = 9.8;

// For handling attack cooldown
float lastHitTime = 0;
float hitCooldown = 0.75f;

// For handling footstep sound and not playing it too often/every frame in scene 1
int scene1FootstepCounter = 0;
const int SCENE1_FOOTSTEP_THRESHOLD = 40;

// For handling footstep sound and not playing it too often/every frame in scene 2
int scene2FootstepCounter = 0;
const int SCENE2_FOOTSTEP_THRESHOLD = 50;

// For handling zombie sounds every interval of time
float lastZombieSoundTime = 0.0f;
float zombieSoundCooldown = 4.0f;
float zombieSoundDistance = 15.0f;

// For handling ghost sounds every interval of time
float lastGhostSoundTime = 0.0f;
float ghostSoundCooldown = 3.0f;
float ghostSoundDistance = 15.0f;

Vector sunPosition(50 + 70, 70, 0);
GLfloat streetlampLightIntensity = 0.5f;

// irrKlang sound engine
ISoundEngine* soundEngine;

// Config
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
void InitSunLight() {
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpenGL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f }; // Increase these values
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Increase these values
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	// Position the light at the same position as the sun
	GLfloat light_position[] = { sunPosition.x, sunPosition.y, sunPosition.z, 1.0f };
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
void Init(void) {
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

	// InitLightSource();

	InitSunLight();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}
void Reshape(int w, int h) {
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
void LoadAssets() {
	// Loading Model files

	/*
	model_car.Load("Models/car/sportsCar.3ds");
	model_car.Materials[1].tex.BuildColorTexture(0, 0, 0);
	model_car.Materials[2].tex.BuildColorTexture(0, 0, 0);
	model_car.Materials[4].tex.BuildColorTexture(0, 0, 0);
	model_car.Materials[5].tex.BuildColorTexture(0, 0, 0);
	model_car.Materials[6].tex.BuildColorTexture(0, 0, 0);
	model_car.Materials[3].tex.BuildColorTexture(129, 12, 12);
	model_car.rot.x = -90.0f;
	// m.rot.y = 30.0f;
	// m.rot.z = 0.0f;
	model_car.pos.z = -2.3f;
	// m.pos.y = 0.0f;
	// m.pos.z = 0.0f;
	*/

	// model_spaceCraft.Load("Models/rocket/Soyuz.3ds");
	// model_statue.Load("Models/obstacles/Acient_Statue_02.3ds");

	model_jeep.Load("Models/jeep/jeep.3ds");

	model_tree.Load("Models/tree/Tree1.3ds");
	model_player.Load("Models/player/Soldier US N260412.3ds");
	//model_player.Materials[5].tex.BuildColorTexture(0, 0, 0);

	model_ghost.Load("Models/monster/death.3ds");
	//model_ghost.Materials[0].tex.BuildColorTexture(0, 0, 0);

	model_medkit.Load("Models/medicine/medicines/Box.3ds");
	model_medicine.Load("Models/medicine/medicines/Bottle.3ds");
	model_rocks.Load("Models/obstacles/Campfire.3ds");
	model_house.Load("Models/house/house.3ds");
	model_bunker.Load("Models/bunker/Ruin.3ds");

	model_zombie.Load("Models/zombie/ZOMBIE.3ds");
	//model_zombie2.Load("Models/zombie2/monster.3ds");

	model_fence.Load("Models/fence/fence_01_3ds.3ds");
	model_streetlamp.Load("Models/streetlamp/Lamp.3ds");

	// Loading Texture files
	tex_ground.Load("Textures/grass-ground.bmp");
	tex_ground2.Load("Textures/stone-ground.bmp");
	loadBMP(&tex, "Textures/clear-sky-spherical.bmp", true);
	loadBMP(&tex2, "Textures/dark-clouds.bmp", true);
	loadBMP(&sunTex, "Textures/sun.bmp", true);
	loadBMP(&sun2Tex, "Textures/sun2.bmp", true);
}

// Misc
void DisableControls() {
	glutKeyboardFunc(NULL);
	glutKeyboardUpFunc(NULL);
	glutPassiveMotionFunc(NULL);
}
void EnableControls() {
	glutKeyboardFunc(KeyboardDown);
	glutKeyboardUpFunc(KeyboardUp);
	glutPassiveMotionFunc(MouseMoved);
}
void ResetControls() {
	for (int i = 0; i < 256; i++) {
		keys[i] = false;
	}
}

// Drawing
void drawGround(GLTexture groundTexture) {
	glEnable(GL_LIGHTING);

	GLfloat mat_ambient[] = { 0.6f, 0.6f, 0.6f, 1.0f };
	GLfloat mat_diffuse[] = { 0.6f, 0.6f, 0.6f, 1.0f };
	GLfloat mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat high_shininess[] = { 50.0 };

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

	glColor3f(1.0f, 1.0f, 1.0f);

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, groundTexture.texture[0]);

	/*
	 * OpenGL calculates lighting on a per-vertex basis, not per-pixel. 
	 * This means that the lighting is calculated at the vertices of your ground quad, 
	 * and the colors are then interpolated across the surface of the quad.
	 * If the ground is a single large quad, the vertices are at the corners, 
	 * so the lighting calculations are done at the corners. 
	 * If the light source is not close to a corner, the lighting effect of a spotlight might not be visible, 
	 * because the interpolated color across the surface of the quad does not vary much.
	 * To fix this, we can divide the ground into smaller quads (a grid of quads instead of a single large quad). 
	 * This way, there will be more vertices close to the light source, and the spotlight effect should be more visible.
	*/
	int groundSize = 90;
	int numQuads = 20;  // Number of quads per side
	float quadSize = (float)groundSize / numQuads;  // Size of each quad

	// Draw a grid of quads
	for (float x = -groundSize / 2; x < groundSize / 2; x += quadSize) {
		for (float z = -groundSize / 2; z < groundSize / 2; z += quadSize) {
			glBegin(GL_QUADS);
			glNormal3f(0, 1, 0);  // Set quad normal direction.
			glTexCoord2f(0, 0);  // Set tex coordinates
			glVertex3f(x, 0, z);
			glTexCoord2f(1, 0);
			glVertex3f(x + quadSize, 0, z);
			glTexCoord2f(1, 1);
			glVertex3f(x + quadSize, 0, z + quadSize);
			glTexCoord2f(0, 1);
			glVertex3f(x, 0, z + quadSize);
			glEnd();
		}
	}

	glDisable(GL_TEXTURE_2D);  // Disable 2D texturing
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
void drawBoundingBox(const BoundingBox& box) {
	// Disable lighting to make the bounding box always visible
	glDisable(GL_LIGHTING);

	// Set the color of the bounding box to red
	glColor3f(1.0f, 0.0f, 0.0f);

	// Calculate the size of the bounding box
	Vector size = box.maxPoint - box.minPoint;

	// Draw a wireframe cube at the position of the bounding box with the size of the bounding box
	glPushMatrix();
	glTranslatef(box.minPoint.x + size.x / 2, box.minPoint.y + size.y / 2, box.minPoint.z + size.z / 2);
	glScalef(size.x, size.y, size.z);
	glutWireCube(1.0);
	glPopMatrix();

	// Re-enable lighting
	glEnable(GL_LIGHTING);
}
void drawTime(int remainingTime) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1.0f, 1.0f, 1.0f);

	glRasterPos2f(0.01f, 0.95f);

	char timeStr[20];
	sprintf(timeStr, "Remaining Time: %d", remainingTime);

	int len = strlen(timeStr);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, timeStr[i]);
	}

	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
}
void drawScore(int score) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(0, 0, 0);  // Set the color of the text to white

	glRasterPos2f(0.01f, 0.90f);  // Position the text

	char scoreStr[20];
	sprintf(scoreStr, "Score: %d", score);  // Convert the score to a string

	int len = strlen(scoreStr);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreStr[i]);  // Draw each character of the score string
	}

	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
}
void drawHealth(float health) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Disable lighting
	glDisable(GL_LIGHTING);

	// Set the line width
	glLineWidth(5.0f);  // Adjust as needed

	// Set the color of the health bar based on the player's health
	if (health > 0.25) {
		glColor3f(0, 1, 0);  // White color for health greater than 25%
	}
	else {
		glColor3f(1, 0, 0);  // Red color for health 25% or less
	}

	// Draw the circular health bar
	float radius = 0.05f;  // Radius of the health bar
	float centerX = 0.9f;  // x-coordinate of the center of the health bar
	float centerY = 0.15f;  // y-coordinate of the center of the health bar
	int numSegments = 100;  // Number of segments to use for the circle
	float angleStep = 2.0f * M_PI / numSegments;  // Angle between each segment

	glBegin(GL_LINE_STRIP);
	for (int i = numSegments; i >= (1.0f - health) * numSegments; i--) {
		float angle = M_PI / 2 - i * angleStep;  // Angle of the current segment, starting from the top of the circle
		float x = centerX + cos(angle) * radius;  // x-coordinate of the current segment
		float y = centerY + sin(angle) * radius;  // y-coordinate of the current segment
		glVertex2f(x, y);
	}
	glEnd();

	// Draw the health value in the center of the health circle
	glRasterPos2f(centerX - 0.02f, centerY - 0.01f);
	char healthStr[20];
	sprintf(healthStr, "%d%%", (int)(health * 100));
	int len = strlen(healthStr);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, healthStr[i]);  // Draw each character of the health string
	}

	// Re-enable lighting
	glEnable(GL_LIGHTING);

	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
}

// Display
void DisplayFirstScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);

	glEnable(GL_LIGHTING);

	// Setup sunlight
	glEnable(GL_LIGHT0);

	GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	GLfloat light_position[] = { sunPosition.x, sunPosition.y, sunPosition.z, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	// Set up player spotlight light source coming from the sky down on the player and following him
	GLfloat player_spotlight_position[] = { scene1.player.playerPosition.x, scene1.player.playerPosition.y + 10, scene1.player.playerPosition.z, 1.0f };
	glLightfv(GL_LIGHT1, GL_POSITION, player_spotlight_position);

	GLfloat player_spotlight_ambient_light[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightfv(GL_LIGHT1, GL_AMBIENT, player_spotlight_ambient_light);

	GLfloat player_spotlight_diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT1, GL_DIFFUSE, player_spotlight_diffuse_light);

	GLfloat player_spotlight_spotDirection[] = { 0.0f, -1.0f, 0.0f }; // assuming the light should shine straight down
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, player_spotlight_spotDirection);

	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.0f); // set cutoff angle
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0f); // set focusing strength

	glEnable(GL_LIGHT1);

	// Draw Bounding Boxes for testing
	/*
	drawBoundingBox(scene1.player.playerBoundingBox);
	drawBoundingBox(scene1.bunker.bunkerBoundingBox);
	for (auto& rock : scene1.rocks) {
		drawBoundingBox(rock.rocksBoundingBox);
	}
	for (auto& tree : scene1.trees) {
		drawBoundingBox(tree.treeBoundingBox);
	}
	for (auto& medicine : scene1.medicines) {
		drawBoundingBox(medicine.medicineBoundingBox);
	}
	for (auto& zombie : scene1.zombies) {
		drawBoundingBox(zombie.zombieBoundingBox);
	}
	*/

	// Draw Time 
	/*
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	int elapsedTime = (currentTime - startTime) / 1000;
	int remainingTime = totalGameTime - elapsedTime;

	if (remainingTime <= 0) {
		// do something
	}
	drawTime(remainingTime);
	*/

	// Draw Health
	float normalizedHealth = scene1.player.health / 100.0f;
	drawHealth(normalizedHealth);

	// Draw Score
	drawScore(scene1.player.score);

	// Draw Ground
	drawGround(tex_ground);

	// Draw Player
	glPushMatrix();
	glTranslatef(scene1.player.playerPosition.x, scene1.player.playerPosition.y, scene1.player.playerPosition.z);
	glRotatef(-90 + scene1.player.playerAngle, 0, 1, 0);
	glScaled(0.03, 0.03, 0.03);
	scene1.player.model_player.Draw();
	glPopMatrix();

	// Draw Bunker
	glPushMatrix();
	glTranslatef(scene1.bunker.bunkerPosition.x, scene1.bunker.bunkerPosition.y, scene1.bunker.bunkerPosition.z);
	glRotatef(90, 0, 1, 0);
	glRotatef(-35, 0, 1, 0);
	glScaled(0.01, 0.01, 0.01);
	scene1.bunker.model_bunker.Draw();
	glPopMatrix();

	// Draw Rocks
	for (auto& rock : scene1.rocks) {
		glPushMatrix();
		glTranslatef(rock.rocksPosition.x, rock.rocksPosition.y, rock.rocksPosition.z);
		glRotatef(130, 0, 1, 0);
		glScaled(0.12, 0.12, 0.12);
		rock.model_rocks.Draw();
		glPopMatrix();
	}

	// Draw Trees
	for (auto& tree : scene1.trees) {
		glPushMatrix();
		glTranslatef(tree.treePosition.x, tree.treePosition.y, tree.treePosition.z);
		glScalef(0.7, 0.7, 0.7);
		tree.model_tree.Draw();
		glPopMatrix();
	}

	// Draw Medicines
	for (auto& medicine : scene1.medicines) {
		glPushMatrix();
		glTranslatef(medicine.medicinePosition.x, medicine.medicinePosition.y, medicine.medicinePosition.z);
		glRotatef(130, 0, 1, 0);
		glScaled(0.05, 0.05, 0.05);
		medicine.model_medicine.Draw();
		glPopMatrix();
	}

	// Draw Zombies
	for (auto& zombie : scene1.zombies) {
		glPushMatrix();
		glTranslatef(zombie.zombiePosition.x, zombie.zombiePosition.y, zombie.zombiePosition.z);
		glRotatef(zombie.zombieAngle, 0, 1, 0);
		glRotatef(90.f, 1, 0, 0);
		glRotatef(270.f, 0, 0, 1);
		glScaled(3, 3, 3);
		zombie.model_zombie.Draw();
		glPopMatrix();
	}

	// Draw the sun
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glTranslated(sunPosition.x, sunPosition.y, sunPosition.z);
	GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	GLfloat shininess[] = { 50.0f }; // Higher values make the object shinier
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	GLUquadricObj* sunQuad;
	sunQuad = gluNewQuadric();
	glBindTexture(GL_TEXTURE_2D, sunTex);
	gluQuadricTexture(sunQuad, true);
	gluQuadricNormals(sunQuad, GL_SMOOTH);
	gluSphere(sunQuad, 5, 100, 100); // Adjust the radius as needed
	gluDeleteQuadric(sunQuad);
	glPopMatrix();
	glEnable(GL_LIGHTING);

	// Draw Skybox
	glDisable(GL_LIGHTING);
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
	glEnable(GL_LIGHTING);

	glutSwapBuffers();
}
void DisplaySecondScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);

	glEnable(GL_LIGHTING);

	// Setup darker sunlight
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	GLfloat specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	GLfloat light_position[] = { sunPosition.x, sunPosition.y, sunPosition.z, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHT0);

	// Set up streetlamp light source
	GLfloat streetlamp_light_position[] = { scene2.streetlamp.streetlampPosition.x, scene2.streetlamp.streetlampPosition.y + 10, scene2.streetlamp.streetlampPosition.z, 1.0f };
	glLightfv(GL_LIGHT1, GL_POSITION, streetlamp_light_position);

	GLfloat streetlamp_ambient_light[] = { streetlampLightIntensity, streetlampLightIntensity, streetlampLightIntensity, 1.0f };
	glLightfv(GL_LIGHT1, GL_AMBIENT, streetlamp_ambient_light);

	GLfloat streetlamp_diffuse_light[] = { streetlampLightIntensity, streetlampLightIntensity * 0.8f, streetlampLightIntensity * 0.1f, 1.0f }; // Yellowish light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, streetlamp_diffuse_light);

	GLfloat streetlamp_specular_light[] = { streetlampLightIntensity, streetlampLightIntensity * 0.8f, streetlampLightIntensity * 0.1f, 1.0f }; // Yellowish light
	glLightfv(GL_LIGHT1, GL_SPECULAR, streetlamp_specular_light);

	GLfloat spotDirection[] = { 0.0f, -1.0f, 0.0f }; // assuming the light should shine straight down
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDirection);

	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.0f); // set cutoff angle
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0f); // set focusing strength

	glEnable(GL_LIGHT1);

	// Set up player spotlight light source coming from the sky down on the player and following him
	GLfloat player_spotlight_position[] = { scene2.player.playerPosition.x, scene2.player.playerPosition.y + 10, scene2.player.playerPosition.z, 1.0f };
	glLightfv(GL_LIGHT2, GL_POSITION, player_spotlight_position);

	GLfloat player_spotlight_ambient_light[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightfv(GL_LIGHT2, GL_AMBIENT, player_spotlight_ambient_light);

	GLfloat player_spotlight_diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT2, GL_DIFFUSE, player_spotlight_diffuse_light);

	GLfloat player_spotlight_spotDirection[] = { 0.0f, -1.0f, 0.0f }; // assuming the light should shine straight down
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, player_spotlight_spotDirection);

	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 45.0f); // set cutoff angle
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 2.0f); // set focusing strength

	glEnable(GL_LIGHT2);

	// Set up fog
	GLfloat fogColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // Set the fog color to gray

	glEnable(GL_FOG); // Enable the fog
	glFogi(GL_FOG_MODE, GL_LINEAR); // Set the fog mode to GL_LINEAR for linear fog
	glFogfv(GL_FOG_COLOR, fogColor); // Set the fog color
	glFogf(GL_FOG_DENSITY, 0.35f); // Set the density for exponential fog
	glHint(GL_FOG_HINT, GL_DONT_CARE); // Let OpenGL decide which fog calculation to use
	glFogf(GL_FOG_START, 10.0f); // For linear fog, specify the start depth
	glFogf(GL_FOG_END, 100.0f); // For linear fog, specify the end depth


	/*
	// Draw Bounding Boxes for testing

	drawBoundingBox(scene2.player.playerBoundingBox);
	drawBoundingBox(scene2.house.houseBoundingBox);
	drawBoundingBox(scene2.streetlamp.streetlampBoundingBox);
	for (auto& jeep : scene2.jeeps) {
		drawBoundingBox(jeep.jeepBoundingBox);
	}
	for (auto& fence : scene2.fences) {
		drawBoundingBox(fence.fenceBoundingBox);
	}
	for (auto& medkit : scene2.medkits) {
		drawBoundingBox(medkit.medkitBoundingBox);
	}
	for (auto& ghost : scene2.ghosts) {
		drawBoundingBox(ghost.ghostBoundingBox);
	}
	*/
	 
	/*
	// Draw Time

	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	int elapsedTime = (currentTime - startTime) / 1000;
	int remainingTime = totalGameTime - elapsedTime;

	if (remainingTime <= 0) {
		// do something
	}
	drawTime(remainingTime);
	*/

	// Draw Health
	float normalizedHealth = scene2.player.health / 100.0f;
	drawHealth(normalizedHealth);

	// Draw Score
	drawScore(scene2.player.score);

	// Draw Ground
	drawGround(tex_ground2);

	// Draw Player
	glPushMatrix();
	glTranslatef(scene2.player.playerPosition.x, scene2.player.playerPosition.y, scene2.player.playerPosition.z);
	glRotatef(-90 + scene2.player.playerAngle, 0, 1, 0);
	glScaled(0.03, 0.03, 0.03);
	scene2.player.model_player.Draw();
	glPopMatrix();

	// Draw Streetlamp
	glPushMatrix();
	glTranslatef(scene2.streetlamp.streetlampPosition.x, scene2.streetlamp.streetlampPosition.y, scene2.streetlamp.streetlampPosition.z);
	glRotatef(0.0f, 1, 0, 0);
	glScaled(1.4, 1.4, 1.4);
	model_streetlamp.Draw();
	glPopMatrix();

	// Draw Ghosts
	for (auto& ghost : scene2.ghosts) {
		glPushMatrix();
		glTranslatef(ghost.ghostPosition.x, ghost.ghostPosition.y, ghost.ghostPosition.z);
		glRotatef(ghost.ghostAngle, 0, 1, 0);
		//glRotatef(120, 0, 1, 0);  // Align the model with the positive x-axis
		//glRotatef(130, 0, 1, 0);
		glScaled(0.03, 0.03, 0.03);
		ghost.model_ghost.Draw();
		glPopMatrix();
	}

	// Draw Medkits
	for (auto& medkit : scene2.medkits) {
		glPushMatrix();
		glTranslatef(medkit.medkitPosition.x, medkit.medkitPosition.y, medkit.medkitPosition.z);
		glRotatef(130, 0, 1, 0);
		glScaled(0.05, 0.05, 0.05);
		medkit.model_medkit.Draw();
		glPopMatrix();
	}

	// Draw Jeeps
	for (auto& jeep : scene2.jeeps) {
		glPushMatrix();
		glTranslatef(jeep.jeepPosition.x, jeep.jeepPosition.y, jeep.jeepPosition.z);
		glRotatef(130, 0, 1, 0);  // Existing rotation
		glRotatef(-315, 0, 1, 0);  // New rotation to align the model with the bounding box
		glScaled(0.25, 0.25, 0.25);
		jeep.model_jeep.Draw();
		glPopMatrix();
	}

	// Draw Fences
	for (auto& fence : scene2.fences) {
		glPushMatrix();
		glTranslatef(fence.fencePosition.x, fence.fencePosition.y, fence.fencePosition.z);
		glRotatef(fence.rotationAngle, 0.0f, 1.0f, 0.0f);
		glRotatef(0.0f, 1, 0, 0);
		glScaled(0.02, 0.02, 0.02);
		fence.model_fence.Draw();
		glPopMatrix();
	}

	// Draw House
	glPushMatrix();
	glTranslatef(scene2.house.housePosition.x, scene2.house.housePosition.y, scene2.house.housePosition.z);
	glRotatef(90.f, 1, 0, 0);
	glScaled(3, 3, 3);
	scene2.house.model_house.Draw();
	glPopMatrix();

	// Draw the sun
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glTranslated(sunPosition.x, sunPosition.y, sunPosition.z);
	GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	GLfloat shininess[] = { 50.0f }; // Higher values make the object shinier
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	GLUquadricObj* sunQuad;
	sunQuad = gluNewQuadric();
	glBindTexture(GL_TEXTURE_2D, sun2Tex);
	gluQuadricTexture(sunQuad, true);
	gluQuadricNormals(sunQuad, GL_SMOOTH);
	gluSphere(sunQuad, 5, 100, 100); // Adjust the radius as needed
	gluDeleteQuadric(sunQuad);
	glPopMatrix();
	glEnable(GL_LIGHTING);

	// Draw Skybox
	glDisable(GL_LIGHTING);
	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex2);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);
	glPopMatrix();
	glEnable(GL_LIGHTING);

	glDisable(GL_FOG);

	glutSwapBuffers();
}
void SwitchScene() {
	if (currentScene == 1) {
		isGameOver = false;
		glutSetCursor(GLUT_CURSOR_NONE);
		scoreToBeDisplayed = 0;
		currentScene = 2;
		glutDisplayFunc(DisplaySecondScene);
		EnableControls();
		ResetControls();
	}
	else if (currentScene == 2) {
		isGameOver = false;
		glutSetCursor(GLUT_CURSOR_NONE);
		scoreToBeDisplayed = 0;
		currentScene = 1;
		glutDisplayFunc(DisplayFirstScene);
		EnableControls();
		ResetControls();
	}

	glutPostRedisplay();
}
void DisplayDeathScreen() {
	glDisable(GL_LIGHTING);

	// Set the clear color to dark red
	glClearColor(0.3f, 0.0f, 0.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set the projection matrix to orthographic for 2D rendering
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);  // Adjust as needed

	// Set the modelview matrix to identity for 2D rendering
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Set the color to red
	glColor3f(1, 0, 0);

	// Position the text in the center of the screen
	glRasterPos2f(WIDTH / 2 - 50, HEIGHT / 2);  // Adjust as needed

	// Render the "YOU DIED" text
	const char* text = "YOU DIED";
	int len = strlen(text);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
	}

	// Restore the modelview matrix
	glPopMatrix();

	// Restore the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Set the matrix mode back to modelview
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_LIGHTING);

	glutSwapBuffers();
}
void DisplayFirstSceneWinScreen() {
	glDisable(GL_LIGHTING);

	// Set the clear color to dark green
	glClearColor(0.0f, 0.3f, 0.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set the projection matrix to orthographic for 2D rendering
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);  // Adjust as needed

	// Set the modelview matrix to identity for 2D rendering
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Set the color to green
	glColor3f(0, 1, 0);

	// Position the text in the center of the screen
	glRasterPos2f(WIDTH / 2 - 50, HEIGHT / 2);  // Adjust as needed

	// Render the "YOU WIN" text
	const char* text = "YOU SURVIVED";
	int len = strlen(text);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
	}

	// Position the score text below the "YOU WIN" text
	glRasterPos2f(WIDTH / 2 - 50, HEIGHT / 2 - 20);  // Adjust as needed

	// Render the score text
	char scoreStr[20];
	sprintf(scoreStr, "SCORE: %d", scoreToBeDisplayed);
	len = strlen(scoreStr);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreStr[i]);
	}

	// Calculate the position of the "button" though we won't draw a button
	float buttonX = WIDTH / 2 - 30;
	float buttonY = HEIGHT / 2 - 70;

	// Position the button text in the center of the button
	glRasterPos2f(buttonX, buttonY + 18 / 2);  // Adjust as needed

	glColor3f(0, 0, 0);  // Set the color to black for the text

	// Render the button text
	const char* buttonText = "NEXT LEVEL";
	len = strlen(buttonText);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buttonText[i]);
	}

	// Restore the modelview matrix
	glPopMatrix();

	// Restore the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Set the matrix mode back to modelview
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_LIGHTING);

	glutSwapBuffers();
}
void DisplaySecondSceneWinScreen() {
	glDisable(GL_LIGHTING);

	// Set the clear color to dark green
	glClearColor(0.0f, 0.3f, 0.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set the projection matrix to orthographic for 2D rendering
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);  // Adjust as needed

	// Set the modelview matrix to identity for 2D rendering
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Set the color to green
	glColor3f(0, 1, 0);

	// Position the text in the center of the screen
	glRasterPos2f(WIDTH / 2 - 50, HEIGHT / 2);  // Adjust as needed

	// Render the "YOU WIN" text
	const char* text = "YOU SURVIVED";
	int len = strlen(text);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
	}

	// Position the score text below the "YOU WIN" text
	glRasterPos2f(WIDTH / 2 - 50, HEIGHT / 2 - 20);  // Adjust as needed

	// Render the score text
	char scoreStr[20];
	sprintf(scoreStr, "SCORE: %d", scoreToBeDisplayed);
	len = strlen(scoreStr);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreStr[i]);
	}

	// Restore the modelview matrix
	glPopMatrix();

	// Restore the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Set the matrix mode back to modelview
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_LIGHTING);

	glutSwapBuffers();
}

// Update
void UpdateFirstScene(float deltaTime) {
	Vector previousPlayerPosition = scene1.player.playerPosition;

	float playerSpeed = 0.1;

	if (keys['w'] || keys['W']) {
		scene1.player.playerPosition.x += playerSpeed * cos(cameraYaw);
		scene1.player.playerPosition.z -= playerSpeed * sin(cameraYaw);
		scene1.player.playerAngle = cameraYaw * (180.0 / M_PI);  // Convert from radians to degrees
	}
	if (keys['s'] || keys['S']) {
		scene1.player.playerPosition.x -= playerSpeed * cos(cameraYaw);
		scene1.player.playerPosition.z += playerSpeed * sin(cameraYaw);
		scene1.player.playerAngle = cameraYaw * (180.0 / M_PI);  // Convert from radians to degrees
	}
	if (keys['a'] || keys['A']) {
		scene1.player.playerPosition.x -= playerSpeed * sin(cameraYaw);
		scene1.player.playerPosition.z -= playerSpeed * cos(cameraYaw);
		scene1.player.playerAngle = cameraYaw * (180.0 / M_PI);  // Convert from radians to degrees
	}
	if (keys['d'] || keys['D']) {
		scene1.player.playerPosition.x += playerSpeed * sin(cameraYaw);
		scene1.player.playerPosition.z += playerSpeed * cos(cameraYaw);
		scene1.player.playerAngle = cameraYaw * (180.0 / M_PI);  // Convert from radians to degrees
	}

	scene1.player.playerBoundingBox.minPoint = scene1.player.playerPosition - Vector(1, 1, 1);
	scene1.player.playerBoundingBox.maxPoint = scene1.player.playerPosition + Vector(1, 1, 1);

	// Check for collisions
	if (scene1.player.playerBoundingBox.intersects(scene1.bunker.bunkerBoundingBox)) {
		isGameOver = true;
		glutSetCursor(GLUT_CURSOR_INHERIT);
		DisableControls();
		soundEngine->stopAllSounds();
		soundEngine->play2D("sounds/win.wav");
		scoreToBeDisplayed = scene1.player.score;
		glutDisplayFunc(DisplayFirstSceneWinScreen);
		return;
	}
	for (auto& rock : scene1.rocks) {
		if (scene1.player.playerBoundingBox.intersects(rock.rocksBoundingBox)) {
			scene1.player.playerPosition = previousPlayerPosition;
		}
	}
	for (auto& tree : scene1.trees) {
		if (scene1.player.playerBoundingBox.intersects(tree.treeBoundingBox)) {
			scene1.player.playerPosition = previousPlayerPosition;
		}
	}
	for (auto it = scene1.medicines.begin(); it != scene1.medicines.end(); ) {
		if (scene1.player.playerBoundingBox.intersects(it->medicineBoundingBox)) {
			soundEngine->play2D("sounds/loot-item.wav");

			// Increase the player's score by the worth of the medicine
			scene1.player.score += it->worth;

			// Remove the medicine from the scene
			it = scene1.medicines.erase(it);
		}
		else {
			++it;
		}
	}

	// Update camera to follow player
	if (cameraMode == THIRD_PERSON) {
		// Calculate the camera's position based on the player's position and the camera angles
		float distanceBehindPlayer = 3.5;
		float cameraHeight = 5.5;
		float cameraLeftOffset = 0;
		Eye.x = scene1.player.playerPosition.x - cos(cameraYaw) * distanceBehindPlayer - sin(cameraYaw) * cameraLeftOffset;
		Eye.y = scene1.player.playerPosition.y + cameraHeight;
		Eye.z = scene1.player.playerPosition.z + sin(cameraYaw) * distanceBehindPlayer + cos(cameraYaw) * cameraLeftOffset;

		// Make the camera look forward from the player's perspective
		float lookAheadDistance = 5;
		At.x = Eye.x + cos(cameraYaw) * cos(cameraPitch) * lookAheadDistance;
		At.y = Eye.y + sin(cameraPitch) * lookAheadDistance;
		At.z = Eye.z - sin(cameraYaw) * cos(cameraPitch) * lookAheadDistance;
	}
	else if (cameraMode == FIRST_PERSON) {
		// Position the camera at the player's eye level
		Eye.x = scene1.player.playerPosition.x;
		Eye.y = scene1.player.playerPosition.y + 5;
		Eye.z = scene1.player.playerPosition.z;

		// Calculate the direction the player is facing
		float rad = scene1.player.playerAngle * (M_PI / 180);  // Convert angle to radians

		// Calculate the direction the camera is looking
		At.x = Eye.x + cos(cameraYaw) * cos(cameraPitch);
		At.y = Eye.y + sin(cameraPitch);
		At.z = Eye.z - sin(cameraYaw) * cos(cameraPitch);
	}

	if (isJumping) {
		// Apply the vertical velocity to the player's position
		scene1.player.playerPosition.y += verticalVelocity * deltaTime;  // deltaTime is the time elapsed since the last frame

		// Decrease the vertical velocity over time (simulate gravity)
		verticalVelocity -= gravity * deltaTime;  // gravity is the acceleration due to gravity

		// Increase the jump time
		jumpTime += deltaTime;

		// End the jump after a certain time or when the player hits the ground
		if (scene1.player.playerPosition.y <= 0) {  // Adjust this condition as needed
			isJumping = false;
			verticalVelocity = 0;  // Reset the vertical velocity
		}
	}
	else if (scene1.player.playerPosition.y > 0) {
		// If the player is not jumping but is above the ground, apply gravity
		scene1.player.playerPosition.y -= gravity * deltaTime;
		if (scene1.player.playerPosition.y < 0) scene1.player.playerPosition.y = 0;  // Prevent the player from going below the ground
	}

	// Make zombies move towards the player
	float zombieSpeed = 0.02;
	float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	for (auto& zombie : scene1.zombies) {
		Vector direction = scene1.player.playerPosition - zombie.zombiePosition;
		direction.y = 0;
		direction.normalize();
		zombie.zombiePosition += direction * zombieSpeed;

		zombie.zombieBoundingBox.minPoint = zombie.zombiePosition - Vector(1, 1, 1);
		zombie.zombieBoundingBox.maxPoint = zombie.zombiePosition + Vector(1, 1, 1);

		zombie.zombieAngle = atan2(direction.x, direction.z) * (180.0 / M_PI);  // Convert from radians to degrees

		// Check if the zombie hits the player and enough time has passed since the last hit
		if (scene1.player.playerBoundingBox.intersects(zombie.zombieBoundingBox) && currentTime - lastHitTime >= hitCooldown) {
			soundEngine->play2D("sounds/pain.wav");

			scene1.player.health -= zombie.damage;

			if (scene1.player.health <= 0) {
				isGameOver = true;
				scene1.player.health = 0;
				glutSetCursor(GLUT_CURSOR_INHERIT);
				DisableControls();
				soundEngine->stopAllSounds();
				soundEngine->play2D("sounds/dying.wav");
				glutDisplayFunc(DisplayDeathScreen);
				return;
			}

			Vector pushBackDirection = scene1.player.playerPosition - zombie.zombiePosition;
			pushBackDirection.y = 0;
			pushBackDirection.normalize();
			scene1.player.playerPosition = previousPlayerPosition + pushBackDirection * 0.1f; // Adjust the multiplier as needed

			scene1.player.playerBoundingBox.minPoint = scene1.player.playerPosition - Vector(1, 1, 1);
			scene1.player.playerBoundingBox.maxPoint = scene1.player.playerPosition + Vector(1, 1, 1);

			lastHitTime = currentTime;
		}

		float distance = (scene1.player.playerPosition - zombie.zombiePosition).length();
		if (distance <= zombieSoundDistance && currentTime - lastZombieSoundTime >= zombieSoundCooldown) {
			soundEngine->play2D("sounds/zombie-growl.wav");

			lastZombieSoundTime = currentTime;
		}

	}

	// Check if the player has moved
	if (scene1.player.playerPosition != previousPlayerPosition) {
		scene1FootstepCounter++;

		// If the counter reaches the threshold, play the footstep sound and reset the counter
		if (scene1FootstepCounter >= SCENE1_FOOTSTEP_THRESHOLD) {
			soundEngine->play2D("sounds/grass-footsteps.wav");

			scene1FootstepCounter = 0;
		}
	}
}
void UpdateSecondScene(float deltaTime) {
	Vector previousPlayerPosition = scene2.player.playerPosition;

	float playerSpeed = 0.1;

	if (keys['w'] || keys['W']) {
		scene2.player.playerPosition.x += playerSpeed * cos(cameraYaw);
		scene2.player.playerPosition.z -= playerSpeed * sin(cameraYaw);
		scene2.player.playerAngle = cameraYaw * (180.0 / M_PI);  // Convert from radians to degrees
	}
	if (keys['s'] || keys['S']) {
		scene2.player.playerPosition.x -= playerSpeed * cos(cameraYaw);
		scene2.player.playerPosition.z += playerSpeed * sin(cameraYaw);
		scene2.player.playerAngle = cameraYaw * (180.0 / M_PI);  // Convert from radians to degrees
	}
	if (keys['a'] || keys['A']) {
		scene2.player.playerPosition.x -= playerSpeed * sin(cameraYaw);
		scene2.player.playerPosition.z -= playerSpeed * cos(cameraYaw);
		scene2.player.playerAngle = cameraYaw * (180.0 / M_PI);  // Convert from radians to degrees
	}
	if (keys['d'] || keys['D']) {
		scene2.player.playerPosition.x += playerSpeed * sin(cameraYaw);
		scene2.player.playerPosition.z += playerSpeed * cos(cameraYaw);
		scene2.player.playerAngle = cameraYaw * (180.0 / M_PI);  // Convert from radians to degrees
	}

	scene2.player.playerBoundingBox.minPoint = scene2.player.playerPosition - Vector(1, 1, 1);
	scene2.player.playerBoundingBox.maxPoint = scene2.player.playerPosition + Vector(1, 1, 1);

	// Check for collisions
	if (scene2.player.playerBoundingBox.intersects(scene2.house.houseBoundingBox)) {
		isGameOver = true;
		glutSetCursor(GLUT_CURSOR_INHERIT);
		DisableControls();
		soundEngine->stopAllSounds();
		soundEngine->play2D("sounds/win.wav");
		scoreToBeDisplayed = scene2.player.score;
		glutDisplayFunc(DisplaySecondSceneWinScreen);
		return;
	}
	if (scene2.player.playerBoundingBox.intersects(scene2.streetlamp.streetlampBoundingBox)) {
		scene2.player.playerPosition = previousPlayerPosition;
	}
	for (auto& jeep : scene2.jeeps) {
		if (scene2.player.playerBoundingBox.intersects(jeep.jeepBoundingBox)) {
			scene2.player.playerPosition = previousPlayerPosition;
		}
	}
	for (auto& fence : scene2.fences) {
		if (scene2.player.playerBoundingBox.intersects(fence.fenceBoundingBox)) {
			scene2.player.playerPosition = previousPlayerPosition;
		}
	}
	for (auto it = scene2.medkits.begin(); it != scene2.medkits.end(); ) {
		if (scene2.player.playerBoundingBox.intersects(it->medkitBoundingBox)) {
			soundEngine->play2D("sounds/loot-item.wav");

			// Increase the player's score by the worth of the medkit
			scene2.player.score += it->worth;

			// Remove the medkit from the scene
			it = scene2.medkits.erase(it);
		}
		else {
			++it;
		}
	}

	// Update camera to follow player
	if (cameraMode == THIRD_PERSON) {
		// Calculate the camera's position based on the player's position and the camera angles
		float distanceBehindPlayer = 3.5;
		float cameraHeight = 5.5;
		float cameraLeftOffset = 0;
		Eye.x = scene2.player.playerPosition.x - cos(cameraYaw) * distanceBehindPlayer - sin(cameraYaw) * cameraLeftOffset;
		Eye.y = scene2.player.playerPosition.y + cameraHeight;
		Eye.z = scene2.player.playerPosition.z + sin(cameraYaw) * distanceBehindPlayer + cos(cameraYaw) * cameraLeftOffset;

		// Make the camera look forward from the player's perspective
		float lookAheadDistance = 5;
		At.x = Eye.x + cos(cameraYaw) * cos(cameraPitch) * lookAheadDistance;
		At.y = Eye.y + sin(cameraPitch) * lookAheadDistance;
		At.z = Eye.z - sin(cameraYaw) * cos(cameraPitch) * lookAheadDistance;
	}
	else if (cameraMode == FIRST_PERSON) {
		// Position the camera at the player's eye level
		Eye.x = scene2.player.playerPosition.x;
		Eye.y = scene2.player.playerPosition.y + 5;
		Eye.z = scene2.player.playerPosition.z;

		// Calculate the direction the player is facing
		float rad = scene2.player.playerAngle * (M_PI / 180);  // Convert angle to radians

		// Calculate the direction the camera is looking
		At.x = Eye.x + cos(cameraYaw) * cos(cameraPitch);
		At.y = Eye.y + sin(cameraPitch);
		At.z = Eye.z - sin(cameraYaw) * cos(cameraPitch);
	}

	if (isJumping) {
		// Apply the vertical velocity to the player's position
		scene2.player.playerPosition.y += verticalVelocity * deltaTime;  // deltaTime is the time elapsed since the last frame

		// Decrease the vertical velocity over time (simulate gravity)
		verticalVelocity -= gravity * deltaTime;  // gravity is the acceleration due to gravity

		// Increase the jump time
		jumpTime += deltaTime;

		// End the jump after a certain time or when the player hits the ground
		if (scene2.player.playerPosition.y <= 0) {  // Adjust this condition as needed
			isJumping = false;
			verticalVelocity = 0;  // Reset the vertical velocity
		}
	}
	else if (scene2.player.playerPosition.y > 0) {
		// If the player is not jumping but is above the ground, apply gravity
		scene2.player.playerPosition.y -= gravity * deltaTime;
		if (scene2.player.playerPosition.y < 0) scene2.player.playerPosition.y = 0;  // Prevent the player from going below the ground
	}

	// Make ghosts move towards the player
	float ghostSpeed = 0.02;
	float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	for (auto& ghost : scene2.ghosts) {
		Vector direction = scene2.player.playerPosition - ghost.ghostPosition;
		direction.y = 0;
		direction.normalize();
		ghost.ghostPosition += direction * ghostSpeed;

		ghost.ghostBoundingBox.minPoint = ghost.ghostPosition - Vector(1, 1, 1);
		ghost.ghostBoundingBox.maxPoint = ghost.ghostPosition + Vector(1, 1, 1);

		ghost.ghostAngle = atan2(direction.x, direction.z) * (180.0 / M_PI);  // Convert from radians to degrees

		// Check if the ghost hits the player and enough time has passed since the last hit
		if (scene2.player.playerBoundingBox.intersects(ghost.ghostBoundingBox) && currentTime - lastHitTime >= hitCooldown) {
			// Play sound effect
			soundEngine->play2D("sounds/pain.wav");


			scene2.player.health -= ghost.damage;

			if (scene2.player.health <= 0) {
				isGameOver = true;
				scene2.player.health = 0;
				glutSetCursor(GLUT_CURSOR_INHERIT);
				DisableControls();
				soundEngine->stopAllSounds();
				soundEngine->play2D("sounds/dying.wav");
				glutDisplayFunc(DisplayDeathScreen);
				return;
			}

			Vector pushBackDirection = scene2.player.playerPosition - ghost.ghostPosition;
			pushBackDirection.y = 0;
			pushBackDirection.normalize();
			scene2.player.playerPosition = previousPlayerPosition + pushBackDirection * 0.1f; // Adjust the multiplier as needed

			scene2.player.playerBoundingBox.minPoint = scene2.player.playerPosition - Vector(1, 1, 1);
			scene2.player.playerBoundingBox.maxPoint = scene2.player.playerPosition + Vector(1, 1, 1);

			lastHitTime = currentTime;
		}

		// Check if the ghost is within a certain distance of the player and enough time has passed since the last sound
		float distance = (scene2.player.playerPosition - ghost.ghostPosition).length();
		if (distance <= ghostSoundDistance && currentTime - lastGhostSoundTime >= ghostSoundCooldown) {
			soundEngine->play2D("sounds/ghost-groan.wav");

			lastGhostSoundTime = currentTime;
		}
	}

	// Check if the player has moved
	if (scene2.player.playerPosition != previousPlayerPosition) {
		scene2FootstepCounter++;

		// If the counter reaches the threshold, play the footstep sound and reset the counter
		if (scene2FootstepCounter >= SCENE2_FOOTSTEP_THRESHOLD) {
			soundEngine->play2D("sounds/concrete-footsteps.wav");

			scene2FootstepCounter = 0;
		}
	}

	// Update streetlamp light intensity
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // get elapsed time in seconds
	streetlampLightIntensity = 0.5f * (sin(time) + 1.0f); // light intensity goes up and down between 0 and 1
}
void Update() {
	if (isGameOver) {
		return;
	}

	auto currentFrameTime = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrameTime - lastFrameTime).count();
	lastFrameTime = currentFrameTime;

	if (currentScene == 1) {
		UpdateFirstScene(deltaTime);
	}
	else if (currentScene == 2) {
		UpdateSecondScene(deltaTime);
	}

	glutPostRedisplay();
}

// Controls
void KeyboardDown(unsigned char button, int x, int y) {
	keys[button] = true;

	if (button == 27) {  // Equivalent to ESC key
		exit(0);
	}
	else if (button == '1') {
		cameraMode = FIRST_PERSON;
	}
	else if (button == '2') {
		cameraMode = THIRD_PERSON;
	}
	else if (button == ' ' && !isJumping) {
		isJumping = true;
		soundEngine->play2D("sounds/jump.wav");
		verticalVelocity = 5;
		jumpTime = 0;
	}
	else if (button == 'v') {
		SwitchScene();
	}
}
void KeyboardUp(unsigned char button, int x, int y) {
	keys[button] = false;
}
void MouseMoved(int x, int y) {
	// Calculate mouse movement since last frame
	int centerX = WIDTH / 2;
	int centerY = HEIGHT / 2;
	int deltaX = x - centerX;
	int deltaY = y - centerY;

	// Adjust camera angles
	float sensitivity = 0.005f;  // Adjust as needed
	cameraYaw -= deltaX * sensitivity;
	cameraPitch -= deltaY * sensitivity;

	if (cameraMode == THIRD_PERSON) {
		// Limit pitch to avoid flipping the camera and looking under the ground
		float upperPitchLimit = M_PI / 6;  // Adjust as needed
		float lowerPitchLimit = -M_PI / 12;  // Adjust as needed
		if (cameraPitch < lowerPitchLimit) cameraPitch = lowerPitchLimit;
		if (cameraPitch > upperPitchLimit) cameraPitch = upperPitchLimit;
	}
	else if (cameraMode == FIRST_PERSON) {
		// Limit pitch to avoid flipping the camera and looking under the ground
		float pitchLimit = M_PI / 2.5;  // Adjust as needed
		if (cameraPitch < -pitchLimit) cameraPitch = -pitchLimit;
		if (cameraPitch > pitchLimit) cameraPitch = pitchLimit;
	}

	// Update player's rotation to match the camera's yaw angle
	if (currentScene == 1) {
		scene1.player.playerAngle = cameraYaw * (180.0 / M_PI);  // Convert from radians to degrees
	}
	else if (currentScene == 2) {
		scene2.player.playerAngle = cameraYaw * (180.0 / M_PI);  // Convert from radians to degrees
	}

	// Warp mouse cursor back to the center of the window
	glutWarpPointer(centerX, centerY);
}
void MouseFunc(int button, int state, int x, int y) {
	y = HEIGHT - y; // Convert from window space to 2D rendering space

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && currentScene == 1) {
		// Calculate the bounds of the text
		int textWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"NEXT LEVEL");
		int textHeight = 18; // Height of GLUT_BITMAP_HELVETICA_18

		// Check if the click is within the text's rectangle
		if (x >= WIDTH / 2 - 30 && x <= WIDTH / 2 - 30 + textWidth && y >= HEIGHT / 2 - 70 && y <= HEIGHT / 2 - 70 + textHeight) {
			printf("Switching scene...\n");

			SwitchScene();
		}
	}
}

// Main
void main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(100, 150);
	glutCreateWindow(title);

	glutDisplayFunc(DisplayFirstScene);
	glutKeyboardFunc(KeyboardDown);
	glutKeyboardUpFunc(KeyboardUp);
	glutReshapeFunc(Reshape);
	glutIdleFunc(Update);
	glutPassiveMotionFunc(MouseMoved);
	glutMouseFunc(MouseFunc);
	glutSetCursor(GLUT_CURSOR_NONE);

	Init();
	LoadAssets();

	// Create an instance of the sound engine
	soundEngine = createIrrKlangDevice();

	if (!soundEngine)
	{
		printf("Could not start up the sound engine\n");
		return;
	}

	// Play background ambience
	//soundEngine->play2D("sounds/manic-whistle.wav", true);

	// Initialize First Scene
	scene1.player = Player(model_player, Vector(20, 0, -20));
	scene1.bunker = Bunker(model_bunker, Vector(-25, 0, 30));

	scene1.rocks.push_back(Rock(model_rocks, Vector(-40, 0, 45)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-30, 0, 45)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-20, 0, 45)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-10, 0, 45)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(0, 0, 45)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(10, 0, 45)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(20, 0, 45)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(30, 0, 45)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(35, 0, 35)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(35, 0, 25)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(35, 0, 15)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(35, 0, 5)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(35, 0, -5)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(35, 0, -15)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(35, 0, -25)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(35, 0, -35)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(25, 0, -35)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(15, 0, -35)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(5, 0, -35)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-5, 0, -35)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-15, 0, -35)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-25, 0, -35)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-35, 0, -35)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-35, 0, -25)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-35, 0, -15)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-35, 0, -5)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-35, 0, 5)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-35, 0, 15)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-35, 0, 25)));
	scene1.rocks.push_back(Rock(model_rocks, Vector(-35, 0, 35)));

	scene1.rocks.push_back(Rock(model_rocks, Vector(10, 0, -10)));

	scene1.trees.push_back(Tree(model_tree, Vector(10, 0, 0)));
	scene1.trees.push_back(Tree(model_tree, Vector(5, 0, 2)));
	//scene1.trees.push_back(Tree(model_tree, Vector(-5, 0, 5)));
	//scene1.trees.push_back(Tree(model_tree, Vector(-12, 0, 7)));
	//scene1.trees.push_back(Tree(model_tree, Vector(-12, 0, -7)));
	//scene1.trees.push_back(Tree(model_tree, Vector(-14, 0, -12)));
	//scene1.trees.push_back(Tree(model_tree, Vector(-19, 0, -14)));
	//scene1.trees.push_back(Tree(model_tree, Vector(-24, 0, -10)));

	scene1.medicines.push_back(Medicine(model_medicine, Vector(14, 0, 20)));
	scene1.medicines.push_back(Medicine(model_medicine, Vector(15, 0, 22)));
	scene1.medicines.push_back(Medicine(model_medicine, Vector(-24, 0, -12)));
	scene1.medicines.push_back(Medicine(model_medicine, Vector(-24, 0, -14)));
	scene1.medicines.push_back(Medicine(model_medicine, Vector(-19, 0, -18)));

	scene1.zombies.push_back(Zombie(model_zombie, Vector(20, 3.3, 10)));
	scene1.zombies.push_back(Zombie(model_zombie, Vector(0, 3.3, 10)));
	scene1.zombies.push_back(Zombie(model_zombie, Vector(-5, 3.3, 10)));

	// Initialize Second Scene
	scene2.player = Player(model_player, Vector(25, 0, 25));
	scene2.house = House(model_house, Vector(-25, 0, -20));
	scene2.streetlamp = Streetlamp(model_streetlamp, Vector(20, 0, -15));

	scene2.fences.push_back(Fence(model_fence, Vector(33, 0, -39), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(25, 0, -39), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(17, 0, -39), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(9, 0, -39), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(1, 0, -39), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(-7, 0, -39), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(-15, 0, -39), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(-23, 0, -39), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(-31, 0, -39), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, -39), 0));

	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, -39), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, -31), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, -23), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, -15), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, -7), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, 1), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, 9), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, 17), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, 25), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, 33), -90));

	scene2.fences.push_back(Fence(model_fence, Vector(-39, 0, 40), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(-31, 0, 40), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(-23, 0, 40), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(-15, 0, 40), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(-7, 0, 40), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(1, 0, 40), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(9, 0, 40), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(17, 0, 40), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(25, 0, 40), 0));
	scene2.fences.push_back(Fence(model_fence, Vector(32, 0, 40), 0));

	scene2.fences.push_back(Fence(model_fence, Vector(40, 0, 31), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(40, 0, 23), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(40, 0, 15), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(40, 0, 7), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(40, 0, -1), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(40, 0, -9), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(40, 0, -17), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(40, 0, -25), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(40, 0, -33), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(40, 0, -38), -90));

	scene2.fences.push_back(Fence(model_fence, Vector(4, 0, 31), -90));
	scene2.fences.push_back(Fence(model_fence, Vector(4, 0, 23), -90));

	scene2.jeeps.push_back(Jeep(model_jeep, Vector(25, 7, -25)));
	scene2.jeeps.push_back(Jeep(model_jeep, Vector(-15, 7, 15)));

	scene2.medkits.push_back(Medkit(model_medkit, Vector(20, 0, 20)));
	scene2.medkits.push_back(Medkit(model_medkit, Vector(15, 0, 10)));
	scene2.medkits.push_back(Medkit(model_medkit, Vector(-10, 0, 20)));
	scene2.medkits.push_back(Medkit(model_medkit, Vector(-15, 0, 5)));
	scene2.medkits.push_back(Medkit(model_medkit, Vector(-18, 0, 7)));
	scene2.medkits.push_back(Medkit(model_medkit, Vector(-19, 0, 8)));
	scene2.medkits.push_back(Medkit(model_medkit, Vector(22, 0, -25)));
	scene2.medkits.push_back(Medkit(model_medkit, Vector(22, 0, -20)));

	scene2.ghosts.push_back(Ghost(model_ghost, Vector(-10, 0, 20)));
	scene2.ghosts.push_back(Ghost(model_ghost, Vector(-15, 0, 10)));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}