/*
* Copyright (C) 2016 Tracy Ma
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "Scene.hpp"
#include "tiny_obj_loader.h"

#include <fbxsdk.h>

Scene::Scene()
{
	FbxManager* fbxManager = FbxManager::Create();

	FbxIOSettings * pFbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
	fbxManager->SetIOSettings(pFbxIOSettings);

	(*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL, true);
	(*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE, true);
	(*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_LINK, false);
	(*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE, false);
	(*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO, false);
	(*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION, true);
	(*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);


	bool bEmbedMedia = true;
	(*(fbxManager->GetIOSettings())).SetBoolProp(EXP_FBX_MATERIAL, true);
	(*(fbxManager->GetIOSettings())).SetBoolProp(EXP_FBX_TEXTURE, true);
	(*(fbxManager->GetIOSettings())).SetBoolProp(EXP_FBX_EMBEDDED, bEmbedMedia);
	(*(fbxManager->GetIOSettings())).SetBoolProp(EXP_FBX_SHAPE, true);
	(*(fbxManager->GetIOSettings())).SetBoolProp(EXP_FBX_GOBO, true);
	(*(fbxManager->GetIOSettings())).SetBoolProp(EXP_FBX_ANIMATION, true);
	(*(fbxManager->GetIOSettings())).SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

	FbxImporter* pFbxImporter = FbxImporter::Create(fbxManager, "");

	const char* fileName = "file.fbx";

	// Initialize the importer.
	bool result = pFbxImporter->Initialize(fileName, -1, fbxManager->GetIOSettings());
	if (!result)
	{
		printf("Get error when init FBX Importer: %s\n\n", pFbxImporter->GetStatus().GetErrorString());
		exit(-1);
	}

	// fbx version number
	int major, minor, revision;
	pFbxImporter->GetFileVersion(major, minor, revision);

	// import pFbxScene
	FbxScene* pFbxScene = FbxScene::Create(fbxManager, "myScene");
	pFbxImporter->Import(pFbxScene);
	pFbxImporter->Destroy();
	pFbxImporter = nullptr;

	// check axis system
	FbxAxisSystem axisSystem = pFbxScene->GetGlobalSettings().GetAxisSystem();
	FbxAxisSystem vulkanAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
	if (axisSystem != vulkanAxisSystem)
	{
		axisSystem.ConvertScene(pFbxScene);
	}

	// check unit system
	FbxSystemUnit systemUnit = pFbxScene->GetGlobalSettings().GetSystemUnit();
	if (systemUnit.GetScaleFactor() != 1.0)
	{
		FbxSystemUnit::cm.ConvertScene(pFbxScene);
	}

	// Triangulate Mesh
	FbxGeometryConverter fbxGeometryConverter(fbxManager);
	fbxGeometryConverter.Triangulate(pFbxScene, true);

	// TODO:
	std::vector<uint32_t> loadedMeshIds = {0};
	LoadMeshes(pFbxScene, fileName, &loadedMeshIds);
}

void Scene::Init()
{
	diffuseMaps = packed_freelist<DiffuseMap>(512);
	materials = packed_freelist<Material>(512);
	meshes = packed_freelist<Mesh>(512);
	transforms = packed_freelist<Transform>(4096);
	instances = packed_freelist<Instance>(4096);
	cameras = packed_freelist<Camera>(32);
}

void LoadMeshes(FbxNode *pFbxNode)
{
	// Material
	const int materialCount = pFbxNode->GetMaterialCount();
	for (int i = 0; i < materialCount; ++i)
	{
		FbxSurfaceMaterial *pFbxMaterial = pFbxNode->GetMaterial(i);
		if (pFbxMaterial && !pFbxMaterial->GetUserDataPtr())
		{
			FbxAutoPtr<Material> pMaterial(new Material);
			if (pMaterial->init(pFbxMaterial))
			{
				pFbxMaterial->SetUserDataPtr(pMaterial.Release());
			}
		}
	}

	FbxNodeAttribute *nodeAttribute = pFbxNode->GetNodeAttribute();
	if (nodeAttribute)
	{
		// Mesh
		if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			FbxMesh *pFbxMesh = pFbxNode->GetMesh();
			if (pFbxMesh && !pFbxMesh->GetUserDataPtr())
			{
				FbxAutoPtr<Mesh> pMesh(new Mesh);
				if (pMesh->init(pFbxMesh))
				{
					pFbxMesh->SetUserDataPtr(pMesh.Release());
				}
			}
		}
		// Light
		else if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eLight)
		{
			FbxLight *pFbxLight = pFbxNode->GetLight();
			if (pFbxLight && !pFbxLight->GetUserDataPtr())
			{
				FbxAutoPtr<Light> pLight(new Light);
				if (pLight->init(pFbxLight))
				{
					pFbxLight->SetUserDataPtr(pLight.Release());
				}
			}
		}
	}

	const int childCount = pFbxNode->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		LoadMeshes(pFbxNode->GetChild(i));
	}
}

void LoadMeshes(
	FbxScene* pFbxScene,
	const char* filename,
	std::vector<uint32_t>* loadedMeshIDs)
{
	// TODO: load texture

	LoadMeshes(pFbxScene->GetRootNode());
}

void AddInstance(
	Scene& pFbxScene,
	uint32_t meshID,
	uint32_t* newInstanceID)
{
	Transform newTransform;
	newTransform.scale = m3d::math::Vector3(1.0f, 1.0f, 1.0f);

	uint32_t newTransformID = pFbxScene.transforms.insert(newTransform);

	Instance newInstance;
	newInstance.meshId = meshID;
	newInstance.transformId = newTransformID;

	uint32_t tmpNewInstanceID = pFbxScene.instances.insert(newInstance);
	if (newInstanceID)
	{
		*newInstanceID = tmpNewInstanceID;
	}
}