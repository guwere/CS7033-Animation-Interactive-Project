#pragma once
#include "stdafx.h"
#include "Mesh.hpp"
#include "Animation.hpp"
#include "SQTTransform.hpp"

#include <luapath/luapath.hpp>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>

struct ShaderProgram;
struct aiScene;
struct aiNode;

/**
@brief A class that is used as the template to instantiate Object instances.
@details Each Object contain a pointer to its Model. Many Object instances can have the same underlying Model.
As such Model member variables are not allowed to be changed except the shader that it uses.
once after initialization. A Model owns the Assimp Scene that it loads. Disgards of the scene after construction once 
the relevant data structure have been loaded. Model also own the its Mesh objects so it manages the lifetime of Mesh.
Between the GameWorld -> Object -> Model -> Mesh -> Vertex hierarchy it is Model that associates with a given ShaderProgram.
some of the code is adapted from http://learnopengl.com/#!Model-Loading/Model
*/
class Model
{
public:
	/**@brief Contruct a model by giving the modelTable loaded from lua config file.
		 @details Usually it is the responsibility of ModelManager to initialize and load models
	 */
	Model(const luapath::Table &modelTable);

	/**Deletes the Mesh objects of the model*/
	virtual ~Model();

	/**render the model meshes using the shader associated with that model*/
	virtual void render() const;
	ShaderProgram* getShaderProgram() const;
protected:
	/** 
	@brief Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	*/
	virtual void loadScene(const luapath::Table &modelTable);
	/**Calls ShaderManager to retrieve a ShaderProgram object */
	void loadShaders(const luapath::Table &modelTable);

	/**Get a single Vertex at position @param i of @param mesh. */
	Vertex retrieveVertex(const aiMesh* mesh, int i);

	/**Get the indices in @param mesh and transfer to @param resultMesh*/
	void retrieveIndices(Mesh *resultMesh, const aiMesh *mesh);

	/**@brief Get the material properties stored in the loaded assimp model.
		@details currently supports ambient, diffuse,specular and shininess components
	*/
	void retrieveMaterials(Mesh *resultMesh, const aiMesh* mesh);

	/**@brief Multiplies all the vertices of the mesh by the (root transforms * mesh transform)
	*/
	void bakeTransform(Mesh *resultMesh, const aiNode* node);
	/**Companion function to retrieveMaterials*/
	aiColor4D loadMaterialProperties(const aiMaterial *mat, const char *aiType, unsigned int type, unsigned int idx);

	/** @brief Processes a node in a recursive fashion.
		@details Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	*/
	virtual void processNode(const aiNode* node);

	/**Creates a newly allocated Mesh for each aiMesh. Fill the Mesh with relevant data (vertices, normals, etc.)*/
	virtual Mesh* processMesh(const aiMesh* mesh);
	
	/** @brief Checks all material textures of a given type and loads the textures if they're not loaded yet.
	 */
	std::vector<Texture> loadMaterialTextures(const aiMaterial* mat, const aiTextureType type);

	/**@brief Loads a single texture from a file.
		@details Relies on @link m_ModelDir to have been set beforehand which happens in the loadScene function*/
	GLint textureFromFile(const std::string &textureDir);

public:
	const std::string m_Name;
	std::vector<Mesh*> m_Meshes; //!< the array of meshes that are rendered
	ShaderProgram *m_ShaderProgram;

protected:
	//used by subclasses only
	Model(const std::string &name);

	Assimp::Importer m_Importer; 
	const aiScene *m_Scene; //!< freed at the end of the constructor
	std::string m_ModelDir;  //!< contains the folder directory of the model
	std::vector<Texture> m_TexturesLoaded; //!< structure to speed up loading textures which have been loaded before
};


/**
@brief Recursive bone structure for SkinnedModel
@todo enforce constness on member variables
*/
struct Bone
{
	/**Deletes bones recursively*/
	~Bone();
	int m_Id;	//!< Handy for faster lookup within vectors sequentially storing the bones
	std::string m_Name; //!< name identifier is useful as it is more descriptive
	SQTTransform m_LocalTransformation; //!< transformation of the bone
	glm::mat4 m_InverseBindPose; //!< converts from model space to bone space
	Bone *m_Parent;
	std::vector<Bone*> m_Children;

