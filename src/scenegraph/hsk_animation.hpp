#pragma once
#include "../hsk_basics.hpp"
#include "../hsk_glm.hpp"
#include "hsk_scenegraph_declares.hpp"

namespace hsk {
    enum class EAnimationInterpolation
    {
        Linear,
        Step,
        Cubicspline
    };

    enum class EAnimationTargetPath
    {
        Translation,
        Rotation,
        Scale
    };

    struct AnimationKeyframe
    {
      public:
        inline AnimationKeyframe() {}
        inline AnimationKeyframe(float time, const glm::vec4& value) : Time(time), Value(value) {}
        inline AnimationKeyframe(float time, const glm::vec4& value, const glm::vec4& intangent, const glm::vec4& outtangent)
            : Time(time), Value(value), InTangent(intangent), OutTangent(outtangent)
        {
        }

        float     Time = 0.f;
        glm::vec4 Value;
        glm::vec4 InTangent;
        glm::vec4 OutTangent;
    };

    struct AnimationSampler
    {
      public:
        glm::vec3 SampleVec(float time) const;
        glm::quat SampleQuat(float time) const;

        static glm::vec4 InterpolateStep(float time, const AnimationKeyframe& lower, const AnimationKeyframe& upper);
        static glm::vec4 InterpolateLinear(float time, const AnimationKeyframe& lower, const AnimationKeyframe& upper);
        static glm::quat InterpolateLinearQuat(float time, const AnimationKeyframe& lower, const AnimationKeyframe& upper);
        static glm::vec4 InterpolateCubicSpline(float time, const AnimationKeyframe& lower, const AnimationKeyframe& upper);
        static glm::quat ReinterpreteAsQuat(glm::vec4);

        EAnimationInterpolation        Interpolation = {};
        std::vector<AnimationKeyframe> Keyframes     = {};

      protected:
        void SelectKeyframe(float time, AnimationKeyframe& lower, AnimationKeyframe& upper) const;
    };
    struct AnimationChannel
    {
      public:
        int32_t              SamplerIndex = {-1};
        Node*                Target       = {nullptr};
        EAnimationTargetPath TargetPath   = {};
    };

    struct PlaybackConfig
    {
      public:
        /// @brief If false, playback is paused
        bool Enable = true;
        /// @brief The animation is looped if set to true
        bool Loop = true;
        /// @brief Speed multiplier
        float PlaybackSpeed = 1.f;
        /// @brief Current playback position used for interpolation
        float Cursor = 0.f;
    };

    class Animation
    {
      public:
        HSK_PROPERTY_ALL(Name)
        HSK_PROPERTY_ALLGET(Samplers)
        HSK_PROPERTY_ALLGET(Channels)
        HSK_PROPERTY_ALL(Start)
        HSK_PROPERTY_ALL(End)
        HSK_PROPERTY_ALL(PlaybackConfig)

        void Update(const FrameUpdateInfo&);

      protected:
        std::string                   mName;
        std::vector<AnimationSampler> mSamplers;
        std::vector<AnimationChannel> mChannels;
        float                         mStart = {};
        float                         mEnd   = {};
        PlaybackConfig                mPlaybackConfig;
    };

}  // namespace hsk