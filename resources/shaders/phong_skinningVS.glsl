#version 440
in vec3 position;
in vec3 normal;
in vec2 texCoord;
in uvec4 boneIndices;
in vec4 boneWeights;

layout (location = 0) uniform mat4 M;
layout (location = 1) uniform mat4 V;
layout (location = 2) uniform mat4 P;

uniform mat4 boneTransform[100]; // transforms the vertex from bone space to model space

out vec2 fsTexCoord;
out vec3 fsNormal;
out vec3 fsPosition; // in world space

void main()
{
    mat4 blendedMatrix =  boneTransform[boneIndices[0]] * boneWeights[0]
    			     	+ boneTransform[boneIndices[1]] * boneWeights[1]
    			     	+ boneTransform[boneIndices[2]] * boneWeights[2]
    			     	+ boneTransform[boneIndices[3]] * boneWeights[3];

    mat4 modelBoneMatrix = M * blendedMatrix;
    vec4 positionWorld = modelBoneMatrix * vec4(position, 1.0f);
    gl_Position = P * V * positionWorld;
    fsPosition = vec3(positionWorld);
    fsNormal = mat3(transpose(inverse(modelBoneMatrix))) * normal;
    // fsNormal = normal;
    // gl_Position = P * V * M * vec4(position, 1.0);
    fsTexCoord = texCoord;	
}