	/**Find a bone in the hierarchy by the given @param boneIndex*/
	const Bone* findBone(int boneIndex) const;
	/**Find a bone in the hierarchy by the given @param boneName*/
	const Bone* findBone(const std::string &boneName) const;

};

/**@brief An extension of the plain Model which can only represent rigid models.
	Stores an array of SkinnedMesh instead of plain Mesh.
	Acts as the base of the skeleton bone hierachy.
@todo enforce proper constness
*/
class SkinnedModel
	: public Model
{
public:
	/**@brief Contruct a model by giving the modelTable loaded from lua config file.
	@details Usually it is the responsibility of ModelManager to initialize and load models
	*/
	SkinnedModel(const luapath::Table &modelTable);
	/**Deletes the skeleton of the model*/
	~SkinnedModel();

	/** index to bone local sqt transform  */
	typedef std::map<unsigned int, SQTTransform> LocalTransformMap;
	/** index to the final transformation matrix */
	typedef std::map<unsigned int, glm::mat4> AbsoluteTransformMap;

	/** index to aiBone map for quick lookup when building bone structure. This structure is only used during initialization */
	typedef std::map<unsigned int, const aiBone*> AiBoneMap;

	/** index to aiBone map for quick lookup when building bone structure */
	typedef std::map<std::string, unsigned int> BoneIdMap;
	BoneIdMap m_BoneIdMap; //!< speed up 

	/** Get the array of (bone id, bone transform) pairs */
	LocalTransformMap getBoneLocalTransforms() const;

	/**Calculate the final blended matrix from the GlobalInverseTransform, 
		the upstream @param localTransforms and the Bone InverseBindPose
		Recursively calculate the bone transform
	*/
	AbsoluteTransformMap getAbsoluteBoneTransforms(const LocalTransformMap &localTransforms, AbsoluteTransformMap &parentTransforms, bool includeIBP) const;

	bool hasAnimation() const;
	/**@brief Get the animation that plays when there is no user input*/
	const Animation* getIdleAnimation() const;
	/**@brief Retrieve the animation structure with @param animId */
	const Animation* getAnimation(unsigned int animId) const;
	/**@brief Retrieve the animation structure with @param animName */
	const Animation* getAnimation(std::string animName) const;

	/**Returns the bone index in the map */
	unsigned int findBoneIndex(const std::string &boneName) const;

protected:
	/**Prints the assimp animation hierachy*/
	void printAnimHierarchy() const;
	/**Create the bone lookup table from the asssimp scene*/
	void createBoneLookupMaps(AiBoneMap &aiBoneMap, BoneIdMap &boneIdMap);

	/**Constructs bone hierarchy from the assimp bone hierarchy*/
	void loadBones(const aiNode *rootBone);
	/**Companion to loadBones*/
	void loadBones(const aiNode *currNode, const AiBoneMap &aiBoneMap, Bone *currBone);

	/**Load the animations from assimp scene into the internal animation representation*/
	void loadAnimations(const luapath::Table &modelTable);

	/**Load the animations from assimp scene into the internal animation representation*/
	void loadAnimation(const aiAnimation *currAnim, const std::string &animName, int numAnim);

	virtual void processNode(const aiNode *node);
	virtual Mesh* processMesh(const aiMesh *mesh);

	/**Companion to getAbsoluteBoneTransforms*/
	void calculateTransformations(const Bone *parentNode,
									const glm::mat4 &parentTransform,
									const LocalTransformMap &localTransforms,
									AbsoluteTransformMap &parentTransforms,
									AbsoluteTransformMap &result,
									bool includeIBP) const;


public:
	Bone *m_Skeleton;
protected:
	//!< lookup quicker with ids
	typedef std::map<unsigned int, Animation*> AnimMap;
	AnimMap m_AnimMap;
	unsigned int m_IdleAnimationId;
	//!< lookup through the actual name of the animation. Mostly used when constructing the model
	typedef std::map<std::string, unsigned int> AnimIdMap;
	AnimIdMap m_AnimIdMap;

	//const aiNode *m_rootBoneNode; //!< the root of the scene. Becomes invalid after initialization

	//glm::mat4 m_GlobalInverseTransform; //!< the matrix fixes model axis to openGL axis
	LocalTransformMap m_LocalTransforms; //!< the array of bone transforms used as template for Object instances

};