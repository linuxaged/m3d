/*
* Copyright (C) 2017 Tracy Ma
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#include "Scene.hpp"
#include "File.hpp"

#include "../../data/schema/scene_generated.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include <fbxsdk.h>

using namespace m3d::schema;

#define TRIANGLE_VERTEX_COUNT 3
#define VERTEX_STRIDE 4
#define NORMAL_STRIDE 3
#define UV_STRIDE 2

namespace m3d {
bool Mesh::init(FbxMesh* pFbxMesh)
{
    uint32_t normalCount = pFbxMesh->GetElementNormalCount();
    uint32_t uvCount = pFbxMesh->GetElementUVCount();
    FbxGeometryElement::EMappingMode normalMappingMode = normalCount ? pFbxMesh->GetElementNormal(0)->GetMappingMode()
                                                                     : FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode uvMappingModel = uvCount ? pFbxMesh->GetElementUV(0)->GetMappingMode()
                                                              : FbxGeometryElement::eNone;

    bool hasNormal = normalMappingMode != FbxGeometryElement::eNone;
    bool hasUV = uvMappingModel != FbxGeometryElement::eNone;

    bool byControlPoint = hasNormal && normalMappingMode == FbxGeometryElement::eByControlPoint && hasUV && uvMappingModel == FbxGeometryElement::eByControlPoint;

    uint32_t polygonCount = pFbxMesh->GetPolygonCount();
    uint32_t controlPointCount = byControlPoint
        ? pFbxMesh->GetControlPointsCount()
        : polygonCount * TRIANGLE_VERTEX_COUNT;

    this->vertices.resize(controlPointCount * VERTEX_STRIDE);
    this->indices.resize(polygonCount * TRIANGLE_VERTEX_COUNT);
    if (hasNormal)
        this->normals.resize(controlPointCount * NORMAL_STRIDE);

    FbxStringList uvNames;
    pFbxMesh->GetUVSetNames(uvNames);
    const char* pUVName = nullptr;
    if (hasUV) {
        this->uvs.resize(controlPointCount * UV_STRIDE);
        pUVName = uvNames[0];
    }

    /* Vertex Attributes */
    const FbxVector4* pControlPoints = pFbxMesh->GetControlPoints();
    FbxVector4 currentVertex;
    FbxVector4 currentNormal;
    FbxVector2 currentUV;
    if (byControlPoint) {
        const FbxGeometryElementNormal* pNormalElement = nullptr;
        const FbxGeometryElementUV* pUVElement = nullptr;
        if (hasNormal)
            pNormalElement = pFbxMesh->GetElementNormal(0);
        if (hasUV)
            pUVElement = pFbxMesh->GetElementUV(0);

        for (uint32_t i = 0; i < controlPointCount; ++i) {
            currentVertex = pControlPoints[i];
            this->vertices[i * VERTEX_STRIDE] = static_cast<float>(currentVertex[0]);
            this->vertices[i * VERTEX_STRIDE + 1] = static_cast<float>(currentVertex[1]);
            this->vertices[i * VERTEX_STRIDE + 2] = static_cast<float>(currentVertex[2]);
            this->vertices[i * VERTEX_STRIDE + 3] = 1.0f;

            if (hasNormal) {
                int normalIndex = i;
                if (pNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect) {
                    normalIndex = pNormalElement->GetIndexArray().GetAt(i);
                }
                currentNormal = pNormalElement->GetDirectArray().GetAt(normalIndex);
                this->normals[i * NORMAL_STRIDE] = static_cast<float>(currentNormal[0]);
                this->normals[i * NORMAL_STRIDE + 1] = static_cast<float>(currentNormal[1]);
                this->normals[i * NORMAL_STRIDE + 2] = static_cast<float>(currentNormal[2]);
            }

            if (hasUV) {
                int uvIndex = i;
                if (pUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect) {
                    uvIndex = pUVElement->GetIndexArray().GetAt(i);
                }
                currentUV = pUVElement->GetDirectArray().GetAt(uvIndex);
                this->uvs[i * UV_STRIDE] = static_cast<float>(currentUV[0]);
                this->uvs[i * UV_STRIDE + 1] = static_cast<float>(currentUV[1]);
            }
        }
    } // end of byControlPoint

    /* Slice the mesh according to materials */
    FbxLayerElementArrayTemplate<int>* pMaterialIndices = nullptr;
    FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;
    if (pFbxMesh->GetElementMaterial()) {
        pMaterialIndices = &pFbxMesh->GetElementMaterial()->GetIndexArray();
        materialMappingMode = pFbxMesh->GetElementMaterial()->GetMappingMode();
        if (pMaterialIndices && materialMappingMode == FbxGeometryElement::eByPolygon) {
            FBX_ASSERT(pMaterialIndices->GetCount() == polygonCount);
            for (uint32_t i = 0; i < polygonCount; ++i) {
                const uint32_t materialIndex = pMaterialIndices->GetAt(i);
                if (materialIndex >= slices.size()) // we meet a new material
                {
                    slices.emplace_back(0, 0);
                }
                slices[materialIndex].triangleCount += 1;
            }

            int offset = 0;
            for (uint32_t i = 0; i < slices.size(); ++i) {
                slices[i].indexOffset = offset;
                offset += slices[i].triangleCount * 3;
                // this will be reused during per-Polygon processing
                slices[i].triangleCount = 0;
            }
            FBX_ASSERT(offset == polygonCount * 3);
        }
    }

    // There is only one material.
    if (slices.size() == 0) {
        slices.emplace_back(0, 0);
    }

    /* Indices */
    for (uint32_t vertexCount = 0, i = 0; i < polygonCount; ++i) {
        int materialIndex = 0;
        if (pMaterialIndices && materialMappingMode == FbxGeometryElement::eByPolygon) {
            materialIndex = pMaterialIndices->GetAt(i);
        }
        const int indexOffset = slices[materialIndex].indexOffset + slices[materialIndex].triangleCount * 3;
        for (int v = 0; v < TRIANGLE_VERTEX_COUNT; ++v) {
            const int controlPointIndex = pFbxMesh->GetPolygonVertex(i, v);

            if (byControlPoint) {
                this->indices[indexOffset + v] = static_cast<unsigned int>(controlPointIndex);
            } else {
                this->indices[indexOffset + v] = static_cast<unsigned int>(vertexCount);

                currentVertex = pControlPoints[controlPointIndex];
                this->vertices[vertexCount * VERTEX_STRIDE] = static_cast<float>(currentVertex[0]);
                this->vertices[vertexCount * VERTEX_STRIDE + 1] = static_cast<float>(currentVertex[1]);
                this->vertices[vertexCount * VERTEX_STRIDE + 2] = static_cast<float>(currentVertex[2]);
                this->vertices[vertexCount * VERTEX_STRIDE + 3] = 1.0f;

                if (hasNormal) {
                    pFbxMesh->GetPolygonVertexNormal(i, v, currentNormal);
                    this->normals[vertexCount * NORMAL_STRIDE] = static_cast<float>(currentNormal[0]);
                    this->normals[vertexCount * NORMAL_STRIDE + 1] = static_cast<float>(currentNormal[1]);
                    this->normals[vertexCount * NORMAL_STRIDE + 2] = static_cast<float>(currentNormal[2]);
                }

                if (hasUV) {
                    bool bUnmappedUV;
                    pFbxMesh->GetPolygonVertexUV(i, v, pUVName, currentUV, bUnmappedUV);
                    this->uvs[vertexCount * UV_STRIDE] = static_cast<float>(currentUV[0]);
                    this->uvs[vertexCount * UV_STRIDE] = static_cast<float>(currentUV[1]);
                }
            }
            ++vertexCount;
        }
        slices[materialIndex].triangleCount += 1;
    }

    return true;
}

