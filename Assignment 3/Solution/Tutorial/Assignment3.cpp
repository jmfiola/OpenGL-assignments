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

#include <AntTweakBar.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "bmpfuncs.h"
#include "shader.h"
#include "Camera.h"

#define MOVEMENT_SENSITIVITY 0.05f		// camera movement sensitivity
#define ROTATION_SENSITIVITY 0.005f		// camera rotation sensitivity
#define ZOOM_SENSITIVITY 0.01f			// camera zoom sensitivity

#define MAX_LIGHTS 2

unsigned char* g_texImage1 = NULL;		//checkbox (floor)
unsigned char* g_texImage2 = NULL;		//smileyFace (pedistal)
unsigned char* g_texImage3[2];		//brickWall (walls)
unsigned char* g_texImage4 = NULL;		//painting 1
unsigned char* g_texImage5 = NULL;		//painting 2
unsigned char* g_texImage6 = NULL;		//painting 2 screenshot
unsigned char* g_texImage[6];			//cube reflection data
GLuint g_textureID;					//cube reflection texture ID
GLuint g_textureID1;					//texture id 1
GLuint g_textureID2;					//texture id 2
GLuint g_textureID3[2];					//texture id 3
GLuint g_textureID4;					//texture id 4
GLuint g_textureID5;					//texture id 5
GLuint g_textureID6;					//texture id 6


enum CUBE_FACE { FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM };
// struct for vertex attributes
typedef struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat tangent[3];
	GLfloat texCoord[2];
	GLfloat colour[3];
} Vertex;

// struct for mesh properties
typedef struct Mesh
{
	Vertex* pMeshVertices;		// pointer to mesh vertices
	GLint numberOfVertices;		// number of vertices in the mesh
	GLint* pMeshIndices;		// pointer to mesh indices
	GLint numberOfFaces;		// number of faces in the mesh
} Mesh;

// light and material structs
typedef struct Light
{
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 attenuation;
	int type;
} Light;



typedef struct Material
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;
	int reflective;
	int textured;
	int blended;
} Material;

GLint g_cube_indices[] = {

	0,1,2, //bottom
	0,3,2, //bottom
	0,1,6, //right face
	0,7,6, //right face
	0,3,4, //back face
	0,7,4, //back face
	1,2,5,//front
	1,6,5, //front
	5,2,3, //left face
	5,4,3, //left face
	5,6,7, //top
	5,4,7 //top
};

GLint g_floor_indices[] = {

	0,1,2, //triangle 1
	2,3,0, //triangle 2

};

GLint g_wall_indices[] = {

	0,1,2,
	0,2,3,
	4,5,6,
	4,6,7,
	8,9,10,
	8,10,11,
	12,13,14,
	12,14,15,
};

GLint g_ceiling_indices[] = {
	4,7,6,
	4,6,5,
};

GLint g_painting_indices[] = {
	0,1,2,
	1,2,3,
};

Vertex g_floor_verts[] = {
	1.0f,-1.0f,-1.0f,			//position 0
	-1.0f,1.0f,1.0f,			//normal vector 0
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	6.0f,6.0f,					//texture coord 0
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,-1.0f,1.0f,			//position 1
	-1.0f,1.0f,-1.0f,			//normal vector 1
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	6.0f,0.0f,					//texture coord 1
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,-1.0f,1.0f,			//position 2
	1.0f,1.0f,-1.0f,			//normal vector 2
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	0.0f,0.0f,					//texture coord 2
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,-1.0f,-1.0f,			//position 3
	1.0f,1.0f,1.0f,			//normal vector 3
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	0.0f,6.0f,					//texture coord 3
	1.0f,1.0f,1.0f,				//placeholder color vector
};
Vertex g_room_verts[] = {
	1.0f,-1.0f,-1.0f,			//position 0
	0.0f,0.0f,1.0f,			//normal vector 0
	1.0f,0.0f,0.0f,					//placeholder tangent vector
	3.0f,0.0f,					//texture coord 0
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,1.0f,-1.0f,			//position 1
	0.0f,0.0f,1.0f,			//normal vector 1
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	3.0f,3.0f,					//texture coord 1
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,1.0f,-1.0f,			//position 2
	0.0f,0.0f,1.0f,			//normal vector 2
	1.0f,0.0f,0.0f,			//placeholder tangent vector
	0.0f,3.0f,					//texture coord 2
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,-1.0f,-1.0f,			//position 3
	0.0f,0.0f,1.0f,			//normal vector 3
	1.0f,0.0f,0.0f,					//placeholder tangent vector
	0.0f,0.0f,					//texture coord 3
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,-1.0f,1.0f,			//position 4
	-1.0f,0.0f,0.0f,			//normal vector 4
	0.0f,0.0f,1.0f,				//placeholder tangent vector
	3.0f,0.0f,					//texture coord 4
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,1.0f,1.0f,			//position 5
	-1.0f,0.0f,0.0f,			//normal vector 5
	0.0f,0.0f,1.0f,					//placeholder tangent vector
	3.0f,3.0f,					//texture coord 5
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,1.0f,-1.0f,			//position 6
	-1.0f,0.0f,0.0f,			//normal vector 6
	0.0f,0.0f,1.0f,				//placeholder tangent vector
	0.0f,3.0f,					//texture coord 6
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,-1.0f,-1.0f,			//position 7
	-1.0f,0.0f,0.0f,			//normal vector 7
	0.0f,0.0f,1.0f,				//placeholder tangent vector
	0.0f,0.0f,					//texture coord 7
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,-1.0f,-1.0f,			//position 8
	1.0f,0.0f,0.0f,			//normal vector 8
	0.0f,0.0f,-1.0f,				//tangent vector 8
	3.0f,0.0f,					//texture coord 8
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,1.0f,-1.0f,			//position 9
	1.0f,0.0f,0.0f,			//normal vector 9
	0.0f,0.0f,-1.0f,				//tangent vector 9
	3.0f,3.0f,					//texture coord 9
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,1.0f,1.0f,			//position 10
	1.0f,0.0f,0.0f,			//normal vector 10
	0.0f,0.0f,-1.0f,				//tangent vector 10
	0.0f,3.0f,					//texture coord 10
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,-1.0f,1.0f,			//position 11
	1.0f,0.0f,0.0f,			//normal vector 11
	0.0f,0.0f,-1.0f,				//tangent vector 11
	0.0f,0.0f,					//texture coord 11
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,-1.0f,1.0f,			//position 12
	0.0f,0.0f,-1.0f,			//normal vector 12
	-1.0f,0.0f,0.0f,				//tangent vector 12
	3.0f,0.0f,					//texture coord 12
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,1.0f,1.0f,			//position 13
	0.0f,0.0f,-1.0f,			//normal vector 13
	-1.0f,0.0f,0.0f,				//tangent vector 13
	3.0f,3.0f,					//texture coord 13
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,1.0f,1.0f,			//position 14
	0.0f,0.0f,-1.0f,			//normal vector 14
	-1.0f,0.0f,0.0f,				//tangent vector 14
	0.0f,3.0f,					//texture coord 14
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,-1.0f,1.0f,			//position 15
	0.0f,0.0f,-1.0f,			//normal vector 15
	-1.0f,0.0f,0.0f,				//tangent vector 15
	0.0f,0.0f,					//texture coord 15
	1.0f,1.0f,1.0f,				//placeholder color vector



				
};

