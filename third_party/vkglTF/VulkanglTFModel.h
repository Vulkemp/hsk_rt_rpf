/**
 * Vulkan glTF model and texture loading class based on tinyglTF (https://github.com/syoyo/tinygltf)
 *
 * Copyright (C) 2018-2021 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#pragma once

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include <vma/vk_mem_alloc.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <gli/gli.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// ERROR is already defined in wingdi.h and collides with a define in the Draco headers
#if defined(_WIN32) && defined(ERROR) && defined(TINYGLTF_ENABLE_DRACO)
#undef ERROR
#pragma message("ERROR constant already defined, undefining")
#endif

#include "tinygltf/tiny_gltf.h"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

// Changing this value here also requires changing it in the vertex shader
#define MAX_NUM_JOINTS 128u

namespace vkglTF {
    struct Node;

    struct BoundingBox
    {
        glm::vec3 min;
        glm::vec3 max;
        bool      valid = false;
        BoundingBox();
        BoundingBox(glm::vec3 min, glm::vec3 max);
        BoundingBox getAABB(glm::mat4 m);
    };

    struct TextureSampler
    {
        VkFilter             magFilter;
        VkFilter             minFilter;
        VkSamplerAddressMode addressModeU;
        VkSamplerAddressMode addressModeV;
        VkSamplerAddressMode addressModeW;
    };

    struct Texture
    {
        VmaAllocator          allocator;
        VmaAllocation         allocation;
        VkDevice              device;
        VkImage               image;
        VkImageLayout         imageLayout;
        VkImageView           view;
        uint32_t              width, height;
        uint32_t              mipLevels;
        uint32_t              layerCount;
        VkDescriptorImageInfo descriptor;
        VkSampler             sampler;
        void                  updateDescriptor();
        void                  destroy();
        // Load a texture from a glTF image (stored as vector of chars loaded via stb_image) and generate a full mip chaing for it
        void fromglTfImage(tinygltf::Image& gltfimage, VmaAllocator allocator, VkDevice device, VkPhysicalDevice physicalDevice, TextureSampler textureSampler, VkQueue copyQueue);
    };

    struct Material
    {
        enum AlphaMode
        {
            ALPHAMODE_OPAQUE,
            ALPHAMODE_MASK,
            ALPHAMODE_BLEND
        };
        AlphaMode        alphaMode       = ALPHAMODE_OPAQUE;
        float            alphaCutoff     = 1.0f;
        float            metallicFactor  = 1.0f;
        float            roughnessFactor = 1.0f;
        glm::vec4        baseColorFactor = glm::vec4(1.0f);
        glm::vec4        emissiveFactor  = glm::vec4(1.0f);
        vkglTF::Texture* baseColorTexture;
        vkglTF::Texture* metallicRoughnessTexture;
        vkglTF::Texture* normalTexture;
        vkglTF::Texture* occlusionTexture;
        vkglTF::Texture* emissiveTexture;
        struct TexCoordSets
        {
            uint8_t baseColor          = 0;
            uint8_t metallicRoughness  = 0;
            uint8_t specularGlossiness = 0;
            uint8_t normal             = 0;
            uint8_t occlusion          = 0;
            uint8_t emissive           = 0;
        } texCoordSets;
        struct Extension
        {
            vkglTF::Texture* specularGlossinessTexture;
            vkglTF::Texture* diffuseTexture;
            glm::vec4        diffuseFactor  = glm::vec4(1.0f);
            glm::vec3        specularFactor = glm::vec3(0.0f);
        } extension;
        struct PbrWorkflows
        {
            bool metallicRoughness  = true;
            bool specularGlossiness = false;
        } pbrWorkflows;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    };

    struct Primitive
    {
        uint32_t    firstIndex;
        uint32_t    indexCount;
        uint32_t    vertexCount;
        Material&   material;
        bool        hasIndices;
        BoundingBox bb;
        Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, Material& material);
        void setBoundingBox(glm::vec3 min, glm::vec3 max);
    };

    struct Mesh
    {
        VmaAllocator            allocator;
        std::vector<Primitive*> primitives;
        BoundingBox             bb;
        BoundingBox             aabb;
        struct UniformBuffer
        {
            VkBuffer               buffer{};
            VmaAllocation          allocation{};
            VkDescriptorBufferInfo descriptor{};
            VkDescriptorSet        descriptorSet{};
            void*                  mapped{};
        } uniformBuffer;
        struct UniformBlock
        {
            glm::mat4 matrix;
            glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
            float     jointcount{0};
        } uniformBlock;
        Mesh(VmaAllocator allocator, glm::mat4 matrix);
        ~Mesh();
        void setBoundingBox(glm::vec3 min, glm::vec3 max);
    };

    struct Skin
    {
        std::string            name;
        Node*                  skeletonRoot = nullptr;
        std::vector<glm::mat4> inverseBindMatrices;
        std::vector<Node*>     joints;
    };

    struct Node
    {
        Node*              parent;
        uint32_t           index;
        std::vector<Node*> children;
        glm::mat4          matrix;
        std::string        name;
        Mesh*              mesh;
        Skin*              skin;
        int32_t            skinIndex = -1;
        glm::vec3          translation{};
        glm::vec3          scale{1.0f};
        glm::quat          rotation{};
        BoundingBox        bvh;
        BoundingBox        aabb;
        glm::mat4          localMatrix();
        glm::mat4          getMatrix();
        void               update();
        ~Node();
    };

    struct AnimationChannel
    {
        enum PathType
        {
            TRANSLATION,
            ROTATION,
            SCALE
        };
        PathType path;
        Node*    node;
        uint32_t samplerIndex;
    };

    struct AnimationSampler
    {
        enum InterpolationType
        {
            LINEAR,
            STEP,
            CUBICSPLINE
        };
        InterpolationType      interpolation;
        std::vector<float>     inputs;
        std::vector<glm::vec4> outputsVec4;
    };

    struct Animation
    {
        std::string                   name;
        std::vector<AnimationSampler> samplers;
        std::vector<AnimationChannel> channels;
        float                         start = std::numeric_limits<float>::max();
        float                         end   = std::numeric_limits<float>::min();
    };

    struct Model
    {
        struct Vertex
        {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv0;
            glm::vec2 uv1;
            glm::vec4 joint0;
            glm::vec4 weight0;
        };

        struct Vertices
        {
            VkBuffer      buffer = VK_NULL_HANDLE;
            VmaAllocation allocation;
        } vertices;
        struct Indices
        {
            int           count;
            VkBuffer      buffer = VK_NULL_HANDLE;
            VmaAllocation allocation;
        } indices;

        glm::mat4 aabb;

        std::vector<Node*> nodes;
        std::vector<Node*> linearNodes;

        std::vector<Skin*> skins;

        std::vector<Texture>        textures;
        std::vector<TextureSampler> textureSamplers;
        std::vector<Material>       materials;
        std::vector<Animation>      animations;
        std::vector<std::string>    extensions;

        struct Dimensions
        {
            glm::vec3 min = glm::vec3(FLT_MAX);
            glm::vec3 max = glm::vec3(-FLT_MAX);
        } dimensions;

        struct Context
        {
            VmaAllocator     allocator;
            VkDevice         device;
            VkPhysicalDevice physicalDevice;
            inline bool      valid() const { return allocator && device && physicalDevice; }
        } context;

        void                 destroy();
        void                 loadNode(vkglTF::Node*          parent,
                                      const tinygltf::Node&  node,
                                      uint32_t               nodeIndex,
                                      const tinygltf::Model& model,
                                      std::vector<uint32_t>& indexBuffer,
                                      std::vector<Vertex>&   vertexBuffer,
                                      float                  globalscale);
        void                 loadSkins(tinygltf::Model& gltfModel);
        void                 loadTextures(tinygltf::Model& gltfModel, VmaAllocator allocator, VkQueue transferQueue);
        VkSamplerAddressMode getVkWrapMode(int32_t wrapMode);
        VkFilter             getVkFilterMode(int32_t filterMode);
        void                 loadTextureSamplers(tinygltf::Model& gltfModel);
        void                 loadMaterials(tinygltf::Model& gltfModel);
        void                 loadAnimations(tinygltf::Model& gltfModel);
        void                 loadFromFile(std::string filename, VmaAllocator allocator, VkQueue transferQueue, float scale = 1.0f);
        void                 drawNode(Node* node, VkCommandBuffer commandBuffer);
        void                 draw(VkCommandBuffer commandBuffer);
        void                 calculateBoundingBox(Node* node, Node* parent);
        void                 getSceneDimensions();
        void                 updateAnimation(uint32_t index, float time);
        Node*                findNode(Node* parent, uint32_t index);
        Node*                nodeFromIndex(uint32_t index);
    };
}  // namespace vkglTF
