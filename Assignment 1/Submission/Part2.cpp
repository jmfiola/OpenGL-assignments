#include <cstdio>		// for C++ i/o
#include <iostream>
#include <string>
#include <cstddef>
using namespace std;	// to avoid having to use std::

#include <GLEW/glew.h>	// include GLEW
#include <GLFW/glfw3.h>	// include GLFW (which includes the OpenGL header)
#include <glm/glm.hpp>	// include GLM (ideally should only use the GLM headers that are actually used)
#include <glm/gtx/transform.hpp>
using namespace glm;	// to avoid having to use glm::


#include "shader.h"

#define PI 3.141592
#define NUM_SIDES 30
#define NUM_VERTS 31
// struct for vertex attributes
struct Vertex
{
	GLfloat position[3];
	GLfloat color[3];
};

// global variables

Vertex g_vertices[] = {
	// vertex 1
	-0.5f, 0.5f, 0.5f,	// position
	1.0f, 0.0f, 1.0f,	// colour
	// vertex 2
	-0.5f, -0.5f, 0.5f,	// position
	1.0f, 0.0f, 0.0f,	// colour
	// vertex 3
	0.5f, 0.5f, 0.5f,	// position
	1.0f, 1.0f, 1.0f,	// colour
	// vertex 4
	0.5f, -0.5f, 0.5f,	// position
	1.0f, 1.0f, 0.0f,	// colour
	// vertex 5
	-0.5f, 0.5f, -0.5f,	// position
	0.0f, 0.0f, 1.0f,	// colour
	// vertex 6
	-0.5f, -0.5f, -0.5f,// position
	0.0f, 0.0f, 0.0f,	// colour
	// vertex 7
	0.5f, 0.5f, -0.5f,	// position
	0.0f, 1.0f, 1.0f,	// colour
	// vertex 8
	0.5f, -0.5f, -0.5f,	// position
	0.0f, 1.0f, 0.0f,	// colour
};

GLuint g_indices[] = {
	0, 1, 2,	// triangle 1
	2, 1, 3,	// triangle 2
	4, 5, 0,	// triangle 3
	0, 5, 1,	// ...
	2, 3, 6,
	6, 3, 7,
	4, 0, 6,
	6, 0, 2,
	1, 5, 3,
	3, 5, 7,
	5, 4, 7,
	7, 4, 6,	// triangle 12
};

GLuint g_circle_indices[NUM_VERTS+1];
Vertex g_circle_verts[NUM_VERTS];
Vertex g_bigger_circle_verts[NUM_VERTS];
Vertex g_unit_circle_verts[NUM_VERTS];

GLuint g_IBO = 0;				// index buffer object identifier
GLuint g_VBO = 0;				// vertex buffer object identifier
GLuint g_VAO = 0;				// vertex array object identifier
GLuint g_shaderProgramID = 0;	// shader program identifier
GLuint g_MVP_Index = 0;			// location in shader
glm::mat4 g_modelMatrix[10];		// object model matrices
glm::mat4 g_viewMatrix;			// view matrix
glm::mat4 g_projectionMatrix;	// projection matrix

float g_orbitSpeed[2] = {0.4f,0.2f};
float g_rotationSpeed[3] = { 0.1f, 0.3f,0.5f };

void drawCircle(GLfloat x, GLfloat y, GLfloat z, GLfloat radius,Vertex vertices[])
{

	GLfloat doublePi = 2.0f * PI;

	GLfloat circleVerticesX[NUM_VERTS];
	GLfloat circleVerticesY[NUM_VERTS];
	GLfloat circleVerticesZ[NUM_VERTS];

	//circleVerticesX[0] = x;
	//circleVerticesY[0] = y;
	//circleVerticesZ[0] = z;

	for (int i = 0; i < NUM_VERTS; i++)
	{
		circleVerticesX[i] = x + (radius * cos(i * doublePi / NUM_SIDES));
		circleVerticesY[i] = y; //+ (radius * sin(i * doublePi / NUM_SIDES));
		circleVerticesZ[i] = z + (radius * sin(i * doublePi / NUM_SIDES));
	}


	for (int i = 0; i < NUM_VERTS; i++)
	{
		vertices[i].position[0] = circleVerticesX[i];
		vertices[i].position[1] = circleVerticesY[i];
		vertices[i].position[2] = circleVerticesZ[i];
		vertices[i].color[0] = 1.0f;
		vertices[i].color[1] = 0.0f;
		vertices[i].color[2] = 0.0f;

		g_circle_indices[i] = i;

	}
	g_circle_indices[NUM_VERTS] = 0;
	return;



	
}


