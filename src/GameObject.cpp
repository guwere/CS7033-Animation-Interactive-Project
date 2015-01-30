#include "stdafx.h"

#include "GameObject.hpp"
#include "Model.hpp"
#include "ShaderProgram.hpp"
#include "ModelManager.hpp"
#include "Timer.hpp"
#include "Animation.hpp"
#include "GameWorld.hpp"
#include "math_utilities.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
using std::vector;
using  std::map;

Object::Object()
{

}
Object::Object(const std::string &objectName,
	const std::string &modelName,
	const SQTTransform &transform)
	:m_Name(objectName), m_Model(ModelManager::get().getModel(modelName)), m_Transform(transform), m_State(State::ACTIVE)

	
{
}

Object::Object(const std::string &objectName,
	const Model *model,
	const SQTTransform &transform)
	: m_Name(objectName), m_Model(model), m_Transform(transform), m_State(State::ACTIVE)

{

}

Object::~Object()
{

}

void Object::setTransform(const SQTTransform &transform)
{
	m_Transform = transform;
}

SQTTransform& Object::getTransform()
{
	return m_Transform;
}

void Object::generateAABB()
{

	for (int i = 0; i < m_Model->m_Meshes.size(); i++)
	{
		m_AABB.add(m_Model->m_Meshes[i]->m_Vertices);
	}
	m_AABB.m_Enabled = true;
	m_AABB.generateDisplayBox();
}

bool Object::intersect(const AABB &other)
{
	return m_AABB.intersect(other);
}

void Object::render(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix) 
{
	glUseProgram(m_Model->m_ShaderProgram->m_Id);
	glm::mat4 modelMatrix = getTransform().getMatrix();

	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(modelMatrix)); // well known locations of uniforms
	glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(viewMatrix)); //@todo change later
	glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projMatrix));

	//glUniform3fv(3,1, glm::value_ptr(glm::vec3(viewMatrix[3]))); // position of camera; straight to fragment shader
	m_Model->render();
	m_AABB.render();
}

void Object::update()
{
	m_AABB.transform(m_Transform.getMatrix());
}

SkinnedObject::SkinnedObject(const std::string &objectName,
	const std::string &modelName,
	const SQTTransform &transform)
	:Object(objectName, ModelManager::get().getSkinnedModel(modelName), SQTTransform()),
	m_alreadyCalculated(false),m_AnimSpeed(1.0)
	
{
	m_SkinnedModel = static_cast<const SkinnedModel*>(m_Model);
	m_SkeletonId = m_SkinnedModel->m_Skeleton->m_Id;
	//m_AnimStopped = !m_SkinnedModel->hasAnimation();
	setTransform(transform);
	m_BoneLocation = glGetUniformLocation(m_Model->m_ShaderProgram->m_Id, "boneTransform");

	if(m_SkinnedModel->hasAnimation())
	{
		const Animation *idleAnim = m_SkinnedModel->getIdleAnimation();
		idleAnim->getTransforms(idleAnim->m_TotalDuration,m_BoneLocalTransforms);
		m_SwapAnim = true;
	}
	else
	{
		m_BoneLocalTransforms = m_SkinnedModel->getBoneLocalTransforms();

	}
}

SkinnedObject::~SkinnedObject()
{

}

void SkinnedObject::setTransform(const SQTTransform &transform)
{
	m_Transform = transform;
}
SQTTransform& SkinnedObject::getTransform()
{
	return m_Transform;
    }



SQTTransform& SkinnedObject::getBoneTransform(const std::string &boneName)
{
	//bones can be found either by id or name
	unsigned int boneId = m_SkinnedModel->m_BoneIdMap.find(boneName)->second;
	return m_BoneLocalTransforms[boneId];

}

SQTTransform SkinnedObject::getBoneGlobalTransform(const Bone *bone)
{
	return convertToSQTTransform(m_Transform.getMatrix() * 
		m_BoneAbsoluteTransforms.at(bone->m_Id)
		* glm::inverse(bone->m_InverseBindPose));
}


