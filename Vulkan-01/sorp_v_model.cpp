#include "sorp_v_model.hpp"

#include <cassert>

namespace sorp_v
{
	SorpModel::SorpModel(SorpRenderDevice &device, const std::vector<Vertex> &vertices) : renderDevice{device}
	{
		createVertexBuffers(vertices);
	}

	SorpModel::~SorpModel()
	{
		vkDestroyBuffer(renderDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(renderDevice.device(), vertexBufferMemory, nullptr);
	}

	void SorpModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	}

	void SorpModel::draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void SorpModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		

		renderDevice.createBuffer(bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			vertexBuffer,
			vertexBufferMemory);
		
		void* data;
		vkMapMemory(renderDevice.device(), vertexBufferMemory, 0, bufferSize, 0,&data);

		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(renderDevice.device(), vertexBufferMemory);
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
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
}
