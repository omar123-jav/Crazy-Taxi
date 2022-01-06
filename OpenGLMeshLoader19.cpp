#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <time.h>
#include <string>
#include <Windows.h>
#include <cmath>
#include <glut.h>

int WIDTH = 1280;
int HEIGHT = 720;

bool keyStates[256];

GLuint tex;
char title[] = "Crazy Taxi";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 500;
GLdouble playerRotation;

//Obstacle Information
GLfloat obstacleXs[134];
GLfloat obstacleZs[134];
GLfloat obstacleLs[134];
GLfloat obstacleWs[134];

//Variables
float score = 0;
int destination = 0;
int gametime = 2000;
int passengertime = 0;
int passenger = 0;
bool cutscenemode = false;
bool cutsceneoutmode = false;
bool levelcomplete = false;
int level = 1;
int passengersAvailable = 2;
int camRotDir = 1;
int lightDir = 1;
bool passenger1Done = false;
bool passenger2Done = false;
float xextra = -5;
float zextra = 0;
float ytranslate = 0;

//Passengers
GLfloat PassengerXs[2];
GLfloat PassengerZs[2];

//Destinations
GLfloat DestinationXs[3];
GLfloat DestinationZs[3];
GLfloat DestinationLs[3];
GLfloat DestinationWs[3];

//Light
GLfloat lightx = 0;
GLfloat lighty = 0;
GLfloat lightz = 0;

#define PI 3.141592654f;

class Vector
{
public:
	GLdouble x, y, z;
	Vector() {}
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}

	void operator +=(Vector v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
	}

	void operator -=(Vector v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}

	Vector operator *(float value)
	{
		return Vector(x * value, y * value, z * value);
	}

	void rotateY(float angle) {
		GLdouble newx = x * cosf(angle) - z * sinf(angle);
		GLdouble newz = x * sinf(angle) + z * cosf(angle);
		x = newx;
		z = newz;
	}

	GLfloat magnitude() {
		return sqrt(x * x + y * y + z * z);
	}

	Vector normalized() {
		return Vector(x / magnitude(), y / magnitude(), z / magnitude());
	}

	
};

Vector Eye(15, 6, 0);
Vector At(0, 2, 0);
Vector Up(0, 1, 0);
Vector PlayerPosition(0, 0, 0);
Vector PlayerSpeed(0, 0, 0);
Vector PlayerAcceleration(0, 0, 0);
Vector Forward(1, 0, 0);
Vector EyeOffset(15, 6, 0);
Vector AtOffset(0, 2, 0);



int cameraZoom = 0;

// Model Variables
Model_3DS model_house;
Model_3DS model_taxi;
Model_3DS model_tree;
Model_3DS model_sheep;
Model_3DS model_streetlamp;
Model_3DS model_bush;
Model_3DS model_man;
Model_3DS model_build;
Model_3DS model_build2;
Model_3DS model_build3;
Model_3DS model_car;
Model_3DS model_dock;
Model_3DS model_police;
Model_3DS model_beach;
Model_3DS model_fence;
Model_3DS model_windmill;
Model_3DS model_animals;
Model_3DS model_cow;

// Textures
GLTexture tex_ground;
GLTexture tex_ground2;
GLTexture tex_water;

char getLetterGrade(int score) {
	if (score > 12000) {
		return 'S';
	}
	else if (score > 10000) {
		return 'A';
	}
	else if (score > 8000) {
		return 'B';
	}
	else if (score > 6000) {
		return 'C';
	}
	else if (score > 4000){
		return 'D';
	}
	else {
		return 'F';
	}
}

void setOrthographicProjection() {

	// switch to projection mode
	glMatrixMode(GL_PROJECTION);

	// save previous matrix which contains the
	//settings for the perspective projection
	glPushMatrix();

	// reset matrix
	glLoadIdentity();

	// set a 2D orthographic projection
	gluOrtho2D(0, WIDTH, HEIGHT, 0);

	// switch back to modelview mode
	glMatrixMode(GL_MODELVIEW);
}

void restorePerspectiveProjection() {

	glMatrixMode(GL_PROJECTION);
	// restore previous projection matrix
	glPopMatrix();

	// get back to modelview mode
	glMatrixMode(GL_MODELVIEW);
}

//=======================================================================
// Text Printing
//=======================================================================
void outputText(int x, int y, float r, float g, float b, void* font, char* string)
{
	glColor3f(r, g, b);
	glRasterPos2f(x, y);
	int len, i;
	len = (int)strlen(string);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(font, string[i]);
	}
}

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1, 0.1, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
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

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
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

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	//glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	if(level == 1)
		glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture
	else
		glBindTexture(GL_TEXTURE_2D, tex_ground2.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-100, 0, -100);
	glTexCoord2f(5, 0);
	glVertex3f(100, 0, -100);
	glTexCoord2f(5, 5);
	glVertex3f(100, 0, 100);
	glTexCoord2f(0, 5);
	glVertex3f(-100, 0, 100);
	glEnd();
	glPopMatrix();

	//glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void RenderWater()
{
	//glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_water.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-300, 0, -300);
	glTexCoord2f(5, 0);
	glVertex3f(300, 0, -300);
	glTexCoord2f(5, 5);
	glVertex3f(300, 0, 300);
	glTexCoord2f(0, 5);
	glVertex3f(-300, 0, 300);
	glEnd();
	glPopMatrix();

	//glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void drawRect(int x, int y, int w, int h) {
	glBegin(GL_POLYGON);
	glVertex2f(x, y);
	glVertex2f(x + w, y);
	glVertex2f(x + w, y + h);
	glVertex2f(x, y + h);
	glEnd();
}

