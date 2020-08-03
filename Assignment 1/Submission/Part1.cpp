#include <cstdio>		// for C++ i/o
#include <iostream>
#include <string>
#include <cstddef>
#include <random>
#include <ctime>
using namespace std;	// to avoid having to use std::

#include <GLEW/glew.h>	// include GLEW
#include <GLFW/glfw3.h>	// include GLFW (which includes the OpenGL header)
#include <glm/glm.hpp>	// include GLM (ideally should only use the GLM headers that are actually used)
#include <glm/gtx/transform.hpp>
using namespace glm;	// to avoid having to use glm::

#include <AntTweakBar.h>

#include "shader.h"

#define PI 3.14159265
#define MAX_SLICES 32
#define MIN_SLICES 10
#define MAX_VERTICES ((MAX_SLICES+2)*3)+9	// a triangle fan should have a minimum of 3 vertices
#define CIRCLE_RADIUS 0.5
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define RAND_MAX 1

// struct for vertex attributes
struct Vertex
{
	GLfloat position[3];
	GLfloat color[3];
};

// global variables

Vertex g_circle_verts[MAX_VERTICES] = {

	// circle origin vertex
	0.0f, 0.0f, 0.0f,	// position
	1.0f, 1.0f, 0.0f,	// colour
	// circle origin vertex 2
	0.0f, 0.0f, 0.0f,	// position
	1.0f, 1.0f, 0.0f,	// colour
};

Vertex g_line_verts[2] =
{
	//left point
	-1.0f,-0.8f,-0.2f,  //position
	1.0f,1.0f,1.0f,    //color

	//right point
	1.0f,-0.8f,-0.2f,
	1.0f,1.0f,1.0f,
};





Vertex g_carriage_verts[7] = {

	// carriage origin vertex
	0.0f, 0.0f, -0.05f,	// position
	1.0f, 1.0f, 0.0f,	// colour

	// carriage origin vertex 2
	-0.05f, 0.0f, -0.05f,	// position
	0.0f, 0.0f, 1.0f,	// colour

	// carriage origin vertex 3
	-0.025f, 0.025f, -0.05f,	// position
	1.0f, 1.0f, 0.0f,	// colour

	// carriage origin vertex 4
	0.025f, 0.025f, -0.05f,	// position
	0.0f, 0.0f, 1.0f,	// colour

	// carriage origin vertex 5
	0.05f, 0.0f, -0.05f,	// position
	1.0f, 1.0f, 0.0f,	// colour

	// carriage origin vertex 6
	0.025f, -0.025f, -0.05f,	// position
	0.0f, 0.0f, 1.0f,	// colour

	//carriage origin vertex 7
	-0.025f,-0.025f,-0.05f,
	1.0f,1.0f,0.0f,
};

Vertex g_triangle_verts[3] = {

	// triangle origin vertex
	0.0f, 0.0f, -0.1f,	// position
	1.0f, 0.0f, 0.0f,	// colour
	
	// triangle vertex 2
	-0.3f, -0.8f, -0.1f,	// position
	1.0f, 0.0f, 0.0f,	// colour

	//triangle vertex 3
	0.3f,-0.8f,-0.1f,
	1.0f,0.0f,0.0f,
};

GLuint g_circle_indices[50] = {
	0,1,	// triangle 1
};

GLuint g_line_indices[2] =
{
	0,1,
};

GLuint g_triangle_indices[3] = {
	0,1,2	// triangle 1
};
GLuint g_carriage_indices[8] = {
	0,1,2,3,4,5,6,1
};

GLuint g_slices = MIN_SLICES;
GLuint g_IBO = 0;				// index buffer object identifier
GLuint g_VBO = 0;				// vertex buffer object identifier
GLuint g_VAO = 0;				// vertex array object identifier
GLuint g_shaderProgramID = 0;	// shader program identifier
GLuint g_MVP_Index = 0;			// location in shader
glm::mat4 g_modelMatrix[3];		// object model matrices

bool stopped = false;
bool clockWise = false;

//glm::mat4 g_viewMatrix;			// view matrix
//glm::mat4 g_projectionMatrix;	// projection matrix

float g_orbitSpeed = 0.3f;
float g_rotationSpeed[2] = { 0.2f, 0.4f };

float slice_angle = PI * 2 / static_cast<float>(g_slices);	// angle for each fan slice
float angle = slice_angle;

void generate_circle();
float rotationAngle[2] = { 0.0f, 0.0f };

