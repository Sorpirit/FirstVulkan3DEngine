#pragma once

#include "SorpRenderDevice.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace sorp_v
{
	class SorpModel
	{

	public:
		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;
			glm::vec2 texCoord;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		SorpModel(SorpRenderDevice &renderDevice, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indexes);
		~SorpModel();

		SorpModel(const SorpModel&) = delete;
		SorpModel &operator=(const SorpModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	private:
		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffers(const std::vector<uint16_t> &indexes);

		SorpRenderDevice& _renderDevice;

		VkBuffer _vertexBuffer;
		VkDeviceMemory _vertexBufferMemory;
		uint32_t _vertexCount;

		VkBuffer _indexBuffer;
		VkDeviceMemory _indexBufferMemory;
		uint32_t _indexCount;
	};
}
