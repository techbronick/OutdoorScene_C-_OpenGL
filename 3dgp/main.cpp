#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;


bool isNight = 0;

// GLSL Objects (Shader Program)
C3dglProgram Program;

// 3D Models
C3dglTerrain terrain, road;

C3dglModel streetLamp, dragon;

// SkyBox
C3dglSkyBox skybox, skyboxNight;

// Textures
GLuint grassTex;
GLuint roadTex;
GLuint lampTex;

GLuint dragonTex;
GLuint dragonNormalTex;

// Time variables used for animation
float t = 0;
float currentTime = 0;

// camera position (for first person type camera navigation)
mat4 matrixView;			// The View Matrix
float angleTilt = 15.f;		// Tilt Angle
vec3 cam(0);				// Camera movement values

bool init()
{

	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!


	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!Program.Create()) return false;
	if (!Program.Attach(VertexShader)) return false;
	if (!Program.Attach(FragmentShader)) return false;
	if (!Program.Link()) return false;
	if (!Program.Use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(Program.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(Program.GetAttribLocation("aNormal"));

	// load your 3D models here!
	if (!skybox.load("models\\mountain\\mft.tga", "models\\mountain\\mlf.tga", "models\\mountain\\mbk.tga", "models\\mountain\\mrt.tga", "models\\mountain\\mup.tga", "models\\mountain\\mdn.tga")) return false;
	if (!skyboxNight.load("models\\nightSky\\nightSky.front.jpg", "models\\nightSky\\nightSky.left.jpg", "models\\nightSky\\nightSky.back.jpg", "models\\nightSky\\nightSky.right.jpg", "models\\nightSky\\nightSky.up.jpg", "models\\nightSky\\nightSky.down.jpg")) return false;
	if (!terrain.loadHeightmap("models\\heightmap.bmp", 10)) return false;
	if (!road.loadHeightmap("models\\roadmap.bmp", 10)) return false;
	if (!streetLamp.load("models\\streetLamp.obj")) return false;
	if (!dragon.load("models\\dragon.dae")) return false;


	//Loadind texture bitmaps

	C3dglBitmap bm;


	bm.Load("models/dragonNormal.jpg", GL_RGBA);
	if (!bm.GetBits()) return false;
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &dragonNormalTex);
	glBindTexture(GL_TEXTURE_2D, dragonNormalTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());
	Program.SendUniform("textureNormal", 1);



	bm.Load("models/dragon.jpg", GL_RGBA);
	if (!bm.GetBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &dragonTex);
	glBindTexture(GL_TEXTURE_2D, dragonTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/grass.jpg", GL_RGBA);
	if (!bm.GetBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &grassTex);
	glBindTexture(GL_TEXTURE_2D, grassTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/lamp.jpg", GL_RGBA);
	if (!bm.GetBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &lampTex);
	glBindTexture(GL_TEXTURE_2D, lampTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/road.jpg", GL_RGBA);
	if (!bm.GetBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &roadTex);
	glBindTexture(GL_TEXTURE_2D, roadTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());

	Program.SendUniform("texture0", 0);


	//Initialising light uniforms and fog

	Program.SendUniform("lightAmbient.on", 1);

	Program.SendUniform("lightDir1.on", 1);

	Program.SendUniform("lightDir1.direction", 0.75f, 2.0f, -1.0f);

	Program.SendUniform("lightDir2.direction", 0.75f, 2.0f, -1.0f);

	Program.SendUniform("att_quadratic", 0.03f);

	Program.SendUniform("spotLight1.position", 6.28f, 5.26f, 20.0f);
	Program.SendUniform("spotLight1.direction", 0.0f, -1.0f, 0.0f);
	Program.SendUniform("spotLight1.cutoff", 20.0f);
	Program.SendUniform("spotLight1.attenuation", 3.0f);
	Program.SendUniform("spotLight1.diffuse", 0.4f, 0.4f, 0.4f);

	Program.SendUniform("spotLight2.position", 6.28f, 5.99f, -14.0f);
	Program.SendUniform("spotLight2.direction", 0.0f, -1.0f, 0.0f);
	Program.SendUniform("spotLight2.cutoff", 20.0f);
	Program.SendUniform("spotLight2.attenuation", 3.0f);
	Program.SendUniform("spotLight2.diffuse", 0.4f, 0.4f, 0.4f);

	Program.SendUniform("spotLight3.position", 4.92f, 2.51f, 30.0f);
	Program.SendUniform("spotLight3.direction", 0.0f, -1.0f, 0.0f);
	Program.SendUniform("spotLight3.cutoff", 20.0f);
	Program.SendUniform("spotLight3.attenuation", 3.0f);
	Program.SendUniform("spotLight3.diffuse", 0.4f, 0.4f, 0.4f);

	Program.SendUniform("spotLight4.position", 4.92f, 4.50f, 0.0f);
	Program.SendUniform("spotLight4.direction", 0.0f, -1.0f, 0.0f);
	Program.SendUniform("spotLight4.cutoff", 20.0f);
	Program.SendUniform("spotLight4.attenuation", 3.0f);
	Program.SendUniform("spotLight4.diffuse", 0.4f, 0.4f, 0.4f);

	Program.SendUniform("spotLight5.position", 4.92f, 3.525f, -25.0f);
	Program.SendUniform("spotLight5.direction", 0.0f, -1.0f, 0.0f);
	Program.SendUniform("spotLight5.cutoff", 20.0f);
	Program.SendUniform("spotLight5.attenuation", 3.0f);
	Program.SendUniform("spotLight5.diffuse", 0.4f, 0.4f, 0.4f);


	Program.SendUniform("materialSpecular", 0.6f, 0.6f, 1.0f);
	Program.SendUniform("shininess", 3);

	Program.SendUniform("fogDensity", 0.04f);


	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));
	matrixView *= lookAt(
		vec3(4.0, 1.5, 0.0),
		vec3(4.0, 1.5, 5.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky background

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void done()
{
}

void render()
{
	// this global variable controls the animation
	float theta = glutGet(GLUT_ELAPSED_TIME) * 0.01f;

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
	matrixView = m * matrixView;

	Program.SendUniform("matrixView", matrixView);


	glActiveTexture(GL_TEXTURE0);


	m = matrixView;

	//Setting up the light corresponding the time of day
	if (!isNight)
	{
		Program.SendUniform("lightAmbient.color", 1.0, 1.0, 1.0);
		Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
		Program.SendUniform("materialDiffuse", 0.0, 0.0, 0.0);
		skybox.render(m);
		Program.SendUniform("lightAmbient.color", 0.1f, 0.1f, 0.1f);
		Program.SendUniform("materialAmbient", 0.1f, 0.1f, 0.1f);
		Program.SendUniform("lightDir1.diffuse", 0.5f, 0.5f, 0.5f);
		Program.SendUniform("lightDir2.diffuse", 0.5f, 0.5f, 0.5f);
		Program.SendUniform("materialDiffuse", 1.0f, 1.0f, 1.0f);
		Program.SendUniform("spotLight1.on", 0);
		Program.SendUniform("spotLight2.on", 0);
		Program.SendUniform("spotLight3.on", 0);
		Program.SendUniform("spotLight4.on", 0);
		Program.SendUniform("spotLight5.on", 0);
		Program.SendUniform("bulbAmbient.on", 0);
		Program.SendUniform("fogColour", 0.4f, 0.4f, 0.5f);

	}
	else
	{
		Program.SendUniform("lightAmbient.color", 0.8, 0.8, 0.8);
		Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
		Program.SendUniform("materialDiffuse", 0.0, 0.0, 0.0);
		skyboxNight.render(m);
		Program.SendUniform("lightAmbient.color", 0.1f, 0.1f, 0.1f);
		Program.SendUniform("lightDir1.diffuse", 0.15f, 0.15f, 0.15f);
		Program.SendUniform("lightDir2.diffuse", 0.15f, 0.15f, 0.15f);
		Program.SendUniform("materialDiffuse", 0.9f, 0.9f, 0.9f);
		Program.SendUniform("spotLight1.on", 1);
		Program.SendUniform("spotLight2.on", 1);
		Program.SendUniform("spotLight3.on", 1);
		Program.SendUniform("spotLight4.on", 1);
		Program.SendUniform("spotLight5.on", 1);
		Program.SendUniform("bulbAmbient.on", 1);
		Program.SendUniform("fogColour", 0.1f, 0.1f, 0.2f);

	}

	// move the camera up following the profile of terrain (Y coordinate of the terrain)
	float terrainY = -terrain.getInterpolatedHeight(inverse(matrixView)[3][0], inverse(matrixView)[3][2]);
	matrixView = translate(matrixView, vec3(0, terrainY, 0));


	// render the terrain

	glBindTexture(GL_TEXTURE_2D, grassTex);

	m = matrixView;
	m = translate(matrixView, vec3(0, 0, 0));
	terrain.render(m);


	// render the road
	glBindTexture(GL_TEXTURE_2D, roadTex);
	m = translate(matrixView, vec3(0, 0, 0));
	m = translate(m, vec3(6.0f, 0.01f, 0.0f));
	road.render(m);


	// render street lamps

	glBindTexture(GL_TEXTURE_2D, lampTex);
	m = matrixView;
	m = translate(matrixView, vec3(4.5f, terrain.getInterpolatedHeight(4.5f, 30) + 0.42f, 30));
	m = scale(m, vec3(0.001f, 0.001f, 0.001f));
	streetLamp.render(m);

	m = matrixView;
	m = translate(matrixView, vec3(4.5f, terrain.getInterpolatedHeight(4.5f, 0) + 0.44f, 0));
	m = scale(m, vec3(0.001f, 0.001f, 0.001f));
	streetLamp.render(m);

	m = matrixView;
	m = translate(matrixView, vec3(4.5f, terrain.getInterpolatedHeight(4.5f, -25) + 0.44f, -25));
	m = scale(m, vec3(0.001f, 0.001f, 0.001f));
	streetLamp.render(m);

	m = matrixView;
	m = translate(matrixView, vec3(6.7f, terrain.getInterpolatedHeight(6.7f, -10) + 0.44f, -14));
	m = scale(m, vec3(0.001f, 0.001f, 0.001f));
	m = rotate(m, radians(180.f), vec3(0.f, 1.f, 0.f));
	streetLamp.render(m);

	m = matrixView;
	m = translate(matrixView, vec3(6.7f, terrain.getInterpolatedHeight(6.7f, 20) + 0.44f, 20));
	m = scale(m, vec3(0.001f, 0.001f, 0.001f));
	m = rotate(m, radians(180.f), vec3(0.f, 1.f, 0.f));
	streetLamp.render(m);


	//render dragon
	Program.SendUniform("lightDir2.on", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dragonTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dragonNormalTex);


	//time logic for animation 
	if (currentTime == 0)
	{
		t = 0;
	}
	else
	{
		t = (float)(GetTickCount64() - currentTime) / 1000.0f;
		if (t >= 23.0f)
		{
			currentTime = 0;
		}
	}

	// calculate and send bone transforms
	std::vector<float> transforms;

	dragon.getBoneTransforms(0, t, transforms);
	Program.SendUniformMatrixv("bones", (float*)&transforms[0], transforms.size() / 16);
	m = matrixView;
	m = translate(matrixView, vec3(-2.4, terrain.getInterpolatedHeight(-2.4, -16.4f), -16.4f));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	m = rotate(m, radians(30.f), vec3(0.f, 1.f, 0.f));
	dragon.render(m);
	Program.SendUniform("lightDir2.on", 0);


	//Bulbs
	Program.SendUniform("bulbAmbient.color", 1.0f, 1.0f, 1.0f);

	m = matrixView;
	m = translate(m, vec3(6.28f, 5.26f, 20.0f));
	m = scale(m, vec3(0.075f, 0.015f, 0.05f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	m = matrixView;
	m = translate(m, vec3(6.28f, 5.99f, -14));
	m = scale(m, vec3(0.075f, 0.015f, 0.05f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	m = matrixView;
	m = translate(m, vec3(4.92f, 2.51f, 30));
	m = scale(m, vec3(0.075f, 0.015f, 0.05f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	m = matrixView;
	m = translate(m, vec3(4.92f, 4.50f, 0));
	m = scale(m, vec3(0.075f, 0.015f, 0.05f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	m = matrixView;
	m = translate(m, vec3(4.92f, 3.525f, -25));
	m = scale(m, vec3(0.075f, 0.015f, 0.05f));
	Program.SendUniform("matrixModelView", m);

	glutSolidSphere(1, 32, 32);

	Program.SendUniform("bulbAmbient.color", 0.0f, 0.0f, 0.0f);


	// the camera must be moved down by terrainY to avoid unwanted effects
	matrixView = translate(matrixView, vec3(0, -terrainY, 0));

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void reshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);

	Program.SendUniform("matrixProjection", perspective(radians(60.f), ratio, 0.02f, 1000.f));
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': cam.z = std::max(cam.z * 1.05f, 0.01f); break;
	case 's': cam.z = std::min(cam.z * 1.05f, -0.01f); break;
	case 'a': cam.x = std::max(cam.x * 1.05f, 0.01f); break;
	case 'd': cam.x = std::min(cam.x * 1.05f, -0.01f); break;
	case 'e': cam.y = std::max(cam.y * 1.05f, 0.01f); break;
	case 'q': cam.y = std::min(cam.y * 1.05f, -0.01f); break;
	case 'n': isNight = (isNight == 0) ? 1 : 0; break;
	case 'k': currentTime = GetTickCount64(); break;
	}
	// speed limit
	cam.x = std::max(-0.15f, std::min(0.15f, cam.x));
	cam.y = std::max(-0.15f, std::min(0.15f, cam.y));
	cam.z = std::max(-0.15f, std::min(0.15f, cam.z));
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': cam.z = 0; break;
	case 'a':
	case 'd': cam.x = 0; break;
	case 'q':
	case 'e': cam.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
bool bJustClicked = false;
void onMouse(int button, int state, int x, int y)
{
	bJustClicked = (state == GLUT_DOWN);
	glutSetCursor(bJustClicked ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

// handle mouse move
void onMotion(int x, int y)
{
	if (bJustClicked)
		bJustClicked = false;
	else
	{
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// find delta (change to) pan & tilt
		float deltaPan = 0.25f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
		float deltaTilt = 0.25f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// View = Tilt * DeltaPan * Tilt^-1 * DeltaTilt * View;
		angleTilt += deltaTilt;
		mat4 m = mat4(1.f);
		m = rotate(m, radians(angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaPan), vec3(0.f, 1.f, 0.f));
		m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaTilt), vec3(1.f, 0.f, 0.f));
		matrixView = m * matrixView;
	}
}

int main(int argc, char** argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}