static void init(GLFWwindow* window)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// set clear background colour

	srand(time(NULL));
	glEnable(GL_DEPTH_TEST);	// enable depth buffer test

	// create and compile our GLSL program from the shader files
	g_shaderProgramID = loadShaders("MVP_VS.vert", "ColorFS.frag");
	// generate vertices of triangle fan
	generate_circle();
	//cout << "----------------" << endl;
	//cout << g_slices << endl;

	// find the location of shader variables
	GLuint positionIndex = glGetAttribLocation(g_shaderProgramID, "aPosition");
	GLuint colorIndex = glGetAttribLocation(g_shaderProgramID, "aColor");
	g_MVP_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewProjectionMatrix");

	// initialise model matrix to the identity matrix
	g_modelMatrix[0] = g_modelMatrix[1] = g_modelMatrix[2] = glm::mat4(1.0f);

	// initialise view matrix
	//g_viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspectRatio = static_cast<float>(width) / height;

	// initialise projection matrix
	//g_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	
	// generate identifier for VBO and copy data to GPU
	glGenBuffers(1, &g_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_circle_verts), g_circle_verts, GL_STATIC_DRAW);

	// generate identifier for IBO and copy data to GPU
	glGenBuffers(1, &g_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);


	// generate identifiers for VAO
	glGenVertexArrays(1, &g_VAO);

	// create VAO and specify VBO data
	glBindVertexArray(g_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	// interleaved attributes
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(colorIndex);
}

void generate_circle()
{

								// angle used to generate x and y coordinates
	float scale_factor = static_cast<float>(WINDOW_HEIGHT) / WINDOW_WIDTH;	// scale to make it a circle instead of an elipse
	int index = 0;	// vertex index
	GLuint asdf = g_slices;
	
	g_circle_verts[1].position[0] = CIRCLE_RADIUS * scale_factor;	// set x coordinate of vertex 1

													// generate vertex coordinates for triangle fan
	//cout << g_slices << endl;
	//cout << angle << endl;
	for (int i = 2; i < g_slices + 2; i++)
	{
		// multiply by 3 because a vertex has x, y, z coordinates

		g_circle_verts[i].position[0] = CIRCLE_RADIUS * cos(angle) * scale_factor;
		g_circle_verts[i].position[1] = CIRCLE_RADIUS * sin(angle);
		g_circle_verts[i].position[2] = 0.0f;
		
		g_circle_verts[i].color[0] = 1.0f;
		g_circle_verts[i].color[1] = 1.0f;
		g_circle_verts[i].color[2] = 0.0f;


		g_circle_indices[i] = i;
	

		// update to next angle
		angle += slice_angle;
		
	}
}
// function used to update the scene
static void update_scene()
{
	// static variables for rotation angles
	static float orbitAngle = 0.0f;
	float scaleFactor = 0.1;
	float increment = 0.1;

	// update rotation angles
	//orbitAngle += g_orbitSpeed * scaleFactor;
	rotationAngle[0] += g_rotationSpeed[0] * scaleFactor;


	if (clockWise)
	{
		rotationAngle[1] += g_rotationSpeed[1] * scaleFactor;
		if (rotationAngle[1] > 3)
		{
			clockWise = false;
		}
	}
	else
	{
		rotationAngle[1] -= g_rotationSpeed[1] * scaleFactor;
		if (rotationAngle[1] < 0)
		{
			clockWise = true;
		}
	}
	
	//cout << rotationAngle[1] << endl;

	// update model matrix
	g_modelMatrix[0] = glm::rotate(rotationAngle[0], glm::vec3(0.0f, 0.0f, 1.0f));



	//g_modelMatrix[1] = glm::translate(glm::vec3(2.0f, 0.0f, 0.0f))
		//* glm::rotate(rotationAngle[1], glm::vec3(0.0f, 1.0f, 0.0f))
		//* glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
}

// function used to render the scene
static void render_scene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear colour buffer and depth buffer

	glUseProgram(g_shaderProgramID);	// use the shaders associated with the shader program

	glBindVertexArray(g_VAO);		// make VAO active
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);

