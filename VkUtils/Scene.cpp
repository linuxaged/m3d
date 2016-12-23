/*
* Copyright (C) 2016 Tracy Ma
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#include "Scene.hpp"
#include "tiny_obj_loader.h"

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "../data/schema/scene_generated.h"


#include <fbxsdk.h>

using namespace m3d::schema;

#define TRIANGLE_VERTEX_COUNT 3
#define VERTEX_STRIDE 4
#define NORMAL_STRIDE 3
#define UV_STRIDE 2

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

    float* pVertices = new float[controlPointCount * VERTEX_STRIDE];
    uint32_t* pIndices = new uint32_t[polygonCount * TRIANGLE_VERTEX_COUNT];

    float* pNormals = nullptr;
    if (hasNormal)
        pNormals = new float[controlPointCount * NORMAL_STRIDE];

    float* pUVs = nullptr;
    FbxStringList uvNames;
    pFbxMesh->GetUVSetNames(uvNames);
    const char* pUVName = nullptr;
    if (hasUV) {
        pUVs = new float[controlPointCount * UV_STRIDE];
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
            pVertices[i * VERTEX_STRIDE] = static_cast<float>(currentVertex[0]);
            pVertices[i * VERTEX_STRIDE + 1] = static_cast<float>(currentVertex[1]);
            pVertices[i * VERTEX_STRIDE + 2] = static_cast<float>(currentVertex[2]);
            pVertices[i * VERTEX_STRIDE + 3] = 1.0f;
            // TODO
            this->vertices.emplace_back(static_cast<float>(currentVertex[0]));
            this->vertices.emplace_back(static_cast<float>(currentVertex[1]));
            this->vertices.emplace_back(static_cast<float>(currentVertex[2]));
            this->vertices.emplace_back(static_cast<float>(currentVertex[3]));

            if (hasNormal) {
                int normalIndex = i;
                if (pNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect) {
                    normalIndex = pNormalElement->GetIndexArray().GetAt(i);
                }
                currentNormal = pNormalElement->GetDirectArray().GetAt(normalIndex);
                pNormals[i * NORMAL_STRIDE] = static_cast<float>(currentNormal[0]);
                pNormals[i * NORMAL_STRIDE + 1] = static_cast<float>(currentNormal[1]);
                pNormals[i * NORMAL_STRIDE + 2] = static_cast<float>(currentNormal[2]);
            }

            if (hasUV) {
                int uvIndex = i;
                if (pUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect) {
                    uvIndex = pUVElement->GetIndexArray().GetAt(i);
                }
                currentUV = pUVElement->GetDirectArray().GetAt(uvIndex);
                pUVs[i * UV_STRIDE] = static_cast<float>(currentUV[0]);
                pUVs[i * UV_STRIDE + 1] = static_cast<float>(currentUV[1]);
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
                pIndices[indexOffset + v] = static_cast<unsigned int>(controlPointIndex);
            } else {
                pIndices[indexOffset + v] = static_cast<unsigned int>(vertexCount);

                currentVertex = pControlPoints[controlPointIndex];
                pVertices[vertexCount * VERTEX_STRIDE] = static_cast<float>(currentVertex[0]);
                pVertices[vertexCount * VERTEX_STRIDE + 1] = static_cast<float>(currentVertex[1]);
                pVertices[vertexCount * VERTEX_STRIDE + 2] = static_cast<float>(currentVertex[2]);
                pVertices[vertexCount * VERTEX_STRIDE + 3] = 1.0f;
                // TODO
                this->vertices.emplace_back(static_cast<float>(currentVertex[0]));
                this->vertices.emplace_back(static_cast<float>(currentVertex[1]));
                this->vertices.emplace_back(static_cast<float>(currentVertex[2]));
                this->vertices.emplace_back(static_cast<float>(currentVertex[3]));

                if (hasNormal) {
                    pFbxMesh->GetPolygonVertexNormal(i, v, currentNormal);
                    pNormals[vertexCount * NORMAL_STRIDE] = static_cast<float>(currentNormal[0]);
                    pNormals[vertexCount * NORMAL_STRIDE + 1] = static_cast<float>(currentNormal[1]);
                    pNormals[vertexCount * NORMAL_STRIDE + 2] = static_cast<float>(currentNormal[2]);
                }

                if (hasUV) {
                    bool bUnmappedUV;
                    pFbxMesh->GetPolygonVertexUV(i, v, pUVName, currentUV, bUnmappedUV);
                    pUVs[vertexCount * UV_STRIDE] = static_cast<float>(currentUV[0]);
                    pUVs[vertexCount * UV_STRIDE] = static_cast<float>(currentUV[1]);
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
    std::string schemafile;
    std::string jsonfile;
    bool ok = flatbuffers::LoadFile("G:\\workspace\\m3d\\data\\schema\\scene.fbs", false, &schemafile) && flatbuffers::LoadFile("G:\\workspace\\m3d\\data\\schema\\scenedata.json", false, &jsonfile);
    if (!ok) {
        printf("couldn't load files!\n");
    }

    flatbuffers::Parser parser;
    const char* include_directories[] = { "G:\\workspace\\m3d\\data\\schema", nullptr };
    ok = parser.Parse(schemafile.c_str(), include_directories) && parser.Parse(jsonfile.c_str(), include_directories);
    assert(ok);

    std::string jsongen;

	ok = GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);

    if (jsongen != jsonfile) {
        printf("%s----------------\n%s", jsongen.c_str(), jsonfile.c_str());
    }


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

void LoadMeshes(Scene* pScene, const char* sceneFileName,
    std::vector<uint32_t>* loadedMeshIDs)
{
    // TODO: load texture
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
    bool result = pFbxImporter->Initialize(sceneFileName, -1, fbxManager->GetIOSettings());
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