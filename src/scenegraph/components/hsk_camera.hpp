#pragma once
#include "../../hsk_glm.hpp"
#include "../../memory/hsk_descriptorsethelper.hpp"
#include "../../memory/hsk_managedubo.hpp"
#include "../../utility/hsk_framerotator.hpp"
#include "../hsk_component.hpp"

namespace hsk {

    struct CameraUboBlock
    {
        glm::mat4 ProjectionMatrix             = {};
        glm::mat4 ViewMatrix                   = {};
        glm::mat4 PreviousProjectionMatrix     = {};
        glm::mat4 PreviousViewMatrix           = {};
        glm::mat4 ProjectionViewMatrix         = {};
        glm::mat4 PreviousProjectionViewMatrix = {};
    };

    class Camera : public NodeComponent, public Component::BeforeDrawCallback, public Component::OnResizedCallback
    {
      public:
        Camera();

        void InitDefault();

        void Update();

        void Cleanup();

        inline virtual ~Camera() { Cleanup(); }

        inline glm::mat4& ProjectionMat() { return mProjectionMatrix; }
        inline glm::mat4& ViewMat() { return mViewMatrix; }

        std::shared_ptr<DescriptorSetHelper::DescriptorInfo> GetUboDescriptorInfo(size_t index);
        std::shared_ptr<DescriptorSetHelper::DescriptorInfo> GetUboDescriptorInfos();

        void SetViewMatrix();
        void SetProjectionMatrix();

        void SetViewMatrix(const glm::vec3& eye, const glm::vec3& lookat, const glm::vec3& up);
        void SetProjectionMatrix(float verticalFov, float aspect, float near, float far);

        virtual void BeforeDraw(const FrameRenderInfo& renderInfo) override;
        virtual void OnResized(VkExtent2D extent) override;

        inline static float CalculateAspect(const VkExtent2D extent);

        HSK_PROPERTY_GET(EyePosition)
        HSK_PROPERTY_CGET(EyePosition)
        HSK_PROPERTY_GET(LookatPosition)
        HSK_PROPERTY_CGET(LookatPosition)
        HSK_PROPERTY_GET(UpDirection)
        HSK_PROPERTY_CGET(UpDirection)
        HSK_PROPERTY_GET(Ubos)
        HSK_PROPERTY_CGET(Ubos)

      protected:
        float     mVerticalFov      = 0;
        float     mAspect           = 0.f;
        float     mNear             = 0;
        float     mFar              = 0;
        glm::vec3 mEyePosition      = glm::vec3(0.f, 0.f, -1.f);
        glm::vec3 mLookatPosition   = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 mUpDirection      = glm::vec3(0.f, 1.f, 0.f);
        glm::mat4 mViewMatrix       = glm::mat4(1);
        glm::mat4 mProjectionMatrix = glm::mat4(1);
        using UboBuffer             = ManagedUbo<CameraUboBlock>;
        FrameRotator<UboBuffer, 2U> mUbos;
    };

    inline float Camera::CalculateAspect(const VkExtent2D extent)
    {
        return (float)extent.width / (float)extent.height;
    }
}  // namespace hsk