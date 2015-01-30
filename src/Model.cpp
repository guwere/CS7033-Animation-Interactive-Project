#include "stdafx.h"
#include "Model.hpp"
#include "ShaderManager.hpp"
#include "ShaderProgram.hpp"
#include "math_utilities.h"

#include <SOIL.h>
#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::map;
using std::vector;
using std::string;
using std::endl;

Model::Model(const std::string &name)
	:m_Name(name)
{

}
Model::Model(const luapath::Table &modelTable)
	: m_Name(modelTable.getKey().key)
{
	loadShaders(modelTable);
	loadScene(modelTable);
	// Process ASSIMP's root node recursively
	processNode(m_Scene->mRootNode);
	m_Importer.FreeScene();
}

Model::~Model()
{
	std::vector<Mesh*>::iterator it = m_Meshes.begin();
	for (; it != m_Meshes.end(); ++it)
		delete *it;
}

// Draws the model, and thus all its meshes
void Model::render() const
{
	std::vector<Mesh*>::const_iterator it = m_Meshes.begin();
	for (; it != m_Meshes.end(); ++it)
		(*it)->render(m_ShaderProgram);
}

void Model::loadScene(const luapath::Table &modelTable)
{
	m_ModelDir = modelTable.getValue(".modelDir");
	// Read file via ASSIMP
	m_Scene = m_Importer.ReadFile(m_ModelDir, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

	// Check for errors
	if (!m_Scene || m_Scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !m_Scene->mRootNode)
	{
		LOG(ERROR) << "Assimp " << m_Importer.GetErrorString() << endl;
	}	
	//save off the directory where the model is saved because later will need it for loading textures relative to that directory
	m_ModelDir = m_ModelDir.substr(0, m_ModelDir.find_last_of('/'));

}

void Model::loadShaders(const luapath::Table &modelTable)
{
	string vertexShaderName = modelTable.getValue(".vertexShader");
	string fragmentShaderName = modelTable.getValue(".fragmentShader");
	//the shaderProgram with those shaders might already be cached. In any case, we don't care at this point. Its up to the Shader Manager to decide
	m_ShaderProgram = ShaderManager::get().getShaderProgram(vertexShaderName, fragmentShaderName);

}

ShaderProgram* Model::getShaderProgram() const
{
	return m_ShaderProgram;
}
void Model::processNode(const aiNode* node)
{
	for (GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh *mesh = m_Scene->mMeshes[node->mMeshes[i]];
		Mesh *resultMesh = processMesh(mesh);
		//allrighty. what is this, you ask. Even static meshes can have a transformation in their corresponding assimp Node. It was an oversight of me to ignore this transformation when designing the class so the following is a patch up
		bakeTransform(resultMesh,node);
		resultMesh->createVAO(m_ShaderProgram);
		m_Meshes.push_back(resultMesh);

	}
	for (GLuint i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i]);
	}
}

Mesh* Model::processMesh(const aiMesh* mesh)
{
	Mesh* resultMesh = new Mesh();
	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex = retrieveVertex(mesh, i);
		resultMesh->m_Vertices.push_back(vertex);
	}

	retrieveIndices(resultMesh, mesh);
	retrieveMaterials(resultMesh, mesh);

	return resultMesh;
}

Vertex Model::retrieveVertex(const aiMesh* mesh, int i)
{
	Vertex vertex;
	glm::vec3 v;
	//Positions
	v.x = mesh->mVertices[i].x;
	v.y = mesh->mVertices[i].y;
	v.z = mesh->mVertices[i].z;
	vertex.m_Position = v;

	if (mesh->HasNormals())
	{
		// Normals
		v.x = mesh->mNormals[i].x;
		v.y = mesh->mNormals[i].y;
		v.z = mesh->mNormals[i].z;
		vertex.m_Normal = v;
	}


	if (mesh->mTextureCoords[0]) // check if the mesh contains texture coordinates
	{
		glm::vec2 vec;
		// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
		// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
		vec.x = mesh->mTextureCoords[0][i].x;
		vec.y = mesh->mTextureCoords[0][i].y;
		vertex.m_TexCoord = vec;

	}
	else
	{
		vertex.m_TexCoord = glm::vec2(0.0f, 0.0f);
	}
	return vertex;

}