void drawLevelClear() {

	glPushMatrix();
	outputText(WIDTH / 2 - 60, HEIGHT / 2 - 40, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, "Level Clear!");
	glPopMatrix();

	if (level == 1) {
		glPushMatrix();
		outputText(WIDTH / 2 - 80, HEIGHT / 2, 0, 0, 0, GLUT_BITMAP_HELVETICA_18, "Press [N] to go to the next stage!");
		glPopMatrix();
	}
	else {
		glPushMatrix();
		std::string timestring = "Final Score: ";
		timestring = timestring + std::to_string((int) score);
		char* cstr = &timestring[0];
		outputText(WIDTH / 2 - 60, HEIGHT / 2, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, cstr);

		std::string gradestring = "Rank: ";
		gradestring = gradestring + getLetterGrade((int)score)+"";
		char* cstr2 = &gradestring[0];
		outputText(WIDTH / 2 - 60, HEIGHT / 2 + 30, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, cstr2);
		glPopMatrix();
	}

	glPushMatrix();
	glColor3f(1, 0.8, 0);
	drawRect(WIDTH / 2 - 139, HEIGHT / 2 - 89, 278, 178);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0, 0, 0);
	drawRect(WIDTH / 2 - 140, HEIGHT / 2 - 90, 280, 180);
	glPopMatrix();

	
}

void drawAnimalPen(float x, float z) {

	glPushMatrix();
	glTranslatef(x + 6, 0, z + 10);
	glRotatef(90, 0, 1, 0);
	glColor3f(0.7, 0.4, 0);
	model_fence.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x - 5, 0, z + 10);
	glRotatef(90, 0, 1, 0);
	glColor3f(0.7, 0.4, 0);
	model_fence.Draw();
	glPopMatrix();

	glTranslatef(x + 6, 0, z + 20);
	glRotatef(90, 0, 1, 0);
	glColor3f(0.7, 0.4, 0);
	model_fence.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x - 5, 0, z + 20);
	glRotatef(90, 0, 1, 0);
	glColor3f(0.7, 0.4, 0);
	model_fence.Draw();
	glPopMatrix();



	glPushMatrix();
	glTranslatef(x, 0, z + 25);
	glRotatef(0, 0, 1, 0);
	glColor3f(0.7, 0.4, 0);
	model_fence.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x, 0, z+4);
	glRotatef(0, 0, 1, 0);
	glColor3f(0.7, 0.4, 0);
	model_fence.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x, 1.5, z+15);
	glRotatef(0, 0, 1, 0);
	model_animals.Draw();
	glPopMatrix();



	glColor3f(1, 1, 1);

}

void drawFarmHouse(float x, float z) {

	glPushMatrix();
	glTranslatef(x, 0, z);
	glRotatef(90, 1, 0, 0);
	glScalef(2, 2, 2);
	model_house.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x+6, 0, z+10);
	glRotatef(90, 0, 1, 0);
	glColor3f(0.7, 0.4, 0);
	model_fence.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x-6, 0, z+10);
	glRotatef(90, 0, 1, 0);
	glColor3f(0.7, 0.4, 0);
	model_fence.Draw();
	glPopMatrix();

	

	glPushMatrix();
	glTranslatef(x, 0, z+15);
	glRotatef(0, 0, 1, 0);
	glColor3f(0.7, 0.4, 0);
	model_fence.Draw();
	glPopMatrix();

	glColor3f(1, 1, 1);

}

