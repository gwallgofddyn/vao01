
#include "core.h"
#include "TextureLoader.h"
#include "shader_setup.h"
#include "ArcballCamera.h"
#include "GUClock.h"
#include "AIMesh.h"
#include "Cylinder.h"


using namespace std;
using namespace glm;


struct DirectionalLight {

	vec3 direction;
	vec3 colour;
	
	DirectionalLight() {

		direction = vec3(0.0f, 1.0f, 0.0f); // default to point upwards
		colour = vec3(4.0f, 0.0f, 0.0f);
	}

	DirectionalLight(vec3 direction, vec3 colour = vec3(1.0f, 1.0f, 1.0f)) {

		this->direction = direction;
		this->colour = colour;
	}
};

struct PointLight {

	vec3 pos;
	vec3 colour;
	vec3 attenuation; // x=constant, y=linear, z=quadratic

	PointLight() {

		pos = vec3(0.0f, 0.0f, 0.0f);
		colour = vec3(1.0f, 1.0f, 0.0f);
		attenuation = vec3(1.0f, 1.0f, 1.0f);
	}

	PointLight(vec3 pos, vec3 colour = vec3(1.0f, 1.0f, 1.0f), vec3 attenuation = vec3(1.0f, 1.0f, 1.0f)) {

		this->pos = pos;
		this->colour = colour;
		this->attenuation = attenuation;
	}
};


#pragma region Global variables

// Window size
unsigned int		windowWidth = 1024;
unsigned int		windowHeight = 768;

// Main clock for tracking time (for animation / interaction)
GUClock*			gameClock = nullptr;

// Main camera
ArcballCamera*		mainCamera = nullptr;

// Mouse tracking
bool				mouseDown = false;
double				prevMouseX, prevMouseY;

// Keyboard tracking
bool				forwardPressed;
bool				backPressed;
bool				rotateLeftPressed;
bool				rotateRightPressed;


// Scene objects
AIMesh*				groundMesh = nullptr;
AIMesh*				creatureMesh = nullptr;
AIMesh*				columnMesh = nullptr;

AIMesh*             moonhutMesh = nullptr;
AIMesh*             moonmanMesh = nullptr;
AIMesh*             lampMesh = nullptr;
AIMesh*             satMesh = nullptr;

Cylinder*			cylinderMesh = nullptr;


// Shaders

// Texture-directional light shader
GLuint				texDirLightShader;
GLint				texDirLightShader_modelMatrix;
GLint				texDirLightShader_viewMatrix;
GLint				texDirLightShader_projMatrix;
GLint				texDirLightShader_texture;
GLint				texDirLightShader_lightDirection;
GLint				texDirLightShader_lightColour;

// Texture-point light shader
GLuint				texPointLightShader;
GLint				texPointLightShader_modelMatrix;
GLint				texPointLightShader_viewMatrix;
GLint				texPointLightShader_projMatrix;
GLint				texPointLightShader_texture;
GLint				texPointLightShader_lightPosition;
GLint				texPointLightShader_lightColour;
GLint				texPointLightShader_lightAttenuation;

//  *** normal mapping *** Normal mapped texture with Directional light
// This is the same as the texture direct light shader above, but with the addtional uniform variable
// to set the normal map sampler2D variable in the fragment shader.
GLuint				nMapDirLightShader;
GLint				nMapDirLightShader_modelMatrix;
GLint				nMapDirLightShader_viewMatrix;
GLint				nMapDirLightShader_projMatrix;
GLint				nMapDirLightShader_diffuseTexture;
GLint				nMapDirLightShader_normalMapTexture;
GLint				nMapDirLightShader_lightDirection;
GLint				nMapDirLightShader_lightColour;

// cylinder model
vec3 cylinderPos = vec3(-5.0f, 0.0f, 0.0f);

// beast model
vec3 beastPos = vec3(2.0f, 0.0f, 0.0f);
float beastRotation = 0.0f;

//moonhut model
vec3 moonhutPos = vec3(1.0f, 1.0f, 0.0f);

//moonman model
vec3 moonmanPos = vec3(1.0f, 1.0f, 5.0f);

//lamp model
vec3 lampPos =  vec3(10.0f, 1.0f, 0.0f);

