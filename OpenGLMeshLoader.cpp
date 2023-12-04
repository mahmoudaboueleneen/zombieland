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

#define DEG2RAD(a) (a * 0.0174532925)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

		Vector operator +(const Vector& v) const {
			return Vector(x + v.x, y + v.y, z + v.z);
		}

		Vector operator -(const Vector& v) const {
			return Vector(x - v.x, y - v.y, z - v.z);
		}

		Vector operator /(float value) const {
			return Vector(x / value, y / value, z / value);
		}

		Vector cross(const Vector& v) const {
			return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
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

// Model Classes
class Ghost {
public:
	Model_3DS model_ghost;
	Vector ghostPosition;
	BoundingBox ghostBoundingBox;
	GLdouble ghostAngle;

	Ghost()
	{
	}

	Ghost(Model_3DS model, Vector position)
		: model_ghost(model),
		ghostPosition(position),
		ghostBoundingBox(position - Vector(1, 1, 1), position + Vector(1, 7, 1)),
		ghostAngle(0)
	{
	}
};
class Medkit {
public:
	Model_3DS model_medkit;
	Vector medkitPosition;
	BoundingBox medkitBoundingBox;

	Medkit()
	{
	}

	Medkit(Model_3DS model, Vector position)
		: model_medkit(model),
		medkitPosition(position),
		medkitBoundingBox(position - Vector(1, 1, 1), position + Vector(1, 1, 1))
	{
	}
};
class Fence {
public:
	Model_3DS model_fence;
	Vector fencePosition;
	BoundingBox fenceBoundingBox;

	Fence()
	{
	}

	Fence(Model_3DS model, Vector position)
		: model_fence(model),
		fencePosition(position),
		fenceBoundingBox(position - Vector(0.1, 1, 0.1), position + Vector(8, 8, 0.5))
	{
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

	Zombie()
	{
	}

	Zombie(Model_3DS model, Vector position)
		: model_zombie(model),
		zombiePosition(position),
		zombieBoundingBox(position - Vector(1, 3.5, 1), position + Vector(1, 3, 1))
	{
	}
};
class Medicine {
public:
	Model_3DS model_medicine;
	Vector medicinePosition;
	BoundingBox medicineBoundingBox;

	Medicine()
	{
	}

	Medicine(Model_3DS model, Vector position)
		: model_medicine(model),
		medicinePosition(position),
		medicineBoundingBox(position - Vector(1, 1, 1), position + Vector(1, 1, 1))
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

	Player()
	{
	}

	Player(Model_3DS model, Vector position)
		: model_player(model),
		playerPosition(position),
		playerBoundingBox(position - Vector(1, 1, 1), position + Vector(1, 1, 1)),
		playerAngle(0)
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
int currentScene = 2;

int WIDTH = 1280;
int HEIGHT = 720;
char title[] = "Zombieland";

GLuint tex; // for sky
GLTexture tex_ground;

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
int currentScore = 0;

// For handling jumping
auto lastFrameTime = std::chrono::high_resolution_clock::now();
bool isJumping = false;
float verticalVelocity = 0;
float jumpTime = 0;
float gravity = 9.8;

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

	InitLightSource();

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
	tex_ground.Load("Textures/ground.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
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
		verticalVelocity = 5;
		jumpTime = 0;
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
    } else if (currentScene == 2) {
        scene2.player.playerAngle = cameraYaw * (180.0 / M_PI);  // Convert from radians to degrees
    }

	// Warp mouse cursor back to the center of the window
	glutWarpPointer(centerX, centerY);
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
		scene1.player.playerPosition = previousPlayerPosition;
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
	for (auto& medicine : scene1.medicines) {
		if (scene1.player.playerBoundingBox.intersects(medicine.medicineBoundingBox)) {
			scene1.player.playerPosition = previousPlayerPosition;
		}
	}
	for (auto& zombie : scene1.zombies) {
		if (scene1.player.playerBoundingBox.intersects(zombie.zombieBoundingBox)) {
			scene1.player.playerPosition = previousPlayerPosition;
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
		scene2.player.playerPosition = previousPlayerPosition;
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
	for (auto& medkit : scene2.medkits) {
		if (scene2.player.playerBoundingBox.intersects(medkit.medkitBoundingBox)) {
			scene2.player.playerPosition = previousPlayerPosition;
		}
	}
	for (auto& ghost : scene2.ghosts) {
		if (scene2.player.playerBoundingBox.intersects(ghost.ghostBoundingBox)) {
			scene2.player.playerPosition = previousPlayerPosition;
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
}
void Update() {
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

// Display
void DisplayFirstScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);

	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	// Draw Bounding Boxes for testing
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

	// Draw Score
	drawScore(currentScore);

	// Draw Ground
	drawGround();

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
		glRotatef(90.f, 1, 0, 0);
		glRotatef(270.f, 0, 0, 1);
		glScaled(3, 3, 3);
		zombie.model_zombie.Draw();
		glPopMatrix();
	}

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
void DisplaySecondScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);

	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

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

	// Draw Score
	drawScore(currentScore);

	// Draw Ground
	drawGround();

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

	// Draw Ghost
	for (auto& ghost : scene2.ghosts) {
		glPushMatrix();
		glTranslatef(ghost.ghostPosition.x, ghost.ghostPosition.y, ghost.ghostPosition.z);
		glRotatef(130, 0, 1, 0);
		glScaled(0.03, 0.03, 0.03);
		ghost.model_ghost.Draw();
		glPopMatrix();
	}

	// Draw Medkit
	for (auto& medkit : scene2.medkits) {
		glPushMatrix();
		glTranslatef(medkit.medkitPosition.x, medkit.medkitPosition.y, medkit.medkitPosition.z);
		glRotatef(130, 0, 1, 0);
		glScaled(0.05, 0.05, 0.05);
		medkit.model_medkit.Draw();
		glPopMatrix();
	}

	// Draw Jeep
	for (auto& jeep : scene2.jeeps) {
		glPushMatrix();
		glTranslatef(jeep.jeepPosition.x, jeep.jeepPosition.y, jeep.jeepPosition.z);
		glRotatef(130, 0, 1, 0);  // Existing rotation
		glRotatef(-315, 0, 1, 0);  // New rotation to align the model with the bounding box
		glScaled(0.25, 0.25, 0.25);
		jeep.model_jeep.Draw();
		glPopMatrix();
	}

	// Draw Fence
	for (auto& fence : scene2.fences) {
		glPushMatrix();
		glTranslatef(fence.fencePosition.x, fence.fencePosition.y, fence.fencePosition.z);
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

// Main
void main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(100, 150);
	glutCreateWindow(title);

	glutDisplayFunc(DisplaySecondScene);
	glutKeyboardFunc(KeyboardDown);
	glutKeyboardUpFunc(KeyboardUp);
	glutReshapeFunc(Reshape);
	glutIdleFunc(Update);
	glutPassiveMotionFunc(MouseMoved);
	glutSetCursor(GLUT_CURSOR_NONE);

	Init();
	LoadAssets();

	// Initialize First Scene
	scene1.player = Player(model_player, Vector(0, 0, 10));
	scene1.bunker = Bunker(model_bunker, Vector(-20, 0, 20));
	scene1.rocks.push_back(Rock(model_rocks, Vector(20, 0, 32)));
	scene1.trees.push_back(Tree(model_tree, Vector(10, 0, 0)));
	scene1.medicines.push_back(Medicine(model_medicine, Vector(14, 0, 20)));
	scene1.zombies.push_back(Zombie(model_zombie, Vector(20, 3.3, 10)));

	// Initialize Second Scene
	scene2.player = Player(model_player, Vector(0, 0, 10));
	scene2.house = House(model_house, Vector(-30, 0, -20));
	scene2.streetlamp = Streetlamp(model_streetlamp, Vector(20, 0, -15));
	scene2.jeeps.push_back(Jeep(model_jeep, Vector(25, 7, -25)));
	scene2.fences.push_back(Fence(model_fence, Vector(15, 0, -5)));
	scene2.medkits.push_back(Medkit(model_medkit, Vector(20, 0, 20)));
	scene2.ghosts.push_back(Ghost(model_ghost, Vector(0, 0, 20)));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}