Scene::Scene() {}

void Scene::Init()
{
    std::vector<uint8_t> sceneData;
    m3d::file::readBinary("D:\\workspace\\m3d\\data\\schema\\scene_data.bin", sceneData);
    auto mainScene = GetSScene(sceneData.data());
    loadPath = mainScene->models()->Get(0)->name()->str();
    printf("fbx path: %s", loadPath.c_str());

    diffuseMaps = packed_freelist<DiffuseMap>(512);
    materials = packed_freelist<Material>(512);
    meshes = packed_freelist<Mesh>(512);
    transforms = packed_freelist<Transform>(4096);
    instances = packed_freelist<Instance>(4096);
    cameras = packed_freelist<Camera>(32);
}

void LoadMeshes(FbxNode* pFbxNode, packed_freelist<Mesh>& sceneMeshes)
{
    // Material
    const uint32_t materialCount = pFbxNode->GetMaterialCount();
    for (uint32_t i = 0; i < materialCount; ++i) {
        FbxSurfaceMaterial* pFbxMaterial = pFbxNode->GetMaterial(i);
        if (pFbxMaterial && !pFbxMaterial->GetUserDataPtr()) {
            FbxAutoPtr<Material> pMaterial(new Material);
            if (pMaterial->init(pFbxMaterial)) {
                pFbxMaterial->SetUserDataPtr(pMaterial.Release());
            }
        }
    }

    FbxNodeAttribute* nodeAttribute = pFbxNode->GetNodeAttribute();
    if (nodeAttribute) {
        // Mesh
        if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
            FbxMesh* pFbxMesh = pFbxNode->GetMesh();
            if (pFbxMesh && !pFbxMesh->GetUserDataPtr()) {
                Mesh mesh;
                if (mesh.init(pFbxMesh)) {
                    sceneMeshes.insert(mesh);
                }
                // TODO:
                FbxAutoPtr<Mesh> pMesh(new Mesh);
                if (pMesh->init(pFbxMesh)) {
                    pFbxMesh->SetUserDataPtr(pMesh.Release());
                }
            }
        }
        // Light
        else if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eLight) {
            FbxLight* pFbxLight = pFbxNode->GetLight();
            if (pFbxLight && !pFbxLight->GetUserDataPtr()) {
                FbxAutoPtr<Light> pLight(new Light);
                if (pLight->init(pFbxLight)) {
                    pFbxLight->SetUserDataPtr(pLight.Release());
                }
            }
        }
    }

    const int childCount = pFbxNode->GetChildCount();
    for (int i = 0; i < childCount; ++i) {
        LoadMeshes(pFbxNode->GetChild(i), sceneMeshes);
    }
}

