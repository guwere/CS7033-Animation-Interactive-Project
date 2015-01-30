#pragma once

#include "stdafx.h"
#include "Model.hpp"
#include "SQTTransform.hpp"
#include "AABB.hpp"

#include <deque>

class Model;
struct Bone;
class SkinnedModel;
class SkinnedObject;
/** 
@brief Represents a transformable instance of Model
@details  Whereas Model is a class that is not suppose to change after being loaded (it serves a template for Object),
Object includes everything needed in order to move around the world, namely, a copy of the boneLocalTransform from the Model
and an SQTTransform instance which enables the object to be transformed.
*/
class Object
{
public:
	enum class State{ ACTIVE, DEACTIVE };

	/**@brief initialize a world Object
		@param objectName the name of the object that uniquely identifies it in GameWorld
		@param modelName the name of the model to load as specified in the lua settings file in the models table. Calls ModelManager
		@param transform the initial scale, quaternion rotation and translation SQLTransform
	*/
	Object(const std::string &objectName,
			const std::string &modelName,
			const SQTTransform &transform = SQTTransform());
	
	/**@brief initialize a world Object
		@param objectName the name of the object that uniquely identifies it in GameWorld
		@param model a model which has already been loaded
		@param transform the initial scale, quaternion rotation and translation SQLTransform
	*/
	Object(const std::string &objectName,
		const Model *model,
		const SQTTransform &transform = SQTTransform());

	virtual ~Object();
	
	virtual void setTransform(const SQTTransform &transform);
	/** @brief Returns a reference to the local transformation used to update the scale, translation and rotation of the object*/
	virtual SQTTransform& getTransform();

	/**writes to the uniform matrices to the assigned shader of the underlying model.
	  Then calls the Model render function to render the primitives
	  */
	virtual void render(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix);

	virtual void update();
	virtual void generateAABB();
	virtual bool intersect(const AABB &other);

protected:
public:
	const Model* m_Model; //!< the Model template representing this Object. Cannot change once loaded in ModelManager
	const std::string m_Name; //!< the name of the object as to be stored in the GameWorld map
	AABB m_AABB;
	Direction m_Direction;
	float m_Velocity;
	float m_MinVelocity;
	float m_MaxRunVelocity;
	float m_MaxWalkVelocity;
	float m_AccelerationRun;
	float m_AccelerationWalk;
	float m_AnimationSpeedModifier;
	float m_LastCollisionTime;
	State m_State;
protected:
	SQTTransform m_Transform; //!< the transform changed to move the object around the world
	Object();
};

struct IKObject
{
public:
	enum class IkType{LOCAL, GLOBAL};
	unsigned int m_MaxTries;
	unsigned int m_ChainLength;
	IkType m_IkType;
	SkinnedObject *m_Parent;
	void move(float x, float y, float z);
	glm::vec3 getPosition() const;
	glm::vec3 m_Position;
private:
};

