/*
* Copyright (C) 2016 Tracy Ma
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <string>
#include <vector>

#include "Matrix.h"
#include "Quaternion.h"

#include "packed_freelist.h"
#include "vulkanTextureLoader.hpp"

namespace fbxsdk {
class FbxScene;
class FbxLight;
class FbxSurfaceMaterial;
class FbxMesh;
}

struct DiffuseMap {
    vkext::VulkanTexture texture;
};

struct Light {
    bool init(fbxsdk::FbxLight* fbxLight) { return true; }
};

struct Material {
    bool init(fbxsdk::FbxSurfaceMaterial* fbxMaterial)
    {
        // TODO
        return true;
    }
    std::string name;
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float shininess;
    uint32_t diffuseMapId;
};

struct Mesh {
    bool init(fbxsdk::FbxMesh* fbxMesh)
    {
        // TODO
        return true;
    }
    std::string name;

    std::vector<float> vertices;
    std::vector<float> uvs;
    std::vector<float> normals;
    std::vector<uint32_t> indices;

    std::vector<vk::CommandBuffer> drawCommands;
    std::vector<uint32_t> materialIds;
};

struct Transform {
    m3d::math::Vector3 position;
    m3d::math::Vector3 scale;
    m3d::math::Quaternion rotation;
};

struct Instance {
    uint32_t meshId;
    uint32_t transformId;
};

struct Camera {
    // view
    m3d::math::Vector3 eye;
    m3d::math::Vector3 target;
    m3d::math::Vector3 up;
    // Proj
    float fovY;
    float aspect;
    float nearZ;
};

class Scene {
public:
    packed_freelist<DiffuseMap> diffuseMaps;
    packed_freelist<Material> materials;
    packed_freelist<Mesh> meshes;
    packed_freelist<Transform> transforms;
    packed_freelist<Instance> instances;
    packed_freelist<Camera> cameras;

    uint32_t mainCameraID;

    Scene();
    void Init();
};

void LoadMeshes(fbxsdk::FbxScene* scene, const char* filename,
    std::vector<uint32_t>* loadedMeshIDs);

void AddInstance(Scene& scene, uint32_t meshID, uint32_t* newInstanceID);
