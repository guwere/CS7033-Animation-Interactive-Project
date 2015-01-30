#version 440

in vec2 fsTexCoord;
in vec3 fsNormal;
in vec3 fsPosition; // in world space

out vec4 color;

uniform sampler2D texture_diffuse1;
//uniform vec3 lightPos = vec3(100.0f, 100.0f, 100.0f);
uniform vec3 lightPos = vec3(80.0f, 32.0f, -14.0f);
//vec4 lightDiffuse = vec4(0.9f);
vec4 lightAmbient = vec4(0.1f);

void main()
{   
	// vec3 norm = normalize(fsNormal);
	// vec3 lightDir = -normalize(lightPos - fsPosition);
	// float diffuse = max(dot(norm, lightDir), 0.0);
	// vec4 finalDiffuse;
	// if(diffuse > 0.1f)
	// {
	// 	finalDiffuse = diffuse * vec4(texture(texture_diffuse1, fsTexCoord));

	// }
	// else
	// {
	// 	finalDiffuse = 0.1f * vec4(texture(texture_diffuse1, fsTexCoord));		
	// }
 //    color = lightAmbient + finalDiffuse;
    color = vec4(texture(texture_diffuse1, fsTexCoord));
    
}