void LoadMeshes(Scene* pScene, std::vector<uint32_t>* loadedMeshIDs)
{
    FbxManager* fbxManager = FbxManager::Create();

    FbxIOSettings* pFbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
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

    // Initialize the importer.
    bool result = pFbxImporter->Initialize(pScene->loadPath.c_str(), -1, fbxManager->GetIOSettings());
    if (!result) {
        printf("Get error when init FBX Importer: %s\n\n",
            pFbxImporter->GetStatus().GetErrorString());
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
    FbxAxisSystem vulkanAxisSystem(FbxAxisSystem::eYAxis,
        FbxAxisSystem::eParityOdd,
        FbxAxisSystem::eRightHanded);
    if (axisSystem != vulkanAxisSystem) {
        axisSystem.ConvertScene(pFbxScene);
    }

    // check unit system
    FbxSystemUnit systemUnit = pFbxScene->GetGlobalSettings().GetSystemUnit();
    if (systemUnit.GetScaleFactor() != 1.0) {
        FbxSystemUnit::cm.ConvertScene(pFbxScene);
    }

    // Triangulate Mesh
    FbxGeometryConverter fbxGeometryConverter(fbxManager);
    fbxGeometryConverter.Triangulate(pFbxScene, true);

    // Load Texture
    int textureCount = pFbxScene->GetTextureCount();
    for (int i = 0; i < textureCount; ++i) {
        FbxTexture* pFbxTexture = pFbxScene->GetTexture(i);
        FbxFileTexture* pFbxFileTexture = FbxCast<FbxFileTexture>(pFbxTexture);
        if (pFbxTexture && pFbxFileTexture->GetUserDataPtr()) {
        }
    }
    LoadMeshes(pFbxScene->GetRootNode(), pScene->meshes);
}

void AddInstance(Scene& pFbxScene, uint32_t meshID, uint32_t* newInstanceID)
{
    Transform newTransform;
    newTransform.scale = m3d::math::Vector3(1.0f, 1.0f, 1.0f);

    uint32_t newTransformID = pFbxScene.transforms.insert(newTransform);

    Instance newInstance;
    newInstance.meshId = meshID;
    newInstance.transformId = newTransformID;

    uint32_t tmpNewInstanceID = pFbxScene.instances.insert(newInstance);
    if (newInstanceID) {
        *newInstanceID = tmpNewInstanceID;
    }
}
} // End of namespace m3d
