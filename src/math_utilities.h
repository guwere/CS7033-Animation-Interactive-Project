#pragma once
#include <assimp/matrix4x4.h>
#include <assimp/color4.h>
#include <assimp/quaternion.h>
#include <assimp/vector3.h>

#include <glm/glm.hpp>

const float PI = 3.1415927f;
#include "SQTTransform.hpp"

#include <cstdlib>
#include <ctime>

//glm : column major , assimp: row major
/**@brief Convert an assimp matrix to glm matrix
*/
inline glm::mat4 convertToGLMMat4(const aiMatrix4x4 &from)
{
	glm::mat4 to;
	//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
	to[0][0] = from.a1; to[1][0] = from.a2;	to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2;	to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2;	to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2;	to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}
/**@brief Convert a glm to assimp matrix*/
inline aiMatrix4x4 convertToAiMatrix4x4(const glm::mat4  &from)
{
	aiMatrix4x4 to;
	to.a1 = from[0][0]; to.a2 = from[1][0];	to.a3 = from[2][0]; to.a4 = from[3][0];
	to.b1 = from[0][1]; to.b2 = from[1][1];	to.b3 = from[2][1]; to.b4 = from[3][1];
	to.c1 = from[0][2]; to.c2 = from[1][2];	to.c3 = from[2][2]; to.c4 = from[3][2];
	to.d1 = from[0][3]; to.d2 = from[1][3];	to.d3 = from[2][3]; to.d4 = from[3][3];
	return to;
}

/**@brief Convert an assimp color container to glm 4d float vector*/
inline glm::vec4 convertToGLMVec4(const aiColor4D &from)
{
	glm::vec4 to;
	to.r = from.r;
	to.g = from.g;
	to.b = from.b;
	to.a = from.a;
	return to;
}

/**@brief Convert an assimp 3d vector to glm 3d float vector*/
inline glm::vec3 convertToGLMVec3(const aiVector3D &from)
{
	glm::vec3 to;
	to.x = from.x;
	to.y = from.y;
	to.z = from.z;
	return to;
}

/**@brief	Convert an assimp quaternion to glm vector to glm quaternion*/
inline glm::quat convertToGLMQuat(const aiQuaternion &from)
{
	glm::quat to;
	to.x = from.x;
	to.y = from.y;
	to.z = from.z;
	to.w = from.w;
	return to;
}


/**@brief convert from matrix to sqt transform representation CAUTION: preserves transformations only if uniform scale is present*/
inline SQTTransform convertToSQTTransform(const aiMatrix4x4 &from)
{
	SQTTransform to;
	aiVector3D scale;
	aiQuaternion rotation;
	aiVector3D position;
	from.Decompose(scale, rotation, position);
	
	glm::vec3 glmScale = convertToGLMVec3(scale);
	glm::quat glmRotation = convertToGLMQuat(rotation);
	glm::vec3 glmTranslation = convertToGLMVec3(position);
	to = SQTTransform(glmTranslation, glmScale, glmRotation);
	return to;
}

/**@brief convert from matrix to sqt transform representation CAUTION: preserves transformations only if uniform scale is present*/
inline SQTTransform convertToSQTTransform(const glm::mat4 &from)
{
	SQTTransform to;
	//extract translation
	glm::vec3 position;
	position.x = from[3].x;
	position.y = from[3].y;
	position.z = from[3].z;

	//extract scale
	glm::vec3 scale;
	scale.x = glm::length(from[0]);
	scale.y = glm::length(from[1]);
	scale.z = glm::length(from[2]);

	if (glm::determinant(from) < 0)
	{
		scale = -scale;
	}
	//extract rotation
	//get mat upper left 3x3
	glm::mat3 rotation(from);
	//remove scaling
	if (scale.x)
		rotation[0] /= scale.x;
	if (scale.y)
		rotation[1] /= scale.y;
	if (scale.z)
		rotation[2] /= scale.z;
	glm::quat rotationQuat = glm::toQuat(rotation);

	to.setPosition(position);
	to.setRotation(rotationQuat);
	to.scale(scale.x, scale.y, scale.z);

	return to;
}

inline float radiansToDegrees(float radians)
{
	return (radians*180.0f) / PI;
}

inline float degreeToRadians(float degrees)
{
	return (degrees * PI) / 180.0f;
}

inline glm::quat eulerToQuaternion(glm::vec3 eulerAngles)
{
	glm::quat result;
	if (eulerAngles.x)
		result = glm::rotate(result, eulerAngles.x, glm::vec3(1.0f, 0, 0));
	if (eulerAngles.y)
		result = glm::rotate(result, eulerAngles.y, glm::vec3(0, 1.0f, 0));
	if (eulerAngles.z)
		result = glm::rotate(result, eulerAngles.z, glm::vec3(0, 0, 1.0f));
	return result;
}

inline glm::mat4 scalarMultiplication(float scalar, glm::mat4 matrix)
{
	matrix[0][0] *= scalar; matrix[1][0] *= scalar;	matrix[2][0] *= scalar; matrix[3][0] *= scalar;
	matrix[0][1] *= scalar; matrix[1][1] *= scalar;	matrix[2][1] *= scalar; matrix[3][1] *= scalar;
	matrix[0][2] *= scalar; matrix[1][2] *= scalar;	matrix[2][2] *= scalar; matrix[3][2] *= scalar;
	matrix[0][3] *= scalar; matrix[1][3] *= scalar;	matrix[2][3] *= scalar; matrix[3][3] *= scalar;
}

inline glm::vec3 dampen(glm::quat from, glm::vec3 dampVec)
{
	glm::vec3 euler = glm::degrees(glm::eulerAngles(from));
	if		(euler.x > 0 && euler.x > dampVec.x) euler.x = dampVec.x;
	else if (euler.x < -dampVec.x) euler.x = -dampVec.x;
	if		(euler.y > 0 && euler.y > dampVec.y) euler.y = dampVec.y;
	else if (euler.y < -dampVec.y) euler.y = -dampVec.y;
	if		(euler.z > 0 && euler.z > dampVec.z) euler.z = dampVec.z;
	else if (euler.z < -dampVec.z) euler.z = -dampVec.z;
	return euler;
}

inline float randomNumber(float Min, float Max)
{
    return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}