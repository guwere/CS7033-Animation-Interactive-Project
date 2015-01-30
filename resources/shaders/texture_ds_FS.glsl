#version 440

in vec2 fsTexCoord;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{    
	vec4 diff = texture(texture_diffuse1, fsTexCoord);
	vec4 spec = texture(texture_specular1, fsTexCoord);
    // color = vec4(1.0f,0.0f,0.0f,1.0f);
    color = 0.5f*diff + 0.5f*spec;
}