//sat model
vec3 satPos = vec3(1.0f, 1.0f, 5.0f);


// Directional light example (declared as a single instance)
float directLightTheta = -30.0f;
DirectionalLight directLight = DirectionalLight(vec3(cosf(directLightTheta), sinf(directLightTheta), 0.0f));

// Setup point light example light (use array to make adding other lights easier later)
PointLight lights[1] = {
	PointLight(vec3(2.0f, 2.0f, 7.0f), 
	vec3(0.0f, 2.0f, -7.0f),
	vec3(1.0f, 1.0f, 1.0f) )
		};

bool rotateDirectionalLight = false;

#pragma endregion


// Function prototypes
void renderScene();
void renderWithDirectionalLight();
void renderWithPointLight();
void renderWithMultipleLights();
void updateScene();
void resizeWindow(GLFWwindow* window, int width, int height);
void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseMoveHandler(GLFWwindow* window, double xpos, double ypos);
void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods);
void mouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset);
void mouseEnterHandler(GLFWwindow* window, int entered);


int main() {

	//
	// 1. Initialisation
	//
	
	gameClock = new GUClock();

#pragma region OpenGL and window setup

	// Initialise glfw and setup window
	glfwInit();

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GLFW_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "CIS5013", NULL, NULL);

	// Check window was created successfully
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window!\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	

	// Set callback functions to handle different events
	glfwSetFramebufferSizeCallback(window, resizeWindow); // resize window callback
	glfwSetKeyCallback(window, keyboardHandler); // Keyboard input callback
	glfwSetCursorPosCallback(window, mouseMoveHandler);
	glfwSetMouseButtonCallback(window, mouseButtonHandler);
	glfwSetScrollCallback(window, mouseScrollHandler);
	glfwSetCursorEnterCallback(window, mouseEnterHandler);

	// Initialise glew
	glewInit();

	
	// Setup window's initial size
	resizeWindow(window, windowWidth, windowHeight);