void Model::retrieveIndices(Mesh *resultMesh, const aiMesh* mesh)
{
	//retrieve the mesh faces a.k.a primitives
	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// Retrieve all indices of the face and store them in the indices vector
		for (GLuint j = 0; j < face.mNumIndices; j++)
			resultMesh->m_Indices.push_back(face.mIndices[j]);

	}

}

void Model::retrieveMaterials(Mesh *resultMesh, const aiMesh* mesh)
{
	//Process materials
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial *material = m_Scene->mMaterials[mesh->mMaterialIndex];
		// 1. Diffuse maps
		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE);
		resultMesh->m_Textures.insert(resultMesh->m_Textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. Specular maps
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR);
		resultMesh->m_Textures.insert(resultMesh->m_Textures.end(), specularMaps.begin(), specularMaps.end());

		//get material properties (if any)
		LOG(DEBUG) << "Loading Materials for : " << this->m_Name;

		aiColor4D ambient, diffuse, specular, shininess, shininessStrength;
		ambient = loadMaterialProperties(material, AI_MATKEY_COLOR_AMBIENT);
		diffuse = loadMaterialProperties(material, AI_MATKEY_COLOR_DIFFUSE);
		specular = loadMaterialProperties(material, AI_MATKEY_COLOR_SPECULAR);
		shininess = loadMaterialProperties(material, AI_MATKEY_SHININESS);
		shininessStrength = loadMaterialProperties(material, AI_MATKEY_SHININESS_STRENGTH);

		glm::vec4 result = convertToGLMVec4(ambient);
			resultMesh->m_Material.ambient = result;
		result = convertToGLMVec4(diffuse);
			resultMesh->m_Material.diffuse = result;
		result = convertToGLMVec4(specular);
			resultMesh->m_Material.specular = result;
				// this is the shininess factor not the strength thingy
			resultMesh->m_Material.shininess = shininess.r; // first position of the vector
		result = convertToGLMVec4(shininessStrength);
			resultMesh->m_Material.shininessStrength = result;
		resultMesh->retrieveMaterialLocations(m_ShaderProgram);
	}

}

void Model::bakeTransform(Mesh *resultMesh, const aiNode* node)
{
	glm::mat4 rootTransform = convertToGLMMat4(m_Scene->mRootNode->mTransformation);
	for (int i = 0; i < resultMesh->m_Vertices.size(); i++)
	{
		glm::mat4 transform = rootTransform * convertToGLMMat4(node->mTransformation);
		//position
		glm::vec4 result = transform * glm::vec4(resultMesh->m_Vertices[i].m_Position,1.0f);
		resultMesh->m_Vertices[i].m_Position = glm::vec3(result.x, result.y, result.z); 
		//normal
		result = transform * glm::vec4(resultMesh->m_Vertices[i].m_Normal,1.0f);
		result = glm::normalize(result);
		resultMesh->m_Vertices[i].m_Normal = glm::vec3(result.x, result.y, result.z);
	}
}
aiColor4D Model::loadMaterialProperties(const aiMaterial *mat, const char *aiType, unsigned int type, unsigned int idx)
{
	aiColor4D value(0, 0, 0, 0);
	if (AI_SUCCESS == mat->Get(aiType, type, idx, value) && value != aiColor4D(0, 0, 0, 0))
	{
		//LOG(DEBUG) << "Found material property " << aiType << " : (" << value.r << "," << value.g << "," << value.b << "," << value.a << ")";
	}
	return value;
}