// Object 1 (base)
	glm::mat4 MVP2 = g_modelMatrix[1];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP2[0][0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_triangle_verts), g_triangle_verts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_triangle_indices), g_triangle_indices, GL_STATIC_DRAW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

	//line
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_line_verts), g_line_verts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_line_indices), g_line_indices, GL_STATIC_DRAW);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);

	//carriages
	glm::mat4 MVP3 = g_modelMatrix[2];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP3[0][0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_carriage_verts), g_carriage_verts, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_carriage_indices), g_carriage_indices, GL_DYNAMIC_DRAW);

	float ratio = (2 * PI / 10);
	bool neg = false;
	for (int i = 0; i < 10; i++)
	{
		float j = i;
		float randomWiggle = (rand() %100) / 100;
		//cout << randomWiggle << endl;
		float rotationAng = (rotationAngle[0] + ratio*j);
		if (!neg)
		{
			g_modelMatrix[2] =
				glm::rotate(rotationAng, glm::vec3(0.0f, 0.0f, 1.0f))
				*glm::translate(glm::vec3(0.5f, 0.0f, 0.0f))
				*glm::rotate(rotationAngle[1] + randomWiggle, glm::vec3(0.0f, 0.0f, 1.0f))
				;
		}
		else
		{
			g_modelMatrix[2] =
				glm::rotate(rotationAng, glm::vec3(0.0f, 0.0f, 1.0f))
				*glm::translate(glm::vec3(0.5f, 0.0f, 0.0f))
				*glm::rotate(-(rotationAngle[1] + randomWiggle), glm::vec3(0.0f, 0.0f, 1.0f))
				;

		}

		glm::mat4 MVP3 = g_modelMatrix[2];
		glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP3[0][0]);
		glDrawElements(GL_TRIANGLE_FAN, 8, GL_UNSIGNED_INT, 0);
		if (neg)
		{
			neg = false;
		}
		else
		{
			neg = true;
		}
	}

	glm::mat4 MVP = g_modelMatrix[0];
	// set uniform model transformation matrix
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);

	//circle
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_circle_verts), g_circle_verts, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_circle_indices), g_circle_indices, GL_DYNAMIC_DRAW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLE_FAN, g_slices+2, GL_UNSIGNED_INT, 0);



	



	//glDrawElements(GL_TRIANGLE_FAN, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

// Object 2 (random ass triangle rn)
	//glm::mat4 = g_projectionMatrix * g_viewMatrix * g_modelMatrix[1];
	//glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	//glDrawElements(GL_TRIANGLE_FAN, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

	glFlush();	// flush the pipeline
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// quit if the ESCAPE key was press
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if (stopped)
		{
			g_rotationSpeed[0] = 0.2;
			g_rotationSpeed[1] = 0.4;
			stopped = false;
		}
		else
		{
			g_rotationSpeed[0] = 0.0;
			g_rotationSpeed[1] = 0.0;
			stopped = true;
		}
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		g_rotationSpeed[0] = -g_rotationSpeed[0];
		return;
	}
}

// mouse movement callback function
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// pass mouse data to tweak bar
	TwEventMousePosGLFW(xpos, ypos);
}

// mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// pass mouse data to tweak bar
	TwEventMouseButtonGLFW(button, action);
}

// error callback function
static void error_callback(int error, const char* description)
{
	cerr << description << endl;	// output error description
}

int main(void)
{
	GLFWwindow* window = NULL;	// pointer to a GLFW window handle
	TwBar *TweakBar;			// pointer to a tweak bar

	glfwSetErrorCallback(error_callback);	// set error callback function

	// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window and its OpenGL context
	window = glfwCreateWindow(1000, 1000, "Assignment 1 Part 1 - Jacob Fiola", NULL, NULL);

	// if failed to create window
	if (window == NULL)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval

	// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		cerr << "GLEW initialisation failed" << endl;
		exit(EXIT_FAILURE);
	}

	// set key callback function
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	

	// initialise rendering states
	init(window);

	// variables for simple time management
	float lastUpdateTime = glfwGetTime();
	float currentTime = lastUpdateTime;

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();

		// only update if more than 0.02 seconds since last update
		if (currentTime - lastUpdateTime > 0.02)
		{
			update_scene();		// update the scene
			render_scene();		// render the scene

			//TwDraw();			// draw tweak bar(s)

			glfwSwapBuffers(window);	// swap buffers
			glfwPollEvents();			// poll for events

			lastUpdateTime = currentTime;	// update last update time
		}
	}

	// clean up
	glDeleteProgram(g_shaderProgramID);
	glDeleteBuffers(1, &g_IBO);
	glDeleteBuffers(1, &g_VBO);
	glDeleteVertexArrays(1, &g_VAO);



	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

