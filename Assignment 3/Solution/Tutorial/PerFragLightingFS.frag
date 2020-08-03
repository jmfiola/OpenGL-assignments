#version 330 core


// interpolated values from the vertex shaders
in vec3 vPos;
in vec3 vN;
in vec2 vTexCoord;
in vec3 vTangent;
in vec3 vColour;



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
	int reflective;
	int textured;
	int blended;
};

// uniform input data
uniform mat4 uViewMatrix;
uniform Light uLight[MAX_LIGHTS];
uniform Material uMaterial;
uniform sampler2D uTextureSampler;
uniform samplerCube uEnvironmentMap;
uniform sampler2D uNormalSampler;
uniform float uAlpha;

// output data
out vec4 fColor;

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
	{
		L = normalize((uViewMatrix * vec4(-uLight[lightIndex].direction, 0.0f)).xyz);
	}
    vec3 E = normalize(-vPos);
    vec3 H = normalize(L + E);

	// calculate the ambient, diffuse and specular components
	vec3 ambient  = uLight[lightIndex].ambient * uMaterial.ambient;
    vec3 diffuse  = uLight[lightIndex].diffuse * uMaterial.diffuse * max(dot(L, N), 0.0);
	vec3 specular = vec3(0.0, 0.0, 0.0);

	if(dot(L, N) > 0.0f)
	    specular = uLight[lightIndex].specular * uMaterial.specular * pow(max(dot(N, H), 0.0), uMaterial.shininess);
	// return color

	if (uMaterial.reflective == 2){
		vec3 reflectEnvMap = reflect(-E, N);
		return (texture(uEnvironmentMap, reflectEnvMap).rgb)*((attenuation * (diffuse + specular)) + ambient);
	}else{
		return ((attenuation * (diffuse + specular)) + ambient);
	}
}

void main()
{
    vec3 N = normalize(vN);
	if(uMaterial.textured == 2){
		vec3 tangent = normalize(vTangent);
		vec3 biTangent = normalize(cross(tangent, N));
		vec3 normalMap = 2.0f * texture(uNormalSampler, vTexCoord).xyz - 1.0f;

		N = normalize(mat3(tangent, biTangent, N) * normalMap);
	}
	
    vec3 colour = vec3(0.0, 0.0, 0.0);

	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		colour += calculateLighting(i, N);
	}

	if(uMaterial.blended == 2){
		fColor = vec4(vColour,uAlpha);
		fColor *= vec4(colour,uAlpha);
	}else{
		fColor = vec4(texture(uTextureSampler, vTexCoord).rgb,1.0f);
		fColor *= vec4(colour,1.0f);
	}

	

}