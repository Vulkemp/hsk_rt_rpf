#pragma once
#include "../base/hsk_vkcontext.hpp"
#include "../utility/hsk_deviceresource.hpp"
#include <vulkan/vulkan.h>

namespace hsk {

    class ManagedBuffer : public DeviceResourceBase
    {
      public:
        struct ManagedBufferCreateInfo
        {
            VmaAllocationCreateInfo AllocationCreateInfo{};
            VkBufferCreateInfo      BufferCreateInfo{};
            std::string             Name{};

            ManagedBufferCreateInfo();
        };

      public:
        ManagedBuffer() { mName = "Unnamed Buffer"; };

        void Create(const VkContext* context, ManagedBufferCreateInfo& createInfo);
        /// @brief Creates the buffer with VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT. If data is set, also maps writes and unmaps
        void CreateForStaging(const VkContext* context, VkDeviceSize size, const void* data = nullptr);
        /// @brief Simplified version of Create that omits the use of a create info but should be sufficient for many usecases
        void Create(const VkContext* context, VkBufferUsageFlags usage, VkDeviceSize size, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = {});

        virtual void Cleanup();

        virtual bool Exists() const { return mAllocation; }

        void WriteDataDeviceLocal(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

        void Map(void*& data);
        void Unmap();

        /// @brief Attempts to
        /// - map the buffer
        /// - write data given in data ptr with given size
        /// - unmap buffer
        void MapAndWrite(const void* data);

        inline virtual ~ManagedBuffer()
        {
            if(mAllocation)
            {
                Cleanup();
            }
        }

        HSK_PROPERTY_CGET(Buffer);
        HSK_PROPERTY_CGET(IsMapped);
        HSK_PROPERTY_CGET(Allocation);
        HSK_PROPERTY_CGET(Name);

        ManagedBuffer& SetName(std::string_view name);

        inline VkDescriptorBufferInfo GetVkDescriptorBufferInfo() { return VkDescriptorBufferInfo{.buffer = mBuffer, .offset = 0, .range = mSize}; }

      protected:
        const VkContext*  mContext{};
        VkBuffer          mBuffer{};
        VmaAllocation     mAllocation{};
        VmaAllocationInfo mAllocationInfo{};
        VkDeviceSize      mSize     = {};
        bool              mIsMapped = false;

        void UpdateDebugNames();
    };

}  // namespace hsk