vector<Texture> Model::loadMaterialTextures(const aiMaterial* mat, const aiTextureType type)
{
	vector<Texture> textures;
	unsigned int textureCount = mat->GetTextureCount(type);
	for (GLuint i = 0; i < textureCount; i++)
	{
		aiString str;
		//retrieve the texture relative directory
		mat->GetTexture(type, i, &str);
		// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		GLboolean skip = false;
		for (GLuint j = 0; j < m_TexturesLoaded.size(); j++)
		{
			if (m_TexturesLoaded[j].m_Path == str)
			{
				textures.push_back(m_TexturesLoaded[j]);
				skip = true; // A texture with the same filepath has already been loaded
				break;
			}
		}
		if (!skip)
		{   // If texture hasn't been loaded already, load it
			Texture texture;
			texture.m_Id = textureFromFile(str.C_Str());
			texture.m_Type = type;
			texture.m_Path = str;
			textures.push_back(texture);
			m_TexturesLoaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}
	return textures;
}

GLint Model::textureFromFile(const string &textureDir)
{
	string filepath = m_ModelDir + "/" + textureDir;
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(filepath.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	if (!image)
	{
		LOG(ERROR) << "Could not load image : " << textureDir;
	}
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}

Bone::~Bone()
{
	vector<Bone*>::iterator it = m_Children.begin();
	for (; it != m_Children.end(); ++it)
		delete *it;
}

const Bone* Bone::findBone(int boneIndex) const
{
	if (boneIndex == m_Id) return this;
	for (unsigned int i = 0; i < m_Children.size(); ++i)
	{
		const Bone* p = m_Children[i]->findBone(boneIndex);
		if (p) {
			return p;
		}
	}
	// there is definitely no sub-bone with this name
	return NULL;
}


const Bone* Bone::findBone(const std::string &boneName) const
{
	if (boneName == m_Name) return this;
	for (unsigned int i = 0; i < m_Children.size(); ++i)
	{
		const Bone* p = m_Children[i]->findBone(boneName);
		if (p) {
			return p;
		}
	}
	// there is definitely no sub-bone with this name
	return NULL;

}

SkinnedModel::SkinnedModel(const luapath::Table &modelTable)
	:Model(modelTable.getKey().key)
{
	loadShaders(modelTable);

	loadScene(modelTable);

	loadBones(m_Scene->mRootNode);
	processNode(m_Scene->mRootNode);


	loadAnimations(modelTable);

	m_Importer.FreeScene();

}
SkinnedModel::~SkinnedModel()
{
	delete m_Skeleton;

	AnimMap::iterator it = m_AnimMap.begin();
	for (; it != m_AnimMap.end(); ++it)
		delete it->second;
}

SkinnedModel::LocalTransformMap SkinnedModel::getBoneLocalTransforms() const
{
	return m_LocalTransforms;
}

//this is where the hierarchial calculation happens
SkinnedModel::AbsoluteTransformMap SkinnedModel::getAbsoluteBoneTransforms(const LocalTransformMap &localTransforms, AbsoluteTransformMap &parentTransforms, bool includeIBP) const
{
	AbsoluteTransformMap finalTransforms;
	//finalTransforms.resize(localTransforms.size());
	parentTransforms[m_Skeleton->m_Id] = glm::mat4();
	calculateTransformations(m_Skeleton, glm::mat4(), localTransforms, parentTransforms, finalTransforms, includeIBP);
	return finalTransforms;
}

void SkinnedModel::calculateTransformations(const Bone *currBone,
	const glm::mat4 &parentTransform,
	const LocalTransformMap &localTransforms,
	AbsoluteTransformMap &parentTransforms,
	AbsoluteTransformMap &result,
	bool includeIBP) const
{
	parentTransforms[currBone->m_Id] = parentTransform;
	BoneIdMap::const_iterator it = m_BoneIdMap.find(currBone->m_Name);
	glm::mat4 globalTransform;
	if (it != m_BoneIdMap.end())
	{
		globalTransform = parentTransforms[currBone->m_Id] * localTransforms.at(it->second).getMatrix();
		if (includeIBP)
			result[it->second] = globalTransform * currBone->m_InverseBindPose;
		else
			result[it->second] = globalTransform;
	}
	else
	{
		globalTransform = parentTransforms[currBone->m_Id] * currBone->m_LocalTransformation.getMatrix();
	}
	for (unsigned int i = 0; i < currBone->m_Children.size(); i++)
	{
		calculateTransformations(currBone->m_Children[i], globalTransform, localTransforms, parentTransforms, result, includeIBP);
	}
}

bool SkinnedModel::hasAnimation() const
{
	return m_AnimMap.size() > 0 ? true : false;
}
const Animation* SkinnedModel::getIdleAnimation() const
{
	return m_AnimMap.at(0);
}

/**@brief Retrieve the animation structure with @param animId */
const Animation* SkinnedModel::getAnimation(unsigned int animId) const
{
	return m_AnimMap.at(animId);
}
/**@brief Retrieve the animation structure with @param animName */
const Animation* SkinnedModel::getAnimation(std::string animName) const
{
	return m_AnimMap.at(m_AnimIdMap.at(animName));
}

void SkinnedModel::createBoneLookupMaps(AiBoneMap &aiBoneMap, BoneIdMap &boneIdMap)
{
	unsigned int boneId = 0;
	//create a lookup map of id to aiBone
	int numDuplicates = 0;
	for (unsigned int i = 0; i < m_Scene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = m_Scene->mMeshes[i];
		for (unsigned int n = 0; n < mesh->mNumBones; ++n)
		{
			string boneName = mesh->mBones[n]->mName.data;
			if (boneIdMap.find(boneName) == boneIdMap.end())
			{
				aiBoneMap[boneId] = mesh->mBones[n];
				boneIdMap[boneName] = boneId++;
			}
			else
			{
				numDuplicates++;
				LOG(DEBUG) << boneName;
			}

		}
	}
	int sfdasfdsf = 0;
}

void SkinnedModel::loadBones(const aiNode *rootBone)
{
	AiBoneMap aiBoneMap;
	createBoneLookupMaps(aiBoneMap, m_BoneIdMap);

	//m_LocalTransforms.resize(aiBoneMap.size());
	m_Skeleton = new Bone;
	m_Skeleton->m_Parent = NULL;
	loadBones(rootBone, aiBoneMap, m_Skeleton);

}

void SkinnedModel::loadBones(const aiNode *currNode, const AiBoneMap &aiBoneMap, Bone *currBone)
{
	//assign name, id, local transform
	currBone->m_Name = currNode->mName.data;
	//note that this step decomposes the matrix
	currBone->m_LocalTransformation = convertToSQTTransform(currNode->mTransformation);
	glm::mat4 temp = convertToGLMMat4(currNode->mTransformation);
	//if the current node is a bone
	BoneIdMap::const_iterator it = m_BoneIdMap.find(currBone->m_Name);
	if(it != m_BoneIdMap.end())
	{
		currBone->m_Id = it->second;
		//currBone->m_Id = m_BoneIdMap[currBone->m_Name];
		currBone->m_InverseBindPose = convertToGLMMat4(aiBoneMap.at(it->second)->mOffsetMatrix);
		m_LocalTransforms[it->second] = currBone->m_LocalTransformation;
	}
	else
	{
		currBone->m_Id = -1;
	}
	for (int i = 0; i < currNode->mNumChildren; i++)
	{
		Bone *newChild = new Bone;
		newChild->m_Parent = currBone;
		currBone->m_Children.push_back(newChild);
		loadBones(currNode->mChildren[i], aiBoneMap, newChild);
	}
}

void SkinnedModel::printAnimHierarchy() const
{
	for (int i = 0; i < m_Scene->mNumAnimations; i++)
	{
		LOG(DEBUG) << "\t Animation name : " << m_Scene->mAnimations[i]->mName.data;
		LOG(DEBUG) << "\t Animation duration : " << m_Scene->mAnimations[i]->mDuration;
		LOG(DEBUG) << "\t Animation Ticks per second : " << m_Scene->mAnimations[i]->mTicksPerSecond;
		for (int j = 0; j < m_Scene->mAnimations[i]->mNumChannels; j++)
		{
			const aiNodeAnim *currNode = m_Scene->mAnimations[i]->mChannels[j];
			LOG(DEBUG) << "\t Channel name : " << currNode->mNodeName.data;
			LOG(DEBUG) << "\t number of pos keys: " << m_Scene->mAnimations[i]->mChannels[j]->mNumPositionKeys;
			LOG(DEBUG) << "\t number of rot keys: " << m_Scene->mAnimations[i]->mChannels[j]->mNumRotationKeys;
			LOG(DEBUG) << "\t number of scale keys: " << m_Scene->mAnimations[i]->mChannels[j]->mNumScalingKeys;

			for (int k = 0; k < currNode->mNumPositionKeys; k++)
			{
				const aiVectorKey &currKey = currNode->mPositionKeys[k];
				LOG(DEBUG) << "\t\t poskey" << k << " -- time : " << currKey.mTime << " -- value: (" << currKey.mValue.x << ", " << currKey.mValue.y << ", " << currKey.mValue.z << ")";
			}
			for (int k = 0; k < currNode->mNumScalingKeys; k++)
			{
				const aiVectorKey &currKey = currNode->mScalingKeys[k];
				LOG(DEBUG) << "\t\t scalekey" << k << " -- time : " << currKey.mTime << " -- value: (" << currKey.mValue.x << ", " << currKey.mValue.y << ", " << currKey.mValue.z << ")";
			}
			for (int k = 0; k < currNode->mNumRotationKeys; k++)
			{
				const aiQuatKey &currKey = currNode->mRotationKeys[k];
				LOG(DEBUG) << "\t\t rotkey" << k << " -- time : " << currKey.mTime << " -- value: (" << currKey.mValue.x << ", " << currKey.mValue.y << ", " << currKey.mValue.z << ", " << currKey.mValue.w << ")";
			}
		}
	}

}

void SkinnedModel::loadAnimations(const luapath::Table &modelTable)
{
	unsigned int numAnim = 0;
	if (!m_Scene->HasAnimations())
		return;

	luapath::Value rootAnimValue;
	string rootAnimName = "";
	if (modelTable.getValue(".animationName", rootAnimValue))
	{
		rootAnimName = rootAnimValue;
		m_IdleAnimationId = numAnim;
	}

	//load the animations from the root model
	for (int i = 0; i < m_Scene->mNumAnimations; i++)
	{
		loadAnimation(m_Scene->mAnimations[i], rootAnimName, numAnim);
		numAnim++;
	}
	//load the animations from the additional animation files specified in the settings
	luapath::Table additionalAnimTable;
	if (modelTable.getTable(".additionalAnimations", additionalAnimTable))
	{
		int additionalAnimNum = 1; // lua indexing is not 0 based
		luapath::Table currAnim;
		Assimp::Importer importer;
		while (additionalAnimTable.getTable(string("#") + std::to_string(additionalAnimNum), currAnim))
		{
			string animDir = currAnim.getValue(".fileDir");
			luapath::Value animValue;
			string animName = "";
			if (currAnim.getValue(".animationName", animValue))
				animName = animValue;

			const aiScene *currScene;
			currScene = importer.ReadFile(animDir, 0); // no optimizations performed

			// Check for errors
			if (!currScene || currScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !currScene->mRootNode)
			{
				LOG(ERROR) << "Assimp " << importer.GetErrorString() << endl;
			}

			for (int i = 0; i < currScene->mNumAnimations; i++)
			{
				loadAnimation(currScene->mAnimations[i], animName, numAnim);
				numAnim++;
			}
			importer.FreeScene();
			additionalAnimNum++;
		}
	}
}

void SkinnedModel::loadAnimation(const aiAnimation *currAnim, const std::string &animName, int numAnim)
{
	Animation *newAnim = new Animation;
	//newAnim->m_BoneAnim.resize(m_BoneIdMap.size());
	if (animName == "")
		newAnim->m_Name = currAnim->mName.data;
	else
		newAnim->m_Name = animName;
		
	newAnim->m_Id = numAnim;
	float tps = currAnim->mTicksPerSecond;
	if (tps == 0)
	{
		LOG(ERROR) << "animation : " << newAnim->m_Name << " for model : " << m_Name << " has no ticks per second specified ";
		tps = 25.0;
	}

	newAnim->m_TicksPerSecond = tps;
	newAnim->m_TotalTicks = currAnim->mDuration;
	newAnim->m_TotalDuration = newAnim->m_TotalTicks / newAnim->m_TicksPerSecond;
	newAnim->m_NumBones = m_LocalTransforms.size();
	//push new animation in the map
	m_AnimMap[numAnim] = newAnim;
	// and the id in the other map
	m_AnimIdMap[newAnim->m_Name] = numAnim;

	//now for each channel/bone of the animation
	for (int j = 0; j < currAnim->mNumChannels; j++)
	{
		const aiNodeAnim *currNode = currAnim->mChannels[j];
		const string boneName = currNode->mNodeName.data;
		//we are using "at" function. It will throw an exception if not existing
		BoneIdMap::const_iterator it = m_BoneIdMap.find(boneName);
		if(it != m_BoneIdMap.end())
		{
			int boneId = m_BoneIdMap[boneName];
			// 1:1 mapping from assimp to glm (for now)
			BoneAnim *newBoneAnim = new BoneAnim(currNode);
			newAnim->m_BoneAnim[boneId] = newBoneAnim;
		}

	}
}

unsigned int SkinnedModel::findBoneIndex(const std::string &boneName) const
{
	BoneIdMap::const_iterator it = m_BoneIdMap.find(boneName);
	if(it != m_BoneIdMap.end())
		return it->second;
	return -1;
}


void SkinnedModel::processNode(const aiNode* node)
{

	for (GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = m_Scene->mMeshes[node->mMeshes[i]];
		m_Meshes.push_back(processMesh(mesh));

	}
	for (GLuint i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i]);
	}

}


