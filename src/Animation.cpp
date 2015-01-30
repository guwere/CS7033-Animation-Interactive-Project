#include "Animation.hpp"
#include "math_utilities.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

BoneAnim::BoneAnim()
{

}

// @todo generalise
BoneAnim::BoneAnim(const aiNodeAnim *boneAnim)
	:m_NumScaleKeys(boneAnim->mNumScalingKeys), 
	m_NumPositionKeys(boneAnim->mNumPositionKeys),
	m_NumRotationKeys(boneAnim->mNumRotationKeys)
{
	//m_ScaleKeys.reserve(m_NumScaleKeys);
	//m_PositionKeys.reserve(m_NumPositionKeys);
	//m_RotationKeys.reserve(m_NumRotationKeys);

	for(int i = 0; i < m_NumScaleKeys; i++)
	{
		ScaleKey key;
		key.m_Time = boneAnim->mScalingKeys[i].mTime; 
		key.m_Value = convertToGLMVec3(boneAnim->mScalingKeys[i].mValue); 
		m_ScaleKeys.push_back(key);
	}

	for(int i = 0; i < m_NumPositionKeys; i++)
	{
		PositionKey key;
		key.m_Time = boneAnim->mPositionKeys[i].mTime; 
		key.m_Value = convertToGLMVec3(boneAnim->mPositionKeys[i].mValue); 
		m_PositionKeys.push_back(key);
	}

	for(int i = 0; i < m_NumRotationKeys; i++)
	{
		RotationKey key;
		key.m_Time = boneAnim->mRotationKeys[i].mTime; 
		key.m_Value = convertToGLMQuat(boneAnim->mRotationKeys[i].mValue); 
		m_RotationKeys.push_back(key);
	}
}

Animation::~Animation()
{
	std::map<unsigned int, BoneAnim*>::iterator it = m_BoneAnim.begin();
	for (; it != m_BoneAnim.end(); ++it)
		delete it->second;
}

bool Animation::getTransforms(double deltaTime,	std::map<unsigned int, SQTTransform> &boneLocalTransforms) const
{
	//boneLocalTransforms.resize(m_NumBones);
	//see if reached the end of the animation
	double leftover = deltaTime - m_TotalDuration;
	//upper bound of ticks is the total ticks
	double expiredTicks = fmod(deltaTime*m_TicksPerSecond, m_TotalTicks);

	for(int i = 0 ; i < m_NumBones; i++)
	{
		glm::vec3 resultScale = m_BoneAnim.at(i)->interpolateScale(expiredTicks);
		glm::vec3 resultPosition = m_BoneAnim.at(i)->interpolatePosition(expiredTicks);
		glm::quat resultRotation = m_BoneAnim.at(i)->interpolateRotation(expiredTicks);

		SQTTransform resultTransform(resultPosition,resultScale,resultRotation);
		boneLocalTransforms[i] = resultTransform;

	}
	if(leftover > 0)
		return true;
	return false;
}

glm::vec3 BoneAnim::interpolateScale(double expiredTicks) const
{
	if(m_NumScaleKeys == 0)
		return glm::vec3(); // return identity
	else if(m_NumScaleKeys == 1)
		return m_ScaleKeys.at(0).m_Value;
	//else interpolate
	//find the two closest keys
	ScaleKey leftKey, rightKey;
	//find left key
	for(int i = 0; i < m_NumScaleKeys - 1; i++)
	{//search as long as the expired time is less than the next key
		if(expiredTicks < m_ScaleKeys.at(i+1).m_Time)
		{
			leftKey = m_ScaleKeys.at(i);
			rightKey = m_ScaleKeys.at(i+1);
			break;
		}
	}
	double diff = rightKey.m_Time - leftKey.m_Time;
	double factor = (expiredTicks - leftKey.m_Time)/ diff; // the weighting factor
	//linear interpolation
	return glm::vec3(1 - factor)*leftKey.m_Value + glm::vec3(factor)*rightKey.m_Value;
}

glm::vec3 BoneAnim::interpolatePosition(double expiredTicks) const
{
	if(m_NumPositionKeys == 0)
		return glm::vec3(); // return identity
	else if(m_NumPositionKeys == 1)
		return m_PositionKeys.at(0).m_Value;
	
	PositionKey leftKey, rightKey;
	//find left key
	for(int i = 0; i < m_NumPositionKeys - 1; i++)
	{//search as long as the expired time is less than the next key
		if(expiredTicks < m_PositionKeys.at(i+1).m_Time)
		{
			leftKey = m_PositionKeys.at(i);
			rightKey = m_PositionKeys.at(i+1);
			break;
		}
	}
	double diff = rightKey.m_Time - leftKey.m_Time;
	double factor = (expiredTicks - leftKey.m_Time)/ diff; // the weighting factor
	return glm::vec3(1 - factor)*leftKey.m_Value + glm::vec3(factor)*rightKey.m_Value;
}

glm::quat BoneAnim::interpolateRotation(double expiredTicks) const
{
	if(m_NumRotationKeys == 0)
		return glm::quat(); // return identity
	else if(m_NumRotationKeys == 1)
		return m_RotationKeys.at(0).m_Value;

	
	RotationKey leftKey, rightKey;
	//find left key
	for(int i = 0; i < m_NumRotationKeys - 1; i++)
	{//search as long as the expired time is less than the next key
		if(expiredTicks < m_RotationKeys.at(i+1).m_Time)
		{
			leftKey = m_RotationKeys.at(i);
			rightKey = m_RotationKeys.at(i+1);
			break;
		}
	}
	double diff = rightKey.m_Time - leftKey.m_Time;
	float factor = (expiredTicks - leftKey.m_Time)/ diff; // the weighting factor
	////slerpy derpy
	glm::quat result = glm::slerp(leftKey.m_Value, rightKey.m_Value, factor);
	return result;
}

AnimInstance::AnimInstance()
	:m_Anim(NULL), m_TimeExpired(0)
{

}

AnimInstance::AnimInstance(const Animation *anim, double timeExpired)
	:m_Anim(anim), m_TimeExpired(timeExpired)
{
}