void SkinnedObject::render(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix) 
{
	glUseProgram(m_Model->m_ShaderProgram->m_Id);

	//write the bone matrices to the gpu
	for (int i = 0; i < m_BoneAbsoluteTransforms.size(); i++)
	{
		glUniformMatrix4fv(m_BoneLocation + i, 1, GL_FALSE, glm::value_ptr(m_BoneAbsoluteTransforms.at(i)));
		//m_BoneAbsoluteTransforms[i] = getTransform().getMatrix() * m_BoneAbsoluteTransforms[i];
	}

	//call base class for transformations and such
	Object::render(viewMatrix, projMatrix);

}

vector<SQTTransform> SkinnedObject::getSelectedBonesInWorld( const SkinnedModel::AbsoluteTransformMap bones,
	const std::vector<int> bonePos)
{
	std::vector<SQTTransform> bonesWorld;
	//get the SQT representation as it is easier to work with
	for (int i = 0; i < bonePos.size(); i++)
	{
		SQTTransform transform;
		transform = convertToSQTTransform(getTransform().getMatrix() * bones.at(bonePos[i]));
		bonesWorld.push_back(transform);
	}
	return bonesWorld;
}



void SkinnedObject::calculateIK(const glm::vec3 &dest, const std::string &boneName, unsigned int linkLen, unsigned int maxTries)
{
	if (linkLen == 0)
		return;

	//get the actual bone
	const Bone* currBone = m_SkinnedModel->m_Skeleton->findBone(boneName);
	if (!currBone)
	{
		LOG(ERROR) << "Could not find bone : " << boneName;
		return;
	}

	int numBones = linkLen;
	//tell the render function not to recalculate transforms
	if(/*!m_alreadyCalculated*/ true)//very important if more than one influence
	{
		m_alreadyCalculated = true;
		//calculate here instead of render function but do not include the inverse bind pose calculation
		// because we want to apply IK in world space. For this reason, convert the absolute matrix that is returned to world space
		// until the IK finishes. Then we revert back to mesh space with the inverse model transform and finally
		// apply the inverse bind pose as usual. Remember that the Inverse Bind Pose moves the vertex from Mesh(Model) space
		// IN bind pose to the local space of the bone. We don't want that until the IK has finished calculating.
		m_BoneAbsoluteTransforms = m_SkinnedModel->getAbsoluteBoneTransforms(m_BoneLocalTransforms,m_ParentTransforms, false);
	}
	
	SkinnedModel::AbsoluteTransformMap &bones = m_BoneAbsoluteTransforms;
	

	// build a list of the bone chain. Left most position is end effector and each subsequent child is a parent
	//we need this because the bone map does not store the bones in a hierarchical order necessarily
	std::vector<int> bonePos;
	std::vector<SQTTransform> bonesWorld;
	glm::mat4 globalTransform;
	do
	{
		bonePos.push_back(currBone->m_Id);
		numBones--;
		currBone = currBone->m_Parent;

	} while (numBones && currBone->m_Id != -1);

	numBones = bonePos.size();

	bonesWorld = getSelectedBonesInWorld(bones, bonePos);

	float totalLength = 0;
	vector<float> boneLength(numBones);
	//gather the bone lengths. Very handy when projecting the bones on new axis
	for (int i = 1; i < numBones; i++)
	{
		float length = glm::length(bonesWorld[i].getPosition() - bonesWorld[i - 1].getPosition());
		totalLength += length;
		boneLength[i-1] = length;

	}

	//------------do the actual CCD---------------------------
	static const float IK_DISTANCE_THRESH = 0.01f;
	int tries = 0;
	int currLink = 1; //leftmost pos in vector is effector

	glm::vec3 effectorPos; // the position of the end effector
	glm::vec3 currPos;// the position of the bone that we are currently rotating

	const glm::vec3 &desiredPos = dest;
	do
	{
		effectorPos = bonesWorld[0].getPosition(); //lower number -> closer to effectorPos
		currPos = bonesWorld[currLink].getPosition();

		//quit if close enough
		float distance = glm::distance2(effectorPos, desiredPos);
		if (distance > IK_DISTANCE_THRESH)
		{
			//vector to current effectorPos pos
			glm::vec3 currDir = effectorPos - currPos;
			//desired effectorPos position
			glm::vec3 targetDir = desiredPos - currPos;

			currDir = glm::normalize(currDir);
			targetDir = glm::normalize(targetDir);

			//how much of the current vector lies on the target vector (how much of it is projected onto the other)
			float cosAngle = glm::dot(currDir, targetDir);

			//if the dot product over some threshold then don't rotate
			if (cosAngle < 0.99999)
			{

				float turnAngle = acos(cosAngle); // radians
				//get the axis of rotation
				glm::vec3 rotAxis = glm::cross(currDir, targetDir);
				//rotate the current bone
				bonesWorld[currLink].pivotOnAngleAxis(turnAngle, rotAxis);
				//dampen
				//bonesWorld[currLink].setRotation(dampen(bonesWorld[currLink].getOrientation(), glm::vec3(90.0f)));
				
				glm::mat4 currMatW = bonesWorld[currLink].getMatrix(); //world
				glm::mat4 currMatM = glm::inverse(getTransform().getMatrix()) * currMatW;//good
				glm::mat4 currMatL = glm::inverse(m_ParentTransforms[bonePos[currLink]]) * currMatM; // bone local

				m_BoneLocalTransforms[bonePos[currLink]] = convertToSQTTransform(currMatL);
				bones = m_SkinnedModel->getAbsoluteBoneTransforms(m_BoneLocalTransforms, m_ParentTransforms, false);
				bonesWorld = getSelectedBonesInWorld(bones, bonePos);
				//----------------------
			}
			currLink++;
			if (currLink > numBones - 1) // start again
				currLink = 1;
		}
	} while (/*currLink != 1||*/ tries++ < maxTries && glm::distance2(effectorPos, desiredPos) > IK_DISTANCE_THRESH);
	

	//we had worked the CCD in world space. Now revert back from world to model 
	for (int i = 0; i < numBones; i++)
	{
		bones[bonePos[i]] = glm::inverse(getTransform().getMatrix()) * bonesWorld[i].getMatrix();
	}
	//... and from model to bone space (for ALL bones of skeleton)
	for (int i = 0; i < bones.size(); i++)
	{
		bones[i] *= m_SkinnedModel->m_Skeleton->findBone(i)->m_InverseBindPose;
	}
}