/**
@brief Represents a transformable instance of SkinnedModel. Build on the functionality of Object
@details  Contains a bone transformation list for all the bones of the SkinnedModel 
@todo playAnimPriority - play new anim first and continue from last after its finished
@todo Allow alternative for IKs that are locally attached to the Model instead of globally
@todo account for overlapping IKs
@todo Fix the alreadyCalculated optimization trick (*ahem* necessity) in calculateIK function if more than one IK
@todo Add biped bone constraints
*/
class SkinnedObject
	:public Object
{
public:
	SkinnedObject(const std::string &objectName,
		const std::string &modelName,
		const SQTTransform &transform = SQTTransform());
	~SkinnedObject();

	virtual void setTransform(const SQTTransform &transform);
	/** @brief Returns a reference to the local transformation of the skeleton used to update the scale, translation and rotation of the object*/
	virtual SQTTransform& getTransform();

	/**@brief get the transformation of the bone so we can apply scale, rotation or translation either on the local or global axis
		@details Note that we return the address of the transform which may be dangerous so we have to be careful not to make operations which may cause the container of transforms to reallocate its memory
		@param boneName the name of the bone to fetch.
	*/
	SQTTransform& getBoneTransform(const std::string &boneName);
	SQTTransform getBoneGlobalTransform(const Bone *bone);

	/**Before calling Object render, calculates and writes the bone matrices to the gpu */
	virtual void render(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix);

	const Animation* getCurrentAnim();
	void flushAnimQueue();
	void playAnimFlushBlend(const std::string &animName, float animSpeed);
	void playAnimBlend(const std::string &animName, float animSpeed);

	typedef std::map<std::string, IKObject> IKObjectMap;//!< Helps the object keep track of the IK attachments
	/**@brief Attach an IK influence to a particular bone */
	void attachIK(IKObject::IkType ikType, const std::string &boneName, glm::vec3 position, unsigned int chainLength, unsigned int maxTries);
	/**@brief remove IK influence from a bone */
	void detachIK(const std::string &boneName);
	/**@brief Retieve IK of a bone so it can be updated */
	IKObject& getIK(const std::string &boneName);
	/**@brief Calls all bones with IK influence to be recomputed */
	void calculateIKs();
	/**@brief Retrieves all IKs */
	IKObjectMap& getAllIKs();

	/**@brief compute the currently running animation . If need be blend with the next animation*/
	void calculateAnimation();


	glm::mat4 calculateGlobalTransform(const Bone *currBone);

	virtual void update();

	virtual void generateAABB();
	
protected:
	/**
	@brief Recalculates the m_BoneLocalTransforms matrix based on the currently playing animation
	@details for the currently playing animation calls the Animation class method to recalculate the bone local transform
		also providing it with the time that has expired for that particular animation. Animation class on gives tells whether
		the animation has ended and if so tells the leftover bit from the end of the animation.
	*/
	//void calculateAnimation(double deltaTime);
	
	/**
	@brief Calculates the IK influence for a single bone using CCD
	*/
	void calculateIK(const glm::vec3 &dest, const std::string &boneName, unsigned int linkLen, unsigned int maxTries);

	/**@brief applies the Model matrix to all the @param bones as specified in @param bonePos 
		@return the resulting subset
	*/
	std::vector<SQTTransform> getSelectedBonesInWorld(const SkinnedModel::AbsoluteTransformMap bones,
		const std::vector<int> bonePos);



public:
	//!<  This is just a reference to @link m_Model as a convenience to we don't have to cast Model* to SkinnedModel*
	const SkinnedModel* m_SkinnedModel;
protected:

	IKObjectMap m_IKObjectMap;
	unsigned int m_SkeletonId;
	//!< false -> play animation at front of queue. true -> stopAnim has been called or there is nothing on the queue
	bool m_AnimStopped;
	//!< The skinned object pops animation instances from here and plays them. the ...Anim... functions control the queue
	typedef std::deque<const Animation*> AnimQueue;
	AnimQueue m_AnimQueue;

	//!< the bone matrix location in the shader
	GLuint m_BoneLocation;
	//!<that is changeable through the rotateBone function. All else can be stored in SkinnedModel because it does not change
	SkinnedModel::LocalTransformMap m_BoneLocalTransforms;
	//!<The individual parent transforms for each bone
	SkinnedModel::AbsoluteTransformMap m_ParentTransforms;
	//!<store the bone matrices which go to the shader
	SkinnedModel::AbsoluteTransformMap m_BoneAbsoluteTransforms;
	//!< signifies whether the bone absolute transforms have already been computed. That would happen if IK was called before render
	bool m_alreadyCalculated;

	float m_TimeExpired;
	float m_TransitionTime;
	bool m_Blending;
	bool m_SwapAnim;
	float m_AnimSpeed;
	std::map<unsigned int, SQTTransform> m_CurrBlendPallete;
	std::map<unsigned int, SQTTransform> m_NextBlendPallete;
};