void drawStreetBlock(int variation, float x, float z) {
	glPushMatrix();
	glTranslatef(x+10, 0, z+10);
	glScalef(0.4, 0.4, 0.4);
	model_tree.Draw();
	glPopMatrix();

	/*glPushMatrix();
	glTranslatef(x - 10, 0, z + 10);
	glScalef(0.4, 0.4, 0.4);
	model_tree.Draw();
	glPopMatrix();*/

	glPushMatrix();
	glTranslatef(x - 10, 0, z - 10);
	glScalef(0.4, 0.4, 0.4);
	model_tree.Draw();
	glPopMatrix();

	/*glPushMatrix();
	glTranslatef(x - 10, 0, z - 10);
	glScalef(0.4, 0.4, 0.4);
	model_tree.Draw();
	glPopMatrix();*/

	//Draw building Model
	glPushMatrix();
	glTranslatef(x-10, 0, z-10);
	if (variation == 1) {
		glScalef(6, 3, 6);
		model_build.Draw();
	}
	else if (variation == 2) {
		glScalef(6, 3, 6);
		model_build2.Draw();
	}
	else if (variation == 0) {
		glScalef(6, 3, 6);
		model_build3.Draw();
	}
	else {
		glTranslatef(10, -0.5, 12);
		glScalef(0.7, 0.7, 0.7);
		glRotatef(90, 0, 1, 0);
		model_police.Draw();
	}
	glPopMatrix();

	// Draw Street Lamp
	glPushMatrix();
	glTranslatef(x+10, -1, z);
	glScalef(0.4, 0.4, 0.4);
	glColor3f(0.5, 0.5, 0.5);
	model_streetlamp.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x, -1, z-10);
	glScalef(0.4, 0.4, 0.4);
	glColor3f(0.5, 0.5, 0.5);
	glRotatef(90.f, 0, 1, 0);
	model_streetlamp.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x-10, -1, z);
	glScalef(0.4, 0.4, 0.4);
	glColor3f(0.5, 0.5, 0.5);
	glRotatef(180.f, 0, 1, 0);
	model_streetlamp.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x, -1, z + 10);
	glScalef(0.4, 0.4, 0.4);
	glColor3f(0.5, 0.5, 0.5);
	glRotatef(270.f, 0, 1, 0);
	model_streetlamp.Draw();
	glPopMatrix();

	glColor3f(1, 1, 1);
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	
	GLfloat lightIntensity[] = {level==1? 0.7:0.1,level ==1? 0.7:0.1,level==1? 0.7:0.3, 1.0f };
	GLfloat lightPosition[] = { 0.0f+lightx, 100.0f+lighty, 0.0f+lightz, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	// Draw Ground
	glPushMatrix();
	RenderGround();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0, -1, 0);
	RenderWater();
	glPopMatrix();

	// Draw Destination Borders
	for (int i = 0; i < 3; i++) {
		if (destination == i + 1) {
			glPushMatrix();
			glTranslatef(DestinationXs[i], 0, DestinationZs[i]);
			glScalef(DestinationLs[i], 0.3, DestinationWs[i]);
			glColor3f(0, 1, 0);
			glutSolidCube(1);
			glPopMatrix();
		}
	}

	glColor3f(1, 1, 1);

	// Draw Taxi Model
	glPushMatrix();
	glTranslatef(PlayerPosition.x, PlayerPosition.y, PlayerPosition.z);
	glRotatef(playerRotation, 0, 1, 0);
	glScalef(0.02, 0.02, 0.02);
	model_taxi.Draw();
	glPopMatrix();
	


	//Draw Passengers
	for (int i = 0; i < 2; i++) {
		glPushMatrix();
		glTranslatef(PassengerXs[i], 0.5, PassengerZs[i]);
		glScalef(0.3, 0.3, 0.3);
		model_man.Draw();
		glPopMatrix();
	}

	
	
	if (level == 1) {

		

		float startx = 60;
		float startz = 60;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				glPushMatrix();
				drawStreetBlock((i == 2 && j == 0) ? 3 : (j + i) % 3, startx - i * 40, startz - j * 40);
				glPopMatrix();
			}
		}



		// Draw Car
		glPushMatrix();
		glTranslatef(-18, 0.5, 2);
		glScalef(0.5, 0.5, 0.5);
		glRotatef(90.f, 0, 1, 0);
		model_car.Draw();
		glPopMatrix();

		// Draw Car
		glPushMatrix();
		glTranslatef(-60, 0.5, 36);
		glScalef(0.5, 0.5, 0.5);
		model_car.Draw();
		glPopMatrix();

		// Draw Car
		glPushMatrix();
		glTranslatef(78, 0.5, 60);
		glScalef(0.5, 0.5, 0.5);
		glRotatef(180.f, 0, 1, 0);
		model_car.Draw();
		glPopMatrix();

		// Draw Car
		glPushMatrix();
		glTranslatef(-45, 0.5, -45);
		glScalef(0.5, 0.5, 0.5);
		glRotatef(270.f, 0, 1, 0);
		model_car.Draw();
		glPopMatrix();

		// Draw Boat
		glPushMatrix();
		glTranslatef(-10, -1, -120);
		glScalef(0.15, 0.15, 0.15);
		model_dock.Draw();
		glPopMatrix();

		// Draw Beach
		glPushMatrix();
		glTranslatef(-115, 1.5, 0);
		glScalef(0.03, 0.02, 0.03);
		glRotatef(90, 0, 1, 0);
		model_beach.Draw();
		glPopMatrix();

	}

	else if (level == 2) {
		
		float theXs[] = {-60,-40,40,60,40,-40};
		float theZs[] = {0,-60,-60,0,60,60};

		for (int i = 0; i < 6; i++) {
			glPushMatrix();
			drawFarmHouse(theXs[i],theZs[i]);
			glPopMatrix();
		}

		float BushXs[] = { 4, 18,30,70,2,-30,-18,-80,22 };
		float BushZs[] = { 4, -17, 1,-80,50,-60,-9,33,40 };

		for (int i = 0; i < 9; i++) {
			glPushMatrix();
			glTranslatef(BushXs[i], 0, BushZs[i]);
			model_bush.Draw();
			glPopMatrix();
		}

		float CowXs[] = { 10, 41,80,-71,62,-1,-56,11,3 };
		float CowZs[] = { -10, 9, -2,-80,-50,-21,40,-90,-40 };

		for (int i = 0; i < 9; i++) {
			glPushMatrix();
			glTranslatef(CowXs[i], 0, CowZs[i]);
			glScalef(0.0075, 0.0075, 0.0075);
			model_cow.Draw();
			glPopMatrix();
		}

		glPushMatrix();
		glTranslatef(90, 10, 30);
		glRotatef(90, 0, 1, 0);
		glScalef(0.04, 0.04, 0.04);
		model_windmill.Draw();
		glPopMatrix();

		glPushMatrix();
		drawAnimalPen(-90,50);
		glPopMatrix();

		glTranslatef(0, -0.1, 80);
		glScalef(0.7, 0.7, 0.7);
		glRotatef(90, 0, 1, 0);
		model_police.Draw();

		

	}


	//sky box
	glPushMatrix();

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(75, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 300, 200, 200);
	gluDeleteQuadric(qobj);


	glPopMatrix();

	
	setOrthographicProjection();
	glPushMatrix();
	glLoadIdentity();
	glColor3f(1, 1, 1);

	outputText(WIDTH/2, 30, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, "Time");
	std::string timestring = "";
	timestring = timestring + std::to_string(gametime/10);
	char* cstr = &timestring[0];
	outputText(WIDTH / 2 + 10, 90, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, cstr);

	outputText(30, 30, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, "Destination");
	switch (destination) {
	case 1: outputText(30, 90, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, "Police Station"); break;
	case 2: outputText(30, 90, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, level==1?"Harbor":"Windmill"); break;
	case 3: outputText(30, 90, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, level==1?"Beach":"Animal Pen"); break;
	default: outputText(30, 90, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, " "); break;
	}
	if (destination != 0) {
		std::string ptimestring = "";
		ptimestring = ptimestring + std::to_string(passengertime / 10);
		char* cstr1 = &ptimestring[0];
		outputText(50, 120, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, cstr1);
	}

	outputText(WIDTH - 100, 30, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, "Score");
	std::string scorestring = "";
	scorestring = scorestring + std::to_string((int) score);
	char* cstr2 = &scorestring[0];
	outputText(WIDTH - 90, 90, 0, 0, 0, GLUT_BITMAP_TIMES_ROMAN_24, cstr2);

	if(levelcomplete)
		drawLevelClear();
	glPopMatrix();
	restorePerspectiveProjection();
	


	glutSwapBuffers();

	glFlush();
}

