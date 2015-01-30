#version 440
in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec2 fsTexCoord;
out vec3 fsNormal;
out vec3 fsPosition; // in world space
out vec3 fsViewPos;

layout (location = 0) uniform mat4 M;
layout (location = 1) uniform mat4 V;
layout (location = 2) uniform mat4 P;
//layout (location = 3) uniform vec3 viewPos;

void main()
{
	vec4 positionWorld = M * vec4(position, 1.0f);
    gl_Position = P * V * positionWorld;
    fsPosition = vec3(positionWorld);
    
    fsTexCoord = texCoord;
    fsNormal = mat3(transpose(inverse(M))) * normal;

    fsViewPos = V[3].xyz;
}