Vertex g_cube_verts[] = {
	1.0f,-1.0f,-1.0f,			//position 0
	1.0f,-1.0f,-1.0f,			//normal vector 0
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	0.0f,0.0f,					//texture coord 0
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,-1.0f,1.0f,			//position 1
	1.0f,-1.0f,1.0f,			//normal vector 1
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	2.0f,0.0f,					//texture coord 1
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,-1.0f,1.0f,			//position 2
	-1.0f,-1.0f,1.0f,			//normal vector 2
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	0.0f,0.0f,					//texture coord 2
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,-1.0f,-1.0f,			//position 3
	-1.0f,-1.0f,-1.0f,			//normal vector 3
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	2.0f,0.0f,					//texture coord 3
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,1.0f,-1.0f,			//position 4
	-1.0f,1.0f,-1.0f,			//normal vector 4
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	2.0f,2.0f,					//texture coord 4
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,1.0f,1.0f,			//position 5
	-1.0f,-1.0f,1.0f,			//normal vector 5
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	0.0f,2.0f,					//texture coord 5
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,1.0f,1.0f,			//position 6
	1.0f,1.0f,1.0f,			//normal vector 6
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	2.0f,2.0f,					//texture coord 6
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,1.0f,-1.0f,			//position 7
	1.0f,1.0f,-1.0f,			//normal vector 7
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	0.0f,2.0f,					//texture coord 7
	1.0f,1.0f,1.0f,				//placeholder color vector

};

Vertex g_painting_verts[] = {
	-1.0f,-0.5f,0.0f,			//position 0
	0.0f,0.0f,1.0f,			//normal vector 0
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	0.0f,0.0f,					//texture coord 0
	0.0f,0.0f,0.0f,				//placeholder color vector

	1.0f,-0.5f,0.0f,			//position 1
	0.0f,0.0f,1.0f,			//normal vector 1
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	1.0f,0.0f,					//texture coord 1
	1.0f,1.0f,1.0f,				//placeholder color vector

	-1.0f,0.5f,0.0f,			//position 2
	0.0f,0.0f,1.0f,			//normal vector 2
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	0.0f,1.0f,					//texture coord 2
	1.0f,1.0f,1.0f,				//placeholder color vector

	1.0f,0.5f,0.0f,			//position 3
	0.0f,0.0f,1.0f,			//normal vector 3
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	1.0f,1.0f,					//texture coord 3
	1.0f,1.0f,1.0f,				//placeholder color vector

};

float g_glass_r = 8.0f;
float g_glass_g = 0.0f;
float g_glass_b = 8.0f;

