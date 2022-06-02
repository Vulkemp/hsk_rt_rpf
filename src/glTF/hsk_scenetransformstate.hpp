#pragma once
#include "../hsk_basics.hpp"
#include "../memory/hsk_managedvectorbuffer.hpp"
#include "hsk_gltf_declares.hpp"
#include "hsk_scenecomponent.hpp"
#include <glm/glm.hpp>
#include <optional>

namespace hsk {

    struct alignas(16) ModelTransformState
    {
        glm::mat4 ModelMatrix;
        glm::mat4 PreviousModelMatrix;
    };


    class SceneTransformState : public SceneComponent, public NoMoveDefaults
    {
      public:
        inline SceneTransformState() {}
        inline explicit SceneTransformState(Scene* scene) : SceneComponent(scene), NoMoveDefaults() {}

        void        InitOrUpdate(std::optional<BufferSection> section = {});
        inline void Cleanup() { mBuffer.Cleanup(); }

        inline virtual ~SceneTransformState() { Cleanup(); }

        inline std::vector<ModelTransformState>& Vector() { return mBuffer.GetVector(); }

      protected:
        void CreateBuffer(VkDeviceSize capacity);
        void UploadToBuffer(BufferSection section);

        ManagedVectorBuffer<ModelTransformState> mBuffer = {};
    };


}  // namespace hsk