//@todo cleanup Id lookup. Currently this function is not used
glm::mat4 SkinnedObject::calculateGlobalTransform(const Bone *currBone)
{
	int currBoneIndex = m_SkinnedModel->findBoneIndex(currBone->m_Name);
	if(currBoneIndex == -1)
		return glm::mat4();
	glm::mat4 currGlobal = m_BoneLocalTransforms.at(currBoneIndex).getMatrix();

	//from current bone go up the chain
	const Bone* parent = currBone->m_Parent;
	do
	{
		//if we can find a bone with that name then use the local transformation stored in the object
		int boneIndex = m_SkinnedModel->findBoneIndex(parent->m_Name);
		if(boneIndex != -1)
		{
			glm::mat4 parentLocal = m_BoneLocalTransforms.at(boneIndex).getMatrix();
			currGlobal = parentLocal * currGlobal;
		} //else the not bone hence use the transformation of the node
		else
			currGlobal = parent->m_LocalTransformation.getMatrix() * currGlobal;

		parent = parent->m_Parent;
	}while(parent);
	//don't forget to account for the world position of the object
	currGlobal = getTransform().getMatrix() * currGlobal;
	return currGlobal;
}


void SkinnedObject::attachIK(IKObject::IkType ikType, const std::string &boneName, glm::vec3 position, unsigned int chainLength, unsigned int maxTries)
{
	IKObject newIK;
	newIK.m_ChainLength = chainLength;
	newIK.m_MaxTries = maxTries;
	newIK.m_Position = position;
	newIK.m_IkType = ikType;
	newIK.m_Parent = this;
	m_IKObjectMap[boneName] = newIK;

}
void SkinnedObject::detachIK(const std::string &boneName)
{
	m_IKObjectMap.erase(boneName);
}

IKObject& SkinnedObject::getIK(const std::string &boneName)
{
	return m_IKObjectMap.at(boneName);
}

void SkinnedObject::calculateIKs()
{
	IKObjectMap::const_iterator it = m_IKObjectMap.begin();
	for (; it != m_IKObjectMap.end(); ++it)
	{
			calculateIK(it->second.getPosition(), it->first, it->second.m_ChainLength, it->second.m_MaxTries);
	}
}

SkinnedObject::IKObjectMap& SkinnedObject::getAllIKs()
{
	return m_IKObjectMap;
}