Mesh* SkinnedModel::processMesh(const aiMesh* mesh)
{
	if (!mesh->HasBones())
	{
		LOG(WARN) << "Loading a mesh without bones for a supposed skinned model" << std::endl;
		return Model::processMesh(mesh);
	}

	SkinnedMesh* resultMesh = new SkinnedMesh();

	// collect weights on all vertices for filling up the SkinnedVertex later
	std::vector<std::vector<aiVertexWeight> > vertexWeights(mesh->mNumVertices);
	for (GLuint a = 0; a < mesh->mNumBones; a++)	{
		const aiBone* bone = mesh->mBones[a];
		for (unsigned int b = 0; b < bone->mNumWeights; b++)
			vertexWeights[bone->mWeights[b].mVertexId].push_back(aiVertexWeight(m_BoneIdMap.at(bone->mName.data), bone->mWeights[b].mWeight));
	}

	for (GLuint v = 0; v < mesh->mNumVertices; v++)
	{
		std::vector<aiVertexWeight> currVertex = vertexWeights[v];
		glm::vec4 boneWeights;
		glm::uvec4 boneIndices;
		int currVertexSize = currVertex.size();
		//no more than 4 bones allowed for now
		for (GLuint b = 0; b < currVertexSize && b < 4; b++)
		{
			boneWeights[b] = currVertex[b].mWeight;
			boneIndices[b] = currVertex[b].mVertexId; // actually mVertexId here stands for the boneIndex. Make sure not to be confused in the future
		}
		//...start of hack: the following code distributes any "spurious" influences (i.e over 4 bones) proportionally across the first 4 bones.
		if(currVertexSize > 4) 
		{
			float goodBoneProportion[4];
			for (int i = 0; i < 4; i++)
			{
				goodBoneProportion[i] = boneWeights[i] * (boneWeights[0]+boneWeights[1]+boneWeights[2]+boneWeights[3]);
			}
			// now distribute bad bones
			for (int i = 4; i < currVertexSize; i++)
			{
				boneWeights[0] += currVertex[i].mWeight * goodBoneProportion[0];
				boneWeights[1] += currVertex[i].mWeight * goodBoneProportion[1];
				boneWeights[2] += currVertex[i].mWeight * goodBoneProportion[2];
				boneWeights[3] += currVertex[i].mWeight * goodBoneProportion[3];
			}
			float newWeight = boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w;
		    boneWeights.x += (1 - newWeight - 0.00001f);
		//...end of hack
		}

		SkinnedVertex vertex = retrieveVertex(mesh, v);
		vertex.m_BoneIndices = boneIndices;
		vertex.m_BoneWeights = boneWeights;

		resultMesh->m_SkinnedVertices.push_back(vertex);
	}



	//at this stage we have the mesh vertices constructed
	retrieveIndices(resultMesh, mesh);
	retrieveMaterials(resultMesh, mesh);

	resultMesh->createVAO(m_ShaderProgram);
	return resultMesh;
}