Vertex g_glass_verts[] = {

	-1.0f,-1.0f,0.0f,			//position 0
	0.0f,0.0f,1.0f,			//normal vector 0
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	0.0f,0.0f,					//texture coord 0
	g_glass_r,g_glass_g,g_glass_b,				//placeholder color vector

	1.0f,-1.0f,0.0f,			//position 1
	0.0f,0.0f,1.0f,			//normal vector 1
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	1.0f,0.0f,					//texture coord 1
	g_glass_r,g_glass_g,g_glass_b,			//placeholder color vector

	-1.0f,1.0f,0.0f,			//position 2
	0.0f,0.0f,1.0f,			//normal vector 2
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	0.0f,1.0f,					//texture coord 2
	g_glass_r,g_glass_g,g_glass_b,				//placeholder color vector

	1.0f,1.0f,0.0f,			//position 3
	0.0f,0.0f,1.0f,			//normal vector 3
	1.0f,0.0f,0.0f,				//placeholder tangent vector
	1.0f,1.0f,					//texture coord 3
	g_glass_r,g_glass_g,g_glass_b,			//placeholder color vector

};

Camera g_camera;
// Global variables
#define NUM_MESHES 3
Mesh g_mesh[NUM_MESHES];					// mesh

//[0] = Sphere
//[1] = cube

GLuint g_IBO = 0;				// index buffer object identifier
GLuint g_VBO =  0;				// vertex buffer object identifier
GLuint g_VAO =  0;				// vertex array object identifier
GLuint g_shaderProgramID = 0;	// shader program identifier

// locations in shader
GLuint g_MVP_Index = 0;
GLuint g_MV_Index = 0;
GLuint g_V_Index = 0;
GLuint g_texSampler_Index = 0;
GLuint g_envMapSamplerIndex;
GLuint g_alphaIndex;
GLuint g_lightPositionIndex[MAX_LIGHTS];
GLuint g_lightDirectionIndex[MAX_LIGHTS];
GLuint g_lightAmbientIndex[MAX_LIGHTS];
GLuint g_lightDiffuseIndex[MAX_LIGHTS];
GLuint g_lightSpecularIndex[MAX_LIGHTS];
GLuint g_lightTypeIndex[MAX_LIGHTS];
GLuint g_lightAttenuationIndex[MAX_LIGHTS];
GLuint g_materialAmbientIndex = 0;
GLuint g_materialDiffuseIndex = 0;
GLuint g_materialSpecularIndex = 0;
GLuint g_materialShininessIndex = 0;
GLuint g_materialReflectiveIndex = 0;
GLuint g_materialTexturedIndex = 0;
GLuint g_materialBlendedIndex = 0;

float g_alpha = 0.5f;


#define NUM_MODEL_MATRICES 9
glm::mat4 g_modelMatrix[NUM_MODEL_MATRICES];		// object's model matrix

//glm::mat4 g_viewMatrix;			// view matrix
//glm::mat4 g_projectionMatrix;	// projection matrix

#define NUM_LIGHTS 2
Light g_light[NUM_LIGHTS];					// light properties

#define NUM_MATERIALS 6
Material g_material[NUM_MATERIALS];			// material properties

GLuint g_windowWidth = 1200;		// window dimensions
GLuint g_windowHeight = 600;
bool g_wireFrame = false;		// wireframe on or off
bool g_light1On = true;
bool g_light2On = true;

float g_scale = 0.3f;
float g_rotateAngle = 0.0f;
float g_rotateSpeed = 2.0f;
float g_light_x = 0.0f;
float g_light_y = -0.4f;
float g_light_z = 0.0f;



bool load_mesh(const char* fileName, Mesh* mesh);