void SkinnedObject::calculateAnimation()
{
	if(!m_SkinnedModel->hasAnimation()) return;
	float deltaTime = Timer::get().getLastInterval();
	//m_TimeExpired += deltaTime;
	float blendTime = GameWorld::get().getBlendTime();
	
	if(m_SwapAnim)
	{
		//save off the last frame of the current animation
		m_TimeExpired = 0;
		m_TransitionTime = 0;
		m_CurrBlendPallete = m_BoneLocalTransforms;
		if(m_AnimQueue.size() > 0)
			m_AnimQueue.pop_front();
		if(m_AnimQueue.size() == 0)
			m_AnimQueue.push_back(m_SkinnedModel->getIdleAnimation());
		m_Blending = true;
		m_AnimQueue.front()->getTransforms(0, m_NextBlendPallete);
	}

	if(m_Blending)
	{
		m_SwapAnim = false;
		float blendFactor = m_TransitionTime/blendTime;
		for(int i = 0; i < m_NextBlendPallete.size(); i++)
			m_BoneLocalTransforms[i] = m_CurrBlendPallete.at(i).interpolate(m_NextBlendPallete.at(i), blendFactor); 
		
		m_TransitionTime += deltaTime;
		if(m_TransitionTime >= blendTime)
			m_Blending = false;
	}
	else
	{
		if(m_TimeExpired >= m_AnimQueue.front()->m_TotalDuration)
		{
			m_SwapAnim = true;
			m_AnimQueue.front()->getTransforms(m_AnimQueue.front()->m_TotalDuration - 0.01, m_BoneLocalTransforms);
		}
		else
		{
			m_AnimQueue.front()->getTransforms(m_TimeExpired,m_BoneLocalTransforms);
			m_TimeExpired += deltaTime*m_AnimSpeed;
		}
	}

}


const Animation* SkinnedObject::getCurrentAnim()
{
	if(m_AnimQueue.size()>0)
		return m_AnimQueue.front();
	m_AnimQueue.push_back(m_SkinnedModel->getAnimation("wait"));
	return m_AnimQueue.front();
}

void SkinnedObject::flushAnimQueue()
{
	m_SwapAnim = true;
	m_AnimQueue.clear();
}
void SkinnedObject::playAnimFlushBlend(const std::string &animName, float animSpeed)
{
	m_SwapAnim = true;
	m_AnimQueue.clear();
	m_AnimSpeed = animSpeed;
	m_AnimQueue.push_back(m_SkinnedModel->getAnimation(animName));
}
void SkinnedObject::playAnimBlend(const std::string &animName, float animSpeed)
{
	m_SwapAnim = true;
	m_AnimSpeed = animSpeed;
	m_AnimQueue.push_back(m_SkinnedModel->getAnimation(animName));
}

void SkinnedObject::generateAABB()
{
	luapath::LuaState settings("config/settings.lua");
	//luapath::Table modelTable = settings.getGlobalTable("skinnedModels").getTable(m_Name);
	luapath::Table modelTable = settings.getGlobalTable("skinnedModels").getTable(std::string(".") + m_SkinnedModel->m_Name);
	luapath::Table aabbTable;
	if(modelTable.getTable(".aabb",aabbTable))
	{
		glm::vec3 min(aabbTable.getValue(".min.x"),aabbTable.getValue(".min.y"),aabbTable.getValue(".min.z"));
		glm::vec3 max(aabbTable.getValue(".max.x"),aabbTable.getValue(".max.y"),aabbTable.getValue(".max.z"));
		m_AABB.add(min);
		m_AABB.add(max);
		m_AABB.m_Enabled = true;
		m_AABB.generateDisplayBox();
	}
}


void SkinnedObject::update()
{
	Object::update();
	calculateAnimation();
	calculateIKs();
	SkinnedModel::AbsoluteTransformMap dummy; // fix later.
	if (!m_alreadyCalculated)
		m_BoneAbsoluteTransforms = m_SkinnedModel->getAbsoluteBoneTransforms(m_BoneLocalTransforms, dummy, true);

	m_alreadyCalculated = false;

}

void IKObject::move(float x, float y, float z)
{
	m_Position += glm::vec3(x, y, z);
}
glm::vec3 IKObject::getPosition() const
{
	if (m_IkType == IkType::GLOBAL)
		return m_Position;
	return glm::vec3((m_Parent->getTransform().getMatrix() * glm::translate(glm::mat4(), m_Position))[3]);
}
