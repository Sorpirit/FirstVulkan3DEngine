#pragma once

#include "sorp_v_device.hpp"

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
			glm::vec2 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		SorpModel(SorpRenderDevice &renderDevice, const std::vector<Vertex> &vertices);
		~SorpModel();

		SorpModel(const SorpModel&) = delete;
		SorpModel &operator=(const SorpModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	private:
		void createVertexBuffers(const std::vector<Vertex> &vertices);

		SorpRenderDevice &renderDevice;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;

	};
}
