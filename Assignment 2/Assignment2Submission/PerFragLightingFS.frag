#version 330 core

// interpolated values from the vertex shaders
in vec3 vPos;
in vec3 vN;

// light and material structs
#define MAX_LIGHTS 2

struct Light
{
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 attenuation;
	int type;
};

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

// uniform input data
uniform mat4 uViewMatrix;
uniform Light uLight[MAX_LIGHTS];
uniform Material uMaterial;

// output data
out vec3 fColor;

vec3 calculateLighting(int lightIndex, vec3 N)
{
    vec3 L;
	float attenuation = 1.0;

	// determine whether the light is a point light source or directional light
	if(uLight[lightIndex].type == 0)
	{
	
		// calculate the attenuation based on distance if it's a point light source
		L = (uViewMatrix * vec4(uLight[lightIndex].position, 1.0f)).xyz - vPos;
		float distance = length(L);
		L = normalize(L);

		attenuation = 1/(uLight[lightIndex].attenuation.x 
			+ uLight[lightIndex].attenuation.y * distance 
			+ uLight[lightIndex].attenuation.z * distance * distance);
	}
	else
		L = normalize((uViewMatrix * vec4(-uLight[lightIndex].direction, 0.0f)).xyz);

    vec3 E = normalize(-vPos);
    vec3 H = normalize(L + E);

	// calculate the ambient, diffuse and specular components
	vec3 ambient  = uLight[lightIndex].ambient * uMaterial.ambient;
    vec3 diffuse  = uLight[lightIndex].diffuse * uMaterial.diffuse * max(dot(L, N), 0.0);
	vec3 specular = vec3(0.0, 0.0, 0.0);

	if(dot(L, N) > 0.0f)
	    specular = uLight[lightIndex].specular * uMaterial.specular * pow(max(dot(N, H), 0.0), uMaterial.shininess);

	// return color
	return ((attenuation * (diffuse + specular)) + ambient);
}

void main()
{
    vec3 N = normalize(vN);
    vec3 colour = vec3(0.0, 0.0, 0.0);

	for(int i = 0; i < MAX_LIGHTS; i++)
	//for(int i = 0; i < 1; i++)
	//for(int i = 1; i < MAX_LIGHTS; i++)
	{
		colour += calculateLighting(i, N);
	}

	// set output color
	fColor = colour;
}