#pragma endregion


	// Initialise scene - geometry and shaders etc
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // setup background colour to be black
	glClearDepth(1.0f);

	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_LINE);
	
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);


	//
	// Setup Textures, VBOs and other scene objects
	//
	mainCamera = new ArcballCamera(-45.0f, 45.0f, 40.0f, 40.0f, (float)windowWidth/(float)windowHeight, 0.1f, 500.0f);

	
	
	groundMesh = new AIMesh(string("Assets\\ground-surface\\surface01.obj"));
	if (groundMesh) {
		groundMesh->addTexture("Assets\\ground-surface\\stone.png", FIF_PNG);
	}

	moonhutMesh = new AIMesh(string("Assets\\moonhut\\moonhut.obj"));
	if (moonhutMesh) {
		moonhutMesh->addTexture(string("Assets\\moonhut\\moonhut_tex.bmp"), FIF_BMP);

	}
	moonmanMesh = new AIMesh(string("Assets\\moonman\\moonman.obj"));
	if (moonhutMesh) {
		moonmanMesh->addTexture(string("Assets\\moonman\\moonman_tex.PNG"), FIF_PNG);

	}

	lampMesh = new AIMesh(string("Assets\\lamp\\lamp.obj"));
	if (lampMesh) {
		lampMesh->addTexture(string("Assets\\lamp\\lamp_tex.PNG"), FIF_PNG);

	}

	satMesh = new AIMesh(string("Assets\\sat\\sat.obj"));
	if (satMesh) {
		satMesh->addTexture(string("Assets\\lamp\\lamp_tex.PNG"), FIF_PNG);

	}



	cylinderMesh = new Cylinder(string("Assets\\cylinder\\cylinderT.obj"));
	

	// Load shaders
	texPointLightShader = setupShaders(string("Assets\\Shaders\\texture-point.vert"), string("Assets\\Shaders\\texture-point.frag"));
	texDirLightShader = setupShaders(string("Assets\\Shaders\\texture-directional.vert"), string("Assets\\Shaders\\texture-directional.frag"));
	nMapDirLightShader = setupShaders(string("Assets\\Shaders\\nmap-directional.vert"), string("Assets\\Shaders\\nmap-directional.frag"));

	// Get uniform variable locations for setting values later during rendering
	texDirLightShader_modelMatrix = glGetUniformLocation(texDirLightShader, "modelMatrix");
	texDirLightShader_viewMatrix = glGetUniformLocation(texDirLightShader, "viewMatrix");
	texDirLightShader_projMatrix = glGetUniformLocation(texDirLightShader, "projMatrix");
	texDirLightShader_texture = glGetUniformLocation(texDirLightShader, "texture");
	texDirLightShader_lightDirection = glGetUniformLocation(texDirLightShader, "lightDirection");
	texDirLightShader_lightColour = glGetUniformLocation(texDirLightShader, "lightColour");

	texPointLightShader_modelMatrix = glGetUniformLocation(texPointLightShader, "modelMatrix");
	texPointLightShader_viewMatrix = glGetUniformLocation(texPointLightShader, "viewMatrix");
	texPointLightShader_projMatrix = glGetUniformLocation(texPointLightShader, "projMatrix");
	texPointLightShader_texture = glGetUniformLocation(texPointLightShader, "texture");
	texPointLightShader_lightPosition = glGetUniformLocation(texPointLightShader, "lightPosition");
	texPointLightShader_lightColour = glGetUniformLocation(texPointLightShader, "lightColour");
	texPointLightShader_lightAttenuation = glGetUniformLocation(texPointLightShader, "lightAttenuation");

	nMapDirLightShader_modelMatrix = glGetUniformLocation(nMapDirLightShader, "modelMatrix");
	nMapDirLightShader_viewMatrix = glGetUniformLocation(nMapDirLightShader, "viewMatrix");
	nMapDirLightShader_projMatrix = glGetUniformLocation(nMapDirLightShader, "projMatrix");
	nMapDirLightShader_diffuseTexture = glGetUniformLocation(nMapDirLightShader, "diffuseTexture");
	nMapDirLightShader_normalMapTexture = glGetUniformLocation(nMapDirLightShader, "normalMapTexture");
	nMapDirLightShader_lightDirection = glGetUniformLocation(nMapDirLightShader, "lightDirection");
	nMapDirLightShader_lightColour = glGetUniformLocation(nMapDirLightShader, "lightColour");


	//
	// 2. Main loop
	// 

	while (!glfwWindowShouldClose(window)) {

		updateScene();
		renderScene();						// Render into the current buffer
		glfwSwapBuffers(window);			// Displays what was just rendered (using double buffering).

		glfwPollEvents();					// Use this version when animating as fast as possible
	
		// update window title
		char timingString[256];
		sprintf_s(timingString, 256, "CIS5013: Average fps: %.0f; Average spf: %f", gameClock->averageFPS(), gameClock->averageSPF() / 1000.0f);
		glfwSetWindowTitle(window, timingString);
	}

	glfwTerminate();

	if (gameClock) {

		gameClock->stop();
		gameClock->reportTimingData();
	}

	return 0;
}


// renderScene - function to render the current scene
void renderScene()
{
	//renderWithDirectionalLight();
	//renderWithPointLight();
	renderWithMultipleLights();
}

