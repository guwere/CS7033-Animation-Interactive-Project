#version 440

in vec2 fsTexCoord;

out vec4 color;

uniform sampler2D texture_diffuse1;

void main()
{   
	vec4 finalDiffuse = vec4(texture(texture_diffuse1, fsTexCoord));
    color = finalDiffuse;
    
}