void keyPressed(unsigned char key, int x, int y)
{
	keyStates[key] = true;         // Set the state of the current key to pressed  
	glutPostRedisplay();

}

void keyUp(unsigned char key, int x, int y)
{
	keyStates[key] = false;     // Set the state of the current key to not pressed  
	glutPostRedisplay();
}

bool collisionCheck(float xtocheck, float ztocheck, float centerx, float centerz, float bblength, float bbwidth) {
	if (xtocheck > centerx - bblength / 2 && xtocheck < centerx + bblength / 2) {
		if (ztocheck > centerz - bbwidth / 2 && ztocheck < centerz + bbwidth / 2) {
			return true;
		}
	}
	return false;
}

bool megaCollisionCheck() {
	bool iscol = false;
	for (int i = 0; i < 134; i++) {
		bool thecol =  collisionCheck(PlayerPosition.x + PlayerSpeed.x, PlayerPosition.z  + PlayerSpeed.z,
			obstacleXs[i], obstacleZs[i], obstacleLs[i], obstacleWs[i]);
		if (thecol) {
			return true;
		}
	}
	return iscol;
}

void generateGame() {

	srand(time(NULL));

	int mus = rand() % 3;

	if(mus == 0)
		PlaySound(TEXT("music/03.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
	else if(mus == 1)
		PlaySound(TEXT("music/04.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
	else
		PlaySound(TEXT("music/05.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);

	for (int i = 0; i < 134; i++) {
		obstacleXs[i] = 0;
		obstacleZs[i] = 0;
		obstacleLs[i] = 0;
		obstacleWs[i] = 0;
	}

	if (level == 1) {

		float startx = 60;
		float startz = 60;

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				float currentx = startx - i * 40;
				float currentz = startz - j * 40;
				obstacleXs[((i * 4) + j)] = currentx;
				obstacleZs[((i * 4) + j)] = currentz + 2;
				obstacleLs[((i * 4) + j)] = 15;
				obstacleWs[((i * 4) + j)] = 15;

				obstacleXs[16 + ((i * 4) + j)] = currentx - 10;
				obstacleZs[16 + ((i * 4) + j)] = currentz - 10;
				obstacleLs[16 + ((i * 4) + j)] = 2;
				obstacleWs[16 + ((i * 4) + j)] = 2;

				obstacleXs[32 + ((i * 4) + j)] = currentx + 10;
				obstacleZs[32 + ((i * 4) + j)] = currentz + 10;
				obstacleLs[32 + ((i * 4) + j)] = 2;
				obstacleWs[32 + ((i * 4) + j)] = 2;

				obstacleXs[48 + ((i * 4) + j)] = currentx + 10;
				obstacleZs[48 + ((i * 4) + j)] = currentz;
				obstacleLs[48 + ((i * 4) + j)] = 2;
				obstacleWs[48 + ((i * 4) + j)] = 2;

				obstacleXs[64 + ((i * 4) + j)] = currentx - 10;
				obstacleZs[64 + ((i * 4) + j)] = currentz;
				obstacleLs[64 + ((i * 4) + j)] = 2;
				obstacleWs[64 + ((i * 4) + j)] = 2;

				obstacleXs[80 + ((i * 4) + j)] = currentx;
				obstacleZs[80 + ((i * 4) + j)] = currentz + 10;
				obstacleLs[80 + ((i * 4) + j)] = 2;
				obstacleWs[80 + ((i * 4) + j)] = 2;

				obstacleXs[96 + ((i * 4) + j)] = currentx;
				obstacleZs[96 + ((i * 4) + j)] = currentz - 10;
				obstacleLs[96 + ((i * 4) + j)] = 2;
				obstacleWs[96 + ((i * 4) + j)] = 2;
			}
		}

		obstacleXs[112] = 100;
		obstacleZs[112] = 0;
		obstacleLs[112] = 10;
		obstacleWs[112] = 200;

		obstacleXs[113] = -100;
		obstacleZs[113] = 0;
		obstacleLs[113] = 10;
		obstacleWs[113] = 200;

		obstacleXs[114] = 0;
		obstacleZs[114] = -100;
		obstacleLs[114] = 200;
		obstacleWs[114] = 10;

		obstacleXs[115] = 0;
		obstacleZs[115] = 100;
		obstacleLs[115] = 200;
		obstacleWs[115] = 10;

		obstacleXs[116] = -18;
		obstacleZs[116] = 2;
		obstacleLs[116] = 5;
		obstacleWs[116] = 3;

		obstacleXs[117] = -60;
		obstacleZs[117] = 36;
		obstacleLs[117] = 3;
		obstacleWs[117] = 5;

		obstacleXs[118] = 78;
		obstacleZs[118] = 60;
		obstacleLs[118] = 3;
		obstacleWs[118] = 5;

		obstacleXs[119] = -45;
		obstacleZs[119] = -45;
		obstacleLs[119] = 5;
		obstacleWs[119] = 3;

		PassengerXs[0] = 80;
		PassengerZs[0] = 0;

		PassengerXs[1] = -80;
		PassengerZs[1] = -80;

		obstacleXs[120] = 80;
		obstacleZs[120] = 0;
		obstacleLs[120] = 2;
		obstacleWs[120] = 2;

		obstacleXs[121] = -80;
		obstacleZs[121] = -80;
		obstacleLs[121] = 2;
		obstacleWs[121] = 2;

		DestinationXs[0] = -20;
		DestinationZs[0] = 60;
		DestinationLs[0] = 30;
		DestinationWs[0] = 20;

		DestinationXs[1] = -10;
		DestinationZs[1] = -80;
		DestinationLs[1] = 50;
		DestinationWs[1] = 30;

		DestinationXs[2] = -80;
		DestinationZs[2] = 0;
		DestinationLs[2] = 40;
		DestinationWs[2] = 80;

	}
	else if (level == 2) {
		float theXs[] = { -60,-40,40,60,40,-40 };
		float theZs[] = { 0,-60,-60,0,60,60 };

		for (int i = 0; i < 6; i++) {
			obstacleXs[i] = theXs[i];
			obstacleZs[i] = theZs[i]+5;
			obstacleLs[i] = 12;
			obstacleWs[i] = 20;
		}

		obstacleXs[6] = 100;
		obstacleZs[6] = 0;
		obstacleLs[6] = 10;
		obstacleWs[6] = 200;

		obstacleXs[7] = -100;
		obstacleZs[7] = 0;
		obstacleLs[7] = 10;
		obstacleWs[7] = 200;

		obstacleXs[8] = 0;
		obstacleZs[8] = -100;
		obstacleLs[8] = 200;
		obstacleWs[8] = 10;

		obstacleXs[9] = 0;
		obstacleZs[9] = 100;
		obstacleLs[9] = 200;
		obstacleWs[9] = 10;

		float BushXs[] = { 4, 18,30,70,2,-30,-18,-80,22 };
		float BushZs[] = { 4, -17, 1,-80,50,-60,-9,33,40 };

		for (int i = 0; i < 9; i++) {
			obstacleXs[10 + i] = BushXs[i];
			obstacleZs[10 + i] = BushZs[i];
			obstacleWs[10 + i] = 2;
			obstacleLs[10 + i] = 2;
		}

		float CowXs[] = { 10, 41,80,-71,62,-1,-56,11,3 };
		float CowZs[] = { -10, 9, -2,-80,-50,-21,40,-90,-40 };

		for (int i = 0; i < 9; i++) {
			obstacleXs[20 + i] = CowXs[i];
			obstacleZs[20 + i] = CowZs[i];
			obstacleLs[20 + i] = 6;
			obstacleWs[20 + i] = 2;
		}

		obstacleXs[30] = 90;
		obstacleZs[30] = 30;
		obstacleLs[30] = 4;
		obstacleWs[30] = 4;

		obstacleXs[31] = -90;
		obstacleZs[31] = 64.5;
		obstacleLs[31] = 13;
		obstacleWs[31] = 23;

		obstacleXs[32] = 0;
		obstacleZs[32] = 80;
		obstacleLs[32] = 16;
		obstacleWs[32] = 16;

		PassengerXs[0] = 50;
		PassengerZs[0] = 50;

		PassengerXs[1] = 0;
		PassengerZs[1] = -80;

		obstacleXs[33] = 50;
		obstacleZs[33] = 50;
		obstacleLs[33] = 2;
		obstacleWs[33] = 2;

		obstacleXs[34] = 0;
		obstacleZs[34] = -80;
		obstacleLs[34] = 2;
		obstacleWs[34] = 2;

		DestinationXs[0] = 0;
		DestinationZs[0] = 80;
		DestinationLs[0] = 30;
		DestinationWs[0] = 30;

		DestinationXs[1] = 90;
		DestinationZs[1] = 30;
		DestinationLs[1] = 30;
		DestinationWs[1] = 30;

		DestinationXs[2] = -90;
		DestinationZs[2] = 50;
		DestinationLs[2] = 40;
		DestinationWs[2] = 40;
	}
}


//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char button, int x, int y)
{
	float PRRadian;
	switch (button)
	{
	case 'z':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'r':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 'w':
		if (PlayerAcceleration.magnitude() < 1) {
			PlayerAcceleration-=Forward;
		}
		break;
	case 's':
		if (PlayerAcceleration.magnitude() < 1) {
			PlayerAcceleration += Forward;
		}
		break;
	case 'a':
		playerRotation += 18/PI;
		PRRadian = playerRotation * PI;
		Forward.rotateY(-0.1f);
		Forward = Forward*(1/cosf(PRRadian/180));
		PlayerSpeed.rotateY(-0.1f);
		PlayerAcceleration.rotateY(-0.1f);
		break;
	case 'd':
		playerRotation -= 18 / PI;
		PRRadian = playerRotation * PI;
		Forward.rotateY(0.1f);
		Forward = Forward * (1 / cosf(PRRadian / 180));
		PlayerSpeed.rotateY(0.1f);
		PlayerAcceleration.rotateY(0.1f);
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void Timer(int value) {
	if (lightx == 100) {
		lightDir = -1;
		lightx -= 2;
	}
	else if (lightx == -100) {
		lightDir = 1;
		lightx += 2;
	}
	else {
		if (lightDir == 1) {
			lightx += 2;
		}
		else {
			lightx -= 2;
		}
	}

	if (lightz == 100) {
		lightDir = -1;
		lightz -= 2;
	}
	else if (lightz == -100) {
		lightDir = 1;
		lightz += 2;
	}
	else {
		if (lightDir == 1) {
			lightz += 2;
		}
		else {
			lightz -= 2;
		}
	}

	if (gametime > 0 && !levelcomplete) {
		gametime--;
	}

	if (passengertime > 0 && !cutscenemode && !cutsceneoutmode && !levelcomplete) {
		passengertime--;
	}

	if (megaCollisionCheck()) {
		PlayerAcceleration = PlayerAcceleration*-0.3;
		PlayerSpeed = PlayerSpeed*-0.3;
	}
	if (PlayerSpeed.magnitude() < 0.5f) {
		PlayerSpeed.x += PlayerAcceleration.x;
		PlayerSpeed.y += PlayerAcceleration.y;
		PlayerSpeed.z += PlayerAcceleration.z;
	}

	PlayerPosition.x += PlayerSpeed.x;
	PlayerPosition.y += PlayerSpeed.y;
	PlayerPosition.z += PlayerSpeed.z;

	if (abs(PlayerAcceleration.x) > 0.0f) {
		if(PlayerAcceleration.x > 0)
			PlayerAcceleration.x -= 0.02f;
		else
			PlayerAcceleration.x += 0.02f;
	}

	if (abs(PlayerAcceleration.z) > 0.0f) {
		if (PlayerAcceleration.z > 0)
			PlayerAcceleration.z -= 0.02f;
		else
			PlayerAcceleration.z += 0.02f;
	}

	if (abs(PlayerSpeed.z) > 0.0f && !keyStates['w']) {
		if (PlayerSpeed.z > 0)
			PlayerSpeed.z -= 0.02f;
		else
			PlayerSpeed.z += 0.02f;
	}

	if (abs(PlayerSpeed.x) > 0.0f && !keyStates['w']) {
		if (PlayerSpeed.x > 0)
			PlayerSpeed.x -= 0.02f;
		else
			PlayerSpeed.x += 0.02f;
	}

	if (PlayerAcceleration.magnitude() < 0.02f) {
		PlayerAcceleration = Vector(0, 0, 0);
	}

	if (PlayerSpeed.magnitude() < 0.02f) {
		PlayerSpeed = Vector(0, 0, 0);
	}

	Eye.x = PlayerPosition.x + EyeOffset.x;
	Eye.y = PlayerPosition.y + EyeOffset.y;
	Eye.z = PlayerPosition.z - EyeOffset.z;

	At.x = PlayerPosition.x + AtOffset.x;
	At.y = PlayerPosition.y + AtOffset.y;
	At.z = PlayerPosition.z + AtOffset.z;

	if (level == 1) {
		if (!passenger1Done && collisionCheck(PlayerPosition.x, PlayerPosition.z, 80, 0, 15, 15) && PlayerSpeed.magnitude() == 0 && passenger == 0) {
			passenger = 1;
			cutscenemode = true;
		}

		if (!passenger2Done && collisionCheck(PlayerPosition.x, PlayerPosition.z, -80, -80, 15, 15) && PlayerSpeed.magnitude() == 0 && passenger == 0) {
			passenger = 2;
			cutscenemode = true;
		}
	}
	else {
		if (!passenger1Done && collisionCheck(PlayerPosition.x, PlayerPosition.z, 50, 50, 15, 15) && PlayerSpeed.magnitude() == 0 && passenger == 0) {
			passenger = 1;
			cutscenemode = true;
		}

		if (!passenger2Done && collisionCheck(PlayerPosition.x, PlayerPosition.z, 0, -80, 15, 15) && PlayerSpeed.magnitude() == 0 && passenger == 0) {
			passenger = 2;
			cutscenemode = true;
		}
	}

	for (int i = 0; i < 3; i++) {
		if (collisionCheck(PlayerPosition.x, PlayerPosition.z, DestinationXs[i], DestinationZs[i]
			, DestinationLs[i], DestinationWs[i]) && PlayerSpeed.magnitude() == 0 && destination == i+1) {
			destination = 0;
			cutsceneoutmode = true;
		}
	}

	if (passengersAvailable == 0 || gametime == 0) {
		levelcomplete = true;
	}

	if (passengertime == 0 && passenger !=0 && !cutscenemode) {
		destination = 0;
		cutsceneoutmode = true;
	}

	srand(time(NULL));

	if (cutscenemode == true) {
		//Passenger 1
		if (passenger == 1) {
			if (PassengerXs[0] < PlayerPosition.x) {
				PassengerXs[0] += 0.05f;
			}
			else if (PassengerXs[0] > PlayerPosition.x) {
				PassengerXs[0] -= 0.05f;
			}

			if (PassengerZs[0] < PlayerPosition.z) {
				PassengerZs[0] += 0.05f;
			}
			else if (PassengerZs[0] > PlayerPosition.z) {
				PassengerZs[0] -= 0.05f;
			}

			if (abs(PassengerXs[0] - PlayerPosition.x) < 0.1f && abs(PassengerZs[0] - PlayerPosition.z) < 0.1f) {
				cutscenemode = false;
				PassengerXs[0] = 400;
				PassengerZs[0] = 400;
				obstacleXs[120] = 400;
				obstacleZs[120] = 400;
				destination = (rand() % 3)+1;
				passengertime = gametime<600?gametime:600;
			}
		}

		//Passenger 2
		if (passenger == 2) {
			if (PassengerXs[1] < PlayerPosition.x) {
				PassengerXs[1] += 0.05f;
			}
			else if (PassengerXs[1] > PlayerPosition.x) {
				PassengerXs[1] -= 0.05f;
			}

			if (PassengerZs[1] < PlayerPosition.z) {
				PassengerZs[1] += 0.05f;
			}
			else if (PassengerZs[1] > PlayerPosition.z) {
				PassengerZs[1] -= 0.05f;
			}

			if (abs(PassengerXs[1] - PlayerPosition.x) < 0.1f && abs(PassengerZs[1] - PlayerPosition.z) < 0.1f) {
				cutscenemode = false;
				PassengerXs[1] = 400;
				PassengerZs[1] = 400;
				obstacleXs[121] = 400;
				obstacleZs[121] = 400;
				destination = (rand() % 3)+1;
				passengertime = gametime<600?gametime:600;
			}
		}
	}

	if (cutsceneoutmode == true) {
		//Passenger 1
		if (passenger == 1) {
			if (PassengerXs[0] == 400 && PassengerZs[0] == 400) {
				PassengerXs[0] = PlayerPosition.x;
				PassengerZs[0] = PlayerPosition.z;
			}

			if (PassengerXs[0] < PlayerPosition.x) {
				PassengerXs[0] -= 0.05f;
			}
			else if (PassengerXs[0] > PlayerPosition.x) {
				PassengerXs[0] += 0.05f;
			}

			if (PassengerZs[0] < PlayerPosition.z) {
				PassengerZs[0] -= 0.05f;
			}
			else if (PassengerZs[0] > PlayerPosition.z) {
				PassengerZs[0] += 0.05f;
			}

			if (abs(PassengerXs[0] - PlayerPosition.x) > 3 && abs(PassengerZs[0] - PlayerPosition.z) > 3) {
				cutsceneoutmode = false;
				obstacleXs[120] = PassengerXs[0];
				obstacleZs[120] = PassengerZs[0];
				passenger1Done = true;
				passenger = 0;
				passengersAvailable--;
				if (passengertime != 0) {
					score += 100 + passengertime * 10;
				}
			}
		}

		//Passenger 2
		if (passenger == 2) {
			if (PassengerXs[1] == 400 && PassengerZs[1] == 400) {
				PassengerXs[1] = PlayerPosition.x;
				PassengerZs[1] = PlayerPosition.z;
			}

			if (PassengerXs[1] < PlayerPosition.x) {
				PassengerXs[1] -= 0.05f;
			}
			else if (PassengerXs[1] > PlayerPosition.x) {
				PassengerXs[1] += 0.05f;
			}

			if (PassengerZs[1] < PlayerPosition.z) {
				PassengerZs[1] -= 0.05f;
			}
			else if (PassengerZs[1] > PlayerPosition.z) {
				PassengerZs[1] += 0.05f;
			}

			if (abs(PassengerXs[1] - PlayerPosition.x) > 3 && abs(PassengerZs[1] - PlayerPosition.z) > 3) {
				cutsceneoutmode = false;
				obstacleXs[121] = PassengerXs[1];
				obstacleZs[121] = PassengerZs[1];
				passenger2Done = true;
				passenger = 0;
				passengersAvailable--;
				if (passengertime != 0) {
					score += 100 + passengertime * 10;
				}
			}
		}
	}

	//Keyboard
	
	if (!cutscenemode && !cutsceneoutmode && !levelcomplete) {
		
		if (keyStates['r'])
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (keyStates['w']) {
			if (PlayerAcceleration.magnitude() < 0.1f) {
				PlayerAcceleration -= Forward * 0.7f;
			}
		}
		if (keyStates['s']) {
			if (PlayerAcceleration.magnitude() < 0.1f) {
				PlayerAcceleration += Forward * 0.7f;
			}
		}
		if (keyStates['a']) {
			playerRotation += 7.2 / PI;
			Forward.rotateY(-0.04f);
			Forward = Forward.normalized();
			PlayerAcceleration.rotateY(-0.04f);
			PlayerSpeed.rotateY(-0.04f);
			EyeOffset.rotateY(0.04f* camRotDir);
			AtOffset.rotateY(0.04f* camRotDir);
		}
		if (keyStates['d']) {
			playerRotation -= 7.2 / PI;
			Forward.rotateY(0.04f);
			Forward = Forward.normalized();
			PlayerAcceleration.rotateY(0.04f);
			PlayerSpeed.rotateY(0.04f);
			EyeOffset.rotateY(-0.04f* camRotDir);
			AtOffset.rotateY(-0.04f* camRotDir);
		}
		if (keyStates[' ']) {
			if (PlayerAcceleration.magnitude() > 0) {
				PlayerSpeed = Vector(0, 0, 0);

			}
		}
		if (keyStates[27]) {
			exit(0);
		}
	}
	else if (levelcomplete) {
		if (keyStates['n']) {
			PlayerPosition = Vector(0, 0, 0);
			passengersAvailable = 2;
			level = 2;
			levelcomplete = false;
			gametime = 3000;
			passenger = 0;
			cutscenemode = false;
			cutsceneoutmode = false;
			passenger1Done = false;
			passenger2Done = false;
			generateGame();
			
		}
	}

	

	// ask OpenGL to recall the display function to reflect the changes on the window
	glutPostRedisplay();

	// recall the Timer function after 20 seconds (20,000 milliseconds)
	glutTimerFunc(10, Timer, 0);
}






//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{
	

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		float playerRad = playerRotation * PI;
		if (camRotDir == 1) {
			
			Vector v = Vector(-1, 2, 0);
			v.rotateY(-playerRad/180);
			EyeOffset = v;
			Vector v2 = Vector(-2, 2, 0);
			v2.rotateY(-playerRad / 180);
			AtOffset = v2;
			camRotDir = -1;
		}
		else {
			Vector v = Vector(15, 6, 0);
			v.rotateY(playerRad / 180);
			EyeOffset = v;
			Vector v2 = Vector(0, 1, 0);
			v2.rotateY(playerRad / 180);
			AtOffset = v2;
			camRotDir = 1;
		}
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
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

void Anim() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	glutPostRedisplay();
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_house.Load("Models/house/house.3DS");
	model_taxi.Load("Models/house/taxi1.3DS");
	model_sheep.Load("Models/sheep/sheep.3DS");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_streetlamp.Load("Models/streetlamp/StreetLamp.3ds");
	model_bush.Load("Models/bush/Bush1.3ds");
	model_man.Load("Models/man/guy.3ds");
	model_build3.Load("Models/building3/citybuilding.3ds");
	model_build2.Load("Models/building2/citybuilding.3ds");
	model_build.Load("Models/building1/citybuilding.3ds");
	model_car.Load("Models/car/Car.3ds");
	model_dock.Load("Models/dock/Lowpoly_boat.3DS");
	model_police.Load("Models/police/police.3DS");
	model_beach.Load("Models/beach/sdraio3ds.3DS");
	model_fence.Load("Models/fence/fence.3DS");
	model_windmill.Load("Models/windmill/windmill.3DS");
	model_animals.Load("Models/animals/animals.3DS");
	model_cow.Load("Models/cow/CowN050213.3DS");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	tex_ground2.Load("Textures/ground2.bmp");
	tex_water.Load("Textures/water.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{


	generateGame();

	//Testing Area
	
	

	//End of testing Area


	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(keyPressed);

	glutKeyboardUpFunc(keyUp);

	glutTimerFunc(0, Timer, 0);

	glutMotionFunc(myMotion);

	glutIdleFunc(Anim);

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