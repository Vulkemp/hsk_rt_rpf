#include "hsk_geometrystore.hpp"
#include "../hsk_scene.hpp"

namespace hsk {

    void Mesh::CmdDraw(VkCommandBuffer commandBuffer, GeometryBufferSet*& currentlyBoundSet)
    {
        if(mBuffer && mPrimitives.size())
        {
            if(mBuffer != currentlyBoundSet)
            {
                mBuffer->CmdBindBuffers(commandBuffer);
                currentlyBoundSet = mBuffer;
            }
            for(auto& primitive : mPrimitives)
            {
                primitive.CmdDraw(commandBuffer);
            }
        }
    }

    bool GeometryBufferSet::CmdBindBuffers(VkCommandBuffer commandBuffer)
    {
        if(mVertices.GetAllocation())
        {
            const VkDeviceSize offsets[1]      = {0};
            VkBuffer           vertexBuffers[] = {mVertices.GetBuffer()};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            if(mIndices.GetAllocation())
            {
                vkCmdBindIndexBuffer(commandBuffer, mIndices.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
            }
            return true;
        }
        return false;
    }

    GeometryBufferSet::GeometryBufferSet()
    {
        mIndices.SetName("Indices");
        mVertices.SetName("Vertices");
    }

    GeometryStore::GeometryStore() {}

    void GeometryBufferSet::Init(const VkContext* context, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        if(vertices.size())
        {
            VkDeviceSize bufferSize = vertices.size() * sizeof(Vertex);
            mVertices.Create(context, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
            mVertices.WriteDataDeviceLocal(vertices.data(), bufferSize);
        }
        if(indices.size())
        {
            VkDeviceSize bufferSize = indices.size() * sizeof(uint32_t);
            mIndices.Create(context, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
            mIndices.WriteDataDeviceLocal(indices.data(), bufferSize);
        }
    }

    void GeometryStore::Cleanup()
    {
        mMeshes.clear();
        mBufferSets.clear();
    }
}  // namespace hsk