// Demonstrate the use of a single directional light source
//  *** normal mapping ***  - since we're demonstrating the use of normal mapping with a directional light,
// the normal mapped objects are rendered here also!
void renderWithDirectionalLight() {

	// Clear the rendering window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Get camera matrices
	mat4 cameraProjection = mainCamera->projectionTransform();
	mat4 cameraView = mainCamera->viewTransform() * translate(identity<mat4>(), -beastPos);

#pragma region Render opaque objects

	// Plug-in texture-directional light shader and setup relevant uniform variables
	// (keep this shader for all textured objects affected by the light source)

	glUseProgram(texDirLightShader);

	glUniformMatrix4fv(texDirLightShader_viewMatrix, 1, GL_FALSE, (GLfloat*)&cameraView);
	glUniformMatrix4fv(texDirLightShader_projMatrix, 1, GL_FALSE, (GLfloat*)&cameraProjection);
	glUniform1i(texDirLightShader_texture, 0); // set to point to texture unit 0 for AIMeshes
	glUniform3fv(texDirLightShader_lightDirection, 1, (GLfloat*)&(directLight.direction));
	glUniform3fv(texDirLightShader_lightColour, 1, (GLfloat*)&(directLight.colour));

//	glUseProgram(texPointLightShader);

//	glUniformMatrix4fv(texPointLightShader_viewMatrix, 1, GL_FALSE, (GLfloat*)&cameraView);
//	glUniformMatrix4fv(texPointLightShader_projMatrix, 1, GL_FALSE, (GLfloat*)&cameraProjection);
//	glUniform1i(texPointLightShader_texture, 0); // set to point to texture unit 0 for AIMeshes
//	glUniform3fv(texPointLightShader_lightPosition, 1, (GLfloat*)&(lights[0].pos));
//	glUniform3fv(texPointLightShader_lightColour, 1, (GLfloat*)&(lights[0].colour));
//	glUniform3fv(texPointLightShader_lightAttenuation, 1, (GLfloat*)&(lights[0].attenuation));

	if (groundMesh) {

		mat4 modelTransform = glm::scale(identity<mat4>(), vec3(10.0f, 1.0f, 10.0f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);
		
		groundMesh->setupTextures();
		groundMesh->render();
	}

	if (moonhutMesh) {

		mat4 modelTransform = glm::translate(identity<mat4>(), vec3(-2.0f, 0.0f, 0.0f)) * glm::scale(identity<mat4>(), vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		moonhutMesh->setupTextures();
		moonhutMesh->render();
	}

	if (moonmanMesh) {

		mat4 modelTransform = glm::translate(identity<mat4>(), vec3(5.0f, 0.0f, 0.0f)) * glm::scale(identity<mat4>(), vec3(0.05f, 0.05f, 0.05f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		moonmanMesh->setupTextures();
		moonmanMesh->render();
	}

#pragma endregion

	// Enable additive blending for ***subsequent*** light sources!!!
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	

#pragma region Render all opaque objects with point light

	glUseProgram(texPointLightShader);

	glUniformMatrix4fv(texPointLightShader_viewMatrix, 1, GL_FALSE, (GLfloat*)&cameraView);
	glUniformMatrix4fv(texPointLightShader_projMatrix, 1, GL_FALSE, (GLfloat*)&cameraProjection);
	glUniform1i(texPointLightShader_texture, 0); // set to point to texture unit 0 for AIMeshes
	glUniform3fv(texPointLightShader_lightPosition, 1, (GLfloat*)&(lights[0].pos));
	glUniform3fv(texPointLightShader_lightColour, 1, (GLfloat*)&(lights[0].colour));
	glUniform3fv(texPointLightShader_lightAttenuation, 1, (GLfloat*)&(lights[0].attenuation));

if (moonhutMesh) {

		mat4 modelTransform = glm::translate(identity<mat4>(), vec3(-2.0f, 0.0f, 0.0f)) * glm::scale(identity<mat4>(), vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		moonhutMesh->setupTextures();
		moonhutMesh->render();
	}

#pragma region Render transparant objects

	// Done with textured meshes - render transparent objects now (cylinder in this example)...

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	
	if (cylinderMesh)
	  {

		mat4 T = cameraProjection * cameraView * glm::translate(identity<mat4>(), cylinderPos);

		cylinderMesh->setupTextures();
       cylinderMesh->render(T);
	}

	//if (moonhutMesh) {

		// mat4 modelTransform = glm::translate(identity<mat4>(), vec3(0.1f, 0.0f, 0.0f)) * glm::scale(identity<mat4>(), vec3(0.1f, 0.1f, 0.1f));
		//mat4 modelTransform = cameraProjection * cameraView * glm::scale(identity<mat4>(), moonhutPos);

		// glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

	//	moonhutMesh->setupTextures();
	//	moonhutMesh->render();


//	}
	glDisable(GL_BLEND);

#pragma endregion

	
	//
	// For demo purposes, render directional light source
	//
	
	// Restore fixed-function pipeline
	glUseProgram(0);
	glBindVertexArray(0);
	glDisable(GL_TEXTURE_2D);

	mat4 cameraT = cameraProjection * cameraView;
	glLoadMatrixf((GLfloat*)&cameraT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(8.0f);
	glBegin(GL_POINTS);

	//glColor3f(directLight.colour.r, directLight.colour.g, directLight.colour.b);
//	glVertex3f(directLight.direction.x * 1.0f, directLight.direction.y * 1.0f, directLight.direction.z * 1.0f);

	glColor3f(lights[0].colour.r, lights[0].colour.g, lights[1].colour.b);
	glVertex3f(lights[0].pos.x, lights[0].pos.y, lights[0].pos.z);

	//glColor3f(lights[0].colour.r, lights[0].colour.g, lights[0].colour.b);
	//glVertex3f(lights[1].pos.x, lights[1].pos.y, lights[1].pos.z);

	glEnd();
}


// Demonstrate the use of a single point light source
void renderWithPointLight() {

	// Clear the rendering window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Get camera matrices
	mat4 cameraProjection = mainCamera->projectionTransform();
	mat4 cameraView = mainCamera->viewTransform() * translate(identity<mat4>(), -beastPos);

	// Plug-in texture-point light shader and setup relevant uniform variables
	// (keep this shader for all textured objects affected by the light source)
	glUseProgram(texPointLightShader);

	glUniformMatrix4fv(texPointLightShader_viewMatrix, 1, GL_FALSE, (GLfloat*)&cameraView);
	glUniformMatrix4fv(texPointLightShader_projMatrix, 1, GL_FALSE, (GLfloat*)&cameraProjection);
	glUniform1i(texPointLightShader_texture, 0); // set to point to texture unit 0 for AIMeshes
	glUniform3fv(texPointLightShader_lightPosition, 1, (GLfloat*)&(lights[0].pos));
	glUniform3fv(texPointLightShader_lightColour, 1, (GLfloat*)&(lights[0].colour));
	glUniform3fv(texPointLightShader_lightAttenuation, 1, (GLfloat*)&(lights[0].attenuation));
	
#pragma region Render opaque objects

	if (moonhutMesh) {

		mat4 modelTransform = glm::translate(identity<mat4>(), vec3(0.01f, 0.0f, 0.0f)) * glm::scale(identity<mat4>(), vec3(0.1f, 0.1f, 0.1f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		moonhutMesh->setupTextures();
		moonhutMesh->render();
	}

	if (groundMesh) {

		mat4 modelTransform = glm::scale(identity<mat4>(), vec3(10.0f, 1.0f, 10.0f));

		glUniformMatrix4fv(texPointLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		groundMesh->setupTextures();
		groundMesh->render();
	}

	

	



#pragma endregion

#pragma region Render transparant objects

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (cylinderMesh) {

		mat4 T = cameraProjection * cameraView * glm::translate(identity<mat4>(), cylinderPos);

		cylinderMesh->setupTextures();
		cylinderMesh->render(T);
	}

	glDisable(GL_BLEND);

#pragma endregion


	//
	// For demo purposes, render point light source
	//

	// Restore fixed-function
	glUseProgram(0);
	glBindVertexArray(0);
	glDisable(GL_TEXTURE_2D);

	mat4 cameraT = cameraProjection * cameraView;
	glLoadMatrixf((GLfloat*)&cameraT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(10.0f);
	glBegin(GL_POINTS);
	glColor3f(lights[0].colour.r, lights[0].colour.g, lights[0].colour.b);
	glVertex3f(lights[0].pos.x, lights[0].pos.y, lights[0].pos.z);
	glEnd();
}


void renderWithMultipleLights() {

	// Clear the rendering window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Get camera matrices
	mat4 cameraProjection = mainCamera->projectionTransform();
	mat4 cameraView = mainCamera->viewTransform() * translate(identity<mat4>(), -beastPos);


#pragma region Render all opaque objects with directional light

	glUseProgram(texDirLightShader);

	glUniformMatrix4fv(texDirLightShader_viewMatrix, 1, GL_FALSE, (GLfloat*)&cameraView);
	glUniformMatrix4fv(texDirLightShader_projMatrix, 1, GL_FALSE, (GLfloat*)&cameraProjection);
	glUniform1i(texDirLightShader_texture, 0); // set to point to texture unit 0 for AIMeshes
	glUniform3fv(texDirLightShader_lightDirection, 1, (GLfloat*)&(directLight.direction));
	glUniform3fv(texDirLightShader_lightColour, 1, (GLfloat*)&(directLight.colour));

	if (groundMesh) {

		mat4 modelTransform = glm::scale(identity<mat4>(), vec3(10.0f, 1.0f, 10.0f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		groundMesh->setupTextures();
		groundMesh->render();
	}

	if (moonhutMesh) {

		mat4 modelTransform = glm::translate(identity<mat4>(), vec3(-0.2f, 0.0f, 0.0f)) * glm::scale(identity<mat4>(), vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		moonhutMesh->setupTextures();
		moonhutMesh->render();
	}

	if (lampMesh) {

		mat4 modelTransform = glm::translate(identity<mat4>(), vec3(2.8f, 0.0f, 7.0f)) * glm::scale(identity<mat4>(), vec3(0.11f, 0.11f, 0.11f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		lampMesh->setupTextures();
		lampMesh->render();
	}
	

	if (moonmanMesh) {

		mat4 modelTransform = glm::translate(identity<mat4>(), vec3(5.0f, 0.0f, 7.0f)) * glm::scale(identity<mat4>(), vec3(0.05f, 0.05f, 0.05f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		moonmanMesh->setupTextures();
		moonmanMesh->render();
	}

	if (satMesh) {

		mat4 modelTransform = glm::translate(identity<mat4>(), vec3(2.8f, -0.1f, -7.0f)) * glm::scale(identity<mat4>(), vec3(0.07f, 0.07f, 0.07f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		satMesh->setupTextures();
		satMesh->render();
	}


#pragma endregion



	// Enable additive blending for ***subsequent*** light sources!!!
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);



#pragma region Render all opaque objects with point light

	glUseProgram(texPointLightShader);

	glUniformMatrix4fv(texPointLightShader_viewMatrix, 1, GL_FALSE, (GLfloat*)&cameraView);
	glUniformMatrix4fv(texPointLightShader_projMatrix, 1, GL_FALSE, (GLfloat*)&cameraProjection);
	glUniform1i(texPointLightShader_texture, 0); // set to point to texture unit 0 for AIMeshes
	glUniform3fv(texPointLightShader_lightPosition, 1, (GLfloat*)&(lights[0].pos));
	glUniform3fv(texPointLightShader_lightColour, 1, (GLfloat*)&(lights[0].colour));
	glUniform3fv(texPointLightShader_lightAttenuation, 1, (GLfloat*)&(lights[0].attenuation));

	if (groundMesh) {

		mat4 modelTransform = glm::scale(identity<mat4>(), vec3(10.0f, 1.0f, 10.0f));

		glUniformMatrix4fv(texPointLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		groundMesh->setupTextures();
		groundMesh->render();
	}



	if (moonhutMesh) {

		mat4 modelTransform = glm::translate(identity<mat4>(), vec3(-0.2f, 0.0f, 0.0f)) * glm::scale(identity<mat4>(), vec3(1.0f, 1.0f, 1.0f));
		;

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		moonhutMesh->setupTextures();
		moonhutMesh->render();
	}
	
	
#pragma endregion


#pragma region Render transparant objects

	glEnable(GL_BLEND);

//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	if (cylinderMesh) {

//		mat4 T = cameraProjection * cameraView * glm::translate(identity<mat4>(), cylinderPos);

//		cylinderMesh->setupTextures();
//		cylinderMesh->render(T);
//	}

//	if (satMesh) {

//		mat4 modelTransform = glm::translate(identity<mat4>(), vec3(2.8f, -0.1f, -7.0f)) * glm::scale(identity<mat4>(), vec3(0.07f, 0.07f, 0.07f));

//		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

//		satMesh->setupTextures();
//		satMesh->render();
//	}

   glDisable(GL_BLEND);

#pragma endregion


	//
	// For demo purposes, render light sources
	//

	// Restore fixed-function
	glUseProgram(0);
	glBindVertexArray(0);
	glDisable(GL_TEXTURE_2D);

	mat4 cameraT = cameraProjection * cameraView;
	glLoadMatrixf((GLfloat*)&cameraT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(8.0f);
	
	glBegin(GL_POINTS);

	glColor3f(directLight.colour.r, directLight.colour.g, directLight.colour.b);
	glVertex3f(directLight.direction.x * 0.0f, directLight.direction.y * 1.0f, directLight.direction.z * 0.0f);

	glColor3f(lights[0].colour.r, lights[0].colour.g, lights[0].colour.b);
	glVertex3f(lights[0].pos.x, lights[0].pos.y, lights[0].pos.z);
	
	glEnd();
}


// Function called to animate elements in the scene
void updateScene() {

	float tDelta = 0.0f;

	if (gameClock) {

		gameClock->tick();
		tDelta = (float)gameClock->gameTimeDelta();
	}

	cylinderMesh->update(tDelta);

	// update main light source
	if (rotateDirectionalLight) {

		directLightTheta += glm::radians(90.0f) * tDelta;
		directLight.direction = vec3(cosf(directLightTheta), sinf(directLightTheta), 0.0f);
	}
	

	//
	// Handle movement based on user input
	//

	float moveSpeed = 3.0f; // movement displacement per second
	float rotateSpeed = 90.0f; // degrees rotation per second

	if (forwardPressed) {

		mat4 R = eulerAngleY<float>(glm::radians<float>(beastRotation)); // local coord space / basis vectors - move along z
		float dPos = moveSpeed * tDelta; // calc movement based on time elapsed
		beastPos += vec3(R[2].x * dPos, R[2].y * dPos, R[2].z * dPos); // add displacement to position vector
	}
	else if (backPressed) {

		mat4 R = eulerAngleY<float>(glm::radians<float>(beastRotation)); // local coord space / basis vectors - move along z
		float dPos = -moveSpeed * tDelta; // calc movement based on time elapsed
		beastPos += vec3(R[2].x * dPos, R[2].y * dPos, R[2].z * dPos); // add displacement to position vector
	}

	if (rotateLeftPressed) {

		beastRotation += rotateSpeed * tDelta;
	}
	else if (rotateRightPressed) {

		beastRotation -= rotateSpeed * tDelta;
	}

}


#pragma region Event handler functions

// Function to call when window resized
void resizeWindow(GLFWwindow* window, int width, int height)
{
	if (mainCamera) {

		mainCamera->setAspect((float)width / (float)height);
	}

	// Update viewport to cover the entire window
	glViewport(0, 0, width, height);

	windowWidth = width;
	windowHeight = height;
}


// Function to call to handle keyboard input
void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {

		// check which key was pressed...
		switch (key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, true);
				break;
			
			case GLFW_KEY_W:
				forwardPressed = true;
				break;

			case GLFW_KEY_S:
				backPressed = true;
				break;

			case GLFW_KEY_A:
				rotateLeftPressed = true;
				break;

			case GLFW_KEY_D:
				rotateRightPressed = true;
				break;

			case GLFW_KEY_SPACE:
				rotateDirectionalLight = !rotateDirectionalLight;
				break;

			default:
			{
			}
		}
	}
	else if (action == GLFW_RELEASE) {
		// handle key release events
		switch (key)
		{
			case GLFW_KEY_W:
				forwardPressed = false;
				break;

			case GLFW_KEY_S:
				backPressed = false;
				break;

			case GLFW_KEY_A:
				rotateLeftPressed = false;
				break;

			case GLFW_KEY_D:
				rotateRightPressed = false;
				break;

			default:
			{
			}
		}
	}
}


void mouseMoveHandler(GLFWwindow* window, double xpos, double ypos) {

	if (mouseDown) {

		float dx = float(xpos - prevMouseX);
		float dy = float(ypos - prevMouseY);

		if (mainCamera)
			mainCamera->rotateCamera(-dy, -dx);

		prevMouseX = xpos;
		prevMouseY = ypos;
	}

}

void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods) {

	if (button == GLFW_MOUSE_BUTTON_LEFT) {

		if (action == GLFW_PRESS) {

			mouseDown = true;
			glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
		}
		else if (action == GLFW_RELEASE) {

			mouseDown = false;
		}
	}
}

void mouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset) {

	if (mainCamera) {

		if (yoffset < 0.0)
			mainCamera->scaleRadius(1.1f);
		else if (yoffset > 0.0)
			mainCamera->scaleRadius(0.9f);
	}
}

void mouseEnterHandler(GLFWwindow* window, int entered) {
}

#pragma endregion