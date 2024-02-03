#include "SorpModel.hpp"

#include <cassert>

namespace sorp_v
{
	SorpModel::SorpModel(SorpRenderDevice &device, const std::vector<Vertex> &vertices, const std::vector<uint16_t>& indexes) : _renderDevice{device}
	{
		createVertexBuffers(vertices);
		createIndexBuffers(indexes);
	}

	SorpModel::~SorpModel()
	{
		vkDestroyBuffer(_renderDevice.device(), _vertexBuffer, nullptr);
		vkFreeMemory(_renderDevice.device(), _vertexBufferMemory, nullptr);

		vkDestroyBuffer(_renderDevice.device(), _indexBuffer, nullptr);
		vkFreeMemory(_renderDevice.device(), _indexBufferMemory, nullptr);
	}

	void SorpModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer vertexBuffers[] = { _vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT16);
	}

	void SorpModel::draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDrawIndexed(commandBuffer, _indexCount, 1, 0, 0, 0);
	}

	void SorpModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		_vertexCount = static_cast<uint32_t>(vertices.size());
		assert(_vertexCount >= 3 && "Vertex count must be at least 3");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		

		_renderDevice.createBuffer(bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			_vertexBuffer,
			_vertexBufferMemory);
		
		void* data;
		vkMapMemory(_renderDevice.device(), _vertexBufferMemory, 0, bufferSize, 0,&data);

		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(_renderDevice.device(), _vertexBufferMemory);
	}

	void SorpModel::createIndexBuffers(const std::vector<uint16_t>& indexes)
	{
		_indexCount = static_cast<uint32_t>(indexes.size());
		assert(_indexCount >= 3 && "Index count must be at least 3");

		VkDeviceSize bufferSize = sizeof(indexes[0]) * indexes.size();

		_renderDevice.createBuffer(bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			_indexBuffer,
			_indexBufferMemory);

		void* data;
		vkMapMemory(_renderDevice.device(), _indexBufferMemory, 0, bufferSize, 0, &data);

		memcpy(data, indexes.data(), (size_t)bufferSize);
		vkUnmapMemory(_renderDevice.device(), _indexBufferMemory);
	}

	std::vector<VkVertexInputBindingDescription> SorpModel::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> SorpModel::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
}
