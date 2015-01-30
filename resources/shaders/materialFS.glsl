#version 440
struct Material
{
	vec4 diffuse;
	vec4 specular;
	vec4 ambient;
	vec4 shininess;
};


out vec4 color;

uniform Material material;

void main()
{    
	//not really shading, is it? I'll do that later.
     //color = 0.40f * material.diffuse + 0.40f * material.specular + 0.10f * material.ambient + 0.10f;
     color = material.diffuse;
     //color = vec4(1.0f,0.0f,0.0f,1.0f);
    
}