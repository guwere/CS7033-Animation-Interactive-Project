#version 440
struct Material
{
	vec4 diffuse;
	vec4 specular;
	vec4 ambient;
	float shininess;
};


in vec2 fsTexCoord;
in vec3 fsNormal;
in vec3 fsPosition; // in world space
in vec3 fsViewPos;

out vec4 color;

uniform vec3 lightPos = vec3(80.0f, 32.0f, -14.0f);
//uniform vec3 lightPos = vec3(0.0f, -2.0f, 0.0f);
vec4 lightAmbient = vec4(0.3f);
vec4 lightDiffuse = vec4(0.7f);
// vec4 lightDiffuse = vec4(1.0f);

uniform Material material;

void main()
{    
    vec4 finalAmbient = material.ambient;

	vec3 norm = normalize(fsNormal);
	vec3 lightDir = normalize(lightPos - fsPosition);

	float diffuse = max(dot(norm, lightDir), 0.0);
    vec4 finalDiffuse = diffuse *  material.diffuse;

    vec3 viewDir = normalize(fsPosition - fsViewPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float specular = pow(max(dot(viewDir,reflectDir), 0.0), material.shininess);
    vec4 finalSpecular = specular * material.specular;

    color = finalAmbient + finalDiffuse + finalSpecular;
    //color = vec4(1.0f,1.0f,0.0f,1.0f);
}