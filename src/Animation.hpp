#pragma once
#include "stdafx.h"
#include "SQTTransform.hpp"
#include <glm/glm.hpp>

#include <assimp\anim.h>

/**@brief Represents rotation key within the keyframes for a particular bone*/
struct RotationKey
{
	double m_Time;
	glm::quat m_Value;
};

/**@brief Represents scale key within the keyframes for a particular bone*/
struct ScaleKey
{
	double m_Time;
	glm::vec3 m_Value;
};

/**@brief Represents position key within the keyframes for a particular bone*/
struct PositionKey
{
	double m_Time;
	glm::vec3 m_Value;
};

/**@brief Represents the keyframes for a particular bone */
struct BoneAnim
{
	int m_NumRotationKeys;
	int m_NumScaleKeys;
	int m_NumPositionKeys;

	std::vector<RotationKey> m_RotationKeys;
	std::vector<ScaleKey> m_ScaleKeys;
	std::vector<PositionKey> m_PositionKeys;

	BoneAnim();
	BoneAnim(const aiNodeAnim *boneAnim);

	//@todo consider promoting this logic to Animation class instead
	glm::vec3 interpolateScale(double expiredTicks) const;
	glm::vec3 interpolatePosition(double expiredTicks) const;
	glm::quat interpolateRotation(double expiredTicks) const;

private:


};

/**
@brief Represents a single animation for a particular bone structure of a model
*/
struct Animation
{
public:

	/**@brief Calculates the local bone transformations for the given bones. 
	   @param deltaTime the time from the last call to animate
	   @param boneLocalTransforms returns the updated bones here
	*/
	bool getTransforms(double deltaTime, std::map<unsigned int, SQTTransform> &boneLocalTransforms) const;

	~Animation();
public:
	unsigned int m_Id; //!< int id used to speed up search
	std::string m_Name; //!< name of animation. Animation across different types of models can share the same name
	double m_TotalDuration; //!< total duration in clock time
	float m_TotalTicks; //!< total ticks
	float m_TicksPerSecond; //!< tps
	unsigned int m_NumBones;
	std::map<unsigned int, BoneAnim*> m_BoneAnim; //!< a single bone anim corresponds to the animation of a single bone

private:
};

/**
@brief Used by SkinnedObject to keep track of the animations still to be played
*/
struct AnimInstance
{
	AnimInstance();
	AnimInstance(const Animation *anim, double timeExpired);
	const Animation *m_Anim;
	double m_TimeExpired;
};