static void init(GLFWwindow* window)
{
	glEnable(GL_DEPTH_TEST);	// enable depth buffer test
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	// create and compile our GLSL program from the shader files
	g_shaderProgramID = loadShaders("PerFragLightingVS.vert", "PerFragLightingFS.frag");

	// find the location of shader variables
	GLuint positionIndex = glGetAttribLocation(g_shaderProgramID, "aPosition");
	GLuint normalIndex = glGetAttribLocation(g_shaderProgramID, "aNormal");
	GLuint tangentIndex = glGetAttribLocation(g_shaderProgramID, "aTangent");
	GLuint texCoordIndex = glGetAttribLocation(g_shaderProgramID, "aTexCoord");
	GLuint colourIndex = glGetAttribLocation(g_shaderProgramID, "aColour");
	g_MVP_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewProjectionMatrix");
	g_MV_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewMatrix");
	g_V_Index = glGetUniformLocation(g_shaderProgramID, "uViewMatrix");
	g_alphaIndex = glGetUniformLocation(g_shaderProgramID, "uAlpha");

	g_lightPositionIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].position");
	g_lightDirectionIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].direction");
	g_lightAmbientIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].ambient");
	g_lightDiffuseIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].diffuse");
	g_lightSpecularIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].specular");
	g_lightAttenuationIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].attenuation");
	g_lightTypeIndex[0] = glGetUniformLocation(g_shaderProgramID, "uLight[0].type");

	g_lightPositionIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].position");
	g_lightDirectionIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].direction");
	g_lightAmbientIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].ambient");
	g_lightDiffuseIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].diffuse");
	g_lightSpecularIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].specular");
	g_lightAttenuationIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].attenuation");
	g_lightTypeIndex[1] = glGetUniformLocation(g_shaderProgramID, "uLight[1].type");

	g_materialAmbientIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.ambient");
	g_materialDiffuseIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.diffuse");
	g_materialSpecularIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.specular");
	g_materialShininessIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.shininess");
	g_materialReflectiveIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.reflective");
	g_materialTexturedIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.textured");
	g_materialBlendedIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.blended");

	g_texSampler_Index = glGetUniformLocation(g_shaderProgramID, "uTextureSampler");
	// initialise model matrix to the identity matrix
	for (int i = 0; i < NUM_MODEL_MATRICES; i++)
	{
		g_modelMatrix[i] = glm::mat4(1.0f);
	}

	//g_camera.setViewMatrix(glm::vec3(0.0f, -2.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	//initial sphere locations


	// initialise view matrix
	//g_viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.6f, 5.0f), glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspectRatio = static_cast<float>(width) / height;

	// initialise projection matrix
	//g_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	g_camera.setProjection(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

	// load mesh
	g_mesh[0].pMeshVertices = NULL;
	g_mesh[0].pMeshIndices = NULL;
	load_mesh("models/sphere.obj", &g_mesh[0]);

	g_mesh[1].pMeshVertices = NULL;
	g_mesh[1].pMeshIndices = NULL;
	load_mesh("models/suzanne.obj", &g_mesh[1]);

	g_mesh[2].pMeshVertices = NULL;
	g_mesh[2].pMeshIndices = NULL;
	load_mesh("models/cylinder.obj", &g_mesh[2]);

	// initialise point light 1 properties
	g_light[0].position = glm::vec3(g_light_x, g_light_y, g_light_z);
	g_light[0].ambient = glm::vec3(1.0f, 1.0f, 1.0f);
	g_light[0].diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	g_light[0].specular = glm::vec3(1.0f, 1.0f, 1.0f);
	g_light[0].attenuation = glm::vec3(0.01f, 0.01f, 0.027f);
	g_light[0].type = 0;

	// initialise point light 2 properties
	g_light[1].position = glm::vec3(0.0f, 0.0f, 0.0f);
	g_light[1].ambient = glm::vec3(0.0f, 0.0f, 0.0f);
	g_light[1].diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
	g_light[1].specular = glm::vec3(0.0f, 0.0f, 0.0f);
	g_light[1].attenuation = glm::vec3(0.01f, 0.01f, 0.025f);
	g_light[1].type = 0;

	// initialise material 0 properties
	g_material[0].ambient = glm::vec3(1.0f, 1.0f, 1.0f);
	g_material[0].diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	g_material[0].specular = glm::vec3(1.0f, 1.0f, 1.0f);
	g_material[0].shininess = 1000.0f;
	g_material[0].reflective = 1;
	g_material[0].textured = 1;
	g_material[0].blended = 1;

	//initialise material 1 properties
	g_material[1].ambient = glm::vec3(1.0f, 1.0f, 1.0f);
	g_material[1].diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	g_material[1].specular = glm::vec3(1.0f, 1.0f, 1.0f);
	g_material[1].shininess = 20.0f;
	g_material[1].reflective = 1;
	g_material[1].textured = 1;
	g_material[1].blended = 1;

	//initialise material 2 properties
	g_material[2].ambient = glm::vec3(0.3f, 0.4f, 0.6f);
	g_material[2].diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
	g_material[2].specular = glm::vec3(0.5f, 0.5f, 0.5f);
	g_material[2].shininess = 100.0f;
	g_material[2].reflective = 1;
	g_material[2].textured = 2;
	g_material[2].blended = 1;

	//initialise material 3 properties
	g_material[3].ambient = glm::vec3(0.8f, 0.8f, 0.8f);
	g_material[3].diffuse = glm::vec3(0.3f, 0.5f, 0.3f);
	g_material[3].specular = glm::vec3(0.8f, 0.8f, 0.8f);
	g_material[3].shininess = 90.0f;
	g_material[3].reflective = 2;
	g_material[3].textured = 1;
	g_material[3].blended = 1;

	//initialise material 4 properties
	g_material[4].ambient = glm::vec3(0.0f, 0.8f, 0.5f);
	g_material[4].diffuse = glm::vec3(0.1f, 0.1f, 0.1f);
	g_material[4].specular = glm::vec3(0.1f, 0.1f, 0.1f);
	g_material[4].shininess = 5.0f;
	g_material[4].reflective = 1;
	g_material[4].textured = 1;
	g_material[4].blended = 2;

	//initialise material 5 properties
	g_material[5].ambient = glm::vec3(0.8f, 0.8f, 0.2f);
	g_material[5].diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	g_material[5].specular = glm::vec3(0.5f, 0.5f, 0.5f);
	g_material[5].shininess = 100.0f;
	g_material[5].reflective = 1;
	g_material[5].textured = 1;
	g_material[5].blended = 1;

	//read the image data for texture 1
	GLint image1Width;			//image 1 width info
	GLint image1Height;			//image 1 height info
	GLint image2Width;			//image 2 width info
	GLint image2Height;			//image 2 height info
	GLint image3Width[2];			//image 3 width info
	GLint image3Height[2];			//image 3 height info
	GLint image4Width;			//image 4 width info
	GLint image4Height;			//image 4 height info
	GLint image5Width;			//image 5 width info
	GLint image5Height;			//image 5 height info
	GLint image6Width;			//image 6 width info
	GLint image6Height;			//image 6 height info
	GLint cameraWidth;
	GLint cameraHeight;

	g_texImage1 = readBitmapRGBImage("images/check.bmp", &image1Width, &image1Height);
	g_texImage2 = readBitmapRGBImage("images/smile.bmp", &image2Width, &image2Height);
	g_texImage3[0] = readBitmapRGBImage("images/Fieldstone.bmp", &image3Width[0], &image3Height[0]);
	g_texImage3[1] = readBitmapRGBImage("images/FieldstoneBumpDOT3.bmp", &image3Width[1], &image3Height[1]);
	g_texImage4 = readBitmapRGBImage("images/painting1.bmp", &image4Width, &image4Height);
	g_texImage5 = readBitmapRGBImage("images/painting2.bmp", &image5Width, &image5Height);

	g_texImage6 = readBitmapRGBImage("images/painting2.bmp", &image6Width, &image6Height); 
	//^^^^^placeHolder for screenshot

	g_texImage[FRONT] = readBitmapRGBImage("images/cm_front.bmp", &cameraWidth, &cameraHeight);
	g_texImage[BACK] = readBitmapRGBImage("images/cm_back.bmp", &cameraWidth, &cameraHeight);
	g_texImage[LEFT] = readBitmapRGBImage("images/cm_left.bmp", &cameraWidth, &cameraHeight);
	g_texImage[RIGHT] = readBitmapRGBImage("images/cm_right.bmp", &cameraWidth, &cameraHeight);
	g_texImage[TOP] = readBitmapRGBImage("images/cm_top.bmp", &cameraWidth, &cameraHeight);
	g_texImage[BOTTOM] = readBitmapRGBImage("images/cm_bottom.bmp", &cameraWidth, &cameraHeight);


	// generate identifier for texture object and set texture properties
	glGenTextures(1, &g_textureID1);
	glGenTextures(1, &g_textureID2);
	glGenTextures(2, g_textureID3);
	glGenTextures(1, &g_textureID4);
	glGenTextures(1, &g_textureID5);
	glGenTextures(1, &g_textureID6);
	glGenTextures(1, &g_textureID);


	//texture 1 stuff
	glBindTexture(GL_TEXTURE_2D, g_textureID1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image1Width, image1Height, 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage1);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//texture 2 stuff
	glBindTexture(GL_TEXTURE_2D, g_textureID2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image2Width, image2Height, 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage2);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//texture 3 stuff
	glBindTexture(GL_TEXTURE_2D, g_textureID3[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image3Width[0], image3Height[0], 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage3[0]);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, g_textureID3[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image3Width[1], image3Height[1], 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage3[1]);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//texture 4 stuff
	glBindTexture(GL_TEXTURE_2D, g_textureID4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image4Width, image4Height, 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage4);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//texture 5 stuff
	glBindTexture(GL_TEXTURE_2D, g_textureID5);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image5Width, image5Height, 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage5);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//texture 6 stuff
	glBindTexture(GL_TEXTURE_2D, g_textureID6);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image6Width, image6Height, 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage6);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//cube map texture stuff
	glBindTexture(GL_TEXTURE_CUBE_MAP, g_textureID);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, cameraWidth, cameraHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage[RIGHT]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, cameraWidth, cameraHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage[LEFT]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, cameraWidth, cameraHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage[TOP]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, cameraWidth, cameraHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage[BOTTOM]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, cameraWidth, cameraHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage[BACK]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, cameraWidth, cameraHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, g_texImage[FRONT]);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	g_envMapSamplerIndex = glGetUniformLocation(g_shaderProgramID, "uEnvironmentMap");
	// generate identifier for VBOs and copy data to GPU
	glGenBuffers(1, &g_VBO);

	// generate identifier for IBO and copy data to GPU
	glGenBuffers(1, &g_IBO);
	
	// generate identifiers for VAO
	glGenVertexArrays(1, &g_VAO);


	// create VAO and specify VBO data
	glBindVertexArray(g_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
	glVertexAttribPointer(tangentIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tangent)));
	glVertexAttribPointer(texCoordIndex, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoord)));
	glVertexAttribPointer(colourIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, colour)));

	// enable vertex attributes
	glEnableVertexAttribArray(positionIndex);	
	glEnableVertexAttribArray(normalIndex);
	glEnableVertexAttribArray(tangentIndex);
	glEnableVertexAttribArray(texCoordIndex);
	glEnableVertexAttribArray(colourIndex);
}