static void init(GLFWwindow* window)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// set clear background colour

	glEnable(GL_DEPTH_TEST);	// enable depth buffer test

	// create and compile our GLSL program from the shader files
	g_shaderProgramID = loadShaders("MVP_VS.vert", "ColorFS.frag");

	// find the location of shader variables
	GLuint positionIndex = glGetAttribLocation(g_shaderProgramID, "aPosition");
	GLuint colorIndex = glGetAttribLocation(g_shaderProgramID, "aColor");
	g_MVP_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewProjectionMatrix");

	// initialise model matrix to the identity matrix
	g_modelMatrix[0] = g_modelMatrix[1] = g_modelMatrix[2]= g_modelMatrix[3] =  glm::mat4(1.0f);
	g_modelMatrix[4] = g_modelMatrix[5] = g_modelMatrix[6] = g_modelMatrix[7] = glm::mat4(1.0f);

	// initialise view matrix
	g_viewMatrix = glm::lookAt(glm::vec3(0.0f, 3.0f, 6.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspectRatio = static_cast<float>(width) / height;

	// initialise projection matrix
	g_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	
	// generate identifier for VBO and copy data to GPU
	glGenBuffers(1, &g_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertices), g_vertices, GL_STATIC_DRAW);

	// generate identifier for IBO and copy data to GPU
	glGenBuffers(1, &g_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_indices), g_indices, GL_STATIC_DRAW);

	// generate identifiers for VAO
	glGenVertexArrays(1, &g_VAO);

	// create VAO and specify VBO data
	glBindVertexArray(g_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	// interleaved attributes
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(colorIndex);

	drawCircle(0.0f, 0.0f, 0.0f, 2.0f,g_circle_verts);
	drawCircle(0.0f, 0.0f, 0.0f, 3.0f,g_bigger_circle_verts);
	drawCircle(0.0f, 0.0f, 0.0f, 1.0f, g_unit_circle_verts);

	
}


// function used to update the scene
static void update_scene()
{
	// static variables for rotation angles
	static float orbitAngle[2] = { 0.0f,0.0f };
	static float rotationAngle[3] = { 0.0f, 0.0f,0.0f };
	float scaleFactor = 0.1;

	
	// update rotation angles
	orbitAngle[0]+= g_orbitSpeed[0] * scaleFactor;
	orbitAngle[1] += g_orbitSpeed[1] * scaleFactor;
	rotationAngle[0] += g_rotationSpeed[0] * scaleFactor;
	rotationAngle[1] += g_rotationSpeed[1] * scaleFactor;
	rotationAngle[2] += g_rotationSpeed[2] * scaleFactor;

	// update model matrix
	//sun
	g_modelMatrix[0] = glm::rotate(rotationAngle[0], glm::vec3(0.0f, 1.0f, 0.0f));

	//first planet
	g_modelMatrix[1] = glm::rotate(orbitAngle[0], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(2.0f, 0.0f, 0.0f))
		* glm::rotate(rotationAngle[1], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));

	//2nd planet
	g_modelMatrix[3] = glm::rotate(orbitAngle[1], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(3.0f, 0.0f, 0.0f))
		* glm::rotate(rotationAngle[2], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));

	//2nd planet ring 1
	g_modelMatrix[4] = glm::rotate(orbitAngle[1], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(3.0f, 0.0f, 0.0f))
		*glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));

	//2nd planet ring 2
	g_modelMatrix[5] = glm::rotate(orbitAngle[1], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(3.0f, 0.0f, 0.0f));

	//moon inner
	g_modelMatrix[6] = glm::translate(glm::vec3(0.5f, 0.0f, 0.0f))
		*glm::rotate(orbitAngle[1], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(3.0f, 0.0f, 0.0f))
		* glm::scale(glm::vec3(0.2f, 0.2f, 0.2));

	//moon outer
	g_modelMatrix[7] =glm::translate(glm::vec3(1.0f, 0.0f, 0.0f))
		*glm::rotate(orbitAngle[1], glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(3.0f, 0.0f, 0.0f))
		* glm::scale(glm::vec3(0.1f, 0.1f, 0.1));

}

// function used to render the scene
static void render_scene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear colour buffer and depth buffer

	glUseProgram(g_shaderProgramID);	// use the shaders associated with the shader program

	glBindVertexArray(g_VAO);		// make VAO active


	//smaller circle
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_circle_verts), g_circle_verts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_circle_indices), g_circle_indices, GL_STATIC_DRAW);
	glm::mat4 MVP = g_projectionMatrix * g_viewMatrix;
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_LINE_STRIP, NUM_VERTS, GL_UNSIGNED_INT, 0);

	//bigger circle
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_bigger_circle_verts), g_bigger_circle_verts, GL_STATIC_DRAW);
	glDrawElements(GL_LINE_STRIP, NUM_VERTS, GL_UNSIGNED_INT, 0);

	//moon circle inner
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_unit_circle_verts), g_unit_circle_verts, GL_STATIC_DRAW);
	MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[4];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_LINE_STRIP, NUM_VERTS, GL_UNSIGNED_INT, 0);

	//moon circle outer
	MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[5];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_LINE_STRIP, NUM_VERTS, GL_UNSIGNED_INT, 0);

	//moon inner
	MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[6];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertices), g_vertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_indices), g_indices, GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	//moon outer
	MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[7];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


// Object 1 (SUN)


	MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[0];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);

	
	// set uniform model transformation matrix
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type


// Object 2 (inner circle planet)
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertices), g_vertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_indices), g_indices, GL_DYNAMIC_DRAW);
	MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[1];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

	//object 3 (outer circle planet)
	MVP = g_projectionMatrix * g_viewMatrix * g_modelMatrix[3];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type


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
}

// mouse movement callback function
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
}

// mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}

// error callback function
static void error_callback(int error, const char* description)
{
	cerr << description << endl;	// output error description
}

int main(void)
{
	GLFWwindow* window = NULL;	// pointer to a GLFW window handle

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
	window = glfwCreateWindow(800, 600, "Part 2 - Jacob Fiola", NULL, NULL);

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

