#pragma once
#include "../hsk_basics.hpp"
#include <unordered_set>
#include <vulkan/vulkan.h>

namespace hsk {

    /// @brief Base class enforcing common interface for all classes wrapping a device resource
    class DeviceResourceBase : public NoMoveDefaults
    {
        /// @brief Every allocation registers with its name, so when clearing up, we can track if there are unfreed resources left.
        static inline std::unordered_set<DeviceResourceBase*> sAllocatedRessources{};

      public:
        static const std::unordered_set<DeviceResourceBase*>* GetTotalAllocatedResources() { return &sAllocatedRessources; }

        virtual bool Exists() const     = 0;
        virtual void Cleanup()          = 0;

        DeviceResourceBase() { sAllocatedRessources.insert(this); }
        inline virtual ~DeviceResourceBase() { sAllocatedRessources.erase(this); }

        std::string_view                   GetName() const { return mName; }
        inline virtual DeviceResourceBase& SetName(std::string_view name);

      protected:
        std::string mName;
    };

    inline DeviceResourceBase& DeviceResourceBase::SetName(std::string_view name)
    {
        mName = name;
        return *this;
    }
}  // namespace hsk