// function used to update the scene
static void update_scene(GLFWwindow* window)
{

	// variables to store forward/back and strafe movement
	float moveForward = 0;
	float strafeRight = 0;
	float zoom = 0;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			moveForward += 1 * MOVEMENT_SENSITIVITY;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			moveForward -= 1 * MOVEMENT_SENSITIVITY;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			strafeRight -= 1 * MOVEMENT_SENSITIVITY;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			strafeRight += 1 * MOVEMENT_SENSITIVITY;

		// update zoom based on keyboard input
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			zoom -= 1 * ZOOM_SENSITIVITY;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			zoom += 1 * ZOOM_SENSITIVITY;

		if (zoom > 0 || zoom < 0)
			g_camera.updateFOV(zoom);				// change camera FOV

		g_camera.update(moveForward, strafeRight);
		
	

	g_light[0].position = glm::vec3(g_light_x, g_light_y, g_light_z);
	g_modelMatrix[1] = //room
		glm::translate(glm::vec3(0.0f,0.0f,0.0f))
		*glm::scale(glm::vec3(4.0f, 4.0f, 4.0f));
	
	g_modelMatrix[2] = //table
		glm::translate(glm::vec3(0.0f,-3.5f,0.0f))
		*glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));

	g_modelMatrix[3] = //suzanne
		glm::translate(glm::vec3(0.0f, -2.6f, 0.0f))
		*glm::scale(glm::vec3(0.3f, 0.3f, 0.3f))
		*glm::rotate(glm::radians(g_rotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	g_rotateAngle += g_rotateSpeed;


	g_modelMatrix[4] = //painting 1
		
		glm::translate(glm::vec3(0.0f, -1.0f, -3.95f))
		*glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));

	g_modelMatrix[5] = //painting 2
		
		glm::rotate(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))
		*glm::translate(glm::vec3(0.0f, -1.0f, -3.95f))
		*glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));

	g_modelMatrix[6] = //glass
		glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))
		*glm::translate(glm::vec3(0.0f, -1.7f, -2.5f))
		*glm::scale(glm::vec3(0.8f, 0.8f, 0.8f));

	g_modelMatrix[7] = //light 1z
		glm::translate(glm::vec3(g_light_x, g_light_y, g_light_z))
		*glm::scale(glm::vec3(0.1f, 0.1f, 0.1f));

	if (g_light1On) {
		g_light[0].ambient = glm::vec3(0.6f, 0.6f, 0.6f);
		g_light[0].diffuse = glm::vec3(0.3f, 0.6f, 0.6f);
		g_light[0].specular = glm::vec3(0.0f, 0.5f, 1.0f);
	}
	else {
		g_light[0].ambient = glm::vec3(0.0f, 0.0f, 0.0f);
		g_light[0].diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
		g_light[0].specular = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	for (int i = 0; i < 4; i++) {
		g_glass_verts[i].colour[0] = g_glass_r;
		g_glass_verts[i].colour[1] = g_glass_g;
		g_glass_verts[i].colour[2] = g_glass_b;
	}

}

// function used to render the scene
static void render_scene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear colour buffer and depth buffer

	glUseProgram(g_shaderProgramID);	// use the shaders associated with the shader program

	glBindVertexArray(g_VAO);		// make VAO active
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*g_mesh[0].numberOfVertices, g_mesh[0].pMeshVertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 3 * g_mesh[0].numberOfFaces, g_mesh[0].pMeshIndices, GL_STATIC_DRAW);
	

	glUniform1i(g_texSampler_Index, 0);
	

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_textureID1);


	glUniform3fv(g_materialAmbientIndex, 1, &g_material[0].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &g_material[0].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &g_material[0].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &g_material[0].shininess);
	glUniform1iv(g_materialReflectiveIndex, 1, &g_material[0].reflective);
	glUniform1iv(g_materialBlendedIndex, 1, &g_material[0].textured);
	glUniform1fv(g_alphaIndex, 1, &g_alpha);


	//
	//glDrawElements(GL_TRIANGLES, g_mesh[0].numberOfFaces * 3, GL_UNSIGNED_INT, 0);	// display the vertices based on their indices and primitive type

	//render light 1
	glm::mat4 MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[7];
	glm::mat4 MV = g_camera.getViewMatrix() * g_modelMatrix[7];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_camera.getViewMatrix()[0][0]);
	glDrawElements(GL_TRIANGLES, g_mesh[0].numberOfFaces * 3, GL_UNSIGNED_INT, 0);

	
	
	//room rendering
	glUniform3fv(g_materialAmbientIndex, 1, &g_material[1].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &g_material[1].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &g_material[1].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &g_material[1].shininess);
	glUniform1iv(g_materialReflectiveIndex, 1, &g_material[1].reflective);
	glUniform1iv(g_materialTexturedIndex, 1, &g_material[1].textured);
	glUniform1iv(g_materialBlendedIndex, 1, &g_material[1].blended);
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[1];
	MV = g_camera.getViewMatrix() * g_modelMatrix[1];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
		//floor rendering

		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*8, g_floor_verts, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_floor_indices), g_floor_indices, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		
		//wall rendering
		glBindTexture(GL_TEXTURE_2D, g_textureID3[0]);
		glUniform3fv(g_materialAmbientIndex, 1, &g_material[2].ambient[0]);
		glUniform3fv(g_materialDiffuseIndex, 1, &g_material[2].diffuse[0]);
		glUniform3fv(g_materialSpecularIndex, 1, &g_material[2].specular[0]);
		glUniform1fv(g_materialShininessIndex, 1, &g_material[2].shininess);
		glUniform1iv(g_materialReflectiveIndex, 1, &g_material[2].reflective);
		glUniform1iv(g_materialTexturedIndex, 1, &g_material[2].textured);
		glUniform1iv(g_materialBlendedIndex, 1, &g_material[2].blended);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 20, g_room_verts, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_wall_indices), g_wall_indices, GL_STATIC_DRAW);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, g_textureID3[1]);
		
		glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
		
		//ceiling rendering


	//table rendering
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_textureID2);
	glUniform3fv(g_materialAmbientIndex, 1, &g_material[5].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &g_material[5].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &g_material[5].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &g_material[5].shininess);
	glUniform1iv(g_materialReflectiveIndex, 1, &g_material[5].reflective);
	glUniform1iv(g_materialTexturedIndex, 1, &g_material[5].textured);
	glUniform1iv(g_materialBlendedIndex, 1, &g_material[5].blended);

	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[2];
	MV = g_camera.getViewMatrix() * g_modelMatrix[2];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 8, g_cube_verts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_cube_indices), g_cube_indices, GL_STATIC_DRAW);
	glDrawElements(GL_TRIANGLES, 50, GL_UNSIGNED_INT, 0);

	//painting 1 rendering

	glBindTexture(GL_TEXTURE_2D, g_textureID4);

	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[4];
	MV = g_camera.getViewMatrix() * g_modelMatrix[4];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 12, g_painting_verts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_painting_indices), g_painting_indices, GL_STATIC_DRAW);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//painting 2 rendering
	glBindTexture(GL_TEXTURE_2D, g_textureID5);

	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[5];
	MV = g_camera.getViewMatrix() * g_modelMatrix[5];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 12, g_painting_verts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_painting_indices), g_painting_indices, GL_STATIC_DRAW);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


	//glBindTexture(GL_TEXTURE_CUBE_MAP, g_textureID);
	//suzanne rendering
	//glUniform1i(g_envMapSamplerIndex, 0);
	glUniform3fv(g_materialAmbientIndex, 1, &g_material[3].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &g_material[3].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &g_material[3].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &g_material[3].shininess);
	glUniform1iv(g_materialReflectiveIndex, 1, &g_material[3].reflective);
	glUniform1iv(g_materialTexturedIndex, 1, &g_material[3].textured);
	glUniform1iv(g_materialBlendedIndex, 1, &g_material[3].blended);
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[3];
	MV = g_camera.getViewMatrix() * g_modelMatrix[3];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*g_mesh[1].numberOfVertices, g_mesh[1].pMeshVertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 3 * g_mesh[1].numberOfFaces, g_mesh[1].pMeshIndices, GL_STATIC_DRAW);
	glDrawElements(GL_TRIANGLES, g_mesh[1].numberOfFaces*3, GL_UNSIGNED_INT, 0);

	//glass rendering
	glUniform3fv(g_materialAmbientIndex, 1, &g_material[4].ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &g_material[4].diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &g_material[4].specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &g_material[4].shininess);
	glUniform1iv(g_materialReflectiveIndex, 1, &g_material[4].reflective);
	glUniform1iv(g_materialTexturedIndex, 1, &g_material[4].textured);
	glUniform1iv(g_materialBlendedIndex, 1, &g_material[4].blended);
	MVP = g_camera.getProjectionMatrix() * g_camera.getViewMatrix() * g_modelMatrix[6];
	MV = g_camera.getViewMatrix() * g_modelMatrix[6];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 12, g_glass_verts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_painting_indices), g_painting_indices, GL_STATIC_DRAW);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUniform1iv(g_materialBlendedIndex, 1, &g_material[5].blended);


	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glUniform3fv(g_lightPositionIndex[i], 1, &g_light[i].position[0]);
		glUniform3fv(g_lightAmbientIndex[i], 1, &g_light[i].ambient[0]);
		glUniform3fv(g_lightDiffuseIndex[i], 1, &g_light[i].diffuse[0]);
		glUniform3fv(g_lightSpecularIndex[i], 1, &g_light[i].specular[0]);
		glUniform3fv(g_lightAttenuationIndex[i], 1, &g_light[i].attenuation[0]);
		glUniform1i(g_lightTypeIndex[i], g_light[i].type);
	}
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

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	
	// variables to store mouse cursor coordinates
	static double previous_xpos = xpos;
	static double previous_ypos = ypos;
	double delta_x = previous_xpos - xpos;
	double delta_y = previous_ypos - ypos;

	// pass mouse data to tweak bar
	TwEventMousePosGLFW(xpos, ypos);
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
	
		// pass mouse movement to camera class to update its yaw and pitch
		g_camera.updateRotation(delta_x * ROTATION_SENSITIVITY, delta_y * ROTATION_SENSITIVITY);

		// update previous mouse coordinates

	}
	previous_xpos = xpos;
	previous_ypos = ypos;
	
}

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
	window = glfwCreateWindow(g_windowWidth, g_windowHeight, "CSCI 336 Assignment 3 - Jacob Fiola", NULL, NULL);

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

	// initialise AntTweakBar
	TwInit(TW_OPENGL_CORE, NULL);

	// give tweak bar the size of graphics window
	TwWindowSize(g_windowWidth, g_windowHeight);
	TwDefine(" TW_HELP visible=false ");	// disable help menu
	TwDefine(" GLOBAL fontsize=3 ");		// set large font size

	// create a tweak bar
	TweakBar = TwNewBar("Main");
	TwDefine(" Main label='Controls' refresh=0.02 text=light size='250 300' ");

	// create display entries
	TwAddVarRW(TweakBar, "Wireframe", TW_TYPE_BOOLCPP, &g_wireFrame, " group='Display' ");

	// display a separator
	TwAddSeparator(TweakBar, NULL, NULL);


	// create attenuation entries
	TwAddVarRW(TweakBar, "X", TW_TYPE_FLOAT, &g_light_x, " group='Light Position' min=-2.0 max=2.0 step=0.01 ");
	TwAddVarRW(TweakBar, "Y", TW_TYPE_FLOAT, &g_light_y, " group='Light Position' min=-2.0 max=2.0 step=0.01 ");
	TwAddVarRW(TweakBar, "Z", TW_TYPE_FLOAT, &g_light_z, " group='Light Position' min=-2.0 max=2.0 step=0.01 ");

	
	// display a separator

	TwAddSeparator(TweakBar, NULL, NULL);
	TwAddVarRW(TweakBar, "Alpha value", TW_TYPE_FLOAT, &g_alpha, " group='Glass Pane' min=0.0 max=1.0 step=0.01 ");
	TwAddVarRW(TweakBar, "R", TW_TYPE_FLOAT, &g_glass_r, " group='Glass Pane' min=0.0 max=12.0 step=1.00 ");
	TwAddVarRW(TweakBar, "G", TW_TYPE_FLOAT, &g_glass_g, " group='Glass Pane' min=0.0 max=12.0 step=1.00 ");
	TwAddVarRW(TweakBar, "B", TW_TYPE_FLOAT, &g_glass_b, " group='Glass Pane' min=0.0 max=12.0 step=1.00 ");
	
	// initialise rendering states
	init(window);

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		update_scene(window);		// update the scene

		if (g_wireFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		render_scene();		// render the scene

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		TwDraw();			// draw tweak bar(s)

		glfwSwapBuffers(window);	// swap buffers
		glfwPollEvents();			// poll for events
	}

	// clean up
	for (int i = 0; i < NUM_MESHES; i++) {
		if (g_mesh[i].pMeshVertices)
			delete[] g_mesh[i].pMeshVertices;
		if (g_mesh[i].pMeshIndices)
			delete[] g_mesh[i].pMeshIndices;
	}
	glDeleteProgram(g_shaderProgramID);
	glDeleteBuffers(1, &g_IBO);
	glDeleteBuffers(1, &g_VBO);
	glDeleteVertexArrays(1, &g_VAO);

	// uninitialise tweak bar
	TwTerminate();

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

bool load_mesh(const char* fileName, Mesh* mesh)
{
	// load file with assimp 
	const aiScene* pScene = aiImportFile(fileName, aiProcess_Triangulate
		| aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

	// check whether scene was loaded
	if (!pScene)
	{
		cout << "Could not load mesh." << endl;
		return false;
	}

	// get pointer to mesh 0
	const aiMesh* pMesh = pScene->mMeshes[0];

	// store number of mesh vertices
	mesh->numberOfVertices = pMesh->mNumVertices;

	// if mesh contains vertex coordinates
	if (pMesh->HasPositions())
	{
		// allocate memory for vertices
		mesh->pMeshVertices = new Vertex[pMesh->mNumVertices];

		// read vertex coordinates and store in the array
		for (int i = 0; i < pMesh->mNumVertices; i++)
		{
			const aiVector3D* pVertexPos = &(pMesh->mVertices[i]);

			mesh->pMeshVertices[i].position[0] = (GLfloat)pVertexPos->x;
			mesh->pMeshVertices[i].position[1] = (GLfloat)pVertexPos->y;
			mesh->pMeshVertices[i].position[2] = (GLfloat)pVertexPos->z;
		}
	}

	// if mesh contains normals
	if (pMesh->HasNormals())
	{
		// read normals and store in the array
		for (int i = 0; i < pMesh->mNumVertices; i++)
		{
			const aiVector3D* pVertexNormal = &(pMesh->mNormals[i]);

			mesh->pMeshVertices[i].normal[0] = (GLfloat)pVertexNormal->x;
			mesh->pMeshVertices[i].normal[1] = (GLfloat)pVertexNormal->y;
			mesh->pMeshVertices[i].normal[2] = (GLfloat)pVertexNormal->z;
		}
	}

	// if mesh contains faces
	if (pMesh->HasFaces())
	{
		// store number of mesh faces
		mesh->numberOfFaces = pMesh->mNumFaces;

		// allocate memory for vertices
		mesh->pMeshIndices = new GLint[pMesh->mNumFaces * 3];

		// read normals and store in the array
		for (int i = 0; i < pMesh->mNumFaces; i++)
		{
			const aiFace* pFace = &(pMesh->mFaces[i]);

			mesh->pMeshIndices[i * 3] = (GLint)pFace->mIndices[0];
			mesh->pMeshIndices[i * 3 + 1] = (GLint)pFace->mIndices[1];
			mesh->pMeshIndices[i * 3 + 2] = (GLint)pFace->mIndices[2];
		}
	}

	// release the scene
	aiReleaseImport(